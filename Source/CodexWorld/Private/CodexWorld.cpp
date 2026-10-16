#include "CodexWorld.h"

DEFINE_LOG_CATEGORY(LogCodexWorld);

void FCodexWorldModule::StartupModule()
{
	UE_LOG(LogCodexWorld, Log, TEXT("CodexWorld module initialized (stub)."));
}

void FCodexWorldModule::ShutdownModule()
{
	UE_LOG(LogCodexWorld, Log, TEXT("CodexWorld module shutdown."));
}

IMPLEMENT_MODULE(FCodexWorldModule, CodexWorld);
