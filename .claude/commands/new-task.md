---
description: Plan a new task: create a task spec and route it to the right agent
---
Plan this task for the Codex project: $ARGUMENTS

1. Read `CLAUDE.md`, `docs/STATE.md` and relevant ADRs/system docs.
2. Use the `planner` agent to write `agents/tasks/T-XXXX.json` (next free id) from `agents/tasks/_template.json`, with owner and reviewer agents, forbidden dependencies and runnable acceptance criteria. (Per ADR 0002 specs are normally written by Gemini 3.1 Pro in Antigravity; use this command when the spec is needed from the Claude side or to review an existing one.)
3. Show the director the spec in plain language and wait for approval.
4. After approval: run the `implementer` agent, then the mechanical gates (build, tests, dependency lint), then review by the task's `reviewer_agent` (the `reviewer` agent for high-risk physics/thread/state work). Stop after 3 failed loops (`strike_count`) and escalate to Opus.
5. Summarise results and anything the director needs to test.
