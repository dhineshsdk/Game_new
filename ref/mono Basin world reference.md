# R-0002: Mono Basin World Reference (UE 5.8 Build Pipeline Specification)

**Document ID:** `docs/research/R-0002-mono-basin-world-reference.md`  
**Replaces:** `P-0001`  
**Target Engine:** Unreal Engine 5.8  
**Coordinate Reference Systems:** WGS84 (`EPSG:4326` / `CRS84`), NAD83 / UTM Zone 11N (`EPSG:26911`), Vertical Datum `NAVD88` (via `GEOID18`).

---

## 1. World Box and Areas

### Boundary & Terrain Source Verification
* **Primary World Box (`[-119.27000, 37.89000, -118.96000, 38.21500]`):** Measures 27.2 km E–W by 36.1 km N–S (981.9 km²).
* **Coverage Audit:**
  * **South Tufa & Panum Crater:** Both **South Tufa (`37.94110° N, -119.02650° W`)** and **Panum Crater (`37.92850° N, -119.04450° W`)** lie comfortably **inside** the `37.89000° N` southern boundary (by 5.6 km and 4.2 km respectively).
  * **Bodie & SR 270:** Bodie State Historic Park (`38.21220° N, -119.01250° W`) and the entire length of SR 270 (paved and gravel sections) lie inside the box. Nudging the north edge to `38.22500° N` (+1.1 km) is optional if the northern crest of Bodie Bluff is desired as a backdrop.
* **USGS 3DEP 1 m DEM (`CA_SierraNevada_B22`) Confirmation:** Confirmed. Project `CA_SierraNevada_B22` (published 2025-05-15) covers the Mono Basin at 1 m resolution.
  * *Tile Index Correction:* Because `37.89000° N` at `-119.27000° W` converts to `4,196,178 m N` (below `4,200,000 m N`), the southern 3.8 km strip requires UTM grid row **`y419`** (`4,190,000–4,200,000 m N`), while row `y424` (`4,240,000+ m N`) is north of the box. Use the 15 tiles spanning **`x30..x32` and `y419..y423`**, supplemented by the USGS 1/3 arc-second (~10 m) Seamless DEM (`USGS_13_n38w120`, `USGS_13_n39w120`) for distant vistas.

### Elevation & Hydrological Reference Table

| Metric / Feature | Value (NAVD88) | Value (NGVD29 / Legacy) | Coordinates (WGS84) | Date / Reference Period | Source |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Mono Lake Surface Elevation** | **1,946.09 m** (6,384.8 ft) | 1,945.27 m (6,382.1 ft) | `38.00000, -119.05000` | 2026 gauge reading | Mono Lake Committee / LADWP & USGS `10287500` |
| **Minimum Elevation in Box** | **1,946.09 m** (6,384.8 ft) | 1,945.27 m | `38.01500, -119.06000` | Present Day (Mono Lake surface) | USGS 3DEP `CA_SierraNevada_B22` / USGS NHD |
| **Maximum Elevation in Box** | **3,979.77 m** (13,057.0 ft) | 3,978.55 m (13,053 ft) | `37.89993, -119.22109` | **Mount Dana Summit** | NGS Data Sheet PID `HR2697` / USGS 7.5' Quad |
| **Tioga Pass Highway Summit** | **3,031.24 m** (9,945.0 ft) | 3,030.0 m | `37.91075, -119.25792` | SR 120 at Yosemite East Entrance Gate | Caltrans State Highway Log D9 / USGS 3DEP |
| **Lee Vining Town Centre** | **2,066.85 m** (6,781.0 ft) | 2,065.6 m | `37.95758, -119.11920` | US-395 at 3rd Street / Mattly Ave | USGS 7.5' Lee Vining Quad / 3DEP |

### `data/reference/world_box.geojson`

```json
{
  "type": "FeatureCollection",
  "name": "mono_basin_world_box",
  "crs": {
    "type": "name",
    "properties": {
      "name": "urn:ogc:def:crs:OGC:1.3:CRS84"
    }
  },
  "features": [
    {
      "type": "Feature",
      "properties": {
        "name": "Mono Basin Primary World Box",
        "kind": "box",
        "source": "Project Spec P-0002 / EPSG:4326",
        "notes": "27.2 km E-W by 36.1 km N-S. Spans UTM 11N tiles x30..x32, y419..y423."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.27000, 37.89000],
          [-118.96000, 37.89000],
          [-118.96000, 38.21500],
          [-119.27000, 38.21500],
          [-119.27000, 37.89000]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Lee Vining Town",
        "kind": "area",
        "source": "OpenStreetMap / USGS GNIS ID 1658952",
        "notes": "Main town grid, utility yard, school, high school, and airstrip bench."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.13050, 37.94800],
          [-119.10600, 37.94800],
          [-119.10600, 37.96850],
          [-119.13050, 37.96850],
          [-119.13050, 37.94800]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Mono Lake Water Surface (West & Central Sector)",
        "kind": "water",
        "source": "USGS NHDWaterbody / OSM Relation 1219705 clipped to -118.96000 E",
        "notes": "Represented at 1946.09 m NAVD88 (6384.8 ft). Clipped at eastern world box edge -118.96000."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.14120, 37.97740],
          [-119.11580, 37.96210],
          [-119.09240, 37.95180],
          [-119.05100, 37.94220],
          [-119.01850, 37.94110],
          [-118.98100, 37.94950],
          [-118.96000, 37.95820],
          [-118.96000, 38.05980],
          [-119.00520, 38.07050],
          [-119.06210, 38.06820],
          [-119.11280, 38.04650],
          [-119.13550, 38.02100],
          [-119.14380, 37.99650],
          [-119.14120, 37.97740]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Lee Vining Canyon",
        "kind": "area",
        "source": "USGS 7.5' Lee Vining & Tioga Pass Quads",
        "notes": "Glacial U-shaped canyon floor and granite walls along SR 120 from US-395 junction to Ellery Lake."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.22800, 37.91500],
          [-119.11800, 37.93500],
          [-119.11800, 37.96200],
          [-119.22800, 37.95200],
          [-119.22800, 37.91500]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Tioga Pass & Dana Plateau Area",
        "kind": "area",
        "source": "USGS 7.5' Tioga Pass & Mount Dana Quads",
        "notes": "Subalpine basin encompassing Ellery Lake, Tioga Lake, Saddlebag junction, Tioga Pass Entrance, and Mount Dana."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.27000, 37.89000],
          [-119.20500, 37.89000],
          [-119.20500, 37.96500],
          [-119.27000, 37.96500],
          [-119.27000, 37.89000]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Lundy Canyon",
        "kind": "area",
        "source": "USGS 7.5' Lundy Quad",
        "notes": "Steep fault-scarp canyon containing Mill Creek, Lundy Lake reservoir, beaver ponds, and Lundy Campground."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.26500, 38.01500],
          [-119.14500, 38.01500],
          [-119.14500, 38.05200],
          [-119.26500, 38.05200],
          [-119.26500, 38.01500]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Conway Summit",
        "kind": "area",
        "source": "USGS 7.5' Conway Summit Quad / Caltrans District 9",
        "notes": "US-395 mountain pass (2,482 m / 8,143 ft) separating Mono Basin from Bridgeport Valley, including Conway Ranch."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.21500, 38.05500],
          [-119.15500, 38.05500],
          [-119.15500, 38.11000],
          [-119.21500, 38.11000],
          [-119.21500, 38.05500]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "Bodie State Historic Park & Mining District",
        "kind": "area",
        "source": "USGS 7.5' Bodie Quad / California State Parks",
        "notes": "High-desert ghost town basin at 2,554 m (8,379 ft), Standard Mill, and Bodie Bluff."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.03500, 38.19500],
          [-118.99500, 38.19500],
          [-118.99500, 38.21500],
          [-119.03500, 38.21500],
          [-119.03500, 38.19500]
        ]]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "name": "June Lake Loop Extension (SR 158)",
        "kind": "extension",
        "source": "USGS 7.5' June Lake Quad / Caltrans",
        "notes": "Future southern expansion bounding SR 158, June Lake, Gull Lake, Silver Lake, Grant Lake, and Mono Craters."
      },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-119.16000, 37.73000],
          [-118.98000, 37.73000],
          [-118.98000, 37.89000],
          [-119.16000, 37.89000],
          [-119.16000, 37.73000]
        ]]
      }
    }
  ]
}