# ADR 0003: Pin Unreal Engine 5.8.3

- Status: accepted
- Date: 2026-10-17
- Phase / workstream: P0 Foundation / I (tooling)

## Context
The docs said "UE 5.4 / 5.5" and `Codex.uproject` said 5.4, but the engine installed through the Epic launcher is 5.8.3 (changelist 58210709, branch `++UE5+Release-5.8`, promoted build). Epic's 5.8 release notes list Visual Studio 2022 17.14 or newer as the minimum, Windows SDK 10.0.22621 minimum (10.0.26100 default) and MSVC toolset 14.38 minimum.

## Decision
Pin the project to **Unreal Engine 5.8.3**. Toolchain on the dev machine: Visual Studio 2022 Community 17.14.42, MSVC 14.44.35207, Windows SDK 10.0.26100.0, Blender 5.2.2 LTS, Python 3.12.10, Git and Git LFS.

## Alternatives considered
| Option | Why rejected |
|---|---|
| Install and pin 5.4 or 5.5 | Older, would leave a second multi-tens-of-GB install and later migration cost; no feature we rely on is missing in 5.8 |
| Visual Studio 2026 | Recommended by Epic for 5.8, but a forum report says 5.8.1 project generation still demanded VS 2022; 2022 is the supported minimum and the safe choice |

## Consequences
- Chaos APIs in the research notes were written against 5.4/5.5; `CodexVehicleAsyncCallback.h` must be verified against the 5.8 headers (names such as `FParticleUtilitiesGT`, `GetPhysicsThreadAPI`, `FRigidBodyHandle_Internal`).
- Documents that say "5.4 / 5.5" are updated to 5.8.
- Upgrading engine minor versions later requires a new ADR and a full test run.

## Verification
Clean UnrealBuildTool build of the `CodexEditor` target on Win64 Development with zero errors; engine source headers checked for each Chaos symbol used.

## Sources
Epic: Unreal Engine 5.8 release notes and Hardware and Software Specifications (VS 2022 17.14 minimum, SDK 10.0.22621 minimum). `Engine/Build/Build.version` on the dev machine.
