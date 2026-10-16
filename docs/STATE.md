# CODEX — Project State & Multi-Agent Handoff Ledger

**Last Updated:** 2026-10-15  
**Current Execution Tier:** TIER 1 (Mono Basin Vertical Slice)  
**Current Phase:** Phase 0 (Foundation & Module Skeleton) -> Moving to Phase 1/2  
**Build Status:** INITIALIZED (UE5 Project Skeleton, Modules, Git & LFS Ready)

---

## 1. Active Sprint Queue

| Task ID | Status | Assigned Agent | Module | Summary |
| :--- | :--- | :--- | :--- | :--- |
| `T-0001` | **COMPLETED** | Claude Sonnet 5.5 / Antigravity | `All` | Initialized UE 5.4/5.5 `.uproject`, Target.cs, `.Build.cs` & C++ module skeletons (`CodexCore`, `CodexVehicleDynamics`, `CodexVehicles`, `CodexTests`). |
| `T-0002` | **PENDING_BUILD_VERIFICATION** | Claude Sonnet 5.5 | `All` | Added stub modules (`CodexWorld`, `CodexTraffic`, `CodexPhoto`, `CodexAudio`, `CodexGameplay`, `CodexUI`, `CodexEditor`) so `Codex.uproject` and both Target files resolve. Files written but not compiled: no Visual Studio, no installed engine. T-0001 is also unbuilt. |
| `T-0101` | **READY** | Claude Sonnet 5.5 | `CodexVehicleDynamics` | Implement pure C++ Pacejka MF94 tire model (`CodexTireModel.h/.cpp`) with combined slip and relaxation length. |
| `T-0102` | **QUEUED** | Gemini 3.1 Pro | `CodexEditor` | Write Python script to fetch USGS 3DEP 1m DEM for Mono Basin (`EPSG:6340`) and convert to 16-bit PNG heightmap (`Scale.Z = 400.396735`). |
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