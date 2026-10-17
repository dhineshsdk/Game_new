#include "CodexVehicles.h"
#include "CodexVehicleAsyncCallback.h" // compiled here so the Chaos callback is checked on every build

DEFINE_LOG_CATEGORY(LogCodexVehicles);

void FCodexVehiclesModule::StartupModule()
{
	UE_LOG(LogCodexVehicles, Log, TEXT("CodexVehicles module initialized (Chaos TSimCallbackObject Integration)."));
}

void FCodexVehiclesModule::ShutdownModule()
{
	UE_LOG(LogCodexVehicles, Log, TEXT("CodexVehicles module shutdown."));
}

IMPLEMENT_MODULE(FCodexVehiclesModule, CodexVehicles);
