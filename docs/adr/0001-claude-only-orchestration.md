# ADR 0001: Claude-only orchestration

- Status: superseded by ADR 0002 (2026-10-15)
- Date: 2026-10-04
- Phase / workstream: I (tooling)

## Context
We considered adding Gemini 3.8 Flash and Kimi K3 to Opus 5.5, Sonnet 5.5 and Haiku 4.5. Using the director's Antigravity subscription from scripts would breach Google's third-party access terms; an API key costs money and sends project data to another provider; Kimi has no plan or key. The project's real bottlenecks are feel tuning, long-term context, and the Unreal/Blender connection, none of which extra models fix.

## Decision
Use Claude models only: Opus for architecture, research and risky-code review, Sonnet for implementation, Haiku for bulk and triage work. Independent review is a fresh Opus context plus deterministic tests. Gemini or Kimi may be added later as optional providers if a measured need appears.

## Alternatives considered
| Option | Why rejected |
|---|---|
| Gemini + Kimi routed through a script | Extra keys, cost, privacy surface, quota limits; no demonstrated benefit yet |
| Borrowing Antigravity OAuth in tooling | Breach of Google terms; accounts have been suspended |
| Manual Gemini lane via Antigravity | Optional later; low quota, human in the loop |

## Consequences
Simpler workflow with fewer failure points. Correlated blind spots between Claude models are mitigated by tests, golden telemetry and fresh-context review. Revisit if review misses recur.

## Verification
Track in `docs/journal/` any bug a review missed that tests later caught; if it recurs, run a bake-off for an external reviewer.
