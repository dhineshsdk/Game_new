#include "CodexTireModel.h"

#include "CodexMath.h"

#include <cmath>

namespace Codex::Dynamics
{
	namespace
	{
		// Slips beyond these magnitudes carry no extra information (the curves are saturated) and
		// would overflow the combined-slip normalisation. Slip angle is physically bounded by pi/2.
		constexpr double MaxAbsSlipAngleRad = 0.5 * Pi;
		constexpr double MaxAbsSlipRatio = 1000.0;

		// NaN becomes 0 (no force); +/-inf and huge values clamp to +/-Limit.
		double SanitizeSlip(double Slip, double Limit)
		{
			if (Slip != Slip)
			{
				return 0.0;
			}
			return Slip > Limit ? Limit : (Slip < -Limit ? -Limit : Slip);
		}

		// D sin(C atan(B x - E (B x - atan(B x)))), written as (1 - E) B x + E atan(B x) so that a
		// clamped large argument never forms inf - inf.
		double MagicFormula(double X, double D, double B, double C, double E)
		{
			const double Bx = B * X;
			return D * std::sin(C * std::atan((1.0 - E) * Bx + E * std::atan(Bx)));
		}

		bool IsSupportedCurve(double B, double C, double E)
		{
			return B > 0.0 && C > 1.0 && C < 2.0 && E < 1.0;
		}

		// Solves (1 - E) u + E atan(u) = tan(pi / (2C)) for u = B * x_peak, then returns x_peak.
		// For a supported curve (B > 0, 1 < C < 2, E < 1) the left side is strictly increasing in u,
		// so bisection with a fixed iteration count converges deterministically. Unsupported curves
		// return 0 ("no finite peak"); CombinedForces then bounds the result explicitly.
		double PeakSlip(double B, double C, double E)
		{
			if (!IsSupportedCurve(B, C, E))
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

	bool IsSupportedSurface(const FTireSurfaceCoeffs& Surface)
	{
		return Surface.Mu >= 0.0 && IsSupportedCurve(Surface.By, Surface.Cy, Surface.Ey) && IsSupportedCurve(Surface.Bx, Surface.Cx, Surface.Ex);
	}

	bool IsValidTireParams(const FTireParams& Params)
	{
		return Params.Fz0N > 0.0 && Params.RelaxationLengthYM > 0.0 && Params.MinRelaxationSpeedMps > 0.0
			&& std::isfinite(Params.PDy2);
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
		return MagicFormula(SanitizeSlip(AlphaRad, MaxAbsSlipAngleRad), D, Surface.By, Surface.Cy, Surface.Ey);
	}

	double PureLongitudinalForce(double Kappa, double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface)
	{
		const double D = LoadScaledPeakForce(FzN, Params, Surface);
		return MagicFormula(SanitizeSlip(Kappa, MaxAbsSlipRatio), D, Surface.Bx, Surface.Cx, Surface.Ex);
	}

	FTirePeakSlips ComputePeakSlips(const FTireSurfaceCoeffs& Surface)
	{
		return { PeakSlip(Surface.By, Surface.Cy, Surface.Ey), PeakSlip(Surface.Bx, Surface.Cx, Surface.Ex) };
	}

	FTireForces CombinedForces(double Kappa, double AlphaRad, double FzN, const FTireParams& Params,
		const FTireSurfaceCoeffs& Surface, const FTirePeakSlips& PeakSlips)
	{
		const double K = SanitizeSlip(Kappa, MaxAbsSlipRatio);
		const double A = SanitizeSlip(AlphaRad, MaxAbsSlipAngleRad);

		// Unsupported data has no finite peak to normalise by: use the pure curves and scale the
		// pair back onto the friction circle so sqrt(Fx^2 + Fy^2) <= D still holds.
		if (!(PeakSlips.KappaPeak > 0.0) || !(PeakSlips.AlphaPeakRad > 0.0))
		{
			const double D = LoadScaledPeakForce(FzN, Params, Surface);
			const double Fx = PureLongitudinalForce(K, FzN, Params, Surface);
			const double Fy = PureLateralForce(A, FzN, Params, Surface);
			const double Magnitude = std::hypot(Fx, Fy);
			const double Scale = Magnitude > D ? D / Magnitude : 1.0;
			return { Fx * Scale, Fy * Scale };
		}

		const double Sx = K / PeakSlips.KappaPeak;
		const double Sy = A / PeakSlips.AlphaPeakRad;
		const double Rho = std::hypot(Sx, Sy);
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
