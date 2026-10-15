# Risks

| Risk | Impact | Mitigation | Owner |
|---|---|---|---|
| Scope: MSFS-scale real-time world, multiplayer, trucks, bikes, ships | Never finishing | Staged phases with gates, slice before scale, parking lot | Director + Opus |
| Feel cannot be verified by Claude | Many tuning loops | Telemetry tests for numbers, director drive-tests, `docs/tuning/` records | Director |
| Chaos is not simulation-grade for bikes/trucks | Rework | Custom dynamics layer on Chaos bodies; substep spike in P1 | Opus |
| Unreal version API churn (async physics, PCG, Water, World Partition) | Rework | Pin version by ADR, check release notes in P1 | Researcher |
| Binary assets cannot be merged | Lost work | Git LFS, serialized editor lane, define content in C++/Python/JSON | Sonnet |
| Art kits per biome are the real cost | Slow regions | Budget art per region, buy/licence kits, pipeline automates placement only | Director |
| Real data licences (OSM ODbL share-alike, DEM attribution) | Legal | Review in P1, attribution file, keep derived data policy in ADR | Researcher |
| Kashmir/Ladakh boundary depiction | Legal, market access | No drawn or labelled disputed borders until advised; revisit before India region | Director |
| Real brands, signs, place names | Legal | Fictional makes/towns, fictional sign text | All |
| Project name "Codex" collides with an existing product | Branding | Treat as codename; trademark search before release | Director |
| Multiplayer physics determinism | Late rework | Replication-ready state struct now; soft MP first, no contact | Opus |
| Ocean/ship simulation scale (Water plugin, buoyancy) | Dead end | Research spike; Stage A is only a scene | Opus |
| Hardware ceiling: 12 GB VRAM, 32 GB RAM | Editor limits | 1440p/60 budget, aggressive streaming, lean editor | Sonnet |
| Context loss across months | Incoherence | CLAUDE.md, STATE.md, journal, ADRs, tests | All |
| MCP bridge security (local code execution socket) | Security | Localhost only, auth token, disable code-exec outside editor sessions | Sonnet |
| Burnout / long timeline | Abandonment | Always a playable build, fortnightly drive day, visible progress each milestone | Director |
