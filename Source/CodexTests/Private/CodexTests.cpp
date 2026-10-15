#include "CodexTests.h"

DEFINE_LOG_CATEGORY(LogCodexTests);

void FCodexTestsModule::StartupModule()
{
	UE_LOG(LogCodexTests, Log, TEXT("CodexTests module initialized (Automation Unit Test Framework)."));
}

void FCodexTestsModule::ShutdownModule()
{
	UE_LOG(LogCodexTests, Log, TEXT("CodexTests module shutdown."));
}

IMPLEMENT_MODULE(FCodexTestsModule, CodexTests);
