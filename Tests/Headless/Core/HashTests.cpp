#include "CodexTest.h"

#include "CodexHash.h"

#include <cstring>

namespace
{
	uint64_t HashString(const char* Text)
	{
		Codex::FFnv1a64 Hash;
		Hash.AddBytes(reinterpret_cast<const uint8_t*>(Text), std::strlen(Text));
		return Hash.Value;
	}
}

// Published FNV-1a 64 test vectors (Fowler, Noll, Vo reference test suite).
CODEX_TEST(Hash_Fnv1a64Vectors)
{
	CODEX_CHECK(HashString("") == 0xCBF29CE484222325ull);
	CODEX_CHECK(HashString("a") == 0xAF63DC4C8601EC8Cull);
	CODEX_CHECK(HashString("foobar") == 0x85944171F73967E8ull);
}
