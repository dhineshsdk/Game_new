#pragma once

#include "CoreMinimal.h"
#include "CodexVehicleSimulationState.generated.h"

/**
 * Single serializable vehicle simulation state struct.
 * Completely captures the 120 Hz physical and condition state for deterministic replay,
 * automated unit tests, and telemetry logging.
 */
USTRUCT(BlueprintType)
struct CODEXVEHICLEDYNAMICS_API FCodexVehicleSimulationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	uint32 SimulationFrame = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	double SimulationTimeSeconds = 0.0;

	// Rigid Body State (SI Units internally: m, m/s, rad/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBody")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBody")
	FQuat WorldRotation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBody")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBody")
	FVector AngularVelocity = FVector::ZeroVector;

	// Powertrain State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float EngineRPM = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float ThrottlePosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float ClutchEngagement = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	int32 CurrentGear = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float DriveshaftRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float CarburetorMixtureTrim = 0.0f;

	// Corner States [0:FL, 1:FR, 2:RL, 3:RR]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionCompression[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionVelocity[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	float WheelAngularVelocity[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	float TireSlipAngle[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	float TireSlipRatio[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	FVector2f TireRelaxedForce[4];

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	float TirePressurePSI[4] = { 32.0f, 32.0f, 32.0f, 32.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float BrakeThermalStateC[4] = { 20.0f, 20.0f, 20.0f, 20.0f };

	// Dynamic Mass, Slosh & Condition State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float FuelRemainingLiters = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FVector2f FuelSloshOffset = FVector2f::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FVector2f FuelSloshVelocity = FVector2f::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float AddedCargoMassKg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FVector AddedCargoLocalCoM = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float EngineCoolantTempC = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float BatteryChargeAh = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	double MechanicalOdometerMeters = 184320000.0;
};
