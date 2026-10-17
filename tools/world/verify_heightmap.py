"""Reads heights back from the finished heightmap tiles and checks them against two independent sources.

Run:  python tools/world/verify_heightmap.py [--online]
For a set of benchmark places (ids in data/reference/landmarks.csv, positions as given there):
  - decode the height from the PNG tile (same maths Unreal uses)
  - compare with the source DEM read directly from the GeoTIFF (mean of the 2 x 2 source pixels around the vertex)
  - compare with the USGS 3DEP point-elevation service (--online)
Prints a table and exits 1 if the median or maximum difference is outside the limits below.
"""

import csv
import json
import math
import os
import sys
import urllib.parse
import urllib.request

import numpy as np
import rasterio
from PIL import Image
from rasterio.warp import transform as warp_transform

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import heightmap_math as hm  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(ROOT, "data", "derived", "heightmap")
RAW = os.path.join(ROOT, "data", "raw", "dem")
BENCHMARKS = ["tioga_pass_summit", "mount_dana_summit", "mono_lake_center", "lee_vining_town_center", "panum_crater_summit",
              "bodie_park_entrance", "jct_us395_sr120", "jct_us395_sr270", "jct_us395_sr167", "tioga_lake", "lundy_lake", "south_tufa_grove"]
LIMITS = {"source_median_m": 0.5, "source_max_m": 3.0, "usgs_median_m": 1.5, "usgs_max_m": 6.0}


def load_sidecar():
    with open(os.path.join(OUT, "heightmap.json"), encoding="utf-8") as f:
        return json.load(f)


def decode_at(sidecar, easting, northing, cache):
    s = sidecar["spacing_m"]
    col = round((easting - sidecar["vertex_origin_utm"]["easting"]) / s)
    row = round((sidecar["vertex_origin_utm"]["northing"] - northing) / s)
    stride = sidecar["tiles"]["stride"]
    tx, ty = min(col // stride, sidecar["tiles"]["x"] - 1), min(row // stride, sidecar["tiles"]["y"] - 1)
    key = (tx, ty)
    if key not in cache:
        cache[key] = np.array(Image.open(os.path.join(OUT, sidecar["tiles"]["pattern"].format(tx=tx, ty=ty))))
    count = cache[key][row - ty * stride, col - tx * stride]
    zp = {"z_center_m": sidecar["z"]["actor_z_m"], "step_m": sidecar["z"]["step_m"]}
    vertex_e = sidecar["vertex_origin_utm"]["easting"] + col * s
    vertex_n = sidecar["vertex_origin_utm"]["northing"] - row * s
    return float(hm.decode_heights(np.array([count]), zp)[0]), vertex_e, vertex_n


def source_mean(easting, northing):
    for name in sorted(os.listdir(RAW)):
        if not name.endswith(".tif"):
            continue
        with rasterio.open(os.path.join(RAW, name)) as src:
            b = src.bounds
            if not (b.left + 2 < easting < b.right - 2 and b.bottom + 2 < northing < b.top - 2):
                continue
            r, c = src.index(easting, northing)
            win = rasterio.windows.Window(c - 1, r - 1, 2, 2)
            block = src.read(1, window=win).astype(np.float64)
            block = block[block != src.nodata]
            return float(block.mean()) if block.size else None
    return None


def usgs_point(lat, lon):
    q = urllib.parse.urlencode({"x": lon, "y": lat, "wkid": 4326, "units": "Meters", "includeDate": "false"})
    req = urllib.request.Request("https://epqs.nationalmap.gov/v1/json?" + q, headers={"User-Agent": "codex-heightmap-verify/1.0"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return float(json.load(r)["value"])


def main():
    online = "--online" in sys.argv
    sidecar = load_sidecar()
    with open(os.path.join(ROOT, "data", "reference", "landmarks.csv"), encoding="utf-8", newline="") as f:
        places = {r["id"]: r for r in csv.DictReader(f)}
    cache = {}
    src_diffs, usgs_diffs = [], []
    print(f"{'place':26} {'heightmap':>10} {'source':>9} {'USGS':>9} {'d_src':>7} {'d_usgs':>7}")
    for pid in BENCHMARKS:
        r = places[pid]
        lat, lon = float(r["lat"]), float(r["lon"])
        e, n = (v[0] for v in warp_transform("EPSG:4326", sidecar["crs"], [lon], [lat]))
        h, ve, vn = decode_at(sidecar, e, n, cache)
        src = source_mean(ve, vn)
        usgs = usgs_point(lat, lon) if online else None
        d_src = h - src if src is not None else None
        d_usgs = h - usgs if usgs is not None else None
        if d_src is not None:
            src_diffs.append(abs(d_src))
        if d_usgs is not None:
            usgs_diffs.append(abs(d_usgs))
        fmt = lambda v: f"{v:9.2f}" if v is not None else "        -"
        print(f"{pid:26} {h:10.2f} {fmt(src)} {fmt(usgs)} {fmt(d_src)[2:]:>7} {fmt(d_usgs)[2:]:>7}")
    ok = True
    for label, diffs, med_lim, max_lim in (("source DEM", src_diffs, LIMITS["source_median_m"], LIMITS["source_max_m"]),
                                           ("USGS point service", usgs_diffs, LIMITS["usgs_median_m"], LIMITS["usgs_max_m"])):
        if not diffs:
            continue
        diffs.sort()
        med, mx = diffs[len(diffs) // 2], diffs[-1]
        good = med <= med_lim and mx <= max_lim
        ok = ok and good
        print(f"{label}: median {med:.2f} m (limit {med_lim}), max {mx:.2f} m (limit {max_lim}) -> {'PASS' if good else 'FAIL'}")
    z = sidecar["z"]
    print(f"vertical step {z['step_m'] * 100:.2f} cm; range {z['min_m']:.1f} to {z['max_m']:.1f} m; ScaleZ {z['scale_z']}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
