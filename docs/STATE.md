# CODEX — Project State & Multi-Agent Handoff Ledger

**Last Updated:** 2026-10-17  
**Current Execution Tier:** TIER 1 (Mono Basin Vertical Slice)  
**Current Phase:** Phase 0 (Foundation & Module Skeleton) complete -> ADR 0005 decision, then Phase 2  
**Build Status:** GREEN (UE 5.8.3, CodexEditor Win64 Development, 0 errors / 0 warnings; 2/2 Codex automation tests pass)

---

## 1. Active Sprint Queue

| Task ID | Status | Assigned Agent | Module | Summary |
| :--- | :--- | :--- | :--- | :--- |
| `T-0001` | **COMPLETED** | Claude Sonnet 5.5 / Antigravity | `All` | Initialized UE 5.4/5.5 `.uproject`, Target.cs, `.Build.cs` & C++ module skeletons (`CodexCore`, `CodexVehicleDynamics`, `CodexVehicles`, `CodexTests`). |
| `T-0002` | **COMPLETED** | Claude Sonnet 5.5 | `All` | Added stub modules (`CodexWorld`, `CodexTraffic`, `CodexPhoto`, `CodexAudio`, `CodexGameplay`, `CodexUI`, `CodexEditor`). Verified by the T-0003 build on 2026-10-17. |
| `T-0003` | **COMPLETED** | Claude Opus 5.5 | `All` | UE 5.8.3 pinned. `CodexEditor` Win64 Development builds with 0 errors, 0 warnings (2026-10-17). `CodexVehicleAsyncCallback.h` now computes world CoM from `GetX()/GetR()/CenterOfMass()` because `FRigidBodyHandle_Internal` has no `GetP/GetQ` in 5.8. Automation tests `Codex.Physics.BaselineParameters` and `Codex.Physics.StateSerialization` pass. |
| `T-0004` | **IN_PROGRESS** | Claude Opus 5.5 | `CodexEditor` | Editor bridge. Verified: Blender runner, UE Python runner, Epic MCP server loaded natively in Claude Code, Blender Lab MCP add-on on port 9876. Pending: viewport capture and log read through MCP. |
| `T-0005`..`T-0007` | **PROPOSED** | Claude Sonnet 5.5 | `CodexCore`, `CodexVehicleDynamics` | Purity, headless harness and dynamics migration per ADR 0005 (status: proposed, awaiting director approval). Task files not yet written. |
| `T-0101` | **QUEUED** | Claude Sonnet 5.5 | `CodexVehicleDynamics` | Pure C++ Pacejka MF94 tyre model. Blocked on T-0005..T-0007 (see `agents/tasks/T-0101.json`). |
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