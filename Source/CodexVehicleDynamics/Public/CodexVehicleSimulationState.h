#pragma once

#include "CoreMinimal.h"
#include "CodexVehicleSimulationState.generated.h"

/**
 * Single serializable vehicle simulation state struct.
 * Completely captures the 120 Hz physical and condition state for deterministic replay,
 * automated unit tests, and telemetry logging.
 */
USTRUCT()
struct CODEXVEHICLEDYNAMICS_API FCodexVehicleSimulationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,Category = "Simulation")
	uint32 SimulationFrame = 0;

	UPROPERTY(EditAnywhere,Category = "Simulation")
	double SimulationTimeSeconds = 0.0;

	// Rigid Body State (SI Units internally: m, m/s, rad/s)
	UPROPERTY(EditAnywhere,Category = "RigidBody")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere,Category = "RigidBody")
	FQuat WorldRotation = FQuat::Identity;

	UPROPERTY(EditAnywhere,Category = "RigidBody")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere,Category = "RigidBody")
	FVector AngularVelocity = FVector::ZeroVector;

	// Powertrain State
	UPROPERTY(EditAnywhere,Category = "Powertrain")
	float EngineRPM = 800.0f;

	UPROPERTY(EditAnywhere,Category = "Powertrain")
	float ThrottlePosition = 0.0f;

	UPROPERTY(EditAnywhere,Category = "Powertrain")
	float ClutchEngagement = 1.0f;

	UPROPERTY(EditAnywhere,Category = "Powertrain")
	int32 CurrentGear = 1;

	UPROPERTY(EditAnywhere,Category = "Powertrain")
	float DriveshaftRPM = 0.0f;

	UPROPERTY(EditAnywhere,Category = "Powertrain")
	float CarburetorMixtureTrim = 0.0f;

	// Corner States [0:FL, 1:FR, 2:RL, 3:RR]
	UPROPERTY(EditAnywhere,Category = "Suspension")
	float SuspensionCompression[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere,Category = "Suspension")
	float SuspensionVelocity[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere,Category = "Tires")
	float WheelAngularVelocity[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere,Category = "Tires")
	float TireSlipAngle[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere,Category = "Tires")
	float TireSlipRatio[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere,Category = "Tires")
	FVector2f TireRelaxedForce[4] = { FVector2f::ZeroVector, FVector2f::ZeroVector, FVector2f::ZeroVector, FVector2f::ZeroVector };

	UPROPERTY(EditAnywhere,Category = "Tires")
	float TirePressurePSI[4] = { 32.0f, 32.0f, 32.0f, 32.0f };

	UPROPERTY(EditAnywhere,Category = "Brakes")
	float BrakeThermalStateC[4] = { 20.0f, 20.0f, 20.0f, 20.0f };

	// Dynamic Mass, Slosh & Condition State
	UPROPERTY(EditAnywhere,Category = "Condition")
	float FuelRemainingLiters = 50.0f;

	UPROPERTY(EditAnywhere,Category = "Condition")
	FVector2f FuelSloshOffset = FVector2f::ZeroVector;

	UPROPERTY(EditAnywhere,Category = "Condition")
	FVector2f FuelSloshVelocity = FVector2f::ZeroVector;

	UPROPERTY(EditAnywhere,Category = "Condition")
	float AddedCargoMassKg = 0.0f;

	UPROPERTY(EditAnywhere,Category = "Condition")
	FVector AddedCargoLocalCoM = FVector::ZeroVector;

	UPROPERTY(EditAnywhere,Category = "Condition")
	float EngineCoolantTempC = 85.0f;

	UPROPERTY(EditAnywhere,Category = "Condition")
	float BatteryChargeAh = 55.0f;

	UPROPERTY(EditAnywhere,Category = "Condition")
	double MechanicalOdometerMeters = 184320000.0;
};
