// Ported from the former UE automation tests Codex.Physics.BaselineParameters and StateSerialization (T-0007).
#include "CodexTest.h"

#include "CodexVehicleParams.h"
#include "CodexVehicleSimulationState.h"

using namespace Codex;
using namespace Codex::Dynamics;

CODEX_TEST(Params_Baseline)
{
	const FVehicleParams P;

	CODEX_CHECK_NEAR(P.TotalMassKg, P.SprungMassKg + 2.0 * P.UnsprungMassFrontKg + 2.0 * P.UnsprungMassRearKg, 1e-12);

	// 1300 kg * 9.80665 = 12748.6 N. Spec says 6759 / 5994 N; computed values are 6756.8 / 5991.9 N
	// (ADR 0005 inconsistency 3), inside this test's +/- 5 N.
	const double Weight = P.TotalMassKg * GravityMps2;
	CODEX_CHECK_NEAR(Weight * P.FrontWeightFraction, 6759.0, 5.0);
	CODEX_CHECK_NEAR(Weight * (1.0 - P.FrontWeightFraction), 5994.0, 5.0);

	CODEX_CHECK_NEAR(P.WheelbaseM, P.DistanceCGToFrontAxleM + P.DistanceCGToRearAxleM, 1e-9);

	const double StaticStabilityFactor = P.TrackWidthFrontM / (2.0 * P.CenterOfGravityHeightM);
	CODEX_CHECK_NEAR(StaticStabilityFactor, 1.412, 0.01);
}

CODEX_TEST(State_DefaultsAndCopy)
{
	FVehicleSimulationState A;
	CODEX_CHECK(A.SimulationFrame == 0);
	CODEX_CHECK(A.WorldRotation.W == 1.0);
	for (int Corner = 0; Corner < NumCorners; ++Corner)
	{
		CODEX_CHECK(A.TireRelaxedForceN[Corner] == (FVec2d { 0.0, 0.0 }));
		CODEX_CHECK(A.TirePressurePsi[Corner] == 32.0);
	}

	A.SimulationFrame = 120;
	A.SimulationTimeS = 1.0;
	A.EngineRpm = 3200.0;
	A.LinearVelocityMps = { 20.0, 0.0, 0.0 };
	A.FuelRemainingL = 42.5;

	const FVehicleSimulationState B = A; // by-value copy is what the PT marshaling relies on
	CODEX_CHECK(B.SimulationFrame == 120 && B.SimulationTimeS == 1.0);
	CODEX_CHECK(B.EngineRpm == 3200.0);
	CODEX_CHECK(B.LinearVelocityMps == A.LinearVelocityMps);
	CODEX_CHECK(B.FuelRemainingL == 42.5);
}
