"""Fetches the USGS 3DEP 1 m DEM tiles for the Mono Basin world box (ADR 0006, T-0102).

Run:  python tools/world/fetch_dem.py [--manifest-only] [--max-tiles N] [--workers N]
Reads tools/world/terrain_config.json, queries the USGS National Map product API, keeps only the
configured project's tiles, writes data/manifests/dem_<project>.json and downloads missing tiles into
data/raw/dem/. Downloads resume (HTTP Range) and each finished file's size and SHA-256 are recorded in
the manifest; a re-run skips files whose size matches and whose recorded hash still verifies.
Python 3.12 standard library only.
"""

import concurrent.futures
import hashlib
import json
import os
import sys
import time
import urllib.parse
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
CONFIG = json.load(open(os.path.join(HERE, "terrain_config.json"), encoding="utf-8"))
RAW_DIR = os.path.join(ROOT, "data", "raw", "dem")
MANIFEST_DIR = os.path.join(ROOT, "data", "manifests")
USER_AGENT = "codex-terrain-fetch/1.0 (game development, USGS public data)"


def query_products():
    box = CONFIG["world_box_wgs84"]
    params = {
        "datasets": CONFIG["source"]["tnm_dataset"],
        "bbox": f"{box['west']},{box['south']},{box['east']},{box['north']}",
        "outputFormat": "JSON",
        "max": 200,
    }
    url = CONFIG["source"]["tnm_api"] + "?" + urllib.parse.urlencode(params)
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(req, timeout=120) as resp:
        data = json.load(resp)
    items = data.get("items", [])
    if len(items) != data.get("total", len(items)):
        raise RuntimeError(f"API returned {len(items)} of {data.get('total')} items; raise the max parameter")
    return items


def select_tiles(items):
    project = CONFIG["source"]["project"]
    tiles = []
    for it in items:
        if project not in it["title"]:
            continue
        tiles.append({
            "title": it["title"],
            "url": it["downloadURL"],
            "size_bytes": it["sizeInBytes"],
            "published": it.get("publicationDate"),
            "file": os.path.basename(urllib.parse.urlparse(it["downloadURL"]).path),
        })
    tiles.sort(key=lambda t: t["file"])
    return tiles


def sha256_of(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def download(tile, retries=5):
    os.makedirs(RAW_DIR, exist_ok=True)
    final = os.path.join(RAW_DIR, tile["file"])
    part = final + ".part"
    want = tile["size_bytes"]
    if os.path.exists(final) and os.path.getsize(final) == want:
        return final
    for attempt in range(1, retries + 1):
        have = os.path.getsize(part) if os.path.exists(part) else 0
        headers = {"User-Agent": USER_AGENT}
        if have:
            headers["Range"] = f"bytes={have}-"
        try:
            req = urllib.request.Request(tile["url"], headers=headers)
            with urllib.request.urlopen(req, timeout=120) as resp:
                mode = "ab" if (have and resp.status == 206) else "wb"
                with open(part, mode) as f:
                    while True:
                        block = resp.read(1 << 20)
                        if not block:
                            break
                        f.write(block)
            if os.path.getsize(part) == want:
                os.replace(part, final)
                return final
            print(f"  size mismatch after download: {os.path.getsize(part)} of {want}")
        except Exception as ex:  # network hiccup: wait and resume
            print(f"  attempt {attempt} failed: {str(ex)[:80]}")
        time.sleep(2 * attempt)
    raise RuntimeError(f"could not download {tile['file']}")


def main():
    manifest_only = "--manifest-only" in sys.argv
    max_tiles = int(sys.argv[sys.argv.index("--max-tiles") + 1]) if "--max-tiles" in sys.argv else None
    items = query_products()
    tiles = select_tiles(items)
    if max_tiles:
        tiles = tiles[:max_tiles]
    total = sum(t["size_bytes"] for t in tiles)
    print(f"{len(tiles)} tiles of project {CONFIG['source']['project']}, {total / 1e9:.2f} GB")

    os.makedirs(MANIFEST_DIR, exist_ok=True)
    manifest_path = os.path.join(MANIFEST_DIR, f"dem_{CONFIG['source']['project']}.json")
    old = {}
    if os.path.exists(manifest_path):
        old = {t["file"]: t for t in json.load(open(manifest_path, encoding="utf-8"))["tiles"]}

    for tile in tiles:
        prior = old.get(tile["file"], {})
        if prior.get("sha256"):
            tile["sha256"] = prior["sha256"]
            tile["verified_size"] = prior.get("verified_size")

    def fetch_one(tile):
        path = download(tile)
        if tile.get("sha256") and tile.get("verified_size") == tile["size_bytes"] == os.path.getsize(path):
            return tile["file"], "already verified"
        tile["sha256"] = sha256_of(path)
        tile["verified_size"] = os.path.getsize(path)
        return tile["file"], f"{tile['size_bytes'] / 1e6:.0f} MB sha256 {tile['sha256'][:12]}"

    if not manifest_only:
        workers = int(sys.argv[sys.argv.index("--workers") + 1]) if "--workers" in sys.argv else 6
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            for done, (name, note) in enumerate(pool.map(fetch_one, tiles), 1):
                print(f"[{done}/{len(tiles)}] {name} {note}", flush=True)

    manifest = {
        "source": CONFIG["source"],
        "world_box_wgs84": CONFIG["world_box_wgs84"],
        "queried_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "tile_count": len(tiles),
        "total_bytes": total,
        "licence": "USGS 3DEP data are U.S. Government works in the public domain; credit USGS 3D Elevation Program.",
        "tiles": tiles,
    }
    with open(manifest_path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(manifest, f, indent=2)
    print("manifest:", os.path.relpath(manifest_path, ROOT))


if __name__ == "__main__":
    main()
