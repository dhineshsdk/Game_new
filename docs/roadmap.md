# Roadmap

Status: **planning only. Nothing is installed or built.** Execution starts when the director says "go". Rule: a phase starts only when the previous phase's gate has passed. Every phase ends with something the director can drive, see or read. Sizes: S = days, M = weeks, L = a few months, XL = many months (iteration loops, not coding speed).

## Workstreams (run in parallel once their dependencies exist)
| Id | Workstream | Owner model | Notes |
|---|---|---|---|
| A | Vehicle dynamics (suspension, tyres, drivetrain, brakes, thermal, condition) | Opus design, Sonnet build | Core; reused by every vehicle |
| B | World pipeline (DEM, OSM, tiles, road kit, PCG, biomes) | Sonnet build, Opus architecture | Data-driven, tile-based, incremental |
| C | Game systems (fuel, wear, jobs, money, save, garage, repair) | Sonnet | All data-driven |
| D | Presentation (cockpit, audio, weather, sky, lighting, UI) | Sonnet, director tuning | Needs editor lane |
| E | Photography (camera model, journal, poster) | Sonnet | Depends on D |
| F | Campaign and story (prologue, acts, cargo tales) | Director + Opus | Content-heavy, data-driven |
| G | Multiplayer (soft MP, then convoys, companies) | Opus design | Structure from day one, build late |
| H | Ocean and ships (Stage A ferry, B, C) | Opus research | Stage C is its own project |
| I | Tooling, tests, orchestration, CI | Sonnet, Haiku | Continuous |

## Phases and gates

### P0. Foundation (not started)
Windows toolchain (Git + LFS, Python 3.11, Visual Studio 2022 C++, Blender, UE version pinned by ADR), git repo, empty modules compile, headless test runs, CLAUDE.md in effect, agent workflow live.
**Gate:** clean CLI build, empty-module tests pass headless, `/session-start` and `/session-end` work. Size: M.

### P1. Research spikes (before committing to designs)
Each spike ends in an ADR: UE version and API check (async physics, Chaos, PCG, World Partition, Water), 120 Hz fixed-substep proof, Chaos buoyancy feasibility, editor MCP bridge design, data licences (SRTM/Copernicus/USGS 3DEP/OSM ODbL). 
**Gate:** ADRs written and reviewed by Opus. Size: M.

### P2. Dynamics core, headless (A)
Tyre, suspension, drivetrain, brake, aero, thermal, condition model as pure C++; analytic unit tests; telemetry CSV.
**Gate:** tests pass against analytic references. Size: L.

### P3. Heirloom car in a test arena (A, D)
Async substep physics in Unreal, input (wheel/gamepad/keyboard), telemetry HUD, Physics Lab, first feel loops.
**Gate (director):** "I'd drive this for five minutes just because." Does not proceed until passed. Size: L.

### P4. Cockpit and sensory layer (D)
Head-motion model, FOV cues, parameterised engine/tyre/wind audio, condition-driven rattles.
**Gate:** the director feels the car's condition through sound and motion. Size: M.

### P5. Owens Valley corridor (B)
~20-30 km real-elevation corridor, road kit (spline to mesh, markings, shoulders, guardrails, signs), surfaces and grip, gravel loops, pass climb, fictional small town. Drive-through validation bot.
**Gate:** the director drives the corridor and likes the rhythm. Size: XL.

### P6. Time, weather, sky (D)
Day/night, night sky with real star positions, clear/overcast/rain, grip and visibility coupling, glass weather basics, headlight physics.
**Gate:** the desert night and a rain squall are memorable. Size: L.

### P7. Systems for the loop (C)
Fuel and basic thermals, tyre wear, gas station, save/load, garage, work pickup, jobs and money, parts and workshop repair (abstract) with a few light hands-on moments.
**Gate:** a complete repair-and-earn loop plays end to end. Size: L.

### P8. Photography (E)
Camera model with Assisted/Semi/Manual, journal, photo tagging, trip poster, recreate-the-photo prototype.
**Gate:** the director enjoys taking and keeping photos. Size: M-L.

### P9. Prologue and Career opening (F)
Inheritance, first repair, first scenic drive with the camera, first clue, hand-off to the money loop.
**Gate:** the prologue makes the director feel something. Size: L.

### P10. Vertical slice release candidate
Heirloom + work pickup, Owens Valley corridor, day/night, clear/rain, fuel, jobs, photography, prologue, simple traffic (a few lane-following AI cars), parameterised audio, packaged build, performance budget met.
**Gate:** 30-minute no-objective drive test + packaged build passes perf and soak tests. Size: M.

## After the slice (each its own project with a gate)
| Phase | Content | Difficulty |
|---|---|---|
| P11 Soft multiplayer | Shared clock/weather, visible players, no contact, session/lobby | Very High |
| P12 Second region | Ladakh/Manali-Leh (or Iceland), altitude engine loss, left-hand traffic, region as data asset | High |
| P13 Ferry interlude (Ocean Stage A) | RoRo ship, loading, crossing scene, port arrival | Medium |
| P14 Rigid trucks | Trailer-less trucks, cargo mass/CoG, business loop begins | High |
| P15 Articulated trucks | Kingpin, trailers, air brakes, load shifting | Very High |
| P16 Depot and drivers | AI drivers, fleet, contracts, company play | High |
| P17 More regions | Iceland, Alps, Western Ghats/coast, village-dense India | Very High each |
| P18 Barn finds and builds | Hidden condition, project vehicles, build sharing | Medium-High |
| P19 Motorcycles | Balance controller headless first, then in-engine | Very High |
| P20 Convoys and company multiplayer | Voice, CB, shared depot, co-op career | Very High |
| P21 Ocean Stage B/C | Explorable ship, then captained ships | Very High |
| P22 Deep repair simulation | Full mechanic sim (optional far future) | High |

## Cross-cutting rules
- Always a playable build at the end of each phase.
- One small verified step per session, docs updated before the session ends.
- Slice before scale: corridor before regions, cars before trucks and bikes, single-player before convoys.
- Stop-rule: a subsystem that exceeds 3 failed tuning/debugging loops escalates to Opus and is re-planned, not ground.
- Ideas outside the current phase go to `docs/ideas.md`, never started early.
