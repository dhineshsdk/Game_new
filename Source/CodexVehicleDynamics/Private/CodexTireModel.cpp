#include "CodexTireModel.h"

#include "CodexMath.h"

#include <cmath>

namespace Codex::Dynamics
{
	namespace
	{
		// D sin(C atan(B x - E (B x - atan(B x))))
		double MagicFormula(double X, double D, double B, double C, double E)
		{
			const double Bx = B * X;
			return D * std::sin(C * std::atan(Bx - E * (Bx - std::atan(Bx))));
		}

		// Solves (1 - E) u + E atan(u) = tan(pi / (2C)) for u = B * x_peak, then returns x_peak.
		// The left side is strictly increasing in u for E < 1, so bisection with a fixed iteration
		// count converges deterministically. C <= 1 or E >= 1 has no finite peak: returns 0 and the
		// caller treats the curve as monotonic (combined slip then degenerates safely, see below).
		double PeakSlip(double B, double C, double E)
		{
			if (B <= 0.0 || C <= 1.0 || E >= 1.0)
			{
				return 0.0;
			}

			const double Target = std::tan(Pi / (2.0 * C));
			double Lo = 0.0;
			double Hi = (Target + std::fabs(E) * 0.5 * Pi) / (1.0 - E) + 1.0;
			for (int Iteration = 0; Iteration < 200; ++Iteration)
			{
				const double Mid = 0.5 * (Lo + Hi);
				const double Value = (1.0 - E) * Mid + E * std::atan(Mid);
				if (Value < Target)
				{
					Lo = Mid;
				}
				else
				{
					Hi = Mid;
				}
			}
			return 0.5 * (Lo + Hi) / B;
		}
	}

	double LoadScaledPeakForce(double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface)
	{
		if (!(FzN > 0.0))
		{
			return 0.0;
		}
		const double D = FzN * Surface.Mu * (1.0 + Params.PDy2 * (FzN - Params.Fz0N) / Params.Fz0N);
		return D > 0.0 ? D : 0.0;
	}

	double PureLateralForce(double AlphaRad, double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface)
	{
		const double D = LoadScaledPeakForce(FzN, Params, Surface);
		return MagicFormula(AlphaRad, D, Surface.By, Surface.Cy, Surface.Ey);
	}

	double PureLongitudinalForce(double Kappa, double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface)
	{
		const double D = LoadScaledPeakForce(FzN, Params, Surface);
		return MagicFormula(Kappa, D, Surface.Bx, Surface.Cx, Surface.Ex);
	}

	FTirePeakSlips ComputePeakSlips(const FTireSurfaceCoeffs& Surface)
	{
		return { PeakSlip(Surface.By, Surface.Cy, Surface.Ey), PeakSlip(Surface.Bx, Surface.Cx, Surface.Ex) };
	}

	FTireForces CombinedForces(double Kappa, double AlphaRad, double FzN, const FTireParams& Params,
		const FTireSurfaceCoeffs& Surface, const FTirePeakSlips& PeakSlips)
	{
		// Without a finite peak the slips cannot be normalised; fall back to the pure curves.
		if (!(PeakSlips.KappaPeak > 0.0) || !(PeakSlips.AlphaPeakRad > 0.0))
		{
			return { PureLongitudinalForce(Kappa, FzN, Params, Surface), PureLateralForce(AlphaRad, FzN, Params, Surface) };
		}

		const double Sx = Kappa / PeakSlips.KappaPeak;
		const double Sy = AlphaRad / PeakSlips.AlphaPeakRad;
		const double Rho = std::sqrt(Sx * Sx + Sy * Sy);
		if (Rho < 1e-12)
		{
			return { 0.0, 0.0 };
		}

		const double Fx0 = PureLongitudinalForce(Rho * PeakSlips.KappaPeak, FzN, Params, Surface);
		const double Fy0 = PureLateralForce(Rho * PeakSlips.AlphaPeakRad, FzN, Params, Surface);
		return { (Sx / Rho) * Fx0, (Sy / Rho) * Fy0 };
	}

	double StepLateralRelaxation(double FyPrevN, double FySteadyN, double VxMps, double DtS, const FTireParams& Params)
	{
		if (!(DtS > 0.0))
		{
			return FyPrevN;
		}
		const double Speed = std::fmax(std::fabs(VxMps), Params.MinRelaxationSpeedMps);
		const double Decay = std::exp(-Speed * DtS / Params.RelaxationLengthYM);
		return FySteadyN + (FyPrevN - FySteadyN) * Decay;
	}
}
