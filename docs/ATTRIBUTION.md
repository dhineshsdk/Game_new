# Attribution and data licences

Credit lines to ship with the game, and the licence facts behind them. Not legal advice: the OpenStreetMap share-alike question below needs the director's decision (and ideally a lawyer) before release.

## Credit lines
- **OpenStreetMap:** Road network geometry and road attributes are derived from OpenStreetMap data. (c) OpenStreetMap contributors. Data available under the Open Database Licence (ODbL) 1.0, https://www.openstreetmap.org/copyright
- **USGS 3DEP:** Terrain elevation derived from the U.S. Geological Survey 3D Elevation Program (3DEP) 1-meter DEM, project CA_SierraNevada_B22 (2025). USGS data are U.S. Government works in the public domain; credit is requested.

## What we use, and where the facts are recorded
| Source | Used for | Recorded in |
|---|---|---|
| USGS 3DEP 1 m DEM, project CA_SierraNevada_B22 | Terrain heightmaps | `data/manifests/dem_CA_SierraNevada_B22.json` (tile list, sizes, SHA-256, query time) |
| OpenStreetMap via the Overpass API | Road centrelines and tags | `data/manifests/osm_roads.json` (query, endpoint, OSM data timestamp, per-road statistics) |
| Gemini research answer R-0002 | Design intent, names, attribute tables. No geometry or elevation is taken from it unchecked | `docs/research/R-0002-validation.md`, `docs/research/R-0002-comparison.md` |
| Google Maps / Street View | Not used as a data source (their terms forbid reuse of map data in a product) | prompt P-0002, rule 1 |

## Open question for the director: OpenStreetMap share-alike
ODbL requires that a "Derivative Database" built from OSM data is offered under ODbL, while a "Produced Work" made from it (for example rendered images or a game that uses the data) needs only the credit line above. Whether the shipped game's road data files count as a Derivative Database or a Produced Work depends on how they are packaged: the Gemini text in R-0002 section 8 asserts the compiled game is a Produced Work, but that is an assertion, not a ruling. Options when we get closer to release:
1. Ship roads only baked into meshes and splines (strongest case for Produced Work), keep any OSM-derived data files out of the shipped package.
2. Publish the OSM-derived road database separately under ODbL.
3. Take legal advice.
Nothing in the repository needs to change until a release is planned. Raw OSM responses stay in `data/raw/` (not committed).

## Terrain notes
USGS asks that data users cite the source; it places no other conditions on 3DEP products. The derived heightmaps are not committed (large, reproducible with `tools/world/fetch_dem.py` and `build_heightmap.py`).
