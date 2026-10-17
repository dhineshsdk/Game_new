"""Heightmap maths for Unreal Landscape import (ADR 0006, T-0102). Pure functions, no file or network access.

Unreal stores landscape height as uint16 in [0, 65535] with 32768 at the actor's origin. A count c maps to
    world_z_cm = actor_z_cm + (c - 32768) * ScaleZ / 128
so with the default Scale Z of 100 one count is 0.78125 cm and the range is +/- 256 m about the origin.
We choose ScaleZ so the box's real elevation span (plus a margin) fills the 16-bit range, and place the
actor at the span's centre. Everything here is in metres until the very last conversion to counts.
"""

import math

import numpy as np

COUNT_MID = 32768
COUNT_MAX = 65535
# Counts spanned by one unit of "ScaleZ = 100": the range is +/- 256 m (512 m) for ScaleZ 100.
SPAN_AT_SCALE_100_M = 512.0


def landscape_z_params(z_min_m, z_max_m, margin_m):
    """Pick ScaleZ and the actor's Z offset so [z_min - margin, z_max + margin] fits the 16-bit range.

    Returns a dict: scale_z, z_center_m (actor origin, metres), step_m (metres per count), lo_m, hi_m.
    ScaleZ is rounded up at 6 decimals so rounding never shrinks the covered range.
    """
    if not (z_max_m > z_min_m):
        raise ValueError("z_max must be greater than z_min")
    if margin_m < 0:
        raise ValueError("margin must not be negative")
    lo = z_min_m - margin_m
    hi = z_max_m + margin_m
    span = hi - lo
    scale_z = math.ceil(span * 100.0 / SPAN_AT_SCALE_100_M * 1e6) / 1e6
    step_m = scale_z / 128.0 / 100.0
    return {
        "scale_z": scale_z,
        "z_center_m": 0.5 * (lo + hi),
        "step_m": step_m,
        "lo_m": lo,
        "hi_m": hi,
    }


def encode_heights(heights_m, params):
    """Metres -> uint16 counts (rounded to nearest, clamped to the 16-bit range)."""
    h = np.asarray(heights_m, dtype=np.float64)
    counts = np.rint((h - params["z_center_m"]) / params["step_m"]) + COUNT_MID
    return np.clip(counts, 0, COUNT_MAX).astype(np.uint16)


def decode_heights(counts, params):
    """uint16 counts -> metres (float64)."""
    c = np.asarray(counts, dtype=np.float64)
    return (c - COUNT_MID) * params["step_m"] + params["z_center_m"]


def component_grid(extent_m, spacing_m, quads_per_section, sections_per_component_axis):
    """How many landscape components cover extent_m at spacing_m.

    Returns (components, vertices): vertices = components * quads_per_component + 1, the size of the heightmap
    along this axis. The grid is rounded up, so it covers at least extent_m.
    """
    if extent_m <= 0 or spacing_m <= 0:
        raise ValueError("extent and spacing must be positive")
    quads_per_component = quads_per_section * sections_per_component_axis
    quads_needed = math.ceil(extent_m / spacing_m)
    components = max(1, math.ceil(quads_needed / quads_per_component))
    return components, components * quads_per_component + 1


def fill_nodata(grid, mask, max_iterations=2000):
    """Fill masked cells from their valid neighbours, working inwards from the edge of each hole.

    Each pass sets a hole cell to the mean of its already-valid 8-neighbours, so a hole is filled ring by ring.
    Returns (filled_grid, cells_filled, cells_left). A grid with no valid cell at all is returned unchanged.
    """
    out = np.array(grid, dtype=np.float64, copy=True)
    hole = np.array(mask, dtype=bool, copy=True)
    total = int(hole.sum())
    if total == 0 or hole.all():
        return out, 0, total
    for _ in range(max_iterations):
        if not hole.any():
            break
        valid = ~hole
        padded_v = np.pad(valid, 1, constant_values=False)
        padded_z = np.pad(np.where(valid, out, 0.0), 1, constant_values=0.0)
        count = np.zeros(out.shape)
        total_z = np.zeros(out.shape)
        h, w = out.shape
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dy == 0 and dx == 0:
                    continue
                count += padded_v[1 + dy:1 + dy + h, 1 + dx:1 + dx + w]
                total_z += padded_z[1 + dy:1 + dy + h, 1 + dx:1 + dx + w]
        fillable = hole & (count > 0)
        if not fillable.any():
            break
        out[fillable] = total_z[fillable] / count[fillable]
        hole &= ~fillable
    return out, total - int(hole.sum()), int(hole.sum())
