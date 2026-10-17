# CODEX — Project State & Multi-Agent Handoff Ledger

**Last Updated:** 2026-10-17  
**Current Execution Tier:** TIER 1 (Mono Basin Vertical Slice)  
**Current Phase:** World lane first (ADR 0006). SOFT PAUSE (2026-10-17, at the director's request until their usage limit resets). Terrain heightmaps (T-0102) and roads (T-0104) are done; T-0103 (Unreal landscape import) is next. Physics lane paused after T-0008; T-0201 waits.  
**Build Status:** GREEN (UE 5.8.3, CodexEditor Win64 Development, 0 errors / 0 warnings; headless harness 31/31; automation smoke test passes; purity check passes)

---

## 1. Active Sprint Queue

| Task ID | Status | Assigned Agent | Module | Summary |
| :--- | :--- | :--- | :--- | :--- |
| `T-0001` | **COMPLETED** | Claude Sonnet 5.5 / Antigravity | `All` | Initialized UE 5.4/5.5 `.uproject`, Target.cs, `.Build.cs` & C++ module skeletons (`CodexCore`, `CodexVehicleDynamics`, `CodexVehicles`, `CodexTests`). |
| `T-0002` | **COMPLETED** | Claude Sonnet 5.5 | `All` | Added stub modules (`CodexWorld`, `CodexTraffic`, `CodexPhoto`, `CodexAudio`, `CodexGameplay`, `CodexUI`, `CodexEditor`). Verified by the T-0003 build on 2026-10-17. |
| `T-0003` | **COMPLETED** | Claude Opus 5.5 | `All` | UE 5.8.3 pinned. `CodexEditor` Win64 Development builds with 0 errors, 0 warnings (2026-10-17). `CodexVehicleAsyncCallback.h` now computes world CoM from `GetX()/GetR()/CenterOfMass()` because `FRigidBodyHandle_Internal` has no `GetP/GetQ` in 5.8. Automation tests `Codex.Physics.BaselineParameters` and `Codex.Physics.StateSerialization` pass. |
| `T-0004` | **IN_PROGRESS** | Claude Opus 5.5 | `CodexEditor` | Editor bridge. Verified: Blender runner, UE Python runner, Epic MCP server loaded natively in Claude Code, Blender Lab MCP add-on on port 9876. Pending: viewport capture and log read through MCP. |
| `T-0005` | **COMPLETED** | Claude Opus 5.5 | `CodexCore` | ADR 0005 accepted by the director. `CodexMath.h` (FVec2d/FVec3d/FQuatd/FMat3d, constants), `CodexExport.h`, glue only in `CodexCoreModule.cpp`, `FPSemantics = Precise` (confirmed in the module response file), `NoPCHs`. `tools/check_purity.ps1` passes. |
| `T-0006` | **COMPLETED** | Claude Opus 5.5 | headless tests | `tools/test_headless.ps1` builds and runs `Tests/Headless` with cl.exe (/W4 /WX /fp:precise), no UE. Build + run 1.7 s. Exits 1 on the deliberate self-test failure. |
| `T-0007` | **COMPLETED** | Claude Opus 5.5 | `CodexVehicleDynamics` | Params and state are plain `Codex::Dynamics` structs (double, trivially copyable). `FrontWeightDistributionPercent` renamed `FrontWeightFraction`. Former automation tests moved to the harness; `Codex.Physics.PureDynamicsSmoke` stays in CodexTests and passes. |
| `T-0101` | **COMPLETED** | Claude Opus 5.5 | `CodexVehicleDynamics` | Pacejka MF94 tyre model (`CodexTireModel.h/.cpp`). Independent review found 2 latent defects and 2 test gaps, all fixed (`agents/reviews/T-0101.review.md`): bounded combined slip for unsupported data, sanitised non-finite slips, distinct longitudinal golden data. 30,405-row golden CSV; 22/22 headless tests. |
| `T-0102` | **COMPLETED** | Claude Opus 5.5 | `tools/world` | Mono Basin terrain: 18 USGS 1 m DEM tiles (3.52 GB) downloaded with checksums; 266 16-bit heightmap tiles built (14113 x 19153 vertices at 2 m, 1945.39 to 3979.90 m, 3.17 cm per count, ScaleZ 405.178285, actor Z 2962.65 m). Verified against the source DEM (median 0.01 m) and the USGS point service (median 0.03 m). Derived files are in `data/derived/heightmap/` (gitignored, rebuild with `build_heightmap.py`). Not independently reviewed yet. |
| `T-0103` | **NEXT** | Claude Opus 5.5 | `CodexEditor` / Unreal | Import the 266 tiles into a World Partition Landscape via headless editor Python and check benchmark elevations in the editor. Needs the Unreal editor open (the MCP connection was down in the last session) or the headless runner `tools/unreal/run_ue_python.ps1`. |
| `T-0104` | **COMPLETED** | Claude Opus 5.5 | `tools/world` | Roads from OpenStreetMap by way ID (106/106 found), ordered into chains and clipped to the box (`data/derived/roads/roads_ordered.geojson`, gitignored; manifest committed). 15 synthetic tests. 9 reference/OSM tag disagreements recorded. `docs/ATTRIBUTION.md` written. Not independently reviewed yet. |
| `T-0008` | **COMPLETED** | Claude Opus 5.5 | `CodexVehicleDynamics` | Versioned binary snapshot (format v1, 556 bytes, little-endian, one shared field list) and FNV-1a 64 state hash. Default-state hash pinned and cross-checked with an independent Python encoder; every field affects the hash; padding never hashed; 72,000-step scripted run hashes identically twice. 31/31 headless tests, UBT 0 warnings. Not independently reviewed yet. |
| `T-0201` | **QUEUED** | Claude Sonnet 5.5 | `CodexVehicleDynamics` | Implement pure C++ 120 Hz solver (`FCodexVehicleSimulationState`) and headless unit tests (`CodexTests`) verifying static axle loads ($6759\text{ N}$ F / $5994\text{ N}$ R) and steady-state lateral G. |

---

## 2. Standard Task Contract Template (`agents/tasks/T-XXXX.json`)

Whenever **Gemini 3.1 Pro** creates a new task for **Claude Sonnet 5.5** or **Opus 5.5**, it must write a JSON file in `agents/tasks/` matching this exact schema:

```json
{
  "task_id": "T-0201",
  "title": "Implement Pure C++ Pacejka MF94 & 4-Corner Suspension Solver",
  "phase": "Phase 2 - Dynamics Core",
  "tier": "Tier 1",
  "owner_agent": "Claude Sonnet 5.5",
  "reviewer_agent": "Claude Opus 5.5",
  "target_module": "CodexVehicleDynamics",
  "reference_docs": [
    "CLAUDE.md",
    "docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md"
  ],
  "files_to_create_or_modify": [
    "Source/CodexVehicleDynamics/Public/CodexVehicleSimulationState.h",
    "Source/CodexVehicleDynamics/Public/CodexVehicleParams.h",
    "Source/CodexVehicleDynamics/Public/CodexTireModel.h",
    "Source/CodexVehicleDynamics/Private/CodexTireModel.cpp",
    "Source/CodexTests/Private/CodexDynamicsUnitTests.cpp"
  ],
  "forbidden_dependencies": [
    "Engine",
    "ChaosVehicles",
    "UnrealEd"
  ],
  "acceptance_criteria": [
    "Compiles cleanly with zero warnings under UnrealBuildTool.",
    "Static equilibrium test settles within 0.5mm of target ride height with Fz_front = 6759N (+/- 5N) and Fz_rear = 5994N (+/- 5N).",
    "Pacejka MF94 lateral force at Fz = 3200N on Dry Asphalt peaks at 3200N and matches load sensitivity D(Fz) with pDy2 = -0.144.",
    "Low-speed relaxation length differential equation remains numerically stable down to Vx = 0.0 m/s via max(|Vx|, 1.5 m/s) clamp."
  ],
  "strike_count": 0,
  "status": "READY"
}

---

## 3. Pause note (2026-10-17) - where work stopped and what comes next

**Stopped at a clean point.** Nothing is running (no downloads, no builds, Unreal editor closed). All work is committed and pushed to `claude/project-thread-qibo9c`.

**State of the world lane**
- Terrain: `data/raw/dem/` (18 tiles, gitignored) and `data/derived/heightmap/` (266 PNG tiles + `heightmap.json`, gitignored) exist on the dev machine. If they are missing: `python tools/world/fetch_dem.py --workers 15` then `python tools/world/build_heightmap.py` (about 40 min of download, 10 min to build).
- Roads: `data/derived/roads/roads_ordered.geojson` (gitignored) from `python tools/world/fetch_osm_roads.py`; manifest committed.
- Reference data: `data/reference` (base), `ref/` (second set, ideas only), `data/merged/` (merge). Elevations from any Gemini file must not be used; use the heightmap.
- Python tools that need packages (numpy, rasterio, Pillow) run without `-I`; `validate_reference.py`, `compare_reference.py` and `merge_reference.py` are stdlib-only and run with `python -I`.

**First action on resume: T-0103.** Write `agents/tasks/T-0103.json`, open the Unreal editor (or use the headless runner), and import `data/derived/heightmap/hm_x*_y*.png` into a World Partition Landscape using `heightmap.json` (ScaleZ 405.178285, actor Z 288 m rounded as given in the sidecar `z.actor_z_cm`, 2 m spacing = scale 200 cm, 112 x 152 components, row 0 = north). Decide the Unreal axis convention (X = east, Y = south is the plan) and verify with the benchmark places in `tools/world/verify_heightmap.py` read back in the editor. The Unreal MCP server was down at the end of the last session (the editor was closed for builds); reopen the editor to bring it back.

**After T-0103:** T-0105 (road splines from `roads_ordered.geojson` projected onto the landscape, densified along the line), then T-0106 to T-0108 (sun, moon and stars math; sky; weather). Open items for the director: the OpenStreetMap share-alike question in `docs/ATTRIBUTION.md`; independent review of T-0008, T-0102 and T-0104; use of `ref/` merge rows flagged `position_quality=approximate`.

**Not started:** T-0201 (120 Hz solver) and anything in Unreal.
