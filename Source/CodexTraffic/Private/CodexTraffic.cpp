#include "CodexTraffic.h"

DEFINE_LOG_CATEGORY(LogCodexTraffic);

void FCodexTrafficModule::StartupModule()
{
	UE_LOG(LogCodexTraffic, Log, TEXT("CodexTraffic module initialized (stub)."));
}

void FCodexTrafficModule::ShutdownModule()
{
	UE_LOG(LogCodexTraffic, Log, TEXT("CodexTraffic module shutdown."));
}

IMPLEMENT_MODULE(FCodexTrafficModule, CodexTraffic);
