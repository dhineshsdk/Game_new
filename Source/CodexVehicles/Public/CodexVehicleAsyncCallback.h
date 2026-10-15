#pragma once

#include "CoreMinimal.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Chaos/SimCallbackObject.h"
#include "Chaos/Particle/ParticleUtilities.h"

/**
 * Double-buffered input data sent from Game Thread to Async Physics Thread (120 Hz).
 */
struct FCodexVehicleAsyncInput : public Chaos::FSimCallbackInput
{
	FPhysicsActorHandle ChassisActor = nullptr;
	float SteerInput = 0.0f;
	float ThrottleInput = 0.0f;
	float BrakeInput = 0.0f;
	float ClutchInput = 0.0f;
	int32 TargetGear = 1;

	void Reset()
	{
		ChassisActor = nullptr;
		SteerInput = 0.0f;
		ThrottleInput = 0.0f;
		BrakeInput = 0.0f;
		ClutchInput = 0.0f;
		TargetGear = 1;
	}
};

/**
 * 120 Hz Async Physics simulation callback for vehicle dynamics on Chaos rigid bodies.
 */
class CODEXVEHICLES_API FCodexVehicleAsyncCallback : public Chaos::TSimCallbackObject<FCodexVehicleAsyncInput, Chaos::FSimCallbackNoOutput>
{
protected:
	virtual void OnPreSimulate_Internal() override
	{
		const float Dt = GetDeltaTime_Internal(); // Target fixed 0.008333s (120 Hz)
		if (const FCodexVehicleAsyncInput* Input = GetConsumerInput_Internal())
		{
			CachedInput = *Input; // Persist control state across sub-steps
		}

		if (!CachedInput.ChassisActor)
		{
			return;
		}

		// Physics Thread (PT) ONLY access via GetPhysicsThreadAPI()
		if (Chaos::FRigidBodyHandle_Internal* Handle = CachedInput.ChassisActor->GetPhysicsThreadAPI())
		{
			const Chaos::FVec3 WorldCoM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(Handle);
			
			// Vehicle dynamics forces are computed in SI units (Newtons),
			// then converted to Chaos force units (kg*cm/s^2) by multiplying by 100.0f:
			// Handle->AddForce(ForceSI * 100.0f, false);
			// Handle->AddTorque(Chaos::FVec3::CrossProduct(WheelPosCm - WorldCoM, ForceSI * 100.0f), false);
		}
	}

private:
	FCodexVehicleAsyncInput CachedInput;
};
