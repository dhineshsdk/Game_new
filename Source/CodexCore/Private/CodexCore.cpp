#include "CodexCore.h"

DEFINE_LOG_CATEGORY(LogCodexCore);

void FCodexCoreModule::StartupModule()
{
	UE_LOG(LogCodexCore, Log, TEXT("CodexCore module initialized."));
}

void FCodexCoreModule::ShutdownModule()
{
	UE_LOG(LogCodexCore, Log, TEXT("CodexCore module shutdown."));
}

IMPLEMENT_MODULE(FCodexCoreModule, CodexCore);
