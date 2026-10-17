#pragma once

// Baseline mechanical constants for the 1985-1990 RWD sedan (~1300 kg).
// Values from docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md section 1. Pure C++ (ADR 0005); the
// editor-tunable UE mirror is added in CodexVehicles when the Physics Lab needs it (T-0009).

#include "CodexMath.h"

#include <type_traits>

namespace Codex::Dynamics
{
	struct FVehicleParams
	{
		// Mass
		double TotalMassKg = 1300.0;
		double SprungMassKg = 1140.0;
		double UnsprungMassFrontKg = 38.0;
		double UnsprungMassRearKg = 42.0;
		double FrontWeightFraction = 0.53; // 0..1 share of total weight on the front axle
		double CenterOfGravityHeightM = 0.510;

		// Geometry
		double WheelbaseM = 2.620;
		double TrackWidthFrontM = 1.440;
		double TrackWidthRearM = 1.450;
		double DistanceCGToFrontAxleM = 1.231;
		double DistanceCGToRearAxleM = 1.389;

		// Principal moments of inertia (kg*m^2): roll, pitch, yaw
		FVec3d InertiaTensorKgM2 { 440.0, 2150.0, 2280.0 };

		// Suspension (wheel rates)
		double WheelSpringRateFrontNpm = 26000.0;
		double WheelSpringRateRearNpm = 29000.0;
		double DamperRateBumpFrontNspm = 2200.0;
		double DamperRateBumpRearNspm = 2400.0;
		double DamperRateReboundFrontNspm = 3400.0;
		double DamperRateReboundRearNspm = 3800.0;
	};

	static_assert(std::is_trivially_copyable_v<FVehicleParams>);
}
