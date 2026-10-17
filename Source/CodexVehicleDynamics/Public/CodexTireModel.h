#pragma once

// Pacejka MF94-style tyre force model (docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md section 2, task T-0101).
// Tyre force math only: slips in, forces out. Computing slips from wheel and body velocities and
// the vehicle-level sign convention belong to the solver (T-0201).
// Sign convention: Fy has the sign of the slip angle, Fx has the sign of the slip ratio.
// Stateless and pure: no globals, no allocation, no UE types (ADR 0005).

#include "CodexExport.h"

#include <type_traits>

namespace Codex::Dynamics
{
	// Magic-formula coefficients for one surface. Lateral (y) and longitudinal (x) share Mu.
	// The spec gives lateral rows only; longitudinal values are data (ADR 0005, inconsistency 5).
	struct FTireSurfaceCoeffs
	{
		double Mu = 0.0;
		double By = 0.0;
		double Cy = 0.0;
		double Ey = 0.0;
		double Bx = 0.0;
		double Cx = 0.0;
		double Ex = 0.0;
	};

	// Defaults from spec section 2.2 (Fz0, pDy2, sigma_y ~0.55 m, 1.5 m/s low-speed clamp).
	struct FTireParams
	{
		double Fz0N = 3200.0;
		double PDy2 = -0.144;
		double RelaxationLengthYM = 0.55;
		double MinRelaxationSpeedMps = 1.5;
	};

	struct FTirePeakSlips
	{
		double AlphaPeakRad = 0.0;
		double KappaPeak = 0.0;
	};

	struct FTireForces
	{
		double FxN = 0.0;
		double FyN = 0.0;
	};

	static_assert(std::is_trivially_copyable_v<FTireSurfaceCoeffs>);
	static_assert(std::is_trivially_copyable_v<FTireParams>);
	static_assert(std::is_trivially_copyable_v<FTirePeakSlips>);
	static_assert(std::is_trivially_copyable_v<FTireForces>);

	// D(Fz) = Fz * Mu * (1 + pDy2 * (Fz - Fz0) / Fz0), clamped to >= 0; 0 for Fz <= 0.
	CODEXVEHICLEDYNAMICS_API double LoadScaledPeakForce(double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface);

	// Fy0(alpha) = D sin(C atan(B alpha - E (B alpha - atan(B alpha)))).
	CODEXVEHICLEDYNAMICS_API double PureLateralForce(double AlphaRad, double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface);

	// Same formula with Bx, Cx, Ex and the shared Mu.
	CODEXVEHICLEDYNAMICS_API double PureLongitudinalForce(double Kappa, double FzN, const FTireParams& Params, const FTireSurfaceCoeffs& Surface);

	// Slip at which each pure curve peaks. Load independent (only D scales with Fz).
	// Fixed iteration count; call once per surface, not per step.
	CODEXVEHICLEDYNAMICS_API FTirePeakSlips ComputePeakSlips(const FTireSurfaceCoeffs& Surface);

	// Combined slip by normalised slip magnitude (spec 2.2 item 3); sqrt(Fx^2 + Fy^2) <= D(Fz).
	CODEXVEHICLEDYNAMICS_API FTireForces CombinedForces(double Kappa, double AlphaRad, double FzN, const FTireParams& Params,
		const FTireSurfaceCoeffs& Surface, const FTirePeakSlips& PeakSlips);

	// First-order lateral relaxation, exact exponential update (unconditionally stable):
	// Fy_next = Fss + (Fy_prev - Fss) * exp(-max(|Vx|, MinSpeed) * dt / sigma_y). dt <= 0 returns Fy_prev.
	CODEXVEHICLEDYNAMICS_API double StepLateralRelaxation(double FyPrevN, double FySteadyN, double VxMps, double DtS, const FTireParams& Params);
}
