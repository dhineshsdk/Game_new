---
description: End a Codex session: update STATE, write the journal entry, record decisions, commit if a repo exists
---
Close the Codex session cleanly.

1. Make sure nothing is half-done. If something is, finish it or revert it and say so.
2. Write `docs/journal/YYYY-MM-DD.md` (use today's date; append if it exists): what changed and why, measurements, the director's feedback verbatim, decisions made, and anything unverified.
3. Rewrite `docs/STATE.md`: what works, what is broken, open director decisions, next 3 tasks.
4. Add or update ADRs for any architecture decision made. Move out-of-scope ideas to `docs/ideas.md`.
5. If a git repository exists, run tests, then commit with a clear message. If not, say that no commit was made.
6. Give the director a short summary and the test card for anything they should try.

Notes: $ARGUMENTS
