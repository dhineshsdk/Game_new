"""Synthetic tests for road_graph. Run: python -m unittest discover -s tools/world/tests -t tools/world"""

import os
import random
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import road_graph as rg  # noqa: E402

STEP = 0.001  # degrees, about 111 m at the equator


def make_way(way_id, node_ids, lon0, lat0, dlon, dlat):
    """A way through the given node ids, laid out on a straight line in steps of STEP."""
    pts = [(lon0 + k * dlon * STEP, lat0 + k * dlat * STEP) for k in range(len(node_ids))]
    return {"id": way_id, "nodes": list(node_ids), "points": pts}


def straight_road(way_specs):
    """Ways along the x axis: way_specs = [(id, first_node, last_node)] with consecutive shared end nodes."""
    ways, x = [], 0
    for way_id, first, last in way_specs:
        nodes = list(range(first, last + 1))
        ways.append(make_way(way_id, nodes, x * STEP, 0.0, 1, 0))
        x += len(nodes) - 1
    return ways


class OrderWaysTests(unittest.TestCase):
    def setUp(self):
        self.road = straight_road([(1, 100, 105), (2, 105, 112), (3, 112, 120)])

    def test_already_ordered(self):
        chains = rg.order_ways(self.road)
        self.assertEqual(len(chains), 1)
        self.assertEqual(chains[0]["way_ids"], [1, 2, 3])
        self.assertEqual(len(chains[0]["points"]), 6 + 8 + 9 - 2)  # joint points not duplicated

    def test_shuffled_and_reversed(self):
        rng = random.Random(7)
        for _ in range(30):
            ways = [dict(w) for w in self.road]
            rng.shuffle(ways)
            for w in ways:
                if rng.random() < 0.5:
                    w["nodes"] = list(reversed(w["nodes"]))
                    w["points"] = list(reversed(w["points"]))
            chains = rg.order_ways(ways)
            self.assertEqual(len(chains), 1)
            self.assertIn(chains[0]["way_ids"], ([1, 2, 3], [3, 2, 1]))
            pts = chains[0]["points"]
            self.assertLess(rg.max_gap_m(pts), 150.0)
            self.assertAlmostEqual(rg.polyline_length_m(pts), rg.haversine_m(pts[0], pts[-1]), delta=1.0)
            self.assertEqual(len(pts), 21)

    def test_t_junction_gives_three_chains(self):
        main = straight_road([(1, 100, 105), (2, 105, 110)])
        branch = make_way(3, [105, 200, 201], 5 * STEP, 0.0, 0, 1)
        chains = rg.order_ways(main + [branch])
        self.assertEqual(len(chains), 3)
        self.assertEqual(sorted(len(c["way_ids"]) for c in chains), [1, 1, 1])
        self.assertTrue(all(105 in (c["start_node"], c["end_node"]) for c in chains))

    def test_gap_gives_two_chains(self):
        a = straight_road([(1, 100, 105)])
        b = make_way(2, [300, 301, 302], 1.0, 0.0, 1, 0)  # nowhere near
        chains = rg.order_ways(a + [b])
        self.assertEqual(len(chains), 2)

    def test_loop(self):
        ring = [
            make_way(1, [1, 2, 3], 0.0, 0.0, 1, 0),
            make_way(2, [3, 4, 5], 2 * STEP, 0.0, 0, 1),
            make_way(3, [5, 6, 1], 2 * STEP, 2 * STEP, -1, 0),
        ]
        ring[2]["points"][-1] = ring[0]["points"][0]
        chains = rg.order_ways(ring)
        self.assertEqual(len(chains), 1)
        self.assertEqual(sorted(chains[0]["way_ids"]), [1, 2, 3])
        self.assertEqual(chains[0]["start_node"], chains[0]["end_node"])

    def test_single_way_and_empty(self):
        self.assertEqual(rg.order_ways([]), [])
        chains = rg.order_ways(self.road[:1])
        self.assertEqual(chains[0]["way_ids"], [1])

    def test_deterministic(self):
        a = rg.order_ways(self.road)
        b = rg.order_ways(list(reversed(self.road)))
        self.assertEqual(len(a), len(b))


class GeometryTests(unittest.TestCase):
    def test_haversine_one_degree_of_latitude(self):
        self.assertAlmostEqual(rg.haversine_m((0.0, 0.0), (0.0, 1.0)), 111195.0, delta=100.0)

    def test_max_gap(self):
        pts = [(0.0, 0.0), (STEP, 0.0), (10 * STEP, 0.0)]
        self.assertAlmostEqual(rg.max_gap_m(pts), 9 * 111.195, delta=2.0)
        self.assertEqual(rg.max_gap_m(pts[:1]), 0.0)


class ClipTests(unittest.TestCase):
    BOX = (0.0, 0.0, 10.0, 10.0)

    def test_fully_inside_is_unchanged(self):
        line = [(1.0, 1.0), (5.0, 5.0), (9.0, 2.0)]
        self.assertEqual(rg.clip_polyline_to_box(line, self.BOX), [line])

    def test_fully_outside_is_dropped(self):
        self.assertEqual(rg.clip_polyline_to_box([(20.0, 20.0), (30.0, 25.0)], self.BOX), [])

    def test_exit_is_cut_on_the_edge(self):
        pieces = rg.clip_polyline_to_box([(5.0, 5.0), (15.0, 5.0)], self.BOX)
        self.assertEqual(pieces, [[(5.0, 5.0), (10.0, 5.0)]])

    def test_crossing_segment_with_both_ends_outside(self):
        pieces = rg.clip_polyline_to_box([(-5.0, 5.0), (15.0, 5.0)], self.BOX)
        self.assertEqual(pieces, [[(0.0, 5.0), (10.0, 5.0)]])

    def test_leave_and_return_gives_two_pieces(self):
        line = [(2.0, 2.0), (2.0, 12.0), (5.0, 12.0), (5.0, 2.0)]
        pieces = rg.clip_polyline_to_box(line, self.BOX)
        self.assertEqual(len(pieces), 2)
        self.assertEqual(pieces[0][-1], (2.0, 10.0))
        self.assertEqual(pieces[1][0], (5.0, 10.0))

    def test_all_points_end_up_inside(self):
        rnd = random.Random(3)
        line = [(rnd.uniform(-5, 15), rnd.uniform(-5, 15)) for _ in range(200)]
        for piece in rg.clip_polyline_to_box(line, self.BOX):
            self.assertTrue(all(-1e-9 <= x <= 10 + 1e-9 and -1e-9 <= y <= 10 + 1e-9 for x, y in piece))


if __name__ == "__main__":
    unittest.main()
