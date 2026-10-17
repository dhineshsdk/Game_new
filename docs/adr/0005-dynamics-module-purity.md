# ADR 0005: Pure C++ core for CodexCore and CodexVehicleDynamics

- Status: proposed (needs director approval for the CLAUDE.md law 4 wording change in section "Consequences")
- Date: 2026-10-17
- Phase / workstream: P0 Foundation into P2 Dynamics core / A (dynamics) and I (tooling)

## Context
CLAUDE.md law 4 says `CodexVehicleDynamics` "depends ONLY on `CodexCore`" and is "100% testable in headless C++ without booting UE5 Editor"; `CodexCore` has "ZERO engine/Chaos dependencies". The code says otherwise:

| Item | Current state (read 2026-10-17) |
|---|---|
| `CodexVehicleDynamics.Build.cs` | Public deps `Core`, `CoreUObject`, `CodexCore` |
| `CodexVehicleParams.h`, `CodexVehicleSimulationState.h` | `USTRUCT`/`UPROPERTY`/`GENERATED_BODY`, `.generated.h`, `FVector`/`FQuat` (double, LWC) mixed with `FVector2f` (float) |
| `CodexCore.h`, `CodexVehicleDynamics.h` | Public headers include `CoreMinimal.h` and `Modules/ModuleManager.h`, declare UE log categories |
| `CodexDynamicsUnitTests.cpp` | `IMPLEMENT_SIMPLE_AUTOMATION_TEST`; needs the editor (or `UnrealEditor-Cmd`) to boot to run |
| `docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md` section 3 | Specifies the state as a `USTRUCT` |

`docs/build_strategy.md` section 10 lists this contradiction and "standalone physics harness: yes or no" as pending. Every later dynamics task (T-0101 tyre, T-0201 solver) inherits the answer, so it must be settled before T-0101.

Facts checked in the installed engine (`C:\Program Files\Epic Games\UE_5.8`):
1. UBT's MSVC default floating-point mode is `/fp:fast` (`VCToolChain.cs` lines 1334-1335: `FPSemanticsMode.Default` and `Imprecise` both add `/fp:fast`). A per-module override exists: `ModuleRules.FPSemantics` (`ModuleRules.cs` line 784), with `Precise` mapping to `/fp:precise`.
2. Default C++ standard is C++20 (`CppCompileEnvironment.cs`: `Default = Cpp20`, `EngineDefault = Cpp20`; C++17 is obsolete in 5.8).
3. `FVector = TVector<double>`, `FQuat = TQuat<double>`, `FVector2f = TVector2<float>` (`Core/Public/Math/MathFwd.h` lines 47, 50, 74).
4. Chaos async inputs are pooled and recycled: `TSimCallbackObject` calls `Reset()` on reused inputs (`Chaos/SimCallbackObject.h` line 590), the game thread writes through `GetProducerInputData_External()` (line 597) and the physics thread reads `GetConsumerInput_Internal()` (line 605). Anything the PT needs must therefore be copied into the input by value.
5. UE Low-Level Tests exist in this install (`UnrealBuildTool/Configuration/Rules/TestTargetRules.cs`, `TestModuleRules.cs`, `Engine/Source/Developer/LowLevelTestsRunner`). Whether they build and run from a launcher (binary) install has not been checked.
6. VS 2022 Community `vcvars64.bat` is present; the CMake bundled with VS is not installed. A headless harness therefore uses `cl.exe` directly from a PowerShell script.

## Decision
**Option (a), pure core, with two narrow concessions.**

1. **Pure code.** Every file in `Source/CodexCore` and `Source/CodexVehicleDynamics` except one module-glue file per module is standard C++20 only: `<cmath>`, `<cstdint>`, `<array>`, `<type_traits>`, `<limits>` and `"Codex*.h"` headers. No `CoreMinimal.h`, no UE types (`FVector`, `FQuat`, `FMath`, `TArray`, `FString`), no reflection (`USTRUCT`, `UPROPERTY`, `GENERATED_BODY`), no `UE_LOG`, no `check()`, no exceptions, no RTTI. Code lives in `namespace Codex` (`Codex::Dynamics` for the dynamics module).
2. **Own math types in CodexCore.** `Source/CodexCore/Public/CodexMath.h` provides `Codex::FVec2d`, `Codex::FVec3d`, `Codex::FQuatd`, `Codex::FMat3d` (all `double`, trivially copyable, no SIMD), the constants `Codex::GravityMps2 = 9.80665` and the Chaos injection factors `ForceToChaos = 100.0`, `TorqueToChaos = 10000.0` from law 3. Names are deliberately distinct from UE's `FVector3d`/`FQuat4d` so the two never collide when both are visible in `CodexVehicles`.
3. **Concession 1, module glue.** UE `Core` stays in both `Build.cs` files only so UBT can register the module. The glue lives in exactly `Source/CodexCore/Private/CodexCoreModule.cpp` and `Source/CodexVehicleDynamics/Private/CodexVehicleDynamicsModule.cpp`; the public `CodexCore.h` / `CodexVehicleDynamics.h` module headers and their log categories are deleted. `CoreUObject` is removed from `CodexVehicleDynamics.Build.cs`. Both modules set `FPSemantics = FPSemanticsMode.Precise` and `PCHUsage = PCHUsageMode.NoPCHs` (so no UE header is force-included and the precise flag cannot be overridden by a shared PCH; the PCH interaction is unverified and is checked by T-0005's build).
4. **Concession 2, UE mirrors only where a consumer exists.** Reflected types live in `CodexVehicles` and convert explicitly to and from the pure structs: `FCodexVehicleParamsData` (USTRUCT) inside `UCodexVehicleDefinition` (UDataAsset) when the Physics Lab needs editor tuning (P3); a state snapshot USTRUCT only when save/replication needs it (P7/P11). No mirror is written speculatively.
5. **Two test lanes.** Pure unit tests run in a standalone harness (`Tests/Headless/`, outside `Source/` so UBT never compiles it) built by `tools/test_headless.ps1` with `cl.exe /std:c++20 /fp:precise /W4 /WX /permissive- /GR-`. `CodexTests` keeps UE automation tests only for integration: mirror round-trips, Chaos input marshaling, data-asset loading.
6. **PT safety by construction.** Pure params and state structs carry `static_assert(std::is_trivially_copyable_v<T>)`. Because `CodexVehicleDynamics` cannot see `CoreUObject`, a `UObject*` cannot appear in anything the PT copies; the compiler enforces law 3 for the dynamics payload.
7. **Determinism target.** Bitwise-identical results for the same inputs in the same binary on the same platform (replays, ghosts, regression hashes). Harness and UBT builds are compared against the same golden values within tolerances, not bitwise. Cross-platform or cross-compiler bit-exactness (needed only for lockstep networking) is out of scope and parked.
8. **Serialization.** The pure state gets a versioned, field-by-field little-endian binary snapshot and an FNV-1a hash per frame (T-0008). No `memcpy` of structs with padding.

### Migration plan (one module, a handful of files per task)
| Order | Task | Size | Owner / reviewer | Content | Gate |
|---|---|---|---|---|---|
| 1 | T-0005 CodexCore purity and math types | S | Sonnet / Opus | `CodexMath.h` (+ small `.cpp` if needed), move glue to `CodexCoreModule.cpp`, delete `CodexCore.h`, `FPSemantics = Precise`, `NoPCHs`; `tools/check_purity.ps1` (include allowlist, banned-token list, `Build.cs` dependency allowlist for both pure modules) | UBT `CodexEditor` Win64 Development builds with zero warnings; purity script passes for CodexCore |
| 2 | T-0006 Headless harness | S | Sonnet / Haiku | `tools/test_headless.ps1` (vswhere/vcvars64, compiles pure `.cpp` files except `*Module.cpp` with `/DCODEXCORE_API= /DCODEXVEHICLEDYNAMICS_API=`), `Tests/Headless/CodexTestMain.cpp` with minimal own assertion macros (no third-party framework), `Tests/Headless/Core/MathTests.cpp` | Script exits 0; non-zero on a deliberately failing test; prints wall time; full build+run under 30 s on the dev machine (target, measured in the task) |
| 3 | T-0007 Dynamics module migration | S | Sonnet / Opus | `FCodexVehicleParams` and `FCodexVehicleSimulationState` become plain `Codex::Dynamics` structs using `FVec3d`/`FQuatd`/`FVec2d`; remove `CoreUObject`; glue to `CodexVehicleDynamicsModule.cpp`; move the two existing automation tests to `Tests/Headless/Dynamics/`; keep one `CodexTests` smoke test that includes the pure headers | Purity script passes for both modules; harness passes; UBT zero warnings |
| 4 | T-0101 Pacejka MF94 tyre model | M | Sonnet / Opus | See `agents/tasks/T-0101.json` | Numeric tests in that file |
| 5 | T-0008 State snapshot and hash | S | Sonnet / Opus | Versioned binary write/read, FNV-1a hash, round-trip and "same inputs twice, same hash" tests | Harness passes |
| 6 | T-0201 120 Hz solver | M | Sonnet / Opus | Unchanged scope, now on pure types; `FCodexVehicleAsyncInput` carries pure structs by value | As in T-0201 |
| 7 | T-0009 UE params mirror (at P3 start) | S | Sonnet / Opus | `FCodexVehicleParamsData`, `UCodexVehicleDefinition`, `ToPure()`/`FromPure()` in `CodexVehicles`; automation test: round-trip equals field-for-field | Automation test passes headless via `UnrealEditor-Cmd` |
| 8 | T-0010 Doc alignment | XS | Haiku | Spec section 3 snippet to the pure struct; `build_strategy.md` section 10 marked decided; CLAUDE.md law 4 wording (director applies or approves) | Docs diff reviewed |

Fallback: if T-0006 cannot meet its gate, the pure sources compile unchanged under UE Low-Level Tests (option (c) below), so the decision is low-regret.

## Alternatives considered
| Option | Why rejected |
|---|---|
| (b) Keep USTRUCT/FVector in dynamics and reword the law ("depends on Core/CoreUObject and CodexCore, no Engine/Chaos") | Cheapest today, but every test needs an editor boot (time not measured; commonly tens of seconds) which kills the seconds-long tuning loop in `build_strategy.md` section 3; leaves `/fp:fast` reassociation in the solver; mixes double `FVector` with float `FVector2f` in one state; reflection gives "free" serialization that is the wrong tool for replay (property-tagged, version-fragile) anyway; nothing stops a `UObject*` creeping into PT data. Cost of switching later grows with every task built on it. |
| (c) Plain structs, but UE `Core` math (`FVector3d`, `FQuat4d`, `FMath`) and UE Low-Level Tests | Removes reflection and editor boot, but binds the harness to UBT and the engine install (launcher-install support for LLT unverified; link times unmeasured), keeps the engine's SIMD math paths in the determinism story, and cannot be built by a plain compiler for a future out-of-engine tuning tool. Kept as the documented fallback for T-0006. |
| (a) fully literal: no UE `Core` dependency at all | UBT modules need `IMPLEMENT_MODULE`, which needs `Core`. Making the dynamics a non-UBT static library adds a second build system to the game build. The one-file glue exception costs nothing and is checked by script. |
| Third-party math (Eigen, glm) or test framework (doctest, Catch2) | Licence review and vendoring for roughly 300 lines of vector/quaternion code and 50 lines of assertion macros. Not worth it at this size. |

### Weighing (summary)
| Criterion | (a) pure core | (b) UE types | (c) UE Core math + LLT |
|---|---|---|---|
| Determinism | Best: own scalar math, module-level `/fp:precise`, harness uses same flag | Weakest: `/fp:fast` default, SIMD paths, mixed precision | Medium |
| Replay / serialization | Explicit versioned snapshot (T-0008) | Reflection-based, implicit | Explicit snapshot |
| Future networking | Net layer in `CodexVehicles` quantizes from pure state; lockstep parked either way | Same, with reflection already present | Same as (a) |
| Test speed | Seconds (to be measured in T-0006) | Editor boot per run | UBT link per change (unmeasured) |
| Effort now | ~3 small tasks (T-0005..T-0007) before T-0101 | Near zero | ~2 small tasks |
| 120 Hz PT safety | Enforced by compiler (no `CoreUObject` visible, trivially-copyable asserts) | By discipline only | Enforced for reflection; UE containers can still allocate on PT |

## Consequences
Positive: law 4 becomes true and checkable by script; tyre and solver tuning loops run without Unreal; the PT payload is safe by construction; replay hashing is meaningful.

Negative: we own a small math library and its tests; conversion code between `Codex::FVec3d` (m) and `FVector` (cm) in `CodexVehicles` is a place for unit bugs, so every conversion function gets a round-trip test; two build paths (UBT and harness) must be kept in step (same `/fp:precise`, C++20, warning level).

Follow-up:
- **Proposed CLAUDE.md law 4 wording (director to approve and apply; agents must not edit CLAUDE.md on their own):**
  "`CodexCore` (math, units, constants; standard C++20 only; UE `Core` is linked solely for module registration in `Private/CodexCoreModule.cpp`)" and
  "`CodexVehicleDynamics` (depends only on `CodexCore` with the same single-file module-glue exception; no UE headers, types or reflection elsewhere; built and tested by the headless harness `tools/test_headless.ps1` without UE)".
- `docs/STATE.md` should show T-0005, T-0006, T-0007 queued ahead of T-0101 and change T-0101 from READY to QUEUED (Haiku).
- Parked to `docs/ideas.md` (not started; to be appended by Haiku): cross-platform bit-exact determinism for lockstep networking; Python bindings or a GUI over the headless harness; longitudinal tyre relaxation; full MF94 load-dependent cornering stiffness (`BCD = a3 sin(2 atan(Fz/a4))`), camber and turn slip.

### Spec inconsistencies found (flagged, not fixed)
These are recorded for Opus (math) and Argon (research) to resolve in the spec; no task silently changes the numbers.

1. **Damping ratios do not match the stated rates and masses.** With `zeta = c / (2 sqrt(k m))`, corner sprung mass from the 53/47 axle split minus unsprung mass (front (689.0 - 76) / 2 = 306.5 kg, rear (611.0 - 84) / 2 = 263.5 kg):
   - Front: critical damping 5646 N s/m, so bump 2200 gives zeta = 0.39, rebound 3400 gives 0.60.
   - Rear: critical damping 5529 N s/m, so bump 2400 gives 0.43, rebound 3800 gives 0.69.
   - Using the sprung mass split 53/47 instead (302.1 / 267.9 kg) gives 0.39 / 0.61 front and 0.43 / 0.68 rear. Using total corner mass (344.5 kg front) gives bump 0.37. No interpretation reproduces the spec's 0.28 / 0.44. Matching the stated ratios would need about 1580 / 1550 N s/m bump and 2480 / 2430 N s/m rebound (F / R). Either the rates or the ratios are wrong, or a motion ratio is implied but not stated.
2. **Ride frequencies are about 4% off.** `f = sqrt(k/m) / (2 pi)` with the masses above gives 1.47 Hz front and 1.67 Hz rear vs spec 1.42 / 1.60 Hz. With total corner mass: 1.38 / 1.55 Hz.
3. **Static axle loads.** 1300 kg x 9.80665 = 12748.6 N gives 6756.8 N front and 5991.9 N rear, not 6759 / 5994 (2.2 N gap; inside T-0201's +/-5 N tolerance, but the spec should state the computed values).
4. **Ice row is super-linear near zero slip.** Small-slip expansion of the magic formula gives `Fy/D = C x + [C(|E| - 1)/3 - C^3/6] x^3` for `E < 0`. For ice (C = 1.2, E = -2.0) the cubic coefficient is +0.112, so the curve stiffens before it saturates (slope at origin is not the maximum slope). Dry, wet and gravel are fine (-0.366, -0.193, -0.617). Physically unusual; needs Opus review.
5. **No longitudinal coefficients.** Section 2.2 item 3 needs `Fx0(kappa)` and `kappa_peak`, but the spec gives only lateral rows. T-0101 makes longitudinal B, C, E separate data fields that default to the lateral values, with one shared mu per surface; real values need an Argon research brief.
6. **Relaxation length per surface is ambiguous.** `sigma_y = C_Falpha / K_y` gives 0.555 m dry but 0.43 / 0.23 / 0.09 m for wet / gravel / ice with constant `K_y = 75000 N/m`. The spec says "about 0.55 m". T-0101 uses one data-driven `sigma_y` (default 0.55 m) per tyre.
7. **Only D scales with load.** B, C, E are constant, so cornering stiffness scales with D(Fz). Accepted simplification for T-0101. `D(Fz)` peaks at Fz = 12711 N and reaches zero at 25422 N (pDy2 = -0.144); T-0101 clamps D to at least zero and returns zero force for Fz <= 0.
8. **Timestep.** CLAUDE.md gives `AsyncFixedTimeStepSize=0.008333`; 1/120 = 0.0083333... All dynamics functions take `dt` as an argument; tests use `1.0/120.0`.

## Verification
- `tools/check_purity.ps1` exits 0: no non-allowlisted include, no banned token, and `Build.cs` dependency lists equal `{Core}` (CodexCore) and `{Core, CodexCore}` (CodexVehicleDynamics) after T-0007.
- `tools/test_headless.ps1` builds and runs all pure tests without any UE path on the include line, and reports its wall time.
- UBT `CodexEditor` Win64 Development build has zero errors and zero warnings with `FPSemantics = Precise` in both pure modules. The UBT command line for those modules must show `/fp:precise` (checked in the build log).
- After T-0008: a fixed 10-minute input script at 120 Hz (72,000 steps) produces the same final state hash on two runs of the same binary.

## Sources
- Engine source in the local install `C:\Program Files\Epic Games\UE_5.8` (5.8.3, CL 58210709 per ADR 0003), read 2026-10-17: `Engine/Source/Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs` (fp flags, lines 1313-1348), `.../Configuration/Rules/ModuleRules.cs` (line 784, `FPSemantics`), `.../System/CppCompileEnvironment.cs` (C++ standard enum), `.../Configuration/Rules/TestTargetRules.cs` and `TestModuleRules.cs` (LLT exists), `Engine/Source/Runtime/Core/Public/Math/MathFwd.h` (LWC typedefs), `Engine/Source/Runtime/Experimental/Chaos/Public/Chaos/SimCallbackObject.h` (input pooling, lines 590-605).
- Verified: VS 2022 Community `VC/Auxiliary/Build/vcvars64.bat` present; bundled CMake absent.
- **Unverified:** LLT on a launcher install; editor-boot and harness timings; whether `FPSemantics = Precise` combines cleanly with `NoPCHs` in 5.8 (T-0005 build checks it); that UBT force-includes shared PCHs into every module source (the reason for `NoPCHs`, believed true, not re-checked in 5.8 source); MSVC CRT `sin`/`atan`/`exp` being identical across CRT versions (not relied on).
- Spec numbers and arithmetic: `docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md` sections 1 and 2; inconsistency arithmetic computed by hand by the author of this ADR (Claude Opus 5.5, 2026-10-17) and to be re-checked by the reference script in T-0101.
