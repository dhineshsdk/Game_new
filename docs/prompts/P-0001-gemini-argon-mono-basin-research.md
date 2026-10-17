# P-0001: Mono Basin terrain and road research (for the Gemini app, Argon 4)

Paste everything below the line into the Gemini app. Save the answer as `docs/research/R-0001-mono-basin-terrain.md`.

---

I am building a driving simulator in Unreal Engine 5.8 around the Mono Basin near Lee Vining, California: SR 120 over Tioga Pass, Mono Lake, the Bodie gravel road, June Lake Loop. I need exact, citable data. Give dense tables, no prose padding. Mark anything you could not verify as UNVERIFIED and give source URLs.

1. USGS 3DEP lidar: which project names / dataset IDs cover a bounding box of about 35.7 km x 27 km around Lee Vining (give WGS84 lat/lon bounds), their acquisition years, point density, vertical datum (NAVD88), horizontal CRS (confirm EPSG:6340 / UTM 11N), tile sizes, total download size, and the exact public access path (The National Map API, AWS `usgs-lidar-public`, or the 1 m DEM product). Say whether a 1 m DEM exists for the whole box or only lidar point clouds.
2. How to convert the 1 m DEM to Unreal Landscape heightmaps: the correct Z scale for 16-bit PNG/RAW for an elevation range of about 1,900 m (Mono Lake) to 3,100 m (Tioga Pass), the Unreal landscape section sizes recommended for World Partition, and how to avoid vertical quantisation. Show the formula, not just a number.
3. OpenStreetMap: how to extract SR 120, US-395, the Bodie road (Route 270), June Lake Loop (SR 158) and surface tags (paved, gravel). Give the Overpass query and the licence obligations (ODbL attribution and share-alike) for use in a commercial game.
4. Real road facts for acceptance tests: Tioga Pass summit elevation, typical grades on the Lee Vining Canyon descent, speed limits, seasonal closure dates, and surface of the Bodie road (washboard gravel section length).
5. Data licensing: confirm USGS 3DEP data are public domain and give the citation text.

Output: one markdown document with sections 1 to 5 and a final "Open questions" list.
