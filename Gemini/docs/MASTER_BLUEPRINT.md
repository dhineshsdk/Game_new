# CODEX — Canonical Master Architecture, World & Narrative Blueprint

**Version:** 2.0.0 (Unified Compendium Edition)  
**Target Engine:** Unreal Engine 5.4 / 5.5 (C++) + Headless Blender (Python / OpenUSD)  
**North Star:** *The most believable long drive ever made: you trust the car, the road feels real, and the journey stays with you.*

---

## 1. Core Pillars & Reference Blend

**Reference Blend:** *Microsoft Flight Simulator* (true geographic scale, GIS/LiDAR fidelity, real-time atmosphere) + *Forza Horizon* (pure open-world driving joy, car culture, barn finds) + *Euro Truck Simulator* (meditative long-haul weight, real-time distance, cargo livelihood).

### The Six Pillars
1. **Presence:** Hyper-tactile cabin ecology—cooling exhaust manifold ticks, cold-start carburetor choke flutter, unsecured dash clutter sliding under lateral G-forces, misting glass, and rain drumming on steel roofs.
2. **Mechanical Honesty:** 100% condition-driven physics. Damper fade, tire compound aging, dynamic trunk cargo inertia, fuel sloshing, barometric altitude power loss, and brake fluid boiling are solved mathematically at 120 Hz—never faked with scripted timers.
3. **Distance Has Weight (Real-Time Journey):** A "1,000 km day" takes a real day. No default fast-travel teleports. Long legs require planning fuel range, weather fronts, mountain pass closures, visas, and driver fatigue.
4. **Living World Ecology:** Multi-continental biomes built from real DEM/LiDAR and OSM data. Roads retain memory (persistent skidmarks, campfire rings, surface expansion joints, washboard gravel, black ice in canyon shadows).
5. **Physical Photography:** A true manual/semi-manual/assisted `UCineCameraComponent` model (focal length, aperture, shutter speed, ISO, 24/36-exposure 35mm film rolls, darkroom development, and 1974 vs. 2026 split-time framing).
6. **Shared Experience (Co-Op & Quiet Multiplayer):** Supports both **Single-Car Multi-Crew Road Trips** (seat-swapping, passenger DJ/navigator, push-starting) and **Two-Vehicle Chase/Support Convoy Co-Op**, paired with non-griefing soft-ghost public roads, 24/7 roadside diners/dhabas, and spatial CB radio.

---

## 2. Narrative Spine & Resolved World Decisions

### 2.1 The Premise: "The Unsent Cartography"
In the 1970s, the protagonist's grandfather—an immigrant trucker originally from Southern India (Tamil Nadu / Western Ghats foothills)—attempted an epic overland journey with his closest companion toward the high Himalayan passes of Spiti and Ladakh before an unforeseen crisis forced him to abandon the climb and emigrate to California. Decades later, after his passing, his grandson (a photographer) inherits his small rural homestead in the **Mono Basin (Lee Vining, CA)**, containing:
1. **The Work Pickup (Late-1980s Utility Truck):** His grandfather's working livelihood—rattly, weathered, but mechanically running from Day 1.
2. **The Heirloom Project Car (1985–1990 RWD Sedan/Coupe, E30/W201/R31 class):** His grandfather's unfinished dream sitting in the barn—non-running, rusted, missing parts, with labeled cardboard parts boxes waiting on the shelves.
3. **The Glovebox Tin:** A 1974 road atlas, a pencil odometer maintenance logbook, cassette mixtapes, a mechanical 35mm film camera, and unsent letters.

### 2.2 The Four-Act Campaign Arc
* **Prologue & Act 1 — The Sierra Frontier (Mono Basin -> Owens Valley -> Death Valley, CA):**
  - Open with a 45-minute driving-first prologue before economy loops begin.
  - Use the running pickup for local freight/courier jobs and photography commissions to fund parts for the heirloom RWD sedan.
  - Test the car across **Mono Lake (1,945m)**, the steep **Tioga Pass climb (SR 120 to 3,031m)**, **Bodie Ghost Town's** washboard gravel, and **June Lake**, later extending south down **US-395** into **Owens Valley** and **Death Valley**.
  - Secure the **Carnet de Passages en Douane (CPD)** temporary export bond to legally ship the Left-Hand-Drive heirloom car overseas.
* **Act 2 — The Maritime Interlude (Ro-Ro Cargo Ship Crossing):**
  - Drive onto a Roll-on/Roll-off vessel crossing the Pacific/Indian Ocean (Ocean Stage A).
  - Contemplative deck walkabouts, developing 35mm film rolls in the ship's darkroom, inspecting the tied-down car in the hold, and studying grandfather's route notes.
* **Act 3 — The Subcontinental Arrival & Southern Roots (Coastal Port -> Western Ghats -> Plains):**
  - Disembark at a Southern Indian coastal port under a 6-month **Carnet de Passages** countdown.
  - Immediate sensory & mechanical shift: driving a **Left-Hand-Drive 1980s sedan in Right-Hand-Drive dense mixed traffic** (where peeking out to overtake a slow decorated lorry on a narrow road requires extreme care or a co-op passenger spotter).
  - Traverse humid coastal corridors and **Western Ghats monsoon hairpins** to visit the grandfather's ancestral village home—where relatives have kept his room and letters intact—before expanding into commercial lorries, dhabas, and motorcycles.
* **Act 4 — The Trans-Himalayan Ascent (Grand Trunk Road -> Kullu -> Manali-Leh / Spiti / Zoji La):**
  - The final overland climb (4,500m–5,300m) through sheer rock shelves, glacial meltwater crossings, and 35–45% naturally aspirated altitude power loss.
  - Deliver the car's final odometer mile and capture the grandfather's last unfinished photograph.

*(Note: **Iceland Ring Road** and **European Alps / Stelvio** operate as "Postcard Detours" from the grandfather's merchant-sailor/trucker friends and as flagship Free Mode regions).*

---

## 3. Global Biomes, Borders, Visas & Toll Logistics

### 3.1 Five Global Biome Archetypes
1. **Infinite Silent Highways:** US-395, Great Basin, Death Valley, Stuart Hwy. (Expansion joint thrum, carburetor vapor lock, asphalt heat mirages, 3 AM red radio towers).
2. **Deep Rural Byways:** Scottish 1-tracks, Swedish Taiga, Japanese Touge, June Lake. (Single-lane passing places, wet autumn leaves, dappled canopy light, deer eyeshine).
3. **Dangerous Mountain Shelves:** Zoji La, Spiti, Gata Loops, Stelvio. (1,000m unguarded vertical drops, waterfalls on cab roof, backing up to cliff cutouts for oncoming lorries, severe altitude power loss).
4. **Nordic & Glacial Frontier:** Iceland Ring Road & F-roads. (Whiteout blizzards, 2m orange snow-pole navigation, studded tires/chains, unbridged glacial river fords, seasonal ice roads).
5. **Monsoon Gorges & Subtropical Corridors:** Western Ghats, Kerala Backwaters, Kullu Basin. (Blinding tropical downpours overwhelming wipers, axle-deep mud ruts, intake snorkel water fording).

### 3.2 Physical Borders, Visas & Tolls
* **Glovebox Documents:** Physical flippable Passport, International Driving Permit, Vehicle Title (Pink Slip), Insurance Card, and **Carnet de Passages en Douane (CPD)**.
* **Border Checkpoints:** Physical queue behind idling lorries, rolling down the window to hand documents to officers, secondary inspection bays (VIN matching, agricultural quarantine checks, optional high-risk black-market vintage parts smuggling).
* **Three Toll Systems:** Manual cash/coin booths with paper receipts; windshield RFID transponders (**FASTag / Telepass / E-ZPass**) with high-speed gantry *BEEP* deduction; regional alpine windshield vignette stickers.

---

## 4. The 3-Tier Staged Execution Roadmap (Anti-Burnout Architecture)

To guarantee completion without scope collapse, development is strictly gated into three concentric tiers. **We design the C++ data hooks for all 3 tiers upfront in Phase 1–2, but build content strictly in Tier order.**

### TIER 1: The Mono Basin Vertical Slice (Phases 0–10 | Target: First Playable Game)
* **World Scope:** 35.7 km × 26.95 km Mono Basin LiDAR corridor (Lee Vining, SR 120 Tioga Pass, Mono Lake, Bodie gravel road). 95% nature/procedural terrain, 5% modular structures (1 barn workshop, 1 gas station, 1 diner).
* **Vehicles:** 1985–1990 Heirloom RWD Sedan (rusty to restored condition states) + Late-1980s Work Pickup.
* **Core Systems:** 120 Hz Chaos C++ solver, Pacejka MF94 combined-slip, mechanical degradation matrix, trunk cargo inertia, fuel slosh, altitude loss, brake fade, 35mm physical camera, odometer log clues, cassette deck + AM night skip radio, 1 full barn-find discovery & rebuild loop.
* **Phase Gate 0–10 Progression:**
  - *P0:* Toolchain, repo, empty C++ modules, headless test runner.
  - *P1:* ADRs, LiDAR ingestion script, reconciled Pacejka & chassis constants.
  - *P2:* Pure C++ `CodexVehicleDynamics` solver passing headless skidpad & load-transfer golden diffs.
  - *P3:* Heirloom in UE5 Physics Lab (**Fun Gate: "I'd drive this for 5 minutes just because"**).
  - *P4:* Cockpit presence, MetaSound engine/wheelbase/rattle audio, dash kinematics.
  - *P5:* Mono Basin 30 km spline + LiDAR terrain corridor with bot drive-through validation.
  - *P6:* True astronomical sky, weather fronts, rain grip transitions, headlight optics.
  - *P7:* Pickup cargo jobs, fuel budgeting, part installation, deterministic save/load.
  - *P8:* `CodexPhoto` 35mm camera, exposure evaluator, darkroom development, scrapbook.
  - *P9:* Driving-first prologue narrative & grandfather's odometer log trail.
  - *P10:* Vertical Slice Release Candidate & performance lock.

### TIER 2: Continental Expansion & Co-Op (Act 1 Extension + Act 2 Ferry + First International Biome)
* **World Scope:** Extend US-395 south through Owens Valley & Death Valley (200+ km); add Stage A Ro-Ro Ferry Crossing; open High-Altitude Ladakh/Spiti or Iceland Ring Road (low-density traffic biomes first).
* **Multiplayer & Co-Op:** 2-Vehicle Co-Op (Heirloom + Support Chase Pickup with winch, tow rope, and mobile workshop), Spatial CB Radio (Ch 1–40 + Ch 9 Samaritan Distress Flare), Roadside Sanctuary Diners/Dhabas with **Traveler's Exchange Box**, and **Single-Car Co-Op Stage 1** (Passenger navigator/DJ/photographer).
* **Systems:** Border checkpoints, Passport/Carnet/Visa rules, RFID/cash tolls, Binocular barn-find scouting, 3-Path Barn-Find Builds (Concours / Restomod / Safari), Car Camping & Rooftop Tents.

### TIER 3: Full Living World, Motorcycles & Heavy Fleet (Full Multi-Region Vision)
* **World Scope:** Southern India Coastal Port + Western Ghats Monsoon Corridor (dense mixed traffic AI), European Alps, full trans-continental highways.
* **New Vehicle Physics Classes:**
  - **Motorcycles:** Dedicated two-wheel counter-steering & rider-lean physics solver, helmet visor rain/dust wipe, kick-starting ritual, pillion passenger co-op.
  - **Heavy Commercial Lorries & Semis:** Multi-axle rigid trucks and articulated trailers, **Desi Truck Art Studio**, fleet depot management.
* **Advanced Co-Op:** Full 4-player single-cabin physical seat-swapping, co-op push-starting, rolling tailgate photo rigs, title-transfer meetups.