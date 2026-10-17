"""Scores two Gemini world-reference folders against independent facts and against each other.

Run:  python -I tools/world/compare_reference.py data/reference ref [--online]
Checks (all independent of the files being judged):
  - landmark overlap: which places appear in both, which only in one, and position differences
  - landmark elevations against USGS 3DEP point elevations (--online; cached in the temp folder)
  - sunrise/sunset at the Lee Vining station against the NOAA solar position algorithm
  - biome bands against the box's real elevation range (gaps at the ends)
  - fictional names that still contain real business names
Python 3.12 standard library only.
"""

import csv
import datetime
import json
import math
import os
import sys
import tempfile
import time
import urllib.parse
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CACHE = os.path.join(tempfile.gettempdir(), "codex_epqs_cache.json")
LEE_VINING = (37.9576, -119.1192)
REAL_BUSINESS_WORDS = ["chevron", "mobil", "nicely", "whoa nellie", "tioga gas", "mono market", "shell", "exxon", "ford", "toyota", "walmart", "mono vista"]


def read_csv_loose(folder, name):
    """CSV by name, or any file in the folder whose first line is that CSV's header (ref/ saved one under its header)."""
    path = os.path.join(ROOT, folder, name + ".csv")
    if not os.path.exists(path):
        head = {"settlements": "site,building_kind"}.get(name, name)
        for fn in os.listdir(os.path.join(ROOT, folder)):
            p = os.path.join(ROOT, folder, fn)
            if os.path.isfile(p) and open(p, encoding="utf-8", errors="replace").readline().startswith(head):
                path = p
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def haversine_m(lat1, lon1, lat2, lon2):
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp, dl = p2 - p1, math.radians(lon2 - lon1)
    h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * 6371000.0 * math.asin(math.sqrt(h))


def epqs(lat, lon):
    cache = json.load(open(CACHE)) if os.path.exists(CACHE) else {}
    key = f"{lat:.5f},{lon:.5f}"
    if key not in cache:
        q = urllib.parse.urlencode({"x": lon, "y": lat, "wkid": 4326, "units": "Meters", "includeDate": "false"})
        req = urllib.request.Request("https://epqs.nationalmap.gov/v1/json?" + q, headers={"User-Agent": "codex-reference-check/1.0"})
        with urllib.request.urlopen(req, timeout=60) as r:
            cache[key] = float(json.load(r)["value"])
        json.dump(cache, open(CACHE, "w"))
        time.sleep(0.2)
    return cache[key]


def solar_times_local(date, lat, lon, utc_offset_h):
    """NOAA solar calculator: sunrise and sunset (clock minutes after local midnight) for a flat horizon."""
    jd = date.toordinal() + 1721424.5
    t = (jd - 2451545.0) / 36525.0
    l0 = math.radians((280.46646 + t * (36000.76983 + t * 0.0003032)) % 360)
    m = math.radians(357.52911 + t * (35999.05029 - 0.0001537 * t))
    e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t)
    c = math.radians((1.914602 - t * (0.004817 + 0.000014 * t)) * math.sin(m) + (0.019993 - 0.000101 * t) * math.sin(2 * m) + 0.000289 * math.sin(3 * m))
    true_long = l0 + c
    omega = math.radians(125.04 - 1934.136 * t)
    app_long = true_long - math.radians(0.00569 + 0.00478 * math.sin(omega))
    eps0 = 23 + (26 + (21.448 - t * (46.815 + t * (0.00059 - t * 0.001813))) / 60) / 60
    eps = math.radians(eps0 + 0.00256 * math.cos(omega))
    decl = math.asin(math.sin(eps) * math.sin(app_long))
    y = math.tan(eps / 2) ** 2
    eqtime = 4 * math.degrees(y * math.sin(2 * l0) - 2 * e * math.sin(m) + 4 * e * y * math.sin(m) * math.cos(2 * l0) - 0.5 * y * y * math.sin(4 * l0) - 1.25 * e * e * math.sin(2 * m))
    ha = math.acos(math.cos(math.radians(90.833)) / (math.cos(math.radians(lat)) * math.cos(decl)) - math.tan(math.radians(lat)) * math.tan(decl))
    solar_noon = 720 - 4 * lon - eqtime + utc_offset_h * 60
    return solar_noon - 4 * math.degrees(ha), solar_noon + 4 * math.degrees(ha)


def us_utc_offset_2026(month, day):
    # US DST 2026: second Sunday of March (Mar 8) to first Sunday of November (Nov 1).
    d = datetime.date(2026, month, day)
    return -7 if datetime.date(2026, 3, 8) <= d < datetime.date(2026, 11, 1) else -8


def hhmm_to_min(text):
    h, m = text.split(":")
    return int(h) * 60 + int(m)


def check_sun(folder):
    rows = read_csv_loose(folder, "climate_monthly")
    station = next(r["station"] for r in rows if "ee" in r["station"].lower() and "vin" in r["station"].lower())
    errs = []
    for r in rows:
        if r["station"] != station:
            continue
        text = r["month"].strip()
        month = int(text) if text.isdigit() else ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"].index(text[:3].lower()) + 1
        rise, sett = solar_times_local(datetime.date(2026, month, 15), LEE_VINING[0], LEE_VINING[1], us_utc_offset_2026(month, 15))
        errs.append((abs(hhmm_to_min(r["sunrise_local"]) - rise), abs(hhmm_to_min(r["sunset_local"]) - sett), month))
    worst = max(max(a, b) for a, b, _ in errs)
    avg = sum(a + b for a, b, _ in errs) / (2 * len(errs))
    return station, len(errs), avg, worst


def check_biomes(folder):
    rows = read_csv_loose(folder, "biomes")
    lo = min(float(r["elev_min_m"]) for r in rows)
    hi = max(float(r["elev_max_m"]) for r in rows)
    return len(rows), lo, hi


def check_fiction(folder):
    rows = read_csv_loose(folder, "fiction_map")
    flagged = []
    for r in rows:
        low = r["fictional_name"].lower()
        hit = [w for w in REAL_BUSINESS_WORDS if w in low]
        if "lee vining" in low or hit:
            flagged.append((r["fictional_name"], hit or ["real town name"]))
    return len(rows), flagged


def landmark_set(folder, online):
    rows = read_csv_loose(folder, "landmarks")
    out = []
    for r in rows:
        item = {"id": r["id"], "name": r["name_real"], "lat": float(r["lat"]), "lon": float(r["lon"]), "elev": float(r["elev_m"]), "kind": r["kind"]}
        if online:
            try:
                item["usgs"] = epqs(item["lat"], item["lon"])
            except Exception as ex:
                item["usgs"] = None
                print("  lookup failed", r["id"], str(ex)[:50])
        out.append(item)
    return out


def elevation_summary(items):
    diffs = sorted(abs(i["elev"] - i["usgs"]) for i in items if i.get("usgs") is not None)
    if not diffs:
        return None
    return {"n": len(diffs), "median_m": round(diffs[len(diffs) // 2], 1), "over_10m": sum(d > 10 for d in diffs),
            "over_50m": sum(d > 50 for d in diffs), "over_100m": sum(d > 100 for d in diffs), "max_m": round(diffs[-1], 1)}


def main():
    folders = [a for a in sys.argv[1:] if not a.startswith("--")]
    online = "--online" in sys.argv
    sets = {}
    for f in folders:
        print(f"\n=== {f}")
        items = landmark_set(f, online)
        sets[f] = items
        print("landmarks:", len(items))
        if online:
            print("elevation vs USGS 3DEP:", elevation_summary(items))
        station, n, avg, worst = check_sun(f)
        print(f"sunrise/sunset ({station}, {n} months) vs astronomy: mean abs error {avg:.1f} min, worst {worst:.0f} min")
        nb, lo, hi = check_biomes(f)
        print(f"biome bands: {nb}, cover {lo:.0f} to {hi:.0f} m (box is about 1945 to 3980 m)")
        nf, flagged = check_fiction(f)
        print(f"fiction_map: {nf} rows; still containing a real name/brand: {flagged}")
    if len(folders) == 2:
        a, b = folders
        print(f"\n=== overlap: {a} ({len(sets[a])}) vs {b} ({len(sets[b])}); a match is the nearest place within 400 m")
        only_b = []
        dists = []
        for ib in sets[b]:
            best = min(sets[a], key=lambda ia: haversine_m(ia["lat"], ia["lon"], ib["lat"], ib["lon"]))
            d = haversine_m(best["lat"], best["lon"], ib["lat"], ib["lon"])
            if d <= 400:
                dists.append(d)
            else:
                only_b.append((ib["name"], ib["kind"], round(d)))
        print(f"  {b} places with a counterpart in {a}: {len(dists)}; median position difference {sorted(dists)[len(dists) // 2]:.0f} m, max {max(dists):.0f} m")
        print(f"  {b} places with NO counterpart in {a} ({len(only_b)}):")
        for n, k, d in only_b:
            print(f"    {n[:58]:58} {k:12} nearest in {a}: {d} m")
        only_a = []
        for ia in sets[a]:
            d = min(haversine_m(ia["lat"], ia["lon"], ib["lat"], ib["lon"]) for ib in sets[b])
            if d > 400:
                only_a.append((ia["name"], ia["kind"]))
        print(f"  {a} places with NO counterpart in {b}: {len(only_a)}")


if __name__ == "__main__":
    main()
