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
    }
};

class FCodexVehicleAsyncCallback : public Chaos::TSimCallbackObject<FCodexVehicleAsyncInput, Chaos::FSimCallbackNoOutput>
{
protected:
    virtual void OnPreSimulate_Internal() override
    {
        const float Dt = GetDeltaTime_Internal(); // Fixed 0.008333s (120 Hz)
        if (const FCodexVehicleAsyncInput* Input = GetConsumerInput_Internal())
        {
            CachedInput = *Input; // Persist controls across multi-substep frames
        }
        if (!CachedInput.ChassisActor) return;

        if (Chaos::FRigidBodyHandle_Internal* Handle = CachedInput.ChassisActor->GetPhysicsThreadAPI())
        {
            const Chaos::FVec3 WorldCoM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(Handle);
            // Solve 120Hz tire/suspension forces in SI (Newtons), then scale by 100.0f for Chaos (kg*cm/s^2):
            // Handle->AddForce(ForceSI * 100.0f, false);
            // Handle->AddTorque(Chaos::FVec3::CrossProduct(WheelPosCm - WorldCoM, ForceSI * 100.0f), false);
        }
    }
private:
    FCodexVehicleAsyncInput CachedInput;
};