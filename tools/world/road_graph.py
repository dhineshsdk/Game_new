"""Joins OpenStreetMap ways into ordered, connected centrelines (ADR 0006, T-0104). Pure functions.

A way is {"id": int, "nodes": [node ids], "points": [(lon, lat), ...]} with one point per node.
order_ways() chains ways end to end through nodes where exactly two way-ends meet; chains stop at
junctions (three or more way-ends) and at dead ends, so a T junction gives three chains that share
the junction node. Every chain is oriented consistently and has no duplicated joint point.
"""

import math


def haversine_m(a, b):
    """Distance in metres between two (lon, lat) points."""
    r = 6371000.0
    p1, p2 = math.radians(a[1]), math.radians(b[1])
    dp, dl = p2 - p1, math.radians(b[0] - a[0])
    h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * r * math.asin(math.sqrt(h))


def polyline_length_m(points):
    return sum(haversine_m(points[i], points[i + 1]) for i in range(len(points) - 1))


def max_gap_m(points):
    """Largest distance between consecutive points (0 for fewer than two points)."""
    return max((haversine_m(points[i], points[i + 1]) for i in range(len(points) - 1)), default=0.0)


def order_ways(ways):
    """Chain ways into centrelines. Returns a list of {"way_ids", "start_node", "end_node", "points"}."""
    ends = {}
    for i, w in enumerate(ways):
        ends.setdefault(w["nodes"][0], []).append((i, 0))
        ends.setdefault(w["nodes"][-1], []).append((i, 1))

    used = set()
    chains = []

    def walk(first_index, from_node):
        index, node = first_index, from_node
        way_ids, points = [], []
        while True:
            used.add(index)
            w = ways[index]
            forward = w["nodes"][0] == node
            pts = list(w["points"]) if forward else list(reversed(w["points"]))
            tail = w["nodes"][-1] if forward else w["nodes"][0]
            way_ids.append(w["id"])
            points.extend(pts if not points else pts[1:])
            partners = [(j, e) for (j, e) in ends[tail] if j != index and j not in used]
            if len(ends[tail]) != 2 or len(partners) != 1:
                return {"way_ids": way_ids, "start_node": from_node, "end_node": tail, "points": points}
            index, node = partners[0][0], tail

    # chains start at junctions and dead ends first, in node-id order so results are deterministic
    for node in sorted(ends):
        if len(ends[node]) == 2:
            continue
        for (i, _end) in sorted(ends[node]):
            if i not in used:
                chains.append(walk(i, node))
    # whatever is left forms loops
    for i, w in enumerate(ways):
        if i not in used:
            chains.append(walk(i, w["nodes"][0]))
    return chains


def _clip_segment(p, q, box):
    """Liang-Barsky: the part of segment p-q inside box = (west, south, east, north), or None."""
    west, south, east, north = box
    dx, dy = q[0] - p[0], q[1] - p[1]
    t0, t1 = 0.0, 1.0
    for pk, qk in ((-dx, p[0] - west), (dx, east - p[0]), (-dy, p[1] - south), (dy, north - p[1])):
        if pk == 0:
            if qk < 0:
                return None
            continue
        t = qk / pk
        if pk < 0:
            if t > t1:
                return None
            t0 = max(t0, t)
        else:
            if t < t0:
                return None
            t1 = min(t1, t)
    return (p[0] + t0 * dx, p[1] + t0 * dy), (p[0] + t1 * dx, p[1] + t1 * dy)


def clip_polyline_to_box(points, box):
    """Pieces of a polyline inside box = (west, south, east, north). A line that leaves and re-enters gives several pieces;
    the cut points lie exactly on the box edge."""
    pieces, current = [], []
    for a, b in zip(points, points[1:]):
        seg = _clip_segment(a, b, box)
        if seg is None:
            if len(current) > 1:
                pieces.append(current)
            current = []
            continue
        start, end = seg
        if current and current[-1] == start:
            current.append(end)
        else:
            if len(current) > 1:
                pieces.append(current)
            current = [start, end]
    if len(current) > 1:
        pieces.append(current)
    return pieces
