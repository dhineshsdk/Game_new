#pragma once

// Versioned binary snapshot and FNV-1a hash of FVehicleSimulationState (ADR 0005 item 8, T-0008).
//
// Format v1, all little-endian:
//   u32 magic 'CXSS' (0x53535843) | u32 version (1) | u32 payload byte count | payload
// The payload is every field in VisitStateFields order: u32 and i32 as 4 bytes, double as its
// 8-byte IEEE-754 bit pattern. Structs are never memcpy'd, so padding never reaches the bytes.
// HashState hashes exactly the bytes WriteSnapshot produces (header included), so a format change
// changes every hash. Any change to the field list must bump SnapshotVersion.

#include "CodexExport.h"
#include "CodexVehicleSimulationState.h"

#include <cstddef>
#include <cstdint>

namespace Codex::Dynamics
{
	inline constexpr uint32_t SnapshotMagic = 0x53535843u; // "CXSS" when read as little-endian bytes
	inline constexpr uint32_t SnapshotVersion = 1u;
	inline constexpr size_t SnapshotHeaderBytes = 12;

	// The single field list. Writer, reader and hasher all go through it, so a field cannot be
	// serialized but left out of the hash, or read in a different order than it was written.
	// TState is FVehicleSimulationState or const FVehicleSimulationState; the visitor is called with
	// uint32_t&, int32_t& or double& (const-qualified to match TState).
	template <typename TState, typename TVisitor>
	constexpr void VisitStateFields(TState& S, TVisitor&& Visit)
	{
		auto Vec2 = [&Visit](auto& V) { Visit(V.X); Visit(V.Y); };
		auto Vec3 = [&Visit](auto& V) { Visit(V.X); Visit(V.Y); Visit(V.Z); };
		auto Quat = [&Visit](auto& Q) { Visit(Q.X); Visit(Q.Y); Visit(Q.Z); Visit(Q.W); };
		auto Corners = [&Visit](auto& Array) { for (auto& Value : Array) { Visit(Value); } };

		Visit(S.SimulationFrame);
		Visit(S.SimulationTimeS);

		Vec3(S.WorldLocationM);
		Quat(S.WorldRotation);
		Vec3(S.LinearVelocityMps);
		Vec3(S.AngularVelocityRadps);

		Visit(S.EngineRpm);
		Visit(S.ThrottlePosition);
		Visit(S.ClutchEngagement);
		Visit(S.CurrentGear);
		Visit(S.DriveshaftRpm);
		Visit(S.CarburetorMixtureTrim);

		Corners(S.SuspensionCompressionM);
		Corners(S.SuspensionVelocityMps);
		Corners(S.WheelAngularVelocityRadps);
		Corners(S.TireSlipAngleRad);
		Corners(S.TireSlipRatio);
		for (auto& Force : S.TireRelaxedForceN)
		{
			Vec2(Force);
		}
		Corners(S.TirePressurePsi);
		Corners(S.BrakeTemperatureC);

		Visit(S.FuelRemainingL);
		Vec2(S.FuelSloshOffsetM);
		Vec2(S.FuelSloshVelocityMps);
		Visit(S.AddedCargoMassKg);
		Vec3(S.AddedCargoLocalCoMM);
		Visit(S.EngineCoolantTempC);
		Visit(S.BatteryChargeAh);
		Visit(S.MechanicalOdometerM);
	}

	namespace Detail
	{
		constexpr size_t ComputePayloadBytes()
		{
			const FVehicleSimulationState State {};
			size_t Bytes = 0;
			VisitStateFields(State, [&Bytes](const auto& Field) { Bytes += sizeof(Field); });
			return Bytes;
		}
	}

	inline constexpr size_t SnapshotPayloadBytes = Detail::ComputePayloadBytes();
	inline constexpr size_t SnapshotTotalBytes = SnapshotHeaderBytes + SnapshotPayloadBytes;

	enum class ESnapshotReadResult : uint8_t
	{
		Ok,
		TooSmall,
		BadMagic,
		UnsupportedVersion,
		SizeMismatch,
	};

	// Writes the snapshot into OutBytes. Returns SnapshotTotalBytes, or 0 (nothing written) if
	// Capacity is too small. No allocation.
	CODEXVEHICLEDYNAMICS_API size_t WriteSnapshot(const FVehicleSimulationState& State, uint8_t* OutBytes, size_t Capacity);

	// Reads a snapshot. OutState is only modified when the result is Ok.
	CODEXVEHICLEDYNAMICS_API ESnapshotReadResult ReadSnapshot(const uint8_t* Bytes, size_t Size, FVehicleSimulationState& OutState);

	// FNV-1a 64 over exactly the bytes WriteSnapshot would produce.
	CODEXVEHICLEDYNAMICS_API uint64_t HashState(const FVehicleSimulationState& State);
}
