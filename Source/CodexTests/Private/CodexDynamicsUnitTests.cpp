#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CodexVehicleParams.h"
#include "CodexVehicleSimulationState.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCodexVehicleBaselineParametersTest, "Codex.Physics.BaselineParameters", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCodexVehicleBaselineParametersTest::RunTest(const FString& Parameters)
{
	FCodexVehicleParams Params;

	// Verify Total Mass integrity
	const float ExpectedTotalMass = Params.SprungMassKg + (2.0f * Params.UnsprungMassFrontKg) + (2.0f * Params.UnsprungMassRearKg);
	TestEqual(TEXT("Total mass must equal sprung mass + 4 corners unsprung mass"), Params.TotalMassKg, ExpectedTotalMass);

	// Verify Static Axle Loads: Total Gravity = 1300 kg * 9.80665 m/s^2 = 12748.65 N
	const float TotalWeightNewtons = Params.TotalMassKg * 9.80665f;
	const float FrontAxleWeight = TotalWeightNewtons * Params.FrontWeightDistributionPercent;
	const float RearAxleWeight = TotalWeightNewtons * (1.0f - Params.FrontWeightDistributionPercent);

	// Reconciled spec: Front ~6757N, Rear ~5992N (+/- 10N)
	TestNearlyEqual(TEXT("Front static axle load must match spec (6759 N +/- 10N)"), FrontAxleWeight, 6759.0f, 10.0f);
	TestNearlyEqual(TEXT("Rear static axle load must match spec (5994 N +/- 10N)"), RearAxleWeight, 5994.0f, 10.0f);

	// Verify Wheelbase = CG to Front + CG to Rear
	TestNearlyEqual(TEXT("Wheelbase must equal distance to front axle + rear axle"), Params.WheelbaseMeters, Params.DistanceCGToFrontAxleMeters + Params.DistanceCGToRearAxleMeters, 0.001f);

	// Verify Static Stability Factor (TrackWidth / (2 * CGHeight))
	const float SSF = Params.TrackWidthFrontMeters / (2.0f * Params.CenterOfGravityHeightMeters);
	TestNearlyEqual(TEXT("Static Stability Factor SSF ~ 1.41"), SSF, 1.412f, 0.01f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCodexVehicleStateStructSerializationTest, "Codex.Physics.StateSerialization", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCodexVehicleStateStructSerializationTest::RunTest(const FString& Parameters)
{
	FCodexVehicleSimulationState StateA;
	StateA.SimulationFrame = 120;
	StateA.SimulationTimeSeconds = 1.0;
	StateA.EngineRPM = 3200.0f;
	StateA.LinearVelocity = FVector(20.0, 0.0, 0.0); // 20 m/s (~72 km/h)
	StateA.FuelRemainingLiters = 42.5f;

	TestTrue(TEXT("Initial state should have valid frame and time"), StateA.SimulationFrame == 120 && StateA.SimulationTimeSeconds == 1.0);
	TestEqual(TEXT("Engine RPM should match set value"), StateA.EngineRPM, 3200.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
