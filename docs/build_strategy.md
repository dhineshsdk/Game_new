# Build Strategy: how we build Codex

Companion to `docs/roadmap.md` (what, in what order) and `docs/orchestration.md` (who does what). This file is the how: ordering principles, spikes, tuning loops and practical limits. Written 2026-10-16.

## 1. Build by risk, not by feature
Four unknowns could sink or reshape the plan. Each gets a short spike with a pass criterion and a fallback, recorded as an ADR.

| Spike | Passes if | Fallback |
|---|---|---|
| 120 Hz async physics callback works in the pinned UE version | A box on four springs is stable and deterministic over 10 minutes | Own 120 Hz integrator on the game thread; Chaos only for collisions |
| Tyre model feel | Skidpad and braking numbers match reference targets and the director accepts the first drive | Simplify to a brush model and iterate on feel |
| Mono Basin terrain at scale | A ~30 km corridor streams and holds 60 fps at 1440p | Coarser far terrain, tighter streaming |
| Editor bridge | Import, place, screenshot and read logs without the director | CLI and Python only |

## 2. Test against real numbers
The car is fictional; its acceptance targets come from published road-test numbers for a car of that class: 0-100 km/h, braking from 100 km/h, skidpad g, top speed. These become pass/fail gates. The director's driving judges only what numbers cannot.

## 3. A tuning loop that is cheap for the director
- In-game debug panel with sliders for tyre, spring and damper values.
- One-key save of a preset with a note field, stored in `docs/tuning/`.
- Hot-reloaded data assets.
- A generated "test card": what to drive and what to look for.
- Optional later: a standalone headless physics harness so tuning loops run in seconds without opening Unreal (decision pending).

## 4. Parallel lanes
Physics (A) and the terrain pipeline (B) do not depend on each other until the corridor phase. While physics tests are built, the USGS-to-heightmap-tiles script can run in a separate lane. The Unreal editor lane stays serialized.

## 5. Walking skeleton early
Within the first milestones aim for an ugly end-to-end demo: a grey box car on real Mono Basin terrain with a spline road. Polish later. It exposes pipeline problems early and keeps motivation up.

## 6. Art strategy
- Cabin first: players look at the interior most of the time, so the heirloom's dashboard, gauges and seat come before the exterior.
- Blockout first, then buy or commission one proper heirloom car and pickup (fictional makes).
- Generated everything else: signs, milestones and props through the Blender factory; terrain, vegetation and rocks through rule-based placement.
- Engine audio is its own research track.

## 7. Operating rhythm
- Weekly shape: research, spec, build, then a drive day.
- Prompts for Antigravity and the Gemini app live in `docs/prompts/` to cut copy-paste friction.
- One module and a handful of files per task so the three-strike rule stays meaningful.
- Each milestone yields a short capture or screenshot for the journal.

## 8. Practical limits
- Disk: D: had 143 GB free on 2026-10-16. Mono Basin data, the UE project, derived-data and shader caches can exceed that; plan a second drive or move the DDC.
- CI: local scripts (`build.ps1`, `test.ps1`, a dependency-check script) and a pre-commit hook. No server needed.
- Scope: keep a written cut list for the slice so we know what drops first.

## 9. First six sessions
1. Toolchain: Visual Studio 2022 with C++, a real Python, Blender, a complete UE install.
2. Pin the engine version in an ADR; get a clean compile.
3. Local build/test scripts and a headless test run.
4. T-0101 tyre model, reviewed by Opus.
5. T-0201 120 Hz solver and static-load tests.
6. Grey box car in a Physics Lab with a telemetry HUD.

## 10. Decisions pending
- Engine version (finish 5.8 or match docs at 5.4/5.5).
- Standalone physics harness: yes or no.
- Resolve the "pure C++ physics" contradiction in `CodexVehicleDynamics` (USTRUCT and CoreUObject vs plain structs) in an ADR.
- Verify the async callback header against the pinned engine's source.
