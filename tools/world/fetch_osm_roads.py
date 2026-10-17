"""Fetches the Mono Basin road ways from OpenStreetMap and orders them into centrelines (ADR 0006, T-0104).

Run:  python tools/world/fetch_osm_roads.py [--refresh]
Clips the result to the world box. Takes the OSM way IDs listed per road in data/reference/roads.geojson (Gemini; every ID was checked to
exist), asks the Overpass API for their geometry and tags, caches the raw response in data/raw/osm/,
orders the ways with road_graph.order_ways and writes:
  data/derived/roads/roads_ordered.geojson   one feature per chain, with the road's name and way IDs
  data/manifests/osm_roads.json              query, timestamps, per-road statistics, tag disagreements
OpenStreetMap data (c) OpenStreetMap contributors, ODbL 1.0; see docs/ATTRIBUTION.md.
Python 3.12 standard library only.
"""

import hashlib
import json
import os
import sys
import time
import urllib.parse
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import road_graph as rg  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(HERE))
REFERENCE = os.path.join(ROOT, "data", "reference", "roads.geojson")
CONFIG = json.load(open(os.path.join(HERE, "terrain_config.json"), encoding="utf-8"))
BOX = CONFIG["world_box_wgs84"]
BOX_TUPLE = (BOX["west"], BOX["south"], BOX["east"], BOX["north"])
CACHE_DIR = os.path.join(ROOT, "data", "raw", "osm")
OUT_DIR = os.path.join(ROOT, "data", "derived", "roads")
MANIFEST = os.path.join(ROOT, "data", "manifests", "osm_roads.json")
ENDPOINTS = ["https://overpass-api.de/api/interpreter", "https://overpass.kumi.systems/api/interpreter"]
USER_AGENT = "codex-osm-fetch/1.0 (game development; contact via project repository)"
MPH_TO_KMH = 1.609344


def load_roads():
    with open(REFERENCE, encoding="utf-8") as f:
        fc = json.load(f)
    return [ft["properties"] for ft in fc["features"]]


def overpass(query):
    last = None
    for endpoint in ENDPOINTS:
        for attempt in range(3):
            try:
                req = urllib.request.Request(endpoint, data=urllib.parse.urlencode({"data": query}).encode(), headers={"User-Agent": USER_AGENT})
                with urllib.request.urlopen(req, timeout=180) as resp:
                    return json.load(resp), endpoint
            except Exception as ex:
                last = ex
                time.sleep(3 * (attempt + 1))
    raise RuntimeError(f"Overpass unreachable: {last}")


def fetch_ways(ids, refresh):
    query = "[out:json][timeout:120];way(id:%s);out geom;" % ",".join(str(i) for i in sorted(ids))
    key = hashlib.sha256(query.encode()).hexdigest()[:16]
    os.makedirs(CACHE_DIR, exist_ok=True)
    path = os.path.join(CACHE_DIR, f"ways_{key}.json")
    if os.path.exists(path) and not refresh:
        with open(path, encoding="utf-8") as f:
            cached = json.load(f)
        return cached["response"], cached["endpoint"], query, path
    response, endpoint = overpass(query)
    with open(path, "w", encoding="utf-8") as f:
        json.dump({"endpoint": endpoint, "fetched_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()), "response": response}, f)
    return response, endpoint, query, path


def way_record(element):
    return {"id": element["id"], "nodes": element["nodes"], "points": [(p["lon"], p["lat"]) for p in element["geometry"]], "tags": element.get("tags", {})}


def tag_disagreements(claimed, ways):
    """Compare what the reference claims for a road with the tags of its OSM ways."""
    notes = []
    claimed_ref = (claimed.get("ref") or "").replace(" / County Road", "")
    osm_refs = {r.strip() for w in ways for r in w["tags"].get("ref", "").split(";") if r.strip()}
    if claimed_ref and osm_refs and not any(claimed_ref.split(" / ")[0] in r or r in claimed_ref for r in osm_refs):
        notes.append(f"ref: reference says '{claimed.get('ref')}', OSM ways say {sorted(osm_refs)}")
    surfaces = {w["tags"].get("surface") for w in ways}
    paved = {"asphalt", "paved", "concrete"}
    unpaved = {"gravel", "dirt", "compacted", "unpaved", "ground", "fine_gravel"}
    claimed_surface = claimed.get("surface")
    known = {s for s in surfaces if s}
    if known and claimed_surface in ("asphalt",) and not known <= paved:
        notes.append(f"surface: reference says asphalt, OSM has {sorted(known)}")
    if known and claimed_surface in ("gravel", "dirt", "compacted") and not known <= unpaved:
        notes.append(f"surface: reference says {claimed_surface}, OSM has {sorted(known)}")
    if not known:
        notes.append("surface: no surface tag on any way (reference value cannot be checked)")
    speeds = {w["tags"].get("maxspeed") for w in ways if w["tags"].get("maxspeed")}
    if speeds and claimed.get("speed_limit_mph"):
        mph = set()
        for s in speeds:
            digits = "".join(c for c in s if c.isdigit())
            if digits:
                mph.add(round(int(digits) / MPH_TO_KMH) if "mph" not in s else int(digits))
        if mph and int(claimed["speed_limit_mph"]) not in mph:
            notes.append(f"maxspeed: reference says {claimed['speed_limit_mph']} mph, OSM has {sorted(speeds)}")
    return notes


def main():
    roads = load_roads()
    all_ids = sorted({i for r in roads for i in r["osm_way_ids"]})
    response, endpoint, query, cache_path = fetch_ways(all_ids, "--refresh" in sys.argv)
    by_id = {e["id"]: e for e in response["elements"] if e["type"] == "way" and "geometry" in e}
    print(f"{len(by_id)} of {len(all_ids)} way IDs returned by {endpoint}")

    os.makedirs(OUT_DIR, exist_ok=True)
    os.makedirs(os.path.dirname(MANIFEST), exist_ok=True)
    features, road_stats = [], []
    for road in roads:
        found = [way_record(by_id[i]) for i in road["osm_way_ids"] if i in by_id]
        missing = [i for i in road["osm_way_ids"] if i not in by_id]
        chains = []
        for chain in rg.order_ways(found):
            # OSM ways run past the world box (SR 167 continues into Nevada); keep only what is inside it.
            for piece in rg.clip_polyline_to_box(chain["points"], BOX_TUPLE):
                chains.append({"way_ids": chain["way_ids"], "points": piece})
        stat = {
            "name": road["name"], "claimed_ref": road["ref"], "ways_claimed": len(road["osm_way_ids"]), "ways_found": len(found),
            "ways_missing": missing, "chains": [], "tag_disagreements": tag_disagreements(road, found),
        }
        for n, chain in enumerate(chains):
            pts = chain["points"]
            info = {"chain": n, "way_ids": chain["way_ids"], "points": len(pts), "length_km": round(rg.polyline_length_m(pts) / 1000, 3),
                    "end_to_end_km": round(rg.haversine_m(pts[0], pts[-1]) / 1000, 3), "max_vertex_spacing_m": round(rg.max_gap_m(pts), 1)}
            stat["chains"].append(info)
            features.append({"type": "Feature", "properties": {"name": road["name"], "ref": road["ref"], "surface": road["surface"], "chain": n,
                                                              "way_ids": chain["way_ids"], "length_km": info["length_km"]},
                             "geometry": {"type": "LineString", "coordinates": [[round(x, 7), round(y, 7)] for x, y in pts]}})
        road_stats.append(stat)
        longest = max((c["length_km"] for c in stat["chains"]), default=0)
        print(f"  {road['name'][:46]:46} ways {len(found)}/{len(road['osm_way_ids'])}  chains {len(chains):2}  longest {longest:6.2f} km  "
              f"max vertex spacing {max((c['max_vertex_spacing_m'] for c in stat['chains']), default=0):6.1f} m  tag notes {len(stat['tag_disagreements'])}")

    with open(os.path.join(OUT_DIR, "roads_ordered.geojson"), "w", encoding="utf-8", newline="\n") as f:
        json.dump({"type": "FeatureCollection", "features": features}, f)
    manifest = {
        "attribution": "(c) OpenStreetMap contributors, ODbL 1.0",
        "overpass_endpoint": endpoint, "query": query, "cache_file": os.path.relpath(cache_path, ROOT),
        "osm_data_timestamp": response.get("osm3s", {}).get("timestamp_osm_base"),
        "generator": response.get("generator"),
        "way_ids_claimed": len(all_ids), "way_ids_found": len(by_id), "way_ids_missing": sorted(set(all_ids) - set(by_id)),
        "roads": road_stats,
    }
    with open(MANIFEST, "w", encoding="utf-8", newline="\n") as f:
        json.dump(manifest, f, indent=2)
    print("wrote", os.path.relpath(MANIFEST, ROOT), "and data/derived/roads/roads_ordered.geojson")


if __name__ == "__main__":
    main()
