#include "CodexEditor.h"

DEFINE_LOG_CATEGORY(LogCodexEditor);

void FCodexEditorModule::StartupModule()
{
	UE_LOG(LogCodexEditor, Log, TEXT("CodexEditor module initialized (stub)."));
}

void FCodexEditorModule::ShutdownModule()
{
	UE_LOG(LogCodexEditor, Log, TEXT("CodexEditor module shutdown."));
}

IMPLEMENT_MODULE(FCodexEditorModule, CodexEditor);
