#include "CodexUI.h"

DEFINE_LOG_CATEGORY(LogCodexUI);

void FCodexUIModule::StartupModule()
{
	UE_LOG(LogCodexUI, Log, TEXT("CodexUI module initialized (stub)."));
}

void FCodexUIModule::ShutdownModule()
{
	UE_LOG(LogCodexUI, Log, TEXT("CodexUI module shutdown."));
}

IMPLEMENT_MODULE(FCodexUIModule, CodexUI);
