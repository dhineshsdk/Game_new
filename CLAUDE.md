# CLAUDE.md — Codex Repository Operating Laws & Agent Entrypoint

**Project Codename:** CODEX (Public title TBD prior to release)
**Engine:** Unreal Engine 5.8.3 (Strict C++ Core; pinned by `docs/adr/0003-pin-unreal-engine-5-8.md`)
**DCC & GIS Pipeline:** Headless Blender (Python / OpenUSD) + USGS 3DEP / Copernicus DEM + OSM
**Philosophy:** Grounded Realism, Mechanical Honesty, Real-Time Distance, Bittersweet Warmth

---

## 1. IRONCLAD ENGINEERING LAWS (NO "AI PROTOTYPE SLOP")

1. **Zero Blueprint Spaghetti:** All simulation math, vehicle dynamics, state serialization, camera exposure math, and economy logic MUST live in compiled C++ modules. Unreal Blueprints (`UBlueprint`) are permitted ONLY as thin data-only asset wrappers (`UDataAsset` / `UVehicleDefinition`) or final visual/audio binding layers.
2. **ChaosVehicles Plugin is BANNED:** Never include or reference Unreal's stock `ChaosVehicles` plugin. All vehicle dynamics run through our custom `CodexVehicleDynamics` module using `Chaos::TSimCallbackObject` at a fixed 120 Hz async physics substep (`bTickPhysicsAsync=True`, `AsyncFixedTimeStepSize=0.008333`).
3. **Physics Thread Safety (`_Internal` vs `_External`):**
   - Game Thread (GT) ONLY: `UObject`, `AActor`, `UPrimitiveComponent`, and `Chaos::FRigidBodyHandle_External`.
   - Physics Thread (PT) ONLY: Inside `OnPreSimulate_Internal()`, access rigid bodies exclusively via `FPhysicsActorHandle->GetPhysicsThreadAPI()` returning `Chaos::FRigidBodyHandle_Internal*`.
   - Unit Conversion: All internal physics calculations in `CodexVehicleDynamics` use SI units (`m`, `kg`, `s`, `N`, `rad`). Multiply forces by `100.0f` and torques by `10000.0f` at the exact point of injection into Chaos (`kg*cm/s^2`).
4. **Strict Module Dependency DAG:** Downward-only dependencies enforced in `.Build.cs`:
   - `CodexCore` (Math, Units, Fixed-Point, Logging — ZERO engine/Chaos dependencies)
   - `CodexVehicleDynamics` (Depends ONLY on `CodexCore` — Pure C++ Pacejka MF94, suspension, drivetrain, inertia, thermal math; 100% testable in headless C++ without booting UE5 Editor)
   - `CodexVehicles` (Depends on `CodexVehicleDynamics`, `Chaos`, `Engine` — `TSimCallbackObject`, double-buffer marshaling, pawn/chassis actors)
   - `CodexWorld` | `CodexTraffic` | `CodexPhoto` | `CodexAudio` (Mid-tier subsystems)
   - `CodexGameplay` | `CodexUI` (Top-level campaign, co-op, border/visa, barn-find, and UI systems)
   - `CodexEditor` | `CodexTests` (Headless commandlets, Python/USD bridges, golden-diff test runners)
5. **Single Writer & Atomic Task Rule:** Agents never "freestyle" features. Every code change must correspond to an active task file in `agents/tasks/T-XXXX.json` and pass headless compilation + unit tests before marking `docs/STATE.md` complete.
6. **Three-Strike Escalation Stop-Rule:** If an implementation agent (Claude Sonnet 5.5) fails a build, unit test, or physics tuning loop 3 consecutive times, STOP immediately. Log the failure in `docs/STATE.md` and escalate to **Claude Opus 5.5** (for math/concurrency redesign) or **Gemini 3.1 Pro** (for architectural/schema refactoring).

---

## 2. MULTI-AGENT ORCHESTRATION MATRIX

| Agent / Model | Environment | Primary Domain & Responsibilities |
| :--- | :--- | :--- |
| **Gemini Argon 4** | Gemini Web App | **Deep Research, GIS & Physics Formulation:** Extracts real-world mechanical constants, Pacejka tire curves, barometric/thermodynamic formulas, USGS/Copernicus DEM metadata, and regional cultural/signage briefs into `docs/research/`. |
| **Gemini 3.1 Pro** | Antigravity IDE | **Lead Systems Architect & Task Planner:** Maintains module interfaces, writes `UDataAsset` / JSON schemas, authors granular task specifications (`agents/tasks/T-XXXX.json`), and writes Headless Blender Python/OpenUSD automation scripts. |
| **Claude Opus 5.5** | Claude Code CLI | **Principal Physics & Concurrency Mathematician:** Designs and reviews the 120 Hz async physics solver, combined-slip Pacejka integration, lock-free GT/PT double-buffers, multi-body towing/winch constraints, and resolves 3-strike escalations. |
| **Claude Sonnet 5.5** | Claude Code CLI | **Lead Production C++ Engineer:** Implements production `.h`/`.cpp` files across all `Codex*` modules, runs UnrealBuildTool (UBT), executes headless unit tests (`CodexTests`), and fixes compiler/linker errors. |
| **Gemini 3.8 Flash / Claude Haiku 5.5** | Antigravity IDE / Claude Code CLI | **Fast Operations, Triage & Hygiene:** Parses verbose UBT build logs, updates `docs/STATE.md`, verifies `.Build.cs` include graphs, formats JSON/CSV telemetry tables, and runs git status/diff checks. |

---

## 3. REPOSITORY KNOWLEDGE MAP (READ ONLY WHAT YOUR TASK REQUIRES)

* `docs/MASTER_BLUEPRINT.md` — High concept, 6 pillars, narrative acts, multi-continental world plan, and the 3-Tier Execution Roadmap.
* `docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md` — Canonical math equations, 1985–1990 RWD sedan chassis constants, reconciled Pacejka MF94 tables, `TSimCallbackObject` C++ pattern, and Mono Basin LiDAR Z-scale math.
* `docs/CREATIVE_SYSTEMS_COMPENDIUM.md` — All 33 approved gameplay mechanics (Physics, Audio, Blender Factory, Single-Car Co-op, Motorcycles, Barn-Find Archaeology, Photography) and their forward-compatible C++ struct hooks.
* `docs/STATE.md` — Current phase, active task ID, build health, and agent handoff log.