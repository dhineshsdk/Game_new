# P-0002: Mono Basin world reference (for Gemini 4 Argon, 3.1 Pro or 3.8 Flash)

Replaces and extends P-0001. Paste everything below the line into Gemini. Save the full answer as `docs/research/R-0002-mono-basin-world-reference.md`, and save each fenced data block as its own file under `data/reference/` using the file name given in its heading (for example `data/reference/roads.geojson`).

Suggested split if one answer is too long: Argon 4 does sections 1, 2, 3 and 6 (data and facts); 3.1 Pro does sections 4, 5 and 7 (look, layout and fiction mapping); 3.8 Flash checks the output against section 8.

---

You are the research and reference lead for a realistic driving game built in Unreal Engine 5.8. The first playable area is the **Mono Basin, California**, around Lee Vining. I need a precise world reference that my build pipeline can read directly. Accuracy matters more than length. Use Google Maps, Street View and live imagery to **look**, but take all geometry and numbers from open sources (see the licence rule below).

## The game, in brief (so your choices fit)
- A grounded, bittersweet road journey, present day (2026). A grandson inherits his late grandfather's rural homestead near Lee Vining, a rusty 1985-1990 rear-wheel-drive sedan in the barn and a late-1980s work pickup. The tone is quiet and warm, with unglamorous beauty: dusty pull-outs, a diner at dawn, wind on Mono Lake.
- Real landscape, **fictional names** for towns, businesses and buildings, and **no real brands** (cars, fuel, shops). Real road numbers and natural features (Mono Lake, Tioga Pass, Mount Dana, Bodie) keep their real names.
- Driving is the core. Roads must be correct in shape, grade, width and surface, because the car physics is realistic.

## Hard rules
1. **Licence rule.** Do not trace or copy geometry, coordinates or text from Google Maps or Street View; their terms forbid reuse in a product. Geometry must come from OpenStreetMap (ODbL), USGS (public domain) or Copernicus. Use Google imagery only to describe what things look like, in your own words.
2. **Coordinates.** Give every point in WGS84 decimal degrees (lat, lon, at least 5 decimals). Where asked, also give NAD83 / UTM zone 11N easting and northing in metres (EPSG:26911). Elevations in metres above NAVD88.
3. **Cite.** Give a source URL or dataset ID for every number. Mark anything unconfirmed as `UNVERIFIED`. Never invent a value to fill a gap; leave it null and list it under Open questions.
4. **Formats.** Data blocks must be valid GeoJSON (RFC 7946) or CSV with a header row, exactly in the schemas below, inside fenced code blocks, with no comments inside the data.

## World box
Use this box unless you find a strong reason to change it (say why):
- West -119.27, East -118.96, South 37.89, North 38.215 (about 27 km east-west by 36 km north-south).
- It must contain: Tioga Pass and the SR 120 climb from Lee Vining (Lee Vining Canyon, Ellery and Tioga lakes), Mount Dana, Lee Vining town, US-395 through the basin including Conway Summit, the west and north shores of Mono Lake (South Tufa may be outside; say whether it is), Lundy Canyon, Mono Basin Scenic Area Visitor Center, and Bodie with SR 270 (Bodie Road) including its gravel section.
- Note for context: June Lake Loop (SR 158, around 37.78 N) is outside and will be a later extension. Give its bounds anyway in section 1.

Terrain source already identified by us (please confirm or correct): USGS 3DEP 1 m DEM, project `CA_SierraNevada_B22` (published 2025-05-15), 15 tiles UTM 11N x30..x32 y420..y424, about 3.5 GB; fallback USGS 1/3 arc-second DEM.

## Sections to deliver

### 1. World box and areas — `world_box.geojson`
A FeatureCollection with: the world box polygon; a polygon for each named area (Lee Vining town, Mono Lake water surface at its current level, Lee Vining Canyon, Tioga Pass area, Lundy Canyon, Conway Summit, Bodie); and the June Lake Loop bounds as a separate feature. Properties: `name`, `kind` (box | area | water | extension), `source`, `notes`.
Also state in a table: the current Mono Lake surface elevation (with date and source), and the elevation range inside the box (minimum and maximum with locations).

### 2. Roads — `roads.geojson` and `road_facts.csv`
GeoJSON LineStrings for SR 120 (Lee Vining to Tioga Pass and the Yosemite entrance station), US-395 (the full length inside the box), SR 167, SR 270 (Bodie Road, paved and gravel parts as separate features), Lundy Lake Road, Test Station Road, Picnic Grounds Road, Poole Power Plant Road and the main streets of Lee Vining. Take geometry from OpenStreetMap and include the OSM way IDs.
Properties per feature: `name`, `ref`, `osm_way_ids` (array), `surface` (asphalt | gravel | dirt | compacted), `lanes`, `width_m`, `shoulder_m`, `speed_limit_mph`, `guardrail` (none | partial | continuous), `centre_line` (none | single_yellow | double_yellow | passing), `seasonal_closure` (text or null), `source`.
`road_facts.csv` columns: `road,segment,from_lat,from_lon,to_lat,to_lon,length_km,elev_start_m,elev_end_m,avg_grade_pct,max_grade_pct,notable_features,source`. Include the Lee Vining Canyon climb in segments of about 2 km, the Bodie Road gravel section (length, washboard notes) and Conway Summit grades. Add Tioga Pass typical opening and closing dates for the last 10 years.

### 3. Landmarks and points of interest — `landmarks.csv`
Columns: `id,name_real,kind,lat,lon,utm_e,utm_n,elev_m,visible_from_road,why_it_matters,source`.
At least: Tioga Pass summit, Mount Dana summit, Ellery Lake, Tioga Lake, Saddlebag Lake turnoff, Lee Vining Canyon vista points, the Lee Vining town centre, the gas station and diner sites in town, Mono Basin Visitor Center, Panum Crater, Negit and Paoha islands, tufa areas on the north and west shore, Conway Summit vista, Lundy Lake, Bodie State Historic Park entrance, the Bodie Road gravel start, county road junctions, and 10 to 20 places that are good for photographs at golden hour (say which direction and season).
Add 3 to 5 candidate sites for the grandfather's rural homestead with barn: plausible private-land locations within 10 km of Lee Vining, on a dirt or gravel spur, with a view of the lake or the Sierra. These become fictional; describe why each fits.

### 4. Look and feel — prose plus `biomes.csv`
Write a visual brief for each area in section 1 and for each road segment, in your own words from imagery: rock types and colours, soil, vegetation, roadside details (signs, posts, culverts, cattle guards, fences, snow poles, pull-outs, rockfall netting), how the light falls at sunrise and sunset, and how it changes across seasons.
`biomes.csv` columns: `band_id,elev_min_m,elev_max_m,aspect_notes,dominant_plants,secondary_plants,ground_cover,rock_types,soil_colour_hex,vegetation_density_0_1,snow_months,source`. Cover from the lake shore to the alpine summits (for example: lake shore and tufa, sagebrush flats, pinyon-juniper, Jeffrey pine, aspen in canyons, lodgepole and whitebark pine, alpine fell-field).

### 5. Settlements and buildings — `settlements.csv`
For Lee Vining and any ranches or sites inside the box: building count by type, typical construction (materials, roof types, colours, age), street layout, signage style, lighting at night. Columns: `site,building_kind,count_estimate,typical_size_m,materials,roof,colour_notes,era,notes,source`.

### 6. Climate and sky — `climate_monthly.csv`
Monthly normals for Lee Vining and for Tioga Pass (or the nearest stations; name them): mean high and low temperature, precipitation, snowfall, snow depth, wind speed and prevailing direction, cloud cover, days with thunderstorms, sunrise and sunset on the 15th. Columns: `station,month,t_max_c,t_min_c,precip_mm,snow_cm,snow_depth_cm,wind_ms,wind_dir_deg,cloud_cover_pct,thunder_days,sunrise_local,sunset_local,source`.
Also describe the typical weather events we should model: summer afternoon thunderstorms on the crest, Mono Lake wind and dust, winter storms and road closures, inversions and lake fog, night-sky darkness (Bortle class) and the Milky Way season.

### 7. Fiction mapping — `fiction_map.csv`
Propose fictional names that fit the place and era for: the town replacing Lee Vining, its gas station, diner, motel, general store, the grandfather's homestead and any business we show. Columns: `real_place_or_role,fictional_name,reason,notes`. Keep them understated and believable; avoid puns and any real business names.

### 8. Checks and open questions
- A checklist confirming each data block parses and every feature lies inside the world box (except the June Lake extension).
- Attribution text we must ship: for OpenStreetMap (ODbL, with the share-alike note for a game) and USGS 3DEP.
- Open questions: everything marked `UNVERIFIED` or null, and anything you think we have wrong.

Output: one markdown document with sections 1 to 8 in this order, each data block in a fenced block headed with its file name.
