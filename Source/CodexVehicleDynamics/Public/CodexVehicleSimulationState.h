#pragma once

// Single vehicle simulation state. Captures the 120 Hz physical and condition state for
// deterministic replay, headless tests and telemetry. Pure C++ (ADR 0005): trivially copyable
// so it can be copied by value across the game/physics thread boundary. Serialization is an
// explicit versioned snapshot (T-0008), never a memcpy of this struct.
// Corner arrays are indexed [0:FL, 1:FR, 2:RL, 3:RR]. SI units throughout.

#include "CodexMath.h"

#include <cstdint>
#include <type_traits>

namespace Codex::Dynamics
{
	inline constexpr int NumCorners = 4;

	struct FVehicleSimulationState
	{
		uint32_t SimulationFrame = 0;
		double SimulationTimeS = 0.0;

		// Rigid body (world frame)
		FVec3d WorldLocationM;
		FQuatd WorldRotation;
		FVec3d LinearVelocityMps;
		FVec3d AngularVelocityRadps;

		// Powertrain
		double EngineRpm = 800.0;
		double ThrottlePosition = 0.0;
		double ClutchEngagement = 1.0;
		int32_t CurrentGear = 1;
		double DriveshaftRpm = 0.0;
		double CarburetorMixtureTrim = 0.0;

		// Corners
		double SuspensionCompressionM[NumCorners] = { 0.0, 0.0, 0.0, 0.0 };
		double SuspensionVelocityMps[NumCorners] = { 0.0, 0.0, 0.0, 0.0 };
		double WheelAngularVelocityRadps[NumCorners] = { 0.0, 0.0, 0.0, 0.0 };
		double TireSlipAngleRad[NumCorners] = { 0.0, 0.0, 0.0, 0.0 };
		double TireSlipRatio[NumCorners] = { 0.0, 0.0, 0.0, 0.0 };
		FVec2d TireRelaxedForceN[NumCorners] = {}; // X: longitudinal, Y: lateral
		double TirePressurePsi[NumCorners] = { 32.0, 32.0, 32.0, 32.0 };
		double BrakeTemperatureC[NumCorners] = { 20.0, 20.0, 20.0, 20.0 };

		// Dynamic mass, slosh and condition
		double FuelRemainingL = 50.0;
		FVec2d FuelSloshOffsetM;
		FVec2d FuelSloshVelocityMps;
		double AddedCargoMassKg = 0.0;
		FVec3d AddedCargoLocalCoMM;
		double EngineCoolantTempC = 85.0;
		double BatteryChargeAh = 55.0;
		double MechanicalOdometerM = 184320000.0;
	};

	static_assert(std::is_trivially_copyable_v<FVehicleSimulationState>);
}
