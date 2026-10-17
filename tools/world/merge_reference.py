"""Merges the two Gemini reference sets into data/merged/ following docs/research/R-0002-comparison.md.

Run:  python tools/world/merge_reference.py
Neither input folder is changed. Every merged row carries `origin` (A = data/reference, B = ref).
  landmarks: all of A, plus B places with no A place within 400 m, marked position_quality=approximate;
             every row gets elev_usgs_m (USGS 3DEP point value) beside the claimed elev_m
  biomes:    A's bands plus the B bands whose plant community A lacks (pumice flats, mountain sagebrush)
  climate:   both sets' station rows, kept apart (they are not averaged; neither is verified)
  settlements, fiction_map: A plus B rows for roles A does not cover
Python 3.12 standard library only (uses the USGS elevation cache of compare_reference.py).
"""

import csv
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(ROOT, "data", "merged")

spec = importlib.util.spec_from_file_location("compare_reference", os.path.join(HERE, "compare_reference.py"))
cmp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cmp)

A, B = "data/reference", "ref"
# B fictional-name rows A has no counterpart for (matched by hand on real_place_or_role).
FICTION_FROM_B = ("Mono Cone", "Town Auto Repair", "Tioga Pass Resort", "Conway Ranch")
# B biome bands with a plant community A lacks.
BIOMES_FROM_B = ("BIO_02_VOLCANIC_PUMICE_FLATS", "BIO_03_MORAINAL_SAGEBRUSH")


def write(name, rows, fields):
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, name), "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    print(f"{name}: {len(rows)} rows")


def with_origin(rows, origin, **extra):
    return [{**r, "origin": origin, **extra} for r in rows]


def merge_landmarks():
    a_rows = cmp.read_csv_loose(A, "landmarks")
    b_rows = cmp.read_csv_loose(B, "landmarks")
    merged = with_origin(a_rows, "A", position_quality="as_given")
    added = 0
    for r in b_rows:
        lat, lon = float(r["lat"]), float(r["lon"])
        nearest = min(cmp.haversine_m(float(x["lat"]), float(x["lon"]), lat, lon) for x in a_rows)
        if nearest > 400:
            row = {**r, "id": "B_" + r["id"], "origin": "B", "position_quality": "approximate"}
            # B's UTM columns are about 112 m off; drop them rather than carry a known error.
            row["utm_e"] = row["utm_n"] = ""
            merged.append(row)
            added += 1
    for r in merged:
        try:
            r["elev_usgs_m"] = f"{cmp.epqs(float(r['lat']), float(r['lon'])):.1f}"
        except Exception:
            r["elev_usgs_m"] = ""
    fields = list(a_rows[0].keys()) + ["origin", "position_quality", "elev_usgs_m"]
    write("landmarks.csv", merged, fields)
    print(f"  A rows {len(a_rows)}, B rows added {added}")


def merge_biomes():
    a_rows = cmp.read_csv_loose(A, "biomes")
    b_rows = cmp.read_csv_loose(B, "biomes")
    merged = with_origin(a_rows, "A") + with_origin([r for r in b_rows if r["band_id"].startswith(BIOMES_FROM_B)], "B")
    write("biomes.csv", merged, list(a_rows[0].keys()) + ["origin"])


def merge_climate():
    a_rows = cmp.read_csv_loose(A, "climate_monthly")
    b_rows = cmp.read_csv_loose(B, "climate_monthly")
    write("climate_monthly.csv", with_origin(a_rows, "A") + with_origin(b_rows, "B"), list(a_rows[0].keys()) + ["origin"])


def merge_settlements():
    a_rows = cmp.read_csv_loose(A, "settlements")
    b_rows = cmp.read_csv_loose(B, "settlements")
    # Some B rows have more values than columns (unquoted commas inside a text field), so the columns
    # after the break are shifted. Keep the extra values in `source` and flag the row for a human check.
    for r in b_rows:
        extra = r.pop(None, None)
        r["row_quality"] = "ok"
        if extra:
            r["source"] = " | ".join([r["source"]] + extra)
            r["row_quality"] = "malformed_csv_row_check_columns_by_hand"
    for r in a_rows:
        r["row_quality"] = "ok"
    fields = list(a_rows[0].keys()) + ["origin"]
    rows = with_origin(a_rows, "A") + with_origin(b_rows, "B")
    write("settlements.csv", rows, fields)
    print("  malformed B rows flagged:", sum(1 for r in rows if r["row_quality"] != "ok"))


def merge_fiction():
    a_rows = cmp.read_csv_loose(A, "fiction_map")
    b_rows = cmp.read_csv_loose(B, "fiction_map")
    extra = [r for r in b_rows if any(r["real_place_or_role"].startswith(k) for k in FICTION_FROM_B)]
    write("fiction_map.csv", with_origin(a_rows, "A") + with_origin(extra, "B"), list(a_rows[0].keys()) + ["origin"])


if __name__ == "__main__":
    merge_landmarks()
    merge_biomes()
    merge_climate()
    merge_settlements()
    merge_fiction()
