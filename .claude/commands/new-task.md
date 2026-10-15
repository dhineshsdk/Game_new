---
description: Plan a new task: create a task spec and route it to the right agent
---
Plan this task for the Codex project: $ARGUMENTS

1. Read `CLAUDE.md`, `docs/STATE.md` and relevant ADRs/system docs.
2. Use the `planner` agent to write `agents/tasks/T-XXXX.json` (next free id) from `agents/tasks/_template.json`, with runnable acceptance tests, the risk level, lane, model and review level.
3. Show the director the spec in plain language and wait for approval.
4. After approval: run the `implementer` agent, then the mechanical gates (build, tests, dependency lint), then review by risk (`none`, self, or the `reviewer` agent). Escalate to Opus after 3 failed loops.
5. Summarise results and anything the director needs to test.
