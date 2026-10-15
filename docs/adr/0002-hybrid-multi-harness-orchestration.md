# ADR 0002: Hybrid Multi-Harness Orchestration

- Status: accepted (supersedes ADR 0001)
- Date: 2026-10-15
- Phase / workstream: I (Tooling & Orchestration)

## Context
ADR 0001 specified a Claude-only workflow due to concerns over API costs and multi-provider fragmentation. However, the production scope of *Codex* is an industrial-scale, multi-month engineering effort requiring deep specialized capabilities:
1. Deep world research, real-world GIS coordinates, and cultural lore without polluting the engineering coding context.
2. Fast in-IDE workspace operations, task tracking, and massive build log triage.
3. Complex mathematical physics derivations (120 Hz fixed-substep Chaos dynamics, Pacejka magic formula).
4. Continuous, heavy C++ compilation and test-fixing loops in the terminal.

The director has access to three distinct native environments:
- **Claude Code CLI:** Anthropic Claude Opus 5.5, Sonnet 5.5, Haiku 5.5.
- **Antigravity IDE:** Google Gemini 3.1 Pro, Gemini 3.8 Flash.
- **Gemini Web App:** Google Gemini Argon 4.

Relying on a single model or single harness either exhausts context windows, hits API rate limits, or forces heavy reasoning models to perform low-level text formatting.

## Decision
Adopt a **Hybrid Multi-Harness Pipeline** with strict separation of concerns across 3 environments and file-based contracts:

1. **Gemini Web App (Argon 4):** Lead Research & World Specialist. Deep web research on GIS datasets (USGS LiDAR, OSM), real vehicle mechanics, and cultural narrative. Outputs markdown files saved to `docs/research/`.
2. **Antigravity IDE (Gemini 3.1 Pro):** Task Architect & Systems Specifier. Translates research into testable task specifications (`agents/tasks/T-XXXX.json`), designs C++ data structs, and manages roadmaps.
3. **Claude Code CLI (Claude Opus 5.5):** Physics Architect & Safety-Critical Reviewer. Derives 120 Hz substepping math, tire differential equations, and performs mandatory code review on core dynamics.
4. **Claude Code CLI (Claude Sonnet 5.5):** Primary C++ Engine Implementer. Writes production headers, sources, and tests; executes UBT builds; self-heals compiler errors.
5. **Antigravity IDE (Gemini 3.8 Flash) & Claude Code CLI (Haiku 5.5):** Fast Triage & Housekeeping. Skims 50,000-line engine build logs, runs asset validation, updates `docs/STATE.md` and `docs/journal/`.

## Working Rules
- **One Writer Rule:** Only one harness executes file edits at any time to prevent git and Unreal asset merge conflicts.
- **File-Based Contracts:** All handoffs occur via disk (`agents/tasks/`, `docs/research/`, `docs/STATE.md`). No hidden conversational memory.
- **Tests Outrank Opinions:** Deterministic C++ unit tests, golden telemetry diffs, and the director's physical driving feel gate progression.

## Alternatives Considered
| Option | Why rejected |
|---|---|
| Single Claude-only harness (ADR 0001) | Context exhaustion on large logs; burns expensive Opus/Sonnet tokens on broad GIS research and documentation chores. |
| Single Gemini-only harness | Lacks native terminal-driven UBT build-fix looping provided by Claude Code CLI. |
| Ad-hoc prompting without file contracts | Leads to model hallucinations, contradictory code implementations, and lost project state. |

## Consequences
- **Positive:** Maximum efficiency per token, zero context contamination between research and code, rapid execution of routine tasks, deep reasoning reserved for core physics.
- **Negative:** Requires the human director to copy-paste research artifacts between the Gemini Web App and local files.

## Verification
- Measure task completion velocity and token consumption across Phase 1 research spikes and Phase 2 headless C++ physics tests.
