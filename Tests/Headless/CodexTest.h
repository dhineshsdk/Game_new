#pragma once

// Minimal assertion macros for the headless harness (ADR 0005: no third-party framework).
// Tests self-register; CodexTestMain.cpp runs them and returns non-zero on any failure.

#include <cmath>
#include <cstdio>

namespace CodexTest
{
	using FTestFn = void (*)();

	struct FRegistrar
	{
		FRegistrar(const char* Name, FTestFn Fn, bool bSelfTestOnly);
	};

	void ReportFailure(const char* File, int Line, const char* Message);
	int FailureCount();
}

#define CODEX_TEST_IMPL(Name, bSelfTest) \
	static void CodexTest_##Name(); \
	static const CodexTest::FRegistrar CodexTestReg_##Name(#Name, &CodexTest_##Name, bSelfTest); \
	static void CodexTest_##Name()

#define CODEX_TEST(Name) CODEX_TEST_IMPL(Name, false)

// Runs only with --selftest-fail; proves the harness reports failures.
#define CODEX_SELFTEST(Name) CODEX_TEST_IMPL(Name, true)

#define CODEX_CHECK(Cond) \
	do { if (!(Cond)) { CodexTest::ReportFailure(__FILE__, __LINE__, "CHECK(" #Cond ")"); } } while (0)

#define CODEX_CHECK_NEAR(Actual, Expected, Tol) \
	do { \
		const double CodexA_ = static_cast<double>(Actual); \
		const double CodexE_ = static_cast<double>(Expected); \
		const double CodexT_ = static_cast<double>(Tol); \
		if (!(std::fabs(CodexA_ - CodexE_) <= CodexT_)) { \
			char CodexBuf_[256]; \
			std::snprintf(CodexBuf_, sizeof(CodexBuf_), "NEAR(%s): got %.12g, expected %.12g +/- %.3g", #Actual, CodexA_, CodexE_, CodexT_); \
			CodexTest::ReportFailure(__FILE__, __LINE__, CodexBuf_); \
		} \
	} while (0)
