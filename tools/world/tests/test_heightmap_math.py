"""Synthetic-data tests for heightmap_math (no network, no real rasters).
Run: python -I -m unittest discover -s tools/world/tests -t tools/world
"""

import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import heightmap_math as hm  # noqa: E402


class ZParamsTests(unittest.TestCase):
    def test_known_values(self):
        # 1950..3970 plus a 20 m margin each side is 1930..3990, a 2060 m span: ScaleZ = 2060 * 100 / 512.
        p = hm.landscape_z_params(1950.0, 3970.0, 20.0)
        self.assertAlmostEqual(p["scale_z"], 402.34375, places=5)
        self.assertAlmostEqual(p["z_center_m"], 2960.0, places=9)
        self.assertAlmostEqual(p["step_m"], 402.34375 / 128 / 100, places=9)  # about 3.1 cm per count
        self.assertAlmostEqual(p["lo_m"], 1930.0)
        self.assertAlmostEqual(p["hi_m"], 3990.0)

    def test_default_scale_matches_unreal_default(self):
        # A 512 m span is exactly Unreal's default Scale Z of 100.
        p = hm.landscape_z_params(0.0, 512.0, 0.0)
        self.assertAlmostEqual(p["scale_z"], 100.0, places=6)
        self.assertAlmostEqual(p["step_m"], 0.0078125, places=12)

    def test_box_extremes_map_inside_the_range(self):
        p = hm.landscape_z_params(1944.0, 3981.0, 20.0)
        lo, hi = hm.encode_heights([1944.0, 3981.0], p)
        self.assertGreater(int(lo), 0)
        self.assertLess(int(hi), 65535)
        # A 20 m margin is about 640 counts at 3.1 cm per count, so the extremes sit well inside the ends.
        self.assertGreater(int(lo), 500)
        self.assertLess(int(hi), 65535 - 500)
        self.assertAlmostEqual(int(lo), 20.0 / p["step_m"], delta=2)

    def test_rounded_up_scale_still_covers_the_span(self):
        p = hm.landscape_z_params(1944.37, 3981.11, 0.0)
        counts = hm.encode_heights([1944.37, 3981.11], p)
        self.assertEqual(int(counts[0]) >= 0 and int(counts[1]) <= 65535, True)

    def test_bad_input(self):
        with self.assertRaises(ValueError):
            hm.landscape_z_params(10.0, 10.0, 0.0)
        with self.assertRaises(ValueError):
            hm.landscape_z_params(0.0, 10.0, -1.0)


class EncodeDecodeTests(unittest.TestCase):
    def setUp(self):
        self.p = hm.landscape_z_params(1944.0, 3981.0, 20.0)

    def test_round_trip_error_at_most_half_a_step(self):
        rng = np.random.default_rng(12345)
        h = rng.uniform(1944.0, 3981.0, size=100000)
        back = hm.decode_heights(hm.encode_heights(h, self.p), self.p)
        self.assertLessEqual(float(np.max(np.abs(back - h))), 0.5 * self.p["step_m"] + 1e-9)

    def test_origin_maps_to_mid_count(self):
        c = hm.encode_heights([self.p["z_center_m"]], self.p)
        self.assertEqual(int(c[0]), 32768)

    def test_clamping(self):
        c = hm.encode_heights([-1.0e6, 1.0e6], self.p)
        self.assertEqual((int(c[0]), int(c[1])), (0, 65535))

    def test_monotonic(self):
        h = np.linspace(1944.0, 3981.0, 5000)
        c = hm.encode_heights(h, self.p).astype(np.int64)
        self.assertTrue(np.all(np.diff(c) >= 0))

    def test_output_dtype(self):
        self.assertEqual(hm.encode_heights([2000.0], self.p).dtype, np.uint16)


class ComponentGridTests(unittest.TestCase):
    def test_world_box_at_2m(self):
        # 63 quads x 2 sections = 126 quads per component. 27,300 m / 2 m = 13,650 quads -> 109 components.
        components, vertices = hm.component_grid(27300.0, 2.0, 63, 2)
        self.assertEqual(components, 109)
        self.assertEqual(vertices, 109 * 126 + 1)
        self.assertGreaterEqual((vertices - 1) * 2.0, 27300.0)

    def test_exact_fit_does_not_add_a_component(self):
        self.assertEqual(hm.component_grid(126 * 2.0 * 10, 2.0, 63, 2)[0], 10)

    def test_one_component_minimum(self):
        self.assertEqual(hm.component_grid(1.0, 2.0, 63, 2), (1, 127))

    def test_bad_input(self):
        with self.assertRaises(ValueError):
            hm.component_grid(0.0, 2.0, 63, 2)


class FillNodataTests(unittest.TestCase):
    def test_no_holes_is_a_no_op(self):
        g = np.arange(25, dtype=float).reshape(5, 5)
        out, filled, left = hm.fill_nodata(g, np.zeros((5, 5), bool))
        self.assertTrue(np.array_equal(out, g))
        self.assertEqual((filled, left), (0, 0))

    def test_plane_stays_a_plane(self):
        y, x = np.mgrid[0:21, 0:21].astype(float)
        plane = 100.0 + 2.0 * x + 3.0 * y
        mask = np.zeros(plane.shape, bool)
        mask[8:13, 8:13] = True
        broken = plane.copy()
        broken[mask] = -9999.0
        out, filled, left = hm.fill_nodata(broken, mask)
        self.assertEqual((filled, left), (25, 0))
        # Ring fill averages one-sided neighbours, so the error grows with the distance from the hole's edge:
        # at most (hole radius in cells) x (slope per cell). Here 2 cells x sqrt(2^2 + 3^2).
        self.assertLessEqual(float(np.max(np.abs(out - plane))), 2 * np.hypot(2.0, 3.0))
        self.assertTrue(np.array_equal(out[~mask], plane[~mask]))  # valid cells untouched

    def test_all_masked_is_left_alone(self):
        out, filled, left = hm.fill_nodata(np.zeros((3, 3)), np.ones((3, 3), bool))
        self.assertEqual((filled, left), (0, 9))

    def test_edge_hole(self):
        g = np.full((6, 6), 50.0)
        mask = np.zeros((6, 6), bool)
        mask[0, :] = True
        out, filled, left = hm.fill_nodata(g, mask)
        self.assertEqual((filled, left), (6, 0))
        self.assertTrue(np.allclose(out, 50.0))


if __name__ == "__main__":
    unittest.main()
