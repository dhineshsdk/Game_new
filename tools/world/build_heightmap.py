"""Builds 16-bit World Partition heightmap tiles from the downloaded USGS DEM (ADR 0006, T-0102).

Run:  python tools/world/build_heightmap.py
Reads tools/world/terrain_config.json and data/raw/dem/*.tif (NAD83 / UTM 11N, 1 m, NAVD88 metres).
Resamples to the configured vertex spacing (area average), fills nodata, measures the real elevation
range, picks ScaleZ and the actor Z offset (heightmap_math), and writes:
  data/derived/heightmap/hm_x<TX>_y<TY>.png   16-bit grayscale tiles, row 0 = north edge
  data/derived/heightmap/heightmap.json       sidecar: grid origin, spacing, sizes, Z parameters, stats
Neighbouring tiles share their edge row/column of vertices (Unreal landscape convention), so tile
(tx, ty) starts at vertex (tx * stride, ty * stride) with stride = tile vertices - 1.
"""

import glob
import json
import os
import sys
import time

import numpy as np
import rasterio
from PIL import Image
from rasterio.transform import from_origin
from rasterio.warp import Resampling, reproject, transform as warp_transform

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import heightmap_math as hm  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(HERE))
CONFIG = json.load(open(os.path.join(HERE, "terrain_config.json"), encoding="utf-8"))
RAW = os.path.join(ROOT, "data", "raw", "dem")
OUT = os.path.join(ROOT, "data", "derived", "heightmap")
CRS = CONFIG["projected_crs"]


def grid_definition():
    box = CONFIG["world_box_wgs84"]
    out = CONFIG["output"]
    xs, ys = warp_transform("EPSG:4326", CRS, [box["west"], box["east"], box["west"], box["east"]],
                            [box["south"], box["south"], box["north"], box["north"]])
    min_e, max_e, min_n, max_n = min(xs), max(xs), min(ys), max(ys)
    spacing = out["vertex_spacing_m"]
    qps, sections = out["quads_per_section"], out["sections_per_component_axis"]
    comps_x, _ = hm.component_grid(max_e - min_e, spacing, qps, sections)
    comps_y, _ = hm.component_grid(max_n - min_n, spacing, qps, sections)
    tile_c = out["tile_components"]
    comps_x = -(-comps_x // tile_c) * tile_c  # whole tiles
    comps_y = -(-comps_y // tile_c) * tile_c
    quads = qps * sections
    return {
        "origin_e": float(np.floor(min_e)), "origin_n": float(np.ceil(max_n)),  # centre of vertex (0, 0): NW corner
        "spacing_m": spacing, "comps_x": comps_x, "comps_y": comps_y, "quads_per_component": quads,
        "nx": comps_x * quads + 1, "ny": comps_y * quads + 1, "tile_components": tile_c,
        "tile_vertices": tile_c * quads + 1, "tile_stride": tile_c * quads,
        "box_utm": {"min_e": min_e, "max_e": max_e, "min_n": min_n, "max_n": max_n},
    }


def assemble(grid):
    """Reproject every source tile onto the output grid (area average) into one float32 array, NaN = no data."""
    nx, ny, s = grid["nx"], grid["ny"], grid["spacing_m"]
    dst = np.full((ny, nx), np.nan, dtype=np.float32)
    # Vertex (i, j) is the centre of a cell of size s: the raster origin is half a cell up and left.
    dst_transform = from_origin(grid["origin_e"] - s / 2, grid["origin_n"] + s / 2, s, s)
    dst_left, dst_top = grid["origin_e"] - s / 2, grid["origin_n"] + s / 2
    primary = CONFIG["source"]["project"]
    # primary project first; supplementary tiles only ever fill cells that are still empty
    files = sorted(glob.glob(os.path.join(RAW, "*.tif")), key=lambda f: (primary not in os.path.basename(f), os.path.basename(f)))
    if not files:
        raise SystemExit("no DEM tiles in data/raw/dem; run fetch_dem.py first")
    for path in files:
        with rasterio.open(path) as src:
            b = src.bounds
            c0 = max(0, int(np.floor((b.left - dst_left) / s)))
            c1 = min(nx, int(np.ceil((b.right - dst_left) / s)))
            r0 = max(0, int(np.floor((dst_top - b.top) / s)))
            r1 = min(ny, int(np.ceil((dst_top - b.bottom) / s)))
            if c1 <= c0 or r1 <= r0:
                continue
            win_t = from_origin(dst_left + c0 * s, dst_top - r0 * s, s, s)
            tmp = np.full((r1 - r0, c1 - c0), np.nan, dtype=np.float32)
            reproject(source=rasterio.band(src, 1), destination=tmp, src_transform=src.transform, src_crs=src.crs,
                      src_nodata=src.nodata, dst_transform=win_t, dst_crs=CRS, dst_nodata=np.nan,
                      resampling=Resampling.average, num_threads=4)
            view = dst[r0:r1, c0:c1]
            take = np.isnan(view) & np.isfinite(tmp)
            view[take] = tmp[take]
        print(f"  {os.path.basename(path)} -> rows {r0}:{r1} cols {c0}:{c1}", flush=True)
    return dst, dst_transform


def fill_holes(grid_arr):
    """Block-wise ring fill (heightmap_math.fill_nodata) so we never run it over the whole 250M-cell array."""
    ny, nx = grid_arr.shape
    total_holes = int(np.isnan(grid_arr).sum())
    if total_holes == 0:
        return 0, 0
    for block, pad in ((512, 64), (512, 512)):
        for r0 in range(0, ny, block):
            for c0 in range(0, nx, block):
                core = np.isnan(grid_arr[r0:r0 + block, c0:c0 + block])
                if not core.any():
                    continue
                ra, rb = max(0, r0 - pad), min(ny, r0 + block + pad)
                ca, cb = max(0, c0 - pad), min(nx, c0 + block + pad)
                sub = grid_arr[ra:rb, ca:cb].astype(np.float64)
                mask = np.isnan(sub)
                filled, _, _ = hm.fill_nodata(np.where(mask, 0.0, sub), mask, max_iterations=pad + 4)
                got = ~np.isnan(grid_arr[r0:r0 + block, c0:c0 + block])
                piece = filled[r0 - ra:r0 - ra + core.shape[0], c0 - ca:c0 - ca + core.shape[1]]
                # fill_nodata leaves unreachable cells at 0.0 from the placeholder; only accept cells it actually reached
                reached = core & (piece != 0.0)
                grid_arr[r0:r0 + block, c0:c0 + block][reached] = piece[reached].astype(np.float32)
    left = int(np.isnan(grid_arr).sum())
    return total_holes, left


def main():
    t_start = time.time()
    grid = grid_definition()
    print(f"grid {grid['nx']} x {grid['ny']} vertices at {grid['spacing_m']} m "
          f"({grid['comps_x']} x {grid['comps_y']} components, {grid['comps_x'] // grid['tile_components']} x "
          f"{grid['comps_y'] // grid['tile_components']} tiles of {grid['tile_vertices']} px)")
    heights, _ = assemble(grid)
    nodata_before = int(np.isnan(heights).sum())
    print(f"cells without data after mosaic: {nodata_before} of {heights.size} ({100 * nodata_before / heights.size:.3f} %)")
    holes, left = fill_holes(heights)
    print(f"nodata filled: {holes - left}, left: {left}")
    if left:
        raise SystemExit(f"{left} cells could not be filled (outside every source tile and not near data); extend the source tiles")

    z_min, z_max = float(np.min(heights)), float(np.max(heights))
    print(f"measured elevation range: {z_min:.2f} to {z_max:.2f} m")
    zp = hm.landscape_z_params(z_min, z_max, CONFIG["output"]["z_margin_m"])
    counts = hm.encode_heights(heights, zp)
    del heights

    os.makedirs(OUT, exist_ok=True)
    for old in glob.glob(os.path.join(OUT, "hm_x*_y*.png")):
        os.remove(old)
    tv, stride = grid["tile_vertices"], grid["tile_stride"]
    tiles_x, tiles_y = grid["comps_x"] // grid["tile_components"], grid["comps_y"] // grid["tile_components"]
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            block = counts[ty * stride:ty * stride + tv, tx * stride:tx * stride + tv]
            Image.fromarray(np.ascontiguousarray(block)).save(os.path.join(OUT, f"hm_x{tx:02d}_y{ty:02d}.png"), optimize=False)
    print(f"wrote {tiles_x * tiles_y} tiles to {os.path.relpath(OUT, ROOT)}")

    sidecar = {
        "crs": CRS, "vertical_datum": CONFIG["vertical_datum"], "source_project": CONFIG["source"]["project"],
        "row0": "north edge", "col0": "west edge",
        "vertex_origin_utm": {"easting": grid["origin_e"], "northing": grid["origin_n"], "note": "centre of vertex (col 0, row 0)"},
        "spacing_m": grid["spacing_m"], "vertices": {"nx": grid["nx"], "ny": grid["ny"]},
        "components": {"x": grid["comps_x"], "y": grid["comps_y"], "quads_per_component": grid["quads_per_component"]},
        "tiles": {"x": tiles_x, "y": tiles_y, "vertices": tv, "stride": stride, "pattern": "hm_x{tx:02d}_y{ty:02d}.png"},
        "z": {"min_m": z_min, "max_m": z_max, "margin_m": CONFIG["output"]["z_margin_m"], "scale_z": zp["scale_z"],
              "actor_z_m": zp["z_center_m"], "actor_z_cm": zp["z_center_m"] * 100.0, "step_m": zp["step_m"],
              "formula": "world_z_cm = actor_z_cm + (count - 32768) * scale_z / 128"},
        "world_box_utm": grid["box_utm"],
        "nodata_cells_before_fill": nodata_before,
        "source_files": sorted(os.path.basename(f) for f in glob.glob(os.path.join(RAW, "*.tif"))),
        "wall_time_s": round(time.time() - t_start, 1),
    }
    with open(os.path.join(OUT, "heightmap.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(sidecar, f, indent=2)
    print(f"ScaleZ {zp['scale_z']}, step {zp['step_m'] * 100:.2f} cm, actor Z {zp['z_center_m']:.2f} m; {sidecar['wall_time_s']} s")


if __name__ == "__main__":
    main()
