#include "CodexTest.h"

#include <chrono>
#include <cstdio>
#include <cstring>

namespace CodexTest
{
	namespace
	{
		struct FEntry
		{
			const char* Name;
			FTestFn Fn;
			bool bSelfTestOnly;
		};

		constexpr int MaxTests = 512;
		FEntry GTests[MaxTests];
		int GNumTests = 0;
		int GFailures = 0;
		int GCurrentFailures = 0;
	}

	FRegistrar::FRegistrar(const char* Name, FTestFn Fn, bool bSelfTestOnly)
	{
		if (GNumTests < MaxTests)
		{
			GTests[GNumTests++] = { Name, Fn, bSelfTestOnly };
		}
	}

	void ReportFailure(const char* File, int Line, const char* Message)
	{
		++GFailures;
		++GCurrentFailures;
		if (GCurrentFailures <= 10)
		{
			std::printf("    FAIL %s(%d): %s\n", File, Line, Message);
		}
	}

	int FailureCount()
	{
		return GFailures;
	}
}

int main(int Argc, char** Argv)
{
	using namespace CodexTest;

	bool bRunSelfTests = false;
	const char* Filter = nullptr;
	for (int I = 1; I < Argc; ++I)
	{
		if (std::strcmp(Argv[I], "--selftest-fail") == 0)
		{
			bRunSelfTests = true;
		}
		else
		{
			Filter = Argv[I];
		}
	}

	int Passed = 0;
	int Failed = 0;
	const auto Start = std::chrono::steady_clock::now();
	for (int I = 0; I < GNumTests; ++I)
	{
		const FEntry& Test = GTests[I];
		if (Test.bSelfTestOnly != bRunSelfTests || (Filter && !std::strstr(Test.Name, Filter)))
		{
			continue;
		}
		GCurrentFailures = 0;
		Test.Fn();
		if (GCurrentFailures == 0)
		{
			++Passed;
			std::printf("[PASS] %s\n", Test.Name);
		}
		else
		{
			++Failed;
			std::printf("[FAIL] %s (%d failed checks)\n", Test.Name, GCurrentFailures);
		}
	}
	const double Seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - Start).count();
	std::printf("RESULT: %d passed, %d failed, run time %.2f s\n", Passed, Failed, Seconds);
	return (Failed == 0 && Passed > 0) ? 0 : 1;
}
