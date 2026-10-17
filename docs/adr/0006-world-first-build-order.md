# ADR 0006: Build the Mono Basin world before the in-engine car

- Status: accepted
- Date: 2026-10-17
- Phase / workstream: P5 corridor and P6 sky moved ahead of P3 / B (world) and D (presentation)

## Context
The roadmap orders Tier 1 as dynamics core (P2), car in a test arena (P3), cockpit (P4), then the Mono Basin corridor (P5) and sky and weather (P6). On 2026-10-17 the director asked to "build it properly" and to start with world building. The physics lane is at a clean stop: pure core (ADR 0005), headless harness, tyre model (T-0101) and state snapshot (T-0008) are done. `docs/build_strategy.md` section 1 lists "Mono Basin terrain at scale" as one of four risks, and section 4 notes that physics and the terrain pipeline are independent until the corridor phase.

The director has Gemini (Argon 4, 3.1 Pro, 3.8 Flash) produce the world reference; the result is in `data/reference/` and `docs/research/R-0002-mono-basin-world-reference.md`. `docs/research/R-0002-validation.md` records what in it can be trusted (area outlines, road property tables, names) and what cannot (landmark elevations, road geometry).

Facts checked 2026-10-17 against the USGS TNM Access API (`tnmaccess.nationalmap.gov/api/v1/products`, bbox -119.27, 37.89, -118.96, 38.215):
- "Digital Elevation Model (DEM) 1 meter" covers the whole box with 15 tiles of project `CA_SierraNevada_B22` (published 2025-05-15): UTM zone 11 tiles x30..x32, y420..y424, 10 km x 10 km each, GeoTIFF, about 3.5 GB in total. Two older `CA_YosemiteNP_2019_D19` tiles and one `NV_WestCentral_EarthMRI_2020_D20` tile overlap and are not needed.
- 1/3 arc-second (~10 m) seamless DEM is available as a fallback (four 1-degree tiles, ~400 MB each).
- USGS 3DEP products are U.S. Government works in the public domain; attribution is requested, not required.

## Decision
1. **World lane first.** Work proceeds on the world (terrain, roads, then sky and weather) before P3 (car in Unreal). The physics lane pauses; T-0201 (120 Hz solver) waits.
2. **World box.** lon -119.27 to -118.96, lat 37.89 to 38.215 (about 27 km east-west by 36 km north-south). It contains Tioga Pass and Mount Dana, Lee Vining Canyon, Lee Vining, the west and north shores of Mono Lake and Bodie. June Lake Loop (about 37.78 N) is outside and is a later extension.
3. **Source data.** Terrain: USGS 3DEP 1 m DEM, project `CA_SierraNevada_B22`, NAD83 / UTM zone 11N, NAVD88 metres. Roads: OpenStreetMap, fetched directly (T-0104). The Gemini reference supplies design intent and attribute tables only. Raw files live in `data/raw/` and derived tiles in `data/derived/`; both are gitignored (large, reproducible by script). Scripts, manifests and checksums are committed.
4. **Order of world tasks** (one concern and a handful of files each; tests before done):
   | Task | Content |
   |---|---|
   | T-0102 | DEM manifest, download, mosaic, crop, 16-bit heightmap tiles for World Partition; Z-scale maths with tests |
   | T-0103 | Import the tiles into a World Partition Landscape (headless editor Python); verify benchmark elevations |
   | T-0104 | Fetch road centrelines from OSM by way ID and name; ordered, connected lines; ODbL attribution file |
   | T-0105 | Project the roads onto the terrain; grey-box road splines and meshes |
   | T-0106 | Pure C++ solar and lunar position, sidereal time, star field (CodexCore) with tests against published ephemeris values |
   | T-0107 | Sky, sun, moon and stars in Unreal driven by T-0106 |
   | T-0108 | Weather states and visuals |
   Task files for T-0103, T-0105 to T-0108 are written when they start, not before.
5. **Grey-box gate for this lane:** real Mono Basin terrain at true scale in Unreal, roads visible, a default pawn can fly and drive the SR 120 descent into Lee Vining, 60 fps at 1440p on the dev machine, correct sun for a given date and time. Judged by the director.

## Alternatives considered
| Option | Why rejected |
|---|---|
| Keep the roadmap order (car first) | The director asked for world first; the physics lane is at a clean stop |
| Use Gemini's landmark elevations and road lines | Validation found 33 of 53 elevations off by more than 10 m (up to 337 m) and non-continuous road lines (R-0002-validation.md) |
| 10 m (1/3 arc-second) DEM only | Too coarse for road cuts and switchbacks in Lee Vining Canyon; kept as fallback and for far terrain |
| Commit raw DEM to git LFS | 3.5 GB of reproducible public data; scripts plus checksums are enough |

## Consequences
- P3 (car in Unreal) and its fun gate move later; T-0201 waits.
- The terrain-at-scale risk is retired early instead of late.
- Disk: D: had 221 GB free on 2026-10-17; raw plus derived terrain is expected well under 20 GB.
- Reading GeoTIFF needs `rasterio` (bundles GDAL) in the dev Python; the download (3.5 GB) and the install need the director's approval before they run.

## Verification
Each task's acceptance criteria; the grey-box gate is judged by the director.

## Sources
USGS TNM Access API results (2026-10-17). USGS 3DEP point-elevation service (`epqs.nationalmap.gov`), used for the landmark check. `docs/roadmap.md`, `docs/build_strategy.md`, `docs/MASTER_BLUEPRINT.md` section 4.
