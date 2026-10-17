# ADR 0004: Use Epic's built-in MCP server for the Unreal editor bridge

- Status: accepted
- Date: 2026-10-17
- Phase / workstream: P0 Foundation / I (tooling)

## Context
The plan was to write a small custom MCP bridge in `CodexEditor` (spawn actors, import assets, screenshots, logs, play-in-editor). UE 5.8.3 ships experimental plugins that already do this: `ModelContextProtocol` (an MCP server, HTTP on `127.0.0.1:8000/mcp`), `EditorToolset` and `Terminal`. The director enabled them in `Codex.uproject` and added `.mcp.json` with the server `unreal-mcp`. Probing the running editor showed three registry tools (`list_toolsets`, `describe_toolset`, `call_tool`) and toolsets for: editor app state (console variables, asset imaging, selection, viewport camera, content browser, PIE start/stop), logs, actors, assets, blueprints, data assets, data tables, curve tables, materials, material instances, objects/properties, primitives, scene/levels, skeletal and static meshes, string tables, textures, and a sandboxed programmatic batch toolset.

## Decision
Use Epic's MCP server as the live editor bridge instead of writing our own. Keep the headless runners (`tools/unreal/run_ue_python.ps1`, `tools/blender/run_blender.ps1`) for builds, tests and batch jobs that run without a window.

## Alternatives considered
| Option | Why rejected |
|---|---|
| Custom bridge plugin in CodexEditor | Duplicates an official, maintained plugin; more code and security surface for us |
| Community Unreal MCP plugins | Unvetted code running through an open socket; Epic's is first-party |
| Headless scripts only | Cannot see the live viewport or drive play-in-editor |

## Consequences
- The plugins are **experimental**; names, toolsets and behaviour may change between 5.8.x versions. Re-check after engine updates.
- The server has no authentication and listens on loopback only. Anything running as the director's user can call it, so keep the editor closed or the plugin disabled when not in use, never forward port 8000, and do not enable it on shared machines.
- `Terminal` (an in-editor terminal emulator) is not needed for our workflow; consider disabling it to reduce surface.
- Claude Code loads `.mcp.json` servers at session start and asks for approval; a new session is needed to get native tools.
- The editor must be open for the bridge to exist; UBT builds fail while the editor holds Live Coding, so builds happen with the editor closed (or via Ctrl+Alt+F11).

## Verification
`tools/list` returns the three registry tools; `list_toolsets` returns the toolsets above (done 2026-10-17). Next: take a viewport capture and read the log through the toolsets.

## Sources
Plugin descriptors in `Engine/Plugins/Experimental/ModelContextProtocol`, `.../Toolsets/EditorToolset` and `.../Terminal` (UE 5.8.3).
