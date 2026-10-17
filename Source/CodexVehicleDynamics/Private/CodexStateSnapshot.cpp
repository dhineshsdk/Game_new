#include "CodexStateSnapshot.h"

#include "CodexHash.h"

#include <bit>
#include <type_traits>

namespace Codex::Dynamics
{
	// Guard for the single field list: 544 payload bytes plus 8 bytes of alignment padding (after
	// SimulationFrame and after CurrentGear) on x64. If this fires, a field was added or changed:
	// update VisitStateFields, bump SnapshotVersion and re-pin the hashes in StateSnapshotTests.cpp.
	static_assert(sizeof(FVehicleSimulationState) == SnapshotPayloadBytes + 8,
		"FVehicleSimulationState changed: update VisitStateFields and bump SnapshotVersion");

	namespace
	{
		// Emits each field's little-endian bytes to Sink(uint8_t), independent of the host's byte order.
		template <typename TSink>
		struct FByteEmitter
		{
			TSink& Sink;

			void EmitU32(uint32_t V)
			{
				for (int Shift = 0; Shift < 32; Shift += 8)
				{
					Sink(static_cast<uint8_t>(V >> Shift));
				}
			}

			void EmitU64(uint64_t V)
			{
				for (int Shift = 0; Shift < 64; Shift += 8)
				{
					Sink(static_cast<uint8_t>(V >> Shift));
				}
			}

			void operator()(const uint32_t& V) { EmitU32(V); }
			void operator()(const int32_t& V) { EmitU32(std::bit_cast<uint32_t>(V)); }
			void operator()(const double& V) { EmitU64(std::bit_cast<uint64_t>(V)); }
		};

		template <typename TSink>
		void EmitSnapshot(const FVehicleSimulationState& State, TSink& Sink)
		{
			FByteEmitter<TSink> Emitter { Sink };
			Emitter.EmitU32(SnapshotMagic);
			Emitter.EmitU32(SnapshotVersion);
			Emitter.EmitU32(static_cast<uint32_t>(SnapshotPayloadBytes));
			VisitStateFields(State, Emitter);
		}

		uint32_t ReadU32(const uint8_t* P)
		{
			return static_cast<uint32_t>(P[0]) | (static_cast<uint32_t>(P[1]) << 8) | (static_cast<uint32_t>(P[2]) << 16)
				| (static_cast<uint32_t>(P[3]) << 24);
		}

		uint64_t ReadU64(const uint8_t* P)
		{
			return static_cast<uint64_t>(ReadU32(P)) | (static_cast<uint64_t>(ReadU32(P + 4)) << 32);
		}

		struct FFieldReader
		{
			const uint8_t* Cursor;

			void operator()(uint32_t& V)
			{
				V = ReadU32(Cursor);
				Cursor += 4;
			}
			void operator()(int32_t& V)
			{
				V = std::bit_cast<int32_t>(ReadU32(Cursor));
				Cursor += 4;
			}
			void operator()(double& V)
			{
				V = std::bit_cast<double>(ReadU64(Cursor));
				Cursor += 8;
			}
		};
	}

	size_t WriteSnapshot(const FVehicleSimulationState& State, uint8_t* OutBytes, size_t Capacity)
	{
		if (!OutBytes || Capacity < SnapshotTotalBytes)
		{
			return 0;
		}
		size_t Written = 0;
		auto Sink = [OutBytes, &Written](uint8_t Byte) { OutBytes[Written++] = Byte; };
		EmitSnapshot(State, Sink);
		return Written;
	}

	ESnapshotReadResult ReadSnapshot(const uint8_t* Bytes, size_t Size, FVehicleSimulationState& OutState)
	{
		if (!Bytes || Size < SnapshotHeaderBytes)
		{
			return ESnapshotReadResult::TooSmall;
		}
		if (ReadU32(Bytes) != SnapshotMagic)
		{
			return ESnapshotReadResult::BadMagic;
		}
		if (ReadU32(Bytes + 4) != SnapshotVersion)
		{
			return ESnapshotReadResult::UnsupportedVersion;
		}
		if (ReadU32(Bytes + 8) != SnapshotPayloadBytes || Size != SnapshotTotalBytes)
		{
			return ESnapshotReadResult::SizeMismatch;
		}

		FVehicleSimulationState Temp;
		VisitStateFields(Temp, FFieldReader { Bytes + SnapshotHeaderBytes });
		OutState = Temp;
		return ESnapshotReadResult::Ok;
	}

	uint64_t HashState(const FVehicleSimulationState& State)
	{
		FFnv1a64 Hash;
		auto Sink = [&Hash](uint8_t Byte) { Hash.AddByte(Byte); };
		EmitSnapshot(State, Sink);
		return Hash.Value;
	}
}
