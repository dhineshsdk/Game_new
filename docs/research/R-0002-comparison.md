# R-0002 comparison: `data/reference/` (set A) vs `ref/` (set B)

Both folders answer prompt P-0002. Checked 2026-10-17 with `tools/world/validate_reference.py` and `tools/world/compare_reference.py` (both re-runnable), plus OSM data from the Overpass API and USGS 3DEP point elevations. Set B was saved by the director in `ref/`; two of its files have odd names (`roads.geojson.json`, and the settlements CSV saved under its header line) and it has no `world_box.geojson` file (the box is inside its markdown answer).

## Scorecard
| Check | Set A `data/reference` | Set B `ref` |
|---|---|---|
| Road geometry vs real OSM roads | Points are real OSM nodes: median distance 0 m (692/697 SR 120 points within 25 m, 435/435 SR 270). 3,131 points. Order within each line is wrong. | Median 200 to 970 m from the real roads, 0 to 2 points of 5 to 20 within 25 m. 84 points in total, 1 to 3.5 km apart: sketches. |
| Claimed OSM way IDs | 106 of 106 exist, all highways, with matching refs and names | 23 of 30 exist, only 9 are highways; the others include "Florentine Place", "Hiker Court", "Abington Executive Park" (not Mono County): invented IDs |
| Junction positions vs OSM junction nodes | US-395/SR 167: 0 m, SR 270: 1 m, SR 120: 0 m | SR 167: 829 m, SR 270: 5,676 m (SR 120 junction not listed) |
| Landmark UTM columns vs own lat/lon | Exact (under 1 m) | All 44 wrong: about 112 m too far north, 20 m west to 30 m east |
| Landmark elevations vs USGS 3DEP | 53 checked: median error 22 m, 33 over 10 m, 8 over 100 m, worst 337 m | 44 checked: median 19 m, 25 over 10 m, 5 over 100 m, worst 479 m. Equally unreliable |
| Sunrise/sunset vs astronomy (Lee Vining, 15th of each month) | mean error 3.2 min, worst 6 | mean error 2.1 min, worst 5 |
| Biome bands vs real elevation range | 7 bands, 1,945 to 3,978 m, full coverage | 8 bands, 1,946 to 3,980 m, full coverage |
| Fictional names still using a real brand | none | none (but "Vining Creek" and "Edison Basin Plant" stay close to real names) |
| Counts | 53 landmarks, 17 road-fact rows, 7 biomes, 16 settlement rows, 17 name rows | 44 landmarks, 16 road-fact rows, 8 biomes, 12 settlement rows, 12 name rows |
| Overlap | 16 of B's 44 places have a counterpart within 400 m in A (median position difference 205 m, so at least one of the two is off for each) | 28 places have no counterpart in A; A has 32 places B lacks |

## Which to use for what
| Data | Use | Why |
|---|---|---|
| Road geometry and way IDs | **A** | On the real OSM roads, valid IDs. B's coordinates and IDs cannot be trusted. Final geometry is still fetched from OSM by ID (T-0104). |
| Road attributes (surface, lanes, width, speed) | **A** | B's `roads.geojson.json` has only 13 features with few points; A has all 15 with the full property set. Both still need a tag check against OSM. |
| Area outlines (`world_box.geojson`) | **A** | B has no file, only text in the markdown. |
| Landmark positions on roads (junctions, passes) | **A** | Exact. B is off by hundreds to thousands of metres. |
| Landmark elevations | **Neither** | Both are about 20 m off on median and wrong by over 100 m in several places. Take every elevation from the DEM once it is built. |
| Landmark list (what to build) | **A as base, B as ideas** | B names 28 places A lacks: Ellery Lake dam, Saddlebag Lake road junction, Lee Vining Canyon vista points, DeChambeau/County Park tufa, Black Point fissures, Lundy Lake Road junction, extra photo spots, 4 different homestead candidates. Their positions are rough (up to 4 km off), so each is snapped to a verified feature or checked in the editor before use. |
| Biomes | **B for content, A for brevity** | B adds a volcanic pumice flats band (Mono Craters, Panum area), a separate mountain-sagebrush band and full Latin names. Merge B's bands into A's table. |
| Climate | **Either, then verify** | Sun times are good in both. B uses real station IDs (NOAA USC00044881, CDEC Dana Meadows at 2,987 m); A labels its high station "Ellery_Lake" with a Tioga Pass source, which does not match. Monthly temperature and rain are plausible in both and differ by 0.5 to 1 degree C in Jan/Jul highs and 13 to 16 % in precipitation; neither has been checked against the source tables. Prefer B's stations. |
| Settlements and buildings | **Both, they are complementary** | A lists specific buildings (named motels, diner, gas station, the homestead and barn): good for placing hero buildings. B gives counts per building type (85 houses, 42 mobile homes) and a Conway Ranch and Tioga Pass Resort site: good for filling the town. Neither has been checked against imagery. |
| Fictional names | **A as base, add from B** | A covers more roles (homestead, barn, school, powerhouse). B adds a walk-up burger stand, a garage and the Conway ranch. Keep "Pumice Wells" (A) for the town; "Vining Creek" (B) is too close to the real name. Both keep Bodie, Tioga Pass, Mono Lake and Mount Dana, which is intended. Names such as "Edison Basin" should be checked before release. |

## Merge plan (nothing changed in either folder yet)
1. Keep A as the base set for all geometry and for landmark positions on roads.
2. Add B's 28 extra landmarks to a separate file with a `position_quality` column set to `approximate`, to be snapped to OSM features (junctions, dams, parking areas) or placed in the editor against the terrain.
3. Add B's two extra biome bands to A's `biomes.csv`; re-key the bands to elevation ranges measured from the DEM.
4. Replace A's climate station labels with B's.
5. Append B's building-count rows and extra names to A's tables.
6. Drop B's roads, junction coordinates and UTM columns.

Do not edit Gemini's original answers; make the merge as new files so both stay available for audit.
