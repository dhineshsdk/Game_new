# Codex: Multi-Harness Hybrid Orchestration

**Status:** Canonical Engineering Workflow  
**Supersedes:** `docs/adr/0001-claude-only-orchestration.md` (Formalized in `docs/adr/0002-hybrid-multi-harness-orchestration.md`)

This document defines how the human director coordinates the multi-model AI engineering team across **Claude Code**, **Antigravity**, and the **Gemini Web App** over the multi-month production lifecycle of *Codex*.

---

## 1. Core Operating Principles

1. **One Writer at a Time:** Only one agent environment modifies repository files at any given moment. Unreal binary assets and C++ source trees must never experience concurrent edits.
2. **Files are the Universal Contract:** Models do not pass instructions via conversational memory. All handoffs occur via disk:
   - Research $\to$ `docs/research/`
   - Task Specifications $\to$ `agents/tasks/T-XXXX.json`
   - Progress & Roadmaps $\to$ `docs/STATE.md` and `docs/journal/`
   - Code & Tests $\to$ `Source/` and `Tests/`
3. **Tests Outrank Opinions:** Deterministic C++ Automation tests, warnings-as-errors (`-Werror`), dependency linting, and telemetry CSV golden diffs determine whether work passes. No model may declare that code "feels good."
4. **Small, Atomic Work Units:** Each session achieves exactly one verified outcome (a few hours of engineering work). Nothing is left half-implemented or uncommitted between sessions.
5. **The Director is the Quality Gate:** The human director tests the build, provides art approval, validates physical steering wheel/gamepad feel, and gives the formal "Go" for phase transitions.

---

## 2. Model Roles Across the 3 Native Harnesses

```
┌────────────────────────────────────────────────────────────────────────┐
│                   MULTI-HARNESS COLLABORATION MATRIX                   │
├───────────────────┬───────────────────┬────────────────────────────────┤
│ HARNESS           │ MODEL             │ CORE RESPONSIBILITY            │
├───────────────────┼───────────────────┼────────────────────────────────┤
│ Gemini Web App    │ Gemini Argon 4    │ Lead Research & World Spec     │
│                   │                   │ • GIS data coordinates (USGS)  │
│                   │                   │ • Mechanical whitepaper review │
│                   │                   │ • Cultural narrative & lore    │
├───────────────────┼───────────────────┼────────────────────────────────┤
│ Antigravity IDE   │ Gemini 3.1 Pro    │ Task Architect & Data Models   │
│                   │                   │ • Writes agents/tasks/T-XXXX   │
│                   │                   │ • Defines C++ USTRUCT schemas  │
│                   │                   │ • Maintains docs/STATE.md      │
├───────────────────┼───────────────────┼────────────────────────────────┤
│ Claude Code CLI   │ Claude Opus 5.5   │ Physics Architect & Reviewer   │
│                   │                   │ • 120 Hz async Chaos physics   │
│                   │                   │ • Pacejka tire derivations     │
│                   │                   │ • High-risk code reviews       │
├───────────────────┼───────────────────┼────────────────────────────────┤
│ Claude Code CLI   │ Claude Sonnet 5.5 │ Primary C++ Engine Implementer │
│                   │                   │ • Production C++ (.h / .cpp)   │
│                   │                   │ • UBT compilation & build-fix  │
│                   │                   │ • Automation unit tests        │
│                   │                   │ • Headless Blender/Python      │
├───────────────────┼───────────────────┼────────────────────────────────┤
│ Antigravity IDE   │ Gemini 3.8 Flash  │ Fast Triage & Housekeeping     │
│ & Claude Code CLI │ & Haiku 5.5       │ • Skims 50,000-line build logs │
│                   │                   │ • Quick git status & doc sync  │
│                   │                   │ • Asset naming validation      │
└───────────────────┴───────────────────┴────────────────────────────────┘
```

---

## 3. The 5-Step Feature Lifecycle

Every single system, module, or gameplay feature in *Codex* follows this strict 5-step lifecycle:

```
[1. Research] ──────► [2. Spec] ──────► [3. Review] ──────► [4. Build] ──────► [5. Verify & Log]
 Argon 4               Gemini 3.1 Pro    Opus 5.5            Sonnet 5.5         Director + 3.8 Flash
 (Gemini App)          (Antigravity)     (Claude Code)       (Claude Code)      (Antigravity)
```

### Step 1: Research (Gemini Argon 4 in Gemini App)
* **Action:** Director prompts Argon 4 for real-world mathematical parameters, GIS bounding boxes, or mechanical curves.
* **Output:** Argon outputs a clean, dense markdown document.
* **Handoff:** Director saves the text to `docs/research/R-XXXX-topic.md`.

### Step 2: Task Specification (Gemini 3.1 Pro in Antigravity)
* **Action:** Gemini 3.1 Pro ingests `docs/research/R-XXXX-topic.md` and generates a formal task contract at `agents/tasks/T-XXXX.json` (using `agents/tasks/_template.json`).
* **Content:** Defines exact file targets, allowed module dependencies, and executable C++ acceptance test criteria.
* **Handoff:** Task file committed to disk.

### Step 3: Architecture & Safety Review (Claude Opus 5.5 in Claude Code)
* **Trigger:** Required for high-risk tasks (any change to `FCodexVehicleSimulationState`, 120 Hz substep math, tire friction formulations, or module dependency additions).
* **Action:** Opus 5.5 inspects the proposed task spec and algorithms, verifies thread safety and mathematical stability, and signs off.

### Step 4: Production Implementation & Build (Claude Sonnet 5.5 in Claude Code)
* **Action:** Sonnet 5.5 implements the C++ headers and source files.
* **Execution Loop:**
  1. Writes code in `Source/`.
  2. Runs Unreal Build Tool (`UBT Development Win64`).
  3. Runs headless Automation Tests (`UnrealEditor-Cmd.exe -ExecCmds="Automation RunTests ..."`).
  4. Self-corrects any compilation or test failures (up to a hard stop-rule of 3 attempts).

### Step 5: Triage, Verification & Journaling (Director & Gemini 3.8 Flash in Antigravity)
* **Director Check:** Director runs the test command or tests vehicle feel in the viewport.
* **Log Triage:** If tests failed with massive logs, Gemini 3.8 Flash skims the logs to isolate the exact line number.
* **Session End:** Flash updates `docs/STATE.md`, appends a session journal entry in `docs/journal/`, and prepares the git commit.

---

## 4. Safety Gates & Stop-Rules

1. **The 3-Loop Stop-Rule:** If Sonnet 5.5 fails a build or test loop 3 consecutive times, it must not grind in circles. Work halts immediately. The task is escalated to Opus 5.5 to diagnose architectural flaws or numerical instability (NaNs).
2. **Deterministic Precedence:** Model opinions never determine whether physics work. Code is judged exclusively by:
   - Zero compilation warnings (`-Werror`).
   - 100% passing unit tests.
   - Telemetry CSV diffs within $\pm 1.0\%$ of mathematical baselines.
3. **Serialized Editor Lane:** Python editor scripts (`-ExecutePythonScript`) and Unreal commandlets run strictly one at a time. No parallel editor instances.
