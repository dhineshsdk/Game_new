#include "CodexVehicleDynamics.h"

DEFINE_LOG_CATEGORY(LogCodexVehicleDynamics);

void FCodexVehicleDynamicsModule::StartupModule()
{
	UE_LOG(LogCodexVehicleDynamics, Log, TEXT("CodexVehicleDynamics module initialized (Pure C++ Dynamics Solver)."));
}

void FCodexVehicleDynamicsModule::ShutdownModule()
{
	UE_LOG(LogCodexVehicleDynamics, Log, TEXT("CodexVehicleDynamics module shutdown."));
}

IMPLEMENT_MODULE(FCodexVehicleDynamicsModule, CodexVehicleDynamics);
