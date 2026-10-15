#pragma once

#include "CoreMinimal.h"
#include "CodexVehicleParams.generated.h"

/**
 * Baseline mechanical constants for 1985-1990 RWD Sedan (~1300 kg).
 * Reconciled from TECHNICAL_PHYSICS_AND_GIS_SPEC.md.
 */
USTRUCT(BlueprintType)
struct CODEXVEHICLEDYNAMICS_API FCodexVehicleParams
{
	GENERATED_BODY()

	// Mass Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float TotalMassKg = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float SprungMassKg = 1140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float UnsprungMassFrontKg = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float UnsprungMassRearKg = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float FrontWeightDistributionPercent = 0.53f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
	float CenterOfGravityHeightMeters = 0.510f;

	// Dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float WheelbaseMeters = 2.620f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float TrackWidthFrontMeters = 1.440f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float TrackWidthRearMeters = 1.450f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float DistanceCGToFrontAxleMeters = 1.231f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float DistanceCGToRearAxleMeters = 1.389f;

	// Moments of Inertia (kg*m^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inertia")
	FVector InertiaTensorKgM2 = FVector(440.0f, 2150.0f, 2280.0f); // Roll, Pitch, Yaw

	// Suspension Rates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float WheelSpringRateFrontNm = 26000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float WheelSpringRateRearNm = 29000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperRateBumpFrontNsm = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperRateBumpRearNsm = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperRateReboundFrontNsm = 3400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperRateReboundRearNsm = 3800.0f;
};
