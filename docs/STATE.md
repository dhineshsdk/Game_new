# CODEX — Project State & Multi-Agent Handoff Ledger

**Last Updated:** 2026-10-17  
**Current Execution Tier:** TIER 1 (Mono Basin Vertical Slice)  
**Current Phase:** Phase 2 (Dynamics Core): pure core and tyre model done; next T-0008 (state snapshot and hash), then T-0201 (120 Hz solver)  
**Build Status:** GREEN (UE 5.8.3, CodexEditor Win64 Development, 0 errors / 0 warnings; headless harness 18/18; automation smoke test passes; purity check passes)

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
| `T-0102` | **WAITING ON RESEARCH** | Gemini 3.1 Pro | `CodexEditor` | Write Python script to fetch USGS 3DEP 1m DEM for Mono Basin (`EPSG:6340`) and convert to 16-bit PNG heightmap (`Scale.Z = 400.396735`). |
| `T-0008` | **QUEUED** | Claude Opus 5.5 | `CodexVehicleDynamics` | Versioned binary snapshot and FNV-1a state hash (spec in `agents/tasks/T-0008.json`). |
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