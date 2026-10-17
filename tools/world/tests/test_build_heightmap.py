"""Tests build_heightmap.assemble and fill_holes on small synthetic GeoTIFF tiles (no real data, no network).
The synthetic terrain is a plane, whose area average over a cell equals its value at the cell centre, so the
expected height at every output vertex is known exactly.
"""

import os
import sys
import tempfile
import unittest

import numpy as np
import rasterio
from rasterio.transform import from_origin

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import build_heightmap as bh  # noqa: E402

E0, N0 = 300000.0, 4200000.0


def plane(e, n):
    return 2000.0 + 0.01 * (e - E0) + 0.02 * (n - N0)


def write_tile(path, left, top, size_m, hole=None):
    cols = rows = int(size_m)
    e = left + 0.5 + np.arange(cols)
    n = top - 0.5 - np.arange(rows)
    data = plane(e[None, :], n[:, None]).astype(np.float32)
    if hole:
        r0, r1, c0, c1 = hole
        data[r0:r1, c0:c1] = -999999.0
    profile = dict(driver="GTiff", height=rows, width=cols, count=1, dtype="float32", crs="EPSG:26911",
                   transform=from_origin(left, top, 1.0, 1.0), nodata=-999999.0)
    with rasterio.open(path, "w", **profile) as dst:
        dst.write(data, 1)


class AssembleTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.old_raw = bh.RAW
        bh.RAW = self.tmp.name
        # two adjacent 400 m tiles; a 12 x 12 m hole in the first one
        write_tile(os.path.join(self.tmp.name, "a.tif"), E0, N0 + 400.0, 400, hole=(190, 202, 250, 262))
        write_tile(os.path.join(self.tmp.name, "b.tif"), E0 + 400.0, N0 + 400.0, 400)
        # grid crossing the seam at E0 + 400, vertices 2 m apart, origin = centre of vertex (0, 0)
        self.grid = {"origin_e": E0 + 190.0, "origin_n": N0 + 390.0, "spacing_m": 2.0, "nx": 120, "ny": 100}

    def tearDown(self):
        bh.RAW = self.old_raw
        self.tmp.cleanup()

    def expected(self):
        i = np.arange(self.grid["nx"])
        j = np.arange(self.grid["ny"])
        e = self.grid["origin_e"] + i * self.grid["spacing_m"]
        n = self.grid["origin_n"] - j * self.grid["spacing_m"]
        return plane(e[None, :], n[:, None])

    def test_heights_match_the_plane_and_the_seam_is_smooth(self):
        heights, _ = bh.assemble(self.grid)
        want = self.expected()
        valid = np.isfinite(heights)
        self.assertGreater(valid.mean(), 0.97)
        self.assertLess(float(np.max(np.abs(heights[valid] - want[valid]))), 0.02)
        # column index where E = E0 + 400 (the seam): neighbouring columns differ by exactly the plane slope
        seam = int((E0 + 400.0 - self.grid["origin_e"]) / 2.0)
        step = np.abs(heights[:, seam + 1] - heights[:, seam])
        ok = np.isfinite(step)
        self.assertLess(float(np.max(np.abs(step[ok] - 0.02))), 0.01)  # 0.01 m per m x 2 m

    def test_hole_is_left_as_nan_then_filled_close_to_the_plane(self):
        heights, _ = bh.assemble(self.grid)
        holes_before = int(np.isnan(heights).sum())
        self.assertGreater(holes_before, 0)
        found, left = bh.fill_holes(heights)
        self.assertEqual((found, left), (holes_before, 0))
        self.assertTrue(np.all(np.isfinite(heights)))
        err = np.abs(heights - self.expected())
        # ring fill on a 6 x 6 cell hole in a plane sloping about 0.045 m per cell: error stays well under half a metre
        self.assertLess(float(err.max()), 0.5)

    def test_tile_outside_the_grid_is_ignored(self):
        write_tile(os.path.join(self.tmp.name, "far.tif"), E0 + 5000.0, N0 + 5400.0, 100)
        heights, _ = bh.assemble(self.grid)
        self.assertLess(float(np.nanmax(heights)), plane(E0 + 440.0, N0 + 400.0) + 0.1)


if __name__ == "__main__":
    unittest.main()
