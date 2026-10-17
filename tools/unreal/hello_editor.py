"""Bridge smoke test: asks the editor who it is and what the project contains.

Run with tools/unreal/run_ue_python.ps1.
"""
import unreal

unreal.log("CODEX: engine_version=" + unreal.SystemLibrary.get_engine_version())
unreal.log("CODEX: project_dir=" + unreal.Paths.project_dir())

assets = unreal.EditorAssetLibrary.list_assets("/Game", recursive=True, include_folder=False)
unreal.log("CODEX: game_asset_count=" + str(len(assets)))
for path in assets[:50]:
    unreal.log("CODEX: asset=" + path)
