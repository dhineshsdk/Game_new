#pragma once

// FNV-1a 64-bit hash, fed byte by byte in a fixed order (ADR 0005 item 8). Used for per-frame state
// hashes in replays and regression tests. Not cryptographic.

#include <cstdint>

namespace Codex
{
	struct FFnv1a64
	{
		static constexpr uint64_t OffsetBasis = 0xCBF29CE484222325ull;
		static constexpr uint64_t Prime = 0x100000001B3ull;

		uint64_t Value = OffsetBasis;

		constexpr void AddByte(uint8_t Byte)
		{
			Value = (Value ^ Byte) * Prime;
		}

		constexpr void AddBytes(const uint8_t* Bytes, uint64_t Count)
		{
			for (uint64_t I = 0; I < Count; ++I)
			{
				AddByte(Bytes[I]);
			}
		}
	};
}
