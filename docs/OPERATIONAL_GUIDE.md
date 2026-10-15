# CODEX — Director's Operational Field Manual

This guide is the human director’s daily reference for orchestrating the multi-month engineering production of *Codex* across **Claude Code**, **Antigravity**, and the **Gemini Web App**.

---

## 1. Directory Structure & File Handoff Map

The workspace is organized so that every AI agent reads from and writes to designated directories:

```
D:\Game_project\
├── CLAUDE.md                  # Project rules & model routing for Claude Code CLI
├── docs\
│   ├── MASTER_BLUEPRINT.md    # Canonical specification of all 8 game design pillars
│   ├── STATE.md               # Live project status: phase, what works, next 3 tasks
│   ├── roadmap.md             # High-level phases (P0 to P10) and exit gates
│   ├── orchestration.md       # Multi-harness rules, stop-rules, and lanes
│   ├── ideas.md               # Brainstorm repository (Rounds 1 through 5)
│   ├── OPERATIONAL_GUIDE.md   # This operational handbook
│   ├── adr\                   # Architectural Decision Records (ADR 0001, ADR 0002...)
│   ├── research\              # Output from Gemini Argon 4 (R-XXXX-topic.md)
│   └── journal\               # Session logs and director driving feedback
└── agents\
    └── tasks\                 # Executable task specs (T-0001.json, T-0002.json...)
```

---

## 2. Daily Session Workflow (How to Build a Feature)

Each feature or task follows this exact chain of custody:

### Step 1: Research (Gemini App — Argon 4)
* **Goal:** Extract real-world data without cluttering your local C++ coding context.
* **Action:** Prompt Argon 4 with a quota-optimized, high-density prompt.
* **Output:** Argon outputs raw tables, formulas, or bounding boxes.
* **Storage:** Save the output into `docs/research/R-XXXX-title.md` (e.g., `docs/research/R-0001-mono-basin-lidar-and-tires.md`).

### Step 2: Task Specification (Antigravity — Gemini 3.1 Pro)
* **Goal:** Turn raw research into a rigorous, testable C++ task contract.
* **Action:** Switch Antigravity to **Gemini 3.1 Pro**. Point it at `docs/research/R-XXXX.md` and ask it to generate `agents/tasks/T-XXXX.json` using `agents/tasks/_template.json`.
* **Requirement:** Must define exact input files, target C++ files, and explicit pass/fail acceptance tests.

### Step 3: Math & Safety Review (Claude Code CLI — Opus 5.5)
* **Goal:** Guard against numerical instability (NaNs), race conditions, and bad math.
* **Action:** Run Claude Code in the terminal with Opus 5.5:
  `claude --model opus`
* **Prompt:** *"Review agents/tasks/T-XXXX.json against docs/MASTER_BLUEPRINT.md. Verify mathematical stability and 120 Hz thread safety."*

### Step 4: C++ Implementation & Build-Fix Loop (Claude Code CLI — Sonnet 5.5)
* **Goal:** Write clean C++ code and make tests pass green.
* **Action:** Switch Claude Code to Sonnet 5.5:
  `claude --model sonnet`
* **Prompt:** *"Implement agents/tasks/T-XXXX.json. Run UBT compile and headless Automation tests until all tests pass."*
* **Stop-Rule:** If Sonnet fails 3 consecutive build loops, STOP. Re-evaluate with Opus 5.5.

### Step 5: Verification & Journaling (Antigravity — Gemini 3.8 Flash)
* **Goal:** Document progress, triage logs, and commit.
* **Action:** In Antigravity with **Gemini 3.8 Flash**:
  - Review automated test results.
  - If tests failed with large logs, have Flash pinpoint the line number.
  - Update `docs/STATE.md` and append to `docs/journal/`.
  - Stage and commit via git.

---

## 3. Strict Production Rules (Preventing "AI Slop")

1. **No Monolithic Single-Prompt Attempts:** Never ask an agent to "build the vehicle system." Build `CodexTireModel` first, test it headless, build `CodexSuspension` second, test it headless, then integrate.
2. **Never Commit Without a Passing Test:** Every C++ class must have a corresponding test in `CodexTests`. If there are no tests, the feature is considered non-existent.
3. **Never Accept "It Feels Good" from an AI:** Models cannot feel steering wheel force feedback or suspension rebound. Models report telemetry numbers; the human director judges feel.
4. **One Writer at a Time:** Do not run Claude Code edits while simultaneously executing file replacements in Antigravity.
