"""Checks the Gemini world reference (data/reference/, prompt P-0002) before anything is built from it.

Offline checks: schema columns, world box membership, UTM columns against our own projection,
road continuity (jumps between consecutive points), grade arithmetic in road_facts.csv.
Online check (--online): landmark elevations against the USGS 3DEP point-elevation service.

Run:  python -I tools/world/validate_reference.py [--online]
Exit code 1 when a hard error is found (schema, box, UTM). Elevation and road-continuity problems
are reported as warnings: they decide how the data may be used, they do not make the files invalid.
Python 3.12 standard library only.
"""

import csv
import json
import math
import os
import sys
import time
import urllib.parse
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REF = os.path.join(ROOT, "data", "reference")
WEST, EAST, SOUTH, NORTH = -119.27, -118.96, 37.89, 38.215

COLUMNS = {
    "landmarks": "id,name_real,kind,lat,lon,utm_e,utm_n,elev_m,visible_from_road,why_it_matters,source",
    "road_facts": "road,segment,from_lat,from_lon,to_lat,to_lon,length_km,elev_start_m,elev_end_m,avg_grade_pct,max_grade_pct,notable_features,source",
    "biomes": "band_id,elev_min_m,elev_max_m,aspect_notes,dominant_plants,secondary_plants,ground_cover,rock_types,soil_colour_hex,vegetation_density_0_1,snow_months,source",
    "settlements": "site,building_kind,count_estimate,typical_size_m,materials,roof,colour_notes,era,notes,source",
    "climate_monthly": "station,month,t_max_c,t_min_c,precip_mm,snow_cm,snow_depth_cm,wind_ms,wind_dir_deg,cloud_cover_pct,thunder_days,sunrise_local,sunset_local,source",
    "fiction_map": "real_place_or_role,fictional_name,reason,notes",
}
ROAD_KEYS = {"name", "ref", "osm_way_ids", "surface", "lanes", "width_m", "shoulder_m", "speed_limit_mph", "guardrail", "centre_line", "seasonal_closure", "source"}

errors = []
warnings = []


def in_box(lon, lat):
    return WEST <= lon <= EAST and SOUTH <= lat <= NORTH


def utm11(lat, lon):
    """WGS84 -> UTM zone 11N (Krueger-free classic series, good to well under 1 m here)."""
    a, f, k0 = 6378137.0, 1 / 298.257223563, 0.9996
    e2 = f * (2 - f)
    ep2 = e2 / (1 - e2)
    lon0 = math.radians(-117.0)
    phi, lam = math.radians(lat), math.radians(lon)
    n = a / math.sqrt(1 - e2 * math.sin(phi) ** 2)
    t, c, aa = math.tan(phi) ** 2, ep2 * math.cos(phi) ** 2, math.cos(phi) * (lam - lon0)
    m = a * ((1 - e2 / 4 - 3 * e2**2 / 64 - 5 * e2**3 / 256) * phi - (3 * e2 / 8 + 3 * e2**2 / 32 + 45 * e2**3 / 1024) * math.sin(2 * phi)
             + (15 * e2**2 / 256 + 45 * e2**3 / 1024) * math.sin(4 * phi) - (35 * e2**3 / 3072) * math.sin(6 * phi))
    x = k0 * n * (aa + (1 - t + c) * aa**3 / 6 + (5 - 18 * t + t * t + 72 * c - 58 * ep2) * aa**5 / 120) + 500000
    y = k0 * (m + n * math.tan(phi) * (aa * aa / 2 + (5 - t + 9 * c + 4 * c * c) * aa**4 / 24 + (61 - 58 * t + t * t + 600 * c - 330 * ep2) * aa**6 / 720))
    return x, y


def haversine_m(a, b):
    r = 6371000.0
    p1, p2 = math.radians(a[1]), math.radians(b[1])
    dp, dl = p2 - p1, math.radians(b[0] - a[0])
    h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * r * math.asin(math.sqrt(h))


def read_csv(name):
    with open(os.path.join(REF, name + ".csv"), encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f))
    if ",".join(rows[0].keys()) != COLUMNS[name]:
        errors.append(f"{name}.csv: columns differ from the prompt")
    return rows


def check_csv_schemas():
    for name in COLUMNS:
        rows = read_csv(name)
        print(f"{name}.csv: {len(rows)} rows")


def check_landmarks():
    rows = read_csv("landmarks")
    for r in rows:
        lat, lon = float(r["lat"]), float(r["lon"])
        if not in_box(lon, lat):
            errors.append(f"landmark {r['id']} outside the world box")
        e, n = utm11(lat, lon)
        if abs(float(r["utm_e"]) - e) > 5 or abs(float(r["utm_n"]) - n) > 5:
            errors.append(f"landmark {r['id']}: utm columns disagree with lat/lon by more than 5 m")
    return rows


def check_world_box():
    with open(os.path.join(REF, "world_box.geojson"), encoding="utf-8") as f:
        fc = json.load(f)
    for ft in fc["features"]:
        name = ft["properties"].get("name")
        outside = [c for c in ft["geometry"]["coordinates"][0] if not in_box(c[0], c[1])]
        if outside and ft["properties"].get("kind") != "extension":
            errors.append(f"world_box: {name} has points outside the box")
    print(f"world_box.geojson: {len(fc['features'])} features")


def check_roads():
    with open(os.path.join(REF, "roads.geojson"), encoding="utf-8") as f:
        fc = json.load(f)
    print(f"roads.geojson: {len(fc['features'])} features")
    for ft in fc["features"]:
        p = ft["properties"]
        if set(p) != ROAD_KEYS:
            errors.append(f"roads: {p.get('name')} property keys differ from the prompt")
        pts = ft["geometry"]["coordinates"]
        if any(not in_box(c[0], c[1]) for c in pts):
            errors.append(f"roads: {p['name']} has points outside the box")
        seg = [haversine_m(pts[i], pts[i + 1]) for i in range(len(pts) - 1)]
        jumps = [s for s in seg if s > 400.0]
        # OSM vertices on a road are tens of metres apart; several 400 m+ gaps mean separate ways
        # were concatenated into one line and the points are not an ordered centreline.
        if jumps:
            warnings.append(f"roads: {p['name']}: {len(jumps)} gaps over 400 m between consecutive points (largest {max(jumps) / 1000:.1f} km); "
                            f"line length {sum(seg) / 1000:.1f} km vs end-to-end {haversine_m(pts[0], pts[-1]) / 1000:.1f} km. Not a continuous centreline")


def check_road_facts():
    for r in read_csv("road_facts"):
        length_m = float(r["length_km"]) * 1000
        calc = (float(r["elev_end_m"]) - float(r["elev_start_m"])) / length_m * 100
        if abs(calc - float(r["avg_grade_pct"])) > 0.6:
            errors.append(f"road_facts: {r['segment']}: avg grade {r['avg_grade_pct']} does not follow from its own elevations ({calc:.1f})")


def check_elevations_online(landmarks):
    worst = []
    for r in landmarks:
        q = urllib.parse.urlencode({"x": r["lon"], "y": r["lat"], "wkid": 4326, "units": "Meters", "includeDate": "false"})
        req = urllib.request.Request("https://epqs.nationalmap.gov/v1/json?" + q, headers={"User-Agent": "codex-reference-check/1.0"})
        try:
            with urllib.request.urlopen(req, timeout=60) as resp:
                z = float(json.load(resp)["value"])
        except Exception as ex:  # network problems are a warning, not a data error
            warnings.append(f"elevation lookup failed for {r['id']}: {str(ex)[:60]}")
            continue
        worst.append((abs(float(r["elev_m"]) - z), r["id"], float(r["elev_m"]), z))
        time.sleep(0.25)
    worst.sort(reverse=True)
    bad = [w for w in worst if w[0] > 10.0]
    print(f"elevations: {len(worst)} checked against USGS 3DEP, {len(bad)} differ by more than 10 m")
    for d, i, claimed, z in bad:
        warnings.append(f"elevation: {i}: claimed {claimed:.0f} m, USGS {z:.0f} m (diff {claimed - z:+.0f} m)")


def main():
    check_csv_schemas()
    check_world_box()
    landmarks = check_landmarks()
    check_roads()
    check_road_facts()
    if "--online" in sys.argv:
        check_elevations_online(landmarks)
    for w in warnings:
        print("WARNING:", w)
    for e in errors:
        print("ERROR:", e)
    print(f"{len(errors)} error(s), {len(warnings)} warning(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
