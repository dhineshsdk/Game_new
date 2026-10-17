// T-0101 acceptance tests for the Pacejka MF94 tyre model. Surface data lives here and in the golden
// CSV only (the model code is data-free). Golden values come from tools/reference/pacejka_mf94_reference.py.
#include "CodexTest.h"

#include "CodexMath.h"
#include "CodexTireModel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

using namespace Codex;
using namespace Codex::Dynamics;

namespace
{
	// Spec section 2.1 rows. Longitudinal coefficients mirror lateral (ADR 0005 item 5).
	constexpr FTireSurfaceCoeffs Dry { 1.00, 10.00, 1.30, -1.00, 10.00, 1.30, -1.00 };
	constexpr FTireSurfaceCoeffs Wet { 0.75, 10.25, 1.30, -1.40, 10.25, 1.30, -1.40 };
	constexpr FTireSurfaceCoeffs Gravel { 0.50, 8.50, 1.25, -0.30, 8.50, 1.25, -0.30 };
	constexpr FTireSurfaceCoeffs Ice { 0.20, 9.00, 1.20, -2.00, 9.00, 1.20, -2.00 };

	struct FNamedSurface
	{
		const char* Name;
		FTireSurfaceCoeffs Coeffs;
	};
	constexpr FNamedSurface Surfaces[] = { { "dry", Dry }, { "wet", Wet }, { "gravel", Gravel }, { "ice", Ice } };

	// Test-only surface with distinct longitudinal coefficients, so the golden data can tell the
	// kappa path from the alpha path (independent review of T-0101, finding 1).
	constexpr FTireSurfaceCoeffs Asym { 1.00, 10.00, 1.30, -1.00, 12.00, 1.65, 0.30 };
	constexpr FNamedSurface AllSurfaces[] = { { "dry", Dry }, { "wet", Wet }, { "gravel", Gravel }, { "ice", Ice }, { "asym", Asym } };
	constexpr int NumAllSurfaces = 5;
	constexpr int GoldenRowCount = 30405;
	constexpr double FzSet[] = { 0.0, 800.0, 1600.0, 2997.0, 3200.0, 3379.5, 6400.0, 9000.0 };

	const FTireParams Params;

	int FindSurfaceIndex(const char* Name)
	{
		for (int I = 0; I < NumAllSurfaces; ++I)
		{
			if (std::strcmp(AllSurfaces[I].Name, Name) == 0)
			{
				return I;
			}
		}
		return -1;
	}

	bool GoldenMatch(double Actual, double Expected)
	{
		const double Tol = std::fmax(1e-6 * std::fabs(Expected), 1e-6);
		return std::fabs(Actual - Expected) <= Tol;
	}

	double SignOf(double V)
	{
		return V > 0.0 ? 1.0 : (V < 0.0 ? -1.0 : 0.0);
	}
}

CODEX_TEST(Tire_LoadSensitivity)
{
	CODEX_CHECK_NEAR(LoadScaledPeakForce(3200.0, Params, Dry), 3200.0, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(1600.0, Params, Dry), 1715.2, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(6400.0, Params, Dry), 5478.4, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(3379.5, Params, Dry), 3352.20, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(2997.0, Params, Dry), 3024.38, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(3200.0, Params, Wet), 2400.0, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(3200.0, Params, Gravel), 1600.0, 0.01);
	CODEX_CHECK_NEAR(LoadScaledPeakForce(3200.0, Params, Ice), 640.0, 0.01);
	CODEX_CHECK(LoadScaledPeakForce(0.0, Params, Dry) == 0.0);
	CODEX_CHECK(LoadScaledPeakForce(-100.0, Params, Dry) == 0.0);
	const double Huge = LoadScaledPeakForce(30000.0, Params, Dry);
	CODEX_CHECK(Huge == 0.0 && std::isfinite(Huge));
	CODEX_CHECK(PureLateralForce(0.1, 30000.0, Params, Dry) == 0.0);
}

CODEX_TEST(Tire_CorneringStiffness)
{
	const double Expected[] = { 41600.0, 31980.0, 17000.0, 6912.0 };
	for (int I = 0; I < 4; ++I)
	{
		const FTireSurfaceCoeffs& S = Surfaces[I].Coeffs;
		const double H = 1e-5;
		const double Slope = (PureLateralForce(H, 3200.0, Params, S) - PureLateralForce(-H, 3200.0, Params, S)) / (2.0 * H);
		CODEX_CHECK_NEAR(Slope, Expected[I], Expected[I] * 0.001);
	}
	const double H = 1e-5;
	const double Slope6400 = (PureLateralForce(H, 6400.0, Params, Dry) - PureLateralForce(-H, 6400.0, Params, Dry)) / (2.0 * H);
	CODEX_CHECK_NEAR(Slope6400, 71219.2, 71219.2 * 0.001);
}

CODEX_TEST(Tire_LateralPeak)
{
	for (const FNamedSurface& S : Surfaces)
	{
		const double D = LoadScaledPeakForce(3200.0, Params, S.Coeffs);
		double Max = 0.0;
		for (int I = 0; I <= 6000; ++I)
		{
			Max = std::fmax(Max, std::fabs(PureLateralForce(I * 1e-4, 3200.0, Params, S.Coeffs)));
		}
		CODEX_CHECK_NEAR(Max, D, D * 0.0005);
		CODEX_CHECK(Max <= D * (1.0 + 1e-9));
	}
	const FTirePeakSlips Peak = ComputePeakSlips(Dry);
	CODEX_CHECK_NEAR(Peak.AlphaPeakRad, 0.18568, 0.0005);
	// At the computed peak slip the curve reaches D.
	CODEX_CHECK_NEAR(PureLateralForce(Peak.AlphaPeakRad, 3200.0, Params, Dry), 3200.0, 1e-6);
	// Deterministic: same answer every call.
	const FTirePeakSlips Again = ComputePeakSlips(Dry);
	CODEX_CHECK(Again.AlphaPeakRad == Peak.AlphaPeakRad && Again.KappaPeak == Peak.KappaPeak);
}

CODEX_TEST(Tire_LateralGoldenPoint)
{
	CODEX_CHECK_NEAR(PureLateralForce(0.05, 3200.0, Params, Dry), 1911.0, 1911.0 * 0.005);
}

CODEX_TEST(Tire_LateralSymmetryAndRange)
{
	for (const FNamedSurface& S : Surfaces)
	{
		for (double Fz : FzSet)
		{
			const double D = LoadScaledPeakForce(Fz, Params, S.Coeffs);
			for (int I = -1000; I <= 1000; ++I)
			{
				const double Alpha = (0.5 * Pi) * (I / 1000.0);
				const double Fy = PureLateralForce(Alpha, Fz, Params, S.Coeffs);
				const double FyNeg = PureLateralForce(-Alpha, Fz, Params, S.Coeffs);
				CODEX_CHECK(std::isfinite(Fy));
				CODEX_CHECK(std::fabs(Fy + FyNeg) <= 1e-9 * std::fmax(std::fabs(Fy), 1.0));
				CODEX_CHECK(std::fabs(Fy) <= D * (1.0 + 1e-9));
			}
		}
	}
}

CODEX_TEST(Tire_LongitudinalMirrorsLateral)
{
	for (const FNamedSurface& S : Surfaces)
	{
		const double D = LoadScaledPeakForce(3200.0, Params, S.Coeffs);
		for (int I = -100; I <= 100; ++I)
		{
			const double X = I / 100.0;
			const double Fx = PureLongitudinalForce(X, 3200.0, Params, S.Coeffs);
			const double Fy = PureLateralForce(X, 3200.0, Params, S.Coeffs);
			CODEX_CHECK(std::fabs(Fx - Fy) <= 1e-12 * std::fmax(std::fabs(Fy), 1.0));
			CODEX_CHECK(std::fabs(Fx) <= D * (1.0 + 1e-9));
		}
	}
}

CODEX_TEST(Tire_CombinedReducesToPure)
{
	for (const FNamedSurface& S : Surfaces)
	{
		const FTirePeakSlips Peak = ComputePeakSlips(S.Coeffs);
		for (int I = -60; I <= 60; ++I)
		{
			const double Alpha = I / 100.0;
			const FTireForces F = CombinedForces(0.0, Alpha, 3200.0, Params, S.Coeffs, Peak);
			const double Pure = PureLateralForce(Alpha, 3200.0, Params, S.Coeffs);
			CODEX_CHECK(F.FxN == 0.0);
			CODEX_CHECK(std::fabs(F.FyN - Pure) <= 1e-9 * std::fmax(std::fabs(Pure), 1.0));

			const double Kappa = I / 60.0;
			const FTireForces G = CombinedForces(Kappa, 0.0, 3200.0, Params, S.Coeffs, Peak);
			const double PureX = PureLongitudinalForce(Kappa, 3200.0, Params, S.Coeffs);
			CODEX_CHECK(G.FyN == 0.0);
			CODEX_CHECK(std::fabs(G.FxN - PureX) <= 1e-9 * std::fmax(std::fabs(PureX), 1.0));
		}
		const FTireForces Zero = CombinedForces(0.0, 0.0, 3200.0, Params, S.Coeffs, Peak);
		CODEX_CHECK(Zero.FxN == 0.0 && Zero.FyN == 0.0);
	}
}

CODEX_TEST(Tire_FrictionCircle)
{
	for (const FNamedSurface& S : Surfaces)
	{
		const FTirePeakSlips Peak = ComputePeakSlips(S.Coeffs);
		for (double Fz : FzSet)
		{
			const double D = LoadScaledPeakForce(Fz, Params, S.Coeffs);
			for (int K = -100; K <= 100; ++K)
			{
				const double Kappa = K / 100.0;
				for (int A = -120; A <= 120; ++A)
				{
					const double Alpha = A * 0.005;
					const FTireForces F = CombinedForces(Kappa, Alpha, Fz, Params, S.Coeffs, Peak);
					CODEX_CHECK(std::isfinite(F.FxN) && std::isfinite(F.FyN));
					CODEX_CHECK(std::sqrt(F.FxN * F.FxN + F.FyN * F.FyN) <= D * (1.0 + 1e-6));
					if (F.FxN != 0.0)
					{
						CODEX_CHECK(SignOf(F.FxN) == SignOf(Kappa));
					}
					if (F.FyN != 0.0)
					{
						CODEX_CHECK(SignOf(F.FyN) == SignOf(Alpha));
					}
				}
			}
		}
	}
}

CODEX_TEST(Tire_CombinedReducesPureForces)
{
	for (const FNamedSurface& S : Surfaces)
	{
		const FTirePeakSlips Peak = ComputePeakSlips(S.Coeffs);
		const FTireForces F = CombinedForces(Peak.KappaPeak, 0.5 * Peak.AlphaPeakRad, 3200.0, Params, S.Coeffs, Peak);
		CODEX_CHECK(std::fabs(F.FyN) < std::fabs(PureLateralForce(0.5 * Peak.AlphaPeakRad, 3200.0, Params, S.Coeffs)));
		CODEX_CHECK(std::fabs(F.FxN) < std::fabs(PureLongitudinalForce(Peak.KappaPeak, 3200.0, Params, S.Coeffs)));
	}
}

CODEX_TEST(Tire_Relaxation)
{
	const double Dt = 1.0 / 120.0;
	CODEX_CHECK_NEAR(StepLateralRelaxation(0.0, 3200.0, 0.0, Dt, Params), 71.91, 0.01);
	CODEX_CHECK_NEAR(StepLateralRelaxation(0.0, 3200.0, 20.0, Dt, Params), 836.55, 0.01);

	double Fy0 = 0.0;
	double FyHalf = 0.0;
	double FyPos = 0.0;
	double FyNeg = 0.0;
	for (int Step = 0; Step < 120; ++Step)
	{
		Fy0 = StepLateralRelaxation(Fy0, 3200.0, 0.0, Dt, Params);
		FyHalf = StepLateralRelaxation(FyHalf, 3200.0, 0.5, Dt, Params);
		FyPos = StepLateralRelaxation(FyPos, 3200.0, 20.0, Dt, Params);
		FyNeg = StepLateralRelaxation(FyNeg, 3200.0, -20.0, Dt, Params);
		CODEX_CHECK(std::memcmp(&Fy0, &FyHalf, sizeof(double)) == 0);
		CODEX_CHECK(std::memcmp(&FyPos, &FyNeg, sizeof(double)) == 0);
	}
	CODEX_CHECK_NEAR(Fy0, 2990.73, 0.05);

	CODEX_CHECK(StepLateralRelaxation(123.0, 3200.0, 20.0, 0.0, Params) == 123.0);
	CODEX_CHECK(StepLateralRelaxation(123.0, 3200.0, 20.0, -Dt, Params) == 123.0);
}

CODEX_TEST(Tire_RelaxationStability)
{
	const double Speeds[] = { 0.0, 1.5, 20.0, 60.0, 200.0 };
	const double Steps[] = { 1.0 / 240.0, 1.0 / 120.0, 1.0 / 60.0, 0.05 };
	for (double Vx : Speeds)
	{
		for (double Dt : Steps)
		{
			const double Target = 2500.0;
			double Fy = -1500.0;
			for (int I = 0; I < 1000; ++I)
			{
				const double Next = StepLateralRelaxation(Fy, Target, Vx, Dt, Params);
				CODEX_CHECK(std::isfinite(Next));
				CODEX_CHECK(std::fabs(Next - Target) <= std::fabs(Fy - Target));
				CODEX_CHECK(SignOf(Next - Target) == SignOf(Fy - Target) || Next == Target);
				Fy = Next;
			}
		}
	}
}

CODEX_TEST(Tire_GoldenCsv)
{
	FILE* File = nullptr;
	if (fopen_s(&File, "Tests/Headless/Dynamics/golden/pacejka_mf94_reference.csv", "r") != 0 || !File)
	{
		CODEX_CHECK(!"golden CSV missing; run python -I tools/reference/pacejka_mf94_reference.py");
		return;
	}

	FTirePeakSlips Peaks[NumAllSurfaces];
	for (int I = 0; I < NumAllSurfaces; ++I)
	{
		Peaks[I] = ComputePeakSlips(AllSurfaces[I].Coeffs);
	}

	char Line[512];
	int Rows = 0;
	int Mismatches = 0;
	double RelaxPrev[4] = { 0.0, 0.0, 0.0, 0.0 };
	const double RelaxVx[4] = { 0.0, 0.5, 20.0, -20.0 };
	fgets(Line, sizeof(Line), File); // header
	while (fgets(Line, sizeof(Line), File))
	{
		char Kind[16] = {};
		char Surface[16] = {};
		double A = 0.0, Alpha = 0.0, K = 0.0, V1 = 0.0, V2 = 0.0;
		if (sscanf_s(Line, "%15[^,],%15[^,],%lf,%lf,%lf,%lf,%lf", Kind, 16u, Surface, 16u, &A, &Alpha, &K, &V1, &V2) != 7)
		{
			++Mismatches; // an unreadable row must fail, not be skipped
			continue;
		}
		++Rows;
		bool bOk = true;
		if (std::strcmp(Kind, "relax") == 0)
		{
			for (int I = 0; I < 4; ++I)
			{
				if (RelaxVx[I] == A)
				{
					if (K == 1.0)
					{
						RelaxPrev[I] = 0.0;
					}
					RelaxPrev[I] = StepLateralRelaxation(RelaxPrev[I], 3200.0, A, 1.0 / 120.0, Params);
					bOk = GoldenMatch(RelaxPrev[I], V1);
				}
			}
		}
		else
		{
			const int SurfaceIndex = FindSurfaceIndex(Surface);
			if (SurfaceIndex < 0)
			{
				++Mismatches;
				continue;
			}
			const FTireSurfaceCoeffs* S = &AllSurfaces[SurfaceIndex].Coeffs;
			const FTirePeakSlips& Peak = Peaks[SurfaceIndex];
			if (std::strcmp(Kind, "peak") == 0)
			{
				bOk = GoldenMatch(Peak.AlphaPeakRad, V1) && GoldenMatch(Peak.KappaPeak, V2);
			}
			else if (std::strcmp(Kind, "D") == 0)
			{
				bOk = GoldenMatch(LoadScaledPeakForce(A, Params, *S), V1);
			}
			else if (std::strcmp(Kind, "fy") == 0)
			{
				bOk = GoldenMatch(PureLateralForce(Alpha, A, Params, *S), V1);
			}
			else if (std::strcmp(Kind, "fx") == 0)
			{
				bOk = GoldenMatch(PureLongitudinalForce(K, A, Params, *S), V1);
			}
			else if (std::strcmp(Kind, "comb") == 0)
			{
				const FTireForces F = CombinedForces(K, Alpha, A, Params, *S, Peak);
				bOk = GoldenMatch(F.FxN, V1) && GoldenMatch(F.FyN, V2);
			}
		}
		if (!bOk)
		{
			++Mismatches;
			if (Mismatches <= 5)
			{
				std::printf("    golden mismatch: %s", Line);
			}
		}
	}
	fclose(File);
	CODEX_CHECK(Rows == GoldenRowCount);
	CODEX_CHECK(Mismatches == 0);
}

CODEX_TEST(Tire_AsymmetricLongitudinal)
{
	CODEX_CHECK(IsSupportedSurface(Asym));
	const FTirePeakSlips Peak = ComputePeakSlips(Asym);
	CODEX_CHECK(std::fabs(Peak.KappaPeak - Peak.AlphaPeakRad) > 0.01);
	CODEX_CHECK_NEAR(Peak.AlphaPeakRad, ComputePeakSlips(Dry).AlphaPeakRad, 0.0);
	// The longitudinal curve peaks at D exactly at its own peak slip.
	CODEX_CHECK_NEAR(PureLongitudinalForce(Peak.KappaPeak, 3200.0, Params, Asym), 3200.0, 1e-6);
	CODEX_CHECK(std::fabs(PureLongitudinalForce(0.05, 3200.0, Params, Asym) - PureLateralForce(0.05, 3200.0, Params, Asym)) > 1.0);
	// Combined slip uses each axis's own peak: pure-axis inputs still reduce to the pure curves.
	const FTireForces F = CombinedForces(Peak.KappaPeak, 0.0, 3200.0, Params, Asym, Peak);
	CODEX_CHECK_NEAR(F.FxN, 3200.0, 1e-6);
	CODEX_CHECK(F.FyN == 0.0);
}

CODEX_TEST(Tire_UnsupportedDataStaysBounded)
{
	// No finite peak: C = 1 (curve never turns over), and E = 1 with C = 2 (outside the supported range).
	const FTireSurfaceCoeffs FlatC { 1.00, 10.00, 1.00, 0.00, 10.00, 1.00, 0.00 };
	const FTireSurfaceCoeffs EOne { 1.00, 10.00, 2.00, 1.00, 10.00, 2.00, 1.00 };
	const FTireSurfaceCoeffs NoLongPeak { 1.00, 10.00, 1.30, -1.00, 10.00, 1.00, 0.00 };
	const FTireSurfaceCoeffs Cases[] = { FlatC, EOne, NoLongPeak };
	CODEX_CHECK(IsSupportedSurface(Dry) && IsSupportedSurface(Ice));
	for (const FTireSurfaceCoeffs& S : Cases)
	{
		CODEX_CHECK(!IsSupportedSurface(S));
		const FTirePeakSlips Peak = ComputePeakSlips(S);
		const double D = LoadScaledPeakForce(3200.0, Params, S);
		for (int K = -50; K <= 50; ++K)
		{
			for (int A = -30; A <= 30; ++A)
			{
				const FTireForces F = CombinedForces(K / 50.0, A * 0.02, 3200.0, Params, S, Peak);
				CODEX_CHECK(std::isfinite(F.FxN) && std::isfinite(F.FyN));
				CODEX_CHECK(std::hypot(F.FxN, F.FyN) <= D * (1.0 + 1e-9));
			}
		}
	}
}

CODEX_TEST(Tire_NonFiniteAndHugeSlips)
{
	const double Inf = std::numeric_limits<double>::infinity();
	const double NaN = std::numeric_limits<double>::quiet_NaN();
	const FTireSurfaceCoeffs PositiveE { 1.00, 10.00, 1.30, 0.50, 10.00, 1.30, 0.50 };
	const FTireSurfaceCoeffs Tested[] = { Dry, Ice, Asym, PositiveE };
	const double Slips[] = { 1e200, -1e200, Inf, -Inf, NaN, 1e6, -1e6 };
	for (const FTireSurfaceCoeffs& S : Tested)
	{
		const FTirePeakSlips Peak = ComputePeakSlips(S);
		const double D = LoadScaledPeakForce(3200.0, Params, S);
		for (double X : Slips)
		{
			const double Fy = PureLateralForce(X, 3200.0, Params, S);
			const double Fx = PureLongitudinalForce(X, 3200.0, Params, S);
			CODEX_CHECK(std::isfinite(Fy) && std::isfinite(Fx));
			CODEX_CHECK(std::fabs(Fy) <= D * (1.0 + 1e-9) && std::fabs(Fx) <= D * (1.0 + 1e-9));
			const FTireForces Fk = CombinedForces(X, 0.0, 3200.0, Params, S, Peak);
			const FTireForces Fa = CombinedForces(0.0, X, 3200.0, Params, S, Peak);
			const FTireForces Fb = CombinedForces(X, X, 3200.0, Params, S, Peak);
			CODEX_CHECK(std::isfinite(Fk.FxN) && std::isfinite(Fk.FyN) && std::isfinite(Fa.FxN) && std::isfinite(Fa.FyN));
			CODEX_CHECK(std::isfinite(Fb.FxN) && std::isfinite(Fb.FyN));
			CODEX_CHECK(std::hypot(Fb.FxN, Fb.FyN) <= D * (1.0 + 1e-9));
			if (X == X)
			{
				// A huge slip is a saturated slip: the force keeps the slip's sign and is not zero.
				CODEX_CHECK(SignOf(Fk.FxN) == SignOf(X) && SignOf(Fa.FyN) == SignOf(X));
				CODEX_CHECK(std::fabs(Fk.FxN - PureLongitudinalForce(X, 3200.0, Params, S)) <= 1e-9 * D);
			}
			else
			{
				CODEX_CHECK(Fk.FxN == 0.0 && Fk.FyN == 0.0 && Fa.FxN == 0.0 && Fa.FyN == 0.0);
			}
		}
	}
}

CODEX_TEST(Tire_ParamsValidation)
{
	CODEX_CHECK(IsValidTireParams(Params));
	FTireParams Bad = Params;
	Bad.RelaxationLengthYM = -0.55;
	CODEX_CHECK(!IsValidTireParams(Bad));
	Bad = Params;
	Bad.Fz0N = 0.0;
	CODEX_CHECK(!IsValidTireParams(Bad));
	Bad = Params;
	Bad.MinRelaxationSpeedMps = 0.0;
	CODEX_CHECK(!IsValidTireParams(Bad));
}

CODEX_TEST(Tire_Determinism)
{
	FTirePeakSlips Peaks[4];
	for (int I = 0; I < 4; ++I)
	{
		Peaks[I] = ComputePeakSlips(Surfaces[I].Coeffs);
	}

	auto RunPass = [&Peaks]() -> uint64_t
	{
		uint64_t Lcg = 0x9E3779B97F4A7C15ull;
		uint64_t Hash = 0xCBF29CE484222325ull; // FNV-1a 64
		auto Next01 = [&Lcg]() -> double
		{
			Lcg = Lcg * 6364136223846793005ull + 1442695040888963407ull;
			return static_cast<double>(Lcg >> 11) * (1.0 / 9007199254740992.0);
		};
		auto HashDouble = [&Hash](double V)
		{
			unsigned char Bytes[sizeof(double)];
			std::memcpy(Bytes, &V, sizeof(double));
			for (unsigned char B : Bytes)
			{
				Hash = (Hash ^ B) * 0x100000001B3ull;
			}
		};
		int NonFinite = 0;
		for (int I = 0; I < 1000000; ++I)
		{
			const int SurfaceIndex = static_cast<int>(Next01() * 4.0) & 3;
			const double Kappa = -1.0 + 2.0 * Next01();
			const double Alpha = -0.6 + 1.2 * Next01();
			const double Fz = 9000.0 * Next01();
			const FTireForces F = CombinedForces(Kappa, Alpha, Fz, Params, Surfaces[SurfaceIndex].Coeffs, Peaks[SurfaceIndex]);
			NonFinite += (std::isfinite(F.FxN) && std::isfinite(F.FyN)) ? 0 : 1;
			HashDouble(F.FxN);
			HashDouble(F.FyN);
		}
		return NonFinite == 0 ? Hash : 0;
	};

	const uint64_t First = RunPass();
	const uint64_t Second = RunPass();
	std::printf("    determinism hash: %016llx\n", static_cast<unsigned long long>(First));
	CODEX_CHECK(First != 0);
	CODEX_CHECK(First == Second);
}
