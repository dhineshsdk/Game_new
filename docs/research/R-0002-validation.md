# R-0002 validation: what the Gemini world reference can and cannot be used for

Checked 2026-10-17 with `python -I tools/world/validate_reference.py --online` against `data/reference/` (Gemini answer to prompt P-0002). Re-run the script after any change to the files.

## Verdict by file
| File | Use it for | Do not use it for |
|---|---|---|
| `world_box.geojson` | Area outlines (town, canyon, Tioga Pass area, Lundy, Conway, Bodie, Mono Lake outline, June Lake extension bounds). All inside the box except the extension. | Exact shorelines: the Mono Lake polygon has 13 points. The real shoreline comes from the DEM. |
| `roads.geojson` | The OSM way IDs (all 106 exist in OSM as highways with matching refs and names, checked 2026-10-17), the property table (surface, lanes, width, speed limit, closures), and the points themselves: they lie on real OSM roads (median distance 0 m, 692 of 697 SR 120 points within 25 m). | The point ORDER. Each "line" is several OSM ways joined in arbitrary order: SR 120 has 10 gaps over 400 m (largest 9.3 km) and a 51 km line for a 13 km end-to-end road; SR 270 ends where it starts. Fetch the ways by ID from OSM and order them (task T-0104). |
| `road_facts.csv` | Targets for acceptance tests once rechecked. The grade arithmetic is internally consistent. | Anything that depends on its elevations: they come from `landmarks.csv` and are partly wrong (below). |
| `landmarks.csv` | Names, kinds, photo-spot ideas, homestead candidate descriptions. Road junction positions are exact (the three US-395 junctions are within 1 m of the OSM junction nodes); UTM columns are consistent with lat/lon (within 1 m). | Elevations. 33 of 53 differ from USGS 3DEP by more than 10 m; worst cases: US-395/SR 270 junction +337 m, homestead candidate A -260 m, Black Point +145 m, Bodie Road end of pavement -134 m, Conway Summit crest -118 m. Summit, lake and town-centre values are accurate (within 2 m). Positions of non-road points (tufa, vistas, homesteads) are not verified. |
| `biomes.csv`, `settlements.csv`, `climate_monthly.csv`, `fiction_map.csv` | First-pass art and design input. Not checked against sources yet; every row has a `source` column, none was opened. | Numeric simulation values (weather, sun times) until checked. Sun times will come from astronomy math, not this table. |
| `R-0002-mono-basin-world-reference.md` | Reading the visual brief and the fiction names. | Trusting its quality checklist: it says every check passed, but the road and elevation problems above were not caught. Only 2 "UNVERIFIED" marks in 300 KB of text. |

## What the checks covered
- Schemas: all six CSV headers match the prompt exactly; GeoJSON properties match; no empty cells.
- World box: every point inside, except the June Lake extension (intended).
- UTM: all 53 landmark UTM pairs match our own WGS84 to UTM 11N calculation within 1 m.
- Elevation: 53 landmarks against the USGS 3DEP point-elevation service (`epqs.nationalmap.gov`).
- Roads: continuity of consecutive points; distance of every point to the real OSM main roads (Overpass, later on 2026-10-17 after earlier 502 errors); existence and type of all 106 way IDs. Not done: widths, speed limits and surfaces against OSM tags.

## Consequences
1. Terrain comes from USGS only. Nothing in `data/reference/` feeds elevations.
2. Road geometry comes from OSM directly (T-0104), keyed by the way IDs in `roads.geojson` (all valid).
3. Any elevation in a design document is replaced by a value sampled from the finished heightmap.
4. The Gemini output is still valuable as a map of what to build; ask it for corrections only if a specific row matters.

See `R-0002-comparison.md` for the second Gemini set in `ref/` and which set to use for what.
