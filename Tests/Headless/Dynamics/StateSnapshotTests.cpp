// T-0008 acceptance tests: versioned binary snapshot and FNV-1a hash of the simulation state.
#include "CodexTest.h"

#include "CodexHash.h"
#include "CodexMath.h"
#include "CodexStateSnapshot.h"
#include "CodexTireModel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <new>

using namespace Codex;
using namespace Codex::Dynamics;

namespace
{
	// Pinned hash of the default state in format v1. A deliberate format change must bump
	// SnapshotVersion and update this value; an accidental one fails here.
	constexpr uint64_t DefaultStateHashV1 = 0x907391139BCFD429ull; // cross-checked with an independent Python struct.pack encoder

	// Sets every field to a distinct, non-default value.
	struct FDistinctFiller
	{
		uint32_t Counter = 1;
		void operator()(uint32_t& V) { V = 1000u + Counter++; }
		void operator()(int32_t& V) { V = -static_cast<int32_t>(Counter++); }
		void operator()(double& V) { V = 0.5 + 1.25 * Counter++; }
	};

	// Perturbs only field number Target (in VisitStateFields order).
	struct FFieldPerturber
	{
		int Target;
		int Index = 0;
		void operator()(uint32_t& V) { if (Index++ == Target) { V += 1u; } }
		void operator()(int32_t& V) { if (Index++ == Target) { V += 1; } }
		void operator()(double& V) { if (Index++ == Target) { V = std::nextafter(V, 1e300); } }
	};

	int CountFields()
	{
		FVehicleSimulationState State;
		int Count = 0;
		VisitStateFields(State, [&Count](const auto&) { ++Count; });
		return Count;
	}

	FVehicleSimulationState MakeDistinctState()
	{
		FVehicleSimulationState State;
		VisitStateFields(State, FDistinctFiller {});
		return State;
	}

	bool FieldsBitwiseEqual(const FVehicleSimulationState& A, const FVehicleSimulationState& B)
	{
		uint8_t BytesA[SnapshotTotalBytes];
		uint8_t BytesB[SnapshotTotalBytes];
		return WriteSnapshot(A, BytesA, sizeof(BytesA)) == SnapshotTotalBytes && WriteSnapshot(B, BytesB, sizeof(BytesB)) == SnapshotTotalBytes
			&& std::memcmp(BytesA, BytesB, SnapshotTotalBytes) == 0;
	}
}

static_assert(SnapshotPayloadBytes == 544, "format v1 payload size changed: bump SnapshotVersion");
static_assert(SnapshotTotalBytes == 556);
static_assert(sizeof(FVehicleSimulationState) > SnapshotPayloadBytes, "the struct has padding, which the padding test relies on");

CODEX_TEST(Snapshot_HeaderAndSize)
{
	uint8_t Bytes[SnapshotTotalBytes + 8] = {};
	const FVehicleSimulationState State;
	CODEX_CHECK(WriteSnapshot(State, Bytes, sizeof(Bytes)) == 556);
	const uint8_t Expected[12] = { 'C', 'X', 'S', 'S', 1, 0, 0, 0, 0x20, 0x02, 0, 0 };
	CODEX_CHECK(std::memcmp(Bytes, Expected, sizeof(Expected)) == 0);
	// SimulationFrame = 0 then SimulationTimeS = 0.0: twelve zero bytes.
	for (int I = 12; I < 24; ++I)
	{
		CODEX_CHECK(Bytes[I] == 0);
	}
	// WorldRotation.W = 1.0 sits at 12 + 4 + 8 + 24 + 24 = 72 and is 0x3FF0000000000000 little-endian.
	const uint8_t One[8] = { 0, 0, 0, 0, 0, 0, 0xF0, 0x3F };
	CODEX_CHECK(std::memcmp(Bytes + 72, One, 8) == 0);
	CODEX_CHECK(CountFields() == 1 + 1 + 3 + 4 + 3 + 3 + 6 + 5 * 4 + 4 * 2 + 4 + 4 + 1 + 2 + 2 + 1 + 3 + 3);
}

CODEX_TEST(Snapshot_RoundTrip)
{
	const FVehicleSimulationState Original = MakeDistinctState();
	uint8_t Bytes[SnapshotTotalBytes];
	CODEX_CHECK(WriteSnapshot(Original, Bytes, sizeof(Bytes)) == SnapshotTotalBytes);

	FVehicleSimulationState Loaded;
	CODEX_CHECK(ReadSnapshot(Bytes, sizeof(Bytes), Loaded) == ESnapshotReadResult::Ok);
	CODEX_CHECK(FieldsBitwiseEqual(Original, Loaded));
	CODEX_CHECK(HashState(Original) == HashState(Loaded));
	CODEX_CHECK(Loaded.SimulationFrame == Original.SimulationFrame);
	CODEX_CHECK(Loaded.CurrentGear == Original.CurrentGear && Loaded.CurrentGear < 0);
	CODEX_CHECK(Loaded.TireRelaxedForceN[3] == Original.TireRelaxedForceN[3]);
	CODEX_CHECK(Loaded.MechanicalOdometerM == Original.MechanicalOdometerM);

	// Special values survive bit for bit.
	FVehicleSimulationState Special;
	Special.EngineRpm = -0.0;
	Special.FuelRemainingL = std::numeric_limits<double>::infinity();
	Special.BatteryChargeAh = std::numeric_limits<double>::denorm_min();
	CODEX_CHECK(WriteSnapshot(Special, Bytes, sizeof(Bytes)) == SnapshotTotalBytes);
	CODEX_CHECK(ReadSnapshot(Bytes, sizeof(Bytes), Loaded) == ESnapshotReadResult::Ok);
	CODEX_CHECK(std::signbit(Loaded.EngineRpm) && Loaded.EngineRpm == 0.0);
	CODEX_CHECK(FieldsBitwiseEqual(Special, Loaded));
}

CODEX_TEST(Snapshot_HashMatchesWrittenBytes)
{
	const FVehicleSimulationState State = MakeDistinctState();
	uint8_t Bytes[SnapshotTotalBytes];
	WriteSnapshot(State, Bytes, sizeof(Bytes));
	FFnv1a64 Hash;
	Hash.AddBytes(Bytes, sizeof(Bytes));
	CODEX_CHECK(Hash.Value == HashState(State));
}

CODEX_TEST(Snapshot_EveryFieldChangesHash)
{
	const FVehicleSimulationState Base = MakeDistinctState();
	const uint64_t BaseHash = HashState(Base);
	const int NumFields = CountFields();
	for (int Field = 0; Field < NumFields; ++Field)
	{
		FVehicleSimulationState Changed = Base;
		VisitStateFields(Changed, FFieldPerturber { Field });
		if (HashState(Changed) == BaseHash)
		{
			char Message[64];
			std::snprintf(Message, sizeof(Message), "field %d does not affect the hash", Field);
			CodexTest::ReportFailure(__FILE__, __LINE__, Message);
		}
	}
}

CODEX_TEST(Snapshot_PaddingNeverHashed)
{
	alignas(FVehicleSimulationState) unsigned char DirtyMemory[sizeof(FVehicleSimulationState)];
	alignas(FVehicleSimulationState) unsigned char CleanMemory[sizeof(FVehicleSimulationState)];
	std::memset(DirtyMemory, 0xAB, sizeof(DirtyMemory));
	std::memset(CleanMemory, 0x00, sizeof(CleanMemory));
	// Default-initialisation (no "()") runs the member initialisers but leaves padding untouched.
	const FVehicleSimulationState* Dirty = new (DirtyMemory) FVehicleSimulationState;
	const FVehicleSimulationState* Clean = new (CleanMemory) FVehicleSimulationState;
	CODEX_CHECK(std::memcmp(DirtyMemory, CleanMemory, sizeof(DirtyMemory)) != 0); // and it differs here
	CODEX_CHECK(HashState(*Dirty) == HashState(*Clean));
	CODEX_CHECK(HashState(*Clean) == HashState(FVehicleSimulationState {}));
}

CODEX_TEST(Snapshot_ReadErrors)
{
	const FVehicleSimulationState Source = MakeDistinctState();
	uint8_t Good[SnapshotTotalBytes];
	WriteSnapshot(Source, Good, sizeof(Good));

	const FVehicleSimulationState Sentinel {};
	const uint64_t SentinelHash = HashState(Sentinel);
	FVehicleSimulationState Out = Sentinel;

	CODEX_CHECK(ReadSnapshot(nullptr, sizeof(Good), Out) == ESnapshotReadResult::TooSmall);
	CODEX_CHECK(ReadSnapshot(Good, 11, Out) == ESnapshotReadResult::TooSmall);

	uint8_t Bad[SnapshotTotalBytes + 1];
	std::memcpy(Bad, Good, sizeof(Good));
	Bad[0] = 'Z';
	CODEX_CHECK(ReadSnapshot(Bad, sizeof(Good), Out) == ESnapshotReadResult::BadMagic);

	std::memcpy(Bad, Good, sizeof(Good));
	Bad[4] = 2;
	CODEX_CHECK(ReadSnapshot(Bad, sizeof(Good), Out) == ESnapshotReadResult::UnsupportedVersion);

	std::memcpy(Bad, Good, sizeof(Good));
	Bad[8] = 0x21;
	CODEX_CHECK(ReadSnapshot(Bad, sizeof(Good), Out) == ESnapshotReadResult::SizeMismatch);

	std::memcpy(Bad, Good, sizeof(Good));
	Bad[SnapshotTotalBytes] = 0;
	CODEX_CHECK(ReadSnapshot(Bad, sizeof(Bad), Out) == ESnapshotReadResult::SizeMismatch);
	CODEX_CHECK(ReadSnapshot(Good, sizeof(Good) - 1, Out) == ESnapshotReadResult::SizeMismatch);

	CODEX_CHECK(HashState(Out) == SentinelHash); // untouched by every failed read

	// Writing into a buffer that is too small writes nothing.
	uint8_t Small[SnapshotTotalBytes - 1];
	std::memset(Small, 0x5A, sizeof(Small));
	CODEX_CHECK(WriteSnapshot(Source, Small, sizeof(Small)) == 0);
	CODEX_CHECK(WriteSnapshot(Source, nullptr, 4096) == 0);
	bool bUntouched = true;
	for (uint8_t B : Small)
	{
		bUntouched = bUntouched && B == 0x5A;
	}
	CODEX_CHECK(bUntouched);
}

CODEX_TEST(Snapshot_PinnedDefaultHash)
{
	const uint64_t Hash = HashState(FVehicleSimulationState {});
	std::printf("    default state hash v1: 0x%016llX\n", static_cast<unsigned long long>(Hash));
	CODEX_CHECK(Hash == DefaultStateHashV1);
}

namespace
{
	// Scripted 10-minute run at 120 Hz: a slalom on dry asphalt using the tyre model at each corner.
	// It exercises the hash over a long, changing state; it is not the vehicle solver (T-0201).
	uint64_t RunScriptedDrive()
	{
		constexpr int Steps = 72000;
		constexpr double Dt = 1.0 / 120.0;
		const FTireSurfaceCoeffs Dry { 1.00, 10.00, 1.30, -1.00, 10.00, 1.30, -1.00 };
		const FTireParams TireParams;
		const FTirePeakSlips Peak = ComputePeakSlips(Dry);
		const double CornerLoadN[NumCorners] = { 3378.4, 3378.4, 2996.0, 2996.0 };

		FVehicleSimulationState State;
		FFnv1a64 Chain;
		for (int Step = 0; Step < Steps; ++Step)
		{
			const double T = Step * Dt;
			const double Steer = 0.06 * std::sin(2.0 * Pi * 0.25 * T);
			const double Throttle = 0.5 + 0.5 * std::sin(2.0 * Pi * 0.05 * T);
			State.SimulationFrame = static_cast<uint32_t>(Step + 1);
			State.SimulationTimeS = (Step + 1) * Dt;
			State.ThrottlePosition = Throttle;
			const double Vx = 15.0 + 10.0 * Throttle;
			State.LinearVelocityMps = { Vx, 0.0, 0.0 };
			for (int Corner = 0; Corner < NumCorners; ++Corner)
			{
				const double Alpha = Corner < 2 ? Steer : 0.5 * Steer;
				const double Kappa = Corner >= 2 ? 0.02 * Throttle : 0.0;
				State.TireSlipAngleRad[Corner] = Alpha;
				State.TireSlipRatio[Corner] = Kappa;
				const FTireForces Steady = CombinedForces(Kappa, Alpha, CornerLoadN[Corner], TireParams, Dry, Peak);
				FVec2d& Relaxed = State.TireRelaxedForceN[Corner];
				Relaxed.X = Steady.FxN;
				Relaxed.Y = StepLateralRelaxation(Relaxed.Y, Steady.FyN, Vx, Dt, TireParams);
			}
			State.MechanicalOdometerM += Vx * Dt;
			const uint64_t FrameHash = HashState(State);
			for (int Shift = 0; Shift < 64; Shift += 8)
			{
				Chain.AddByte(static_cast<uint8_t>(FrameHash >> Shift));
			}
		}
		return Chain.Value;
	}
}

CODEX_TEST(Snapshot_DeterministicTenMinuteRun)
{
	const uint64_t First = RunScriptedDrive();
	const uint64_t Second = RunScriptedDrive();
	std::printf("    72,000-step chained hash: 0x%016llX\n", static_cast<unsigned long long>(First));
	CODEX_CHECK(First == Second);
	CODEX_CHECK(First != FFnv1a64 {}.Value);
}
