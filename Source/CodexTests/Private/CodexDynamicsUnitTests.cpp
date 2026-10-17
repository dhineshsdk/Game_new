// Integration smoke test: the pure dynamics headers compile inside a UE module and behave the same.
// Unit tests for the pure modules run in the headless harness (tools/test_headless.ps1, ADR 0005).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CodexTireModel.h"
#include "CodexVehicleParams.h"
#include "CodexVehicleSimulationState.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCodexPureDynamicsSmokeTest, "Codex.Physics.PureDynamicsSmoke", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCodexPureDynamicsSmokeTest::RunTest(const FString& Parameters)
{
	static_assert(std::is_trivially_copyable_v<Codex::Dynamics::FVehicleSimulationState>);

	const Codex::Dynamics::FVehicleParams VehicleParams;
	TestEqual(TEXT("Baseline mass"), VehicleParams.TotalMassKg, 1300.0);

	// Same build flags question as the harness: the UBT-built model must match the oracle's golden point.
	const Codex::Dynamics::FTireSurfaceCoeffs DryAsphalt { 1.00, 10.00, 1.30, -1.00, 10.00, 1.30, -1.00 };
	const Codex::Dynamics::FTireParams TireParams;
	const double Fy = Codex::Dynamics::PureLateralForce(0.05, 3200.0, TireParams, DryAsphalt);
	TestNearlyEqual(TEXT("Dry Fy at 0.05 rad, 3200 N matches oracle"), Fy, 1911.015950, 1e-3);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
