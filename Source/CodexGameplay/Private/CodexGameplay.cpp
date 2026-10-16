#include "CodexGameplay.h"

DEFINE_LOG_CATEGORY(LogCodexGameplay);

void FCodexGameplayModule::StartupModule()
{
	UE_LOG(LogCodexGameplay, Log, TEXT("CodexGameplay module initialized (stub)."));
}

void FCodexGameplayModule::ShutdownModule()
{
	UE_LOG(LogCodexGameplay, Log, TEXT("CodexGameplay module shutdown."));
}

IMPLEMENT_MODULE(FCodexGameplayModule, CodexGameplay);
