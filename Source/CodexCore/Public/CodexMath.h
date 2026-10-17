#pragma once

// Pure C++20 math types for the Codex simulation core (ADR 0005).
// No UE headers. All types are double precision, scalar (no SIMD) and trivially copyable so
// they can be copied by value into Chaos async inputs. Names differ from UE's FVector3d/FQuat4d
// on purpose: both are visible in CodexVehicles and must never be confused.
// Units are SI (m, kg, s, N, rad) unless a name says otherwise.

#include <cmath>
#include <type_traits>

namespace Codex
{
	inline constexpr double Pi = 3.14159265358979323846;
	inline constexpr double GravityMps2 = 9.80665;

	// CLAUDE.md law 3: SI force (N) -> Chaos (kg*cm/s^2), SI torque (N*m) -> Chaos (kg*cm^2/s^2).
	inline constexpr double ForceToChaos = 100.0;
	inline constexpr double TorqueToChaos = 10000.0;

	struct FVec2d
	{
		double X = 0.0;
		double Y = 0.0;

		constexpr FVec2d operator+(const FVec2d& O) const { return { X + O.X, Y + O.Y }; }
		constexpr FVec2d operator-(const FVec2d& O) const { return { X - O.X, Y - O.Y }; }
		constexpr FVec2d operator*(double S) const { return { X * S, Y * S }; }
		constexpr bool operator==(const FVec2d& O) const = default;

		constexpr double Dot(const FVec2d& O) const { return X * O.X + Y * O.Y; }
		double Length() const { return std::sqrt(Dot(*this)); }
	};

	struct FVec3d
	{
		double X = 0.0;
		double Y = 0.0;
		double Z = 0.0;

		constexpr FVec3d operator+(const FVec3d& O) const { return { X + O.X, Y + O.Y, Z + O.Z }; }
		constexpr FVec3d operator-(const FVec3d& O) const { return { X - O.X, Y - O.Y, Z - O.Z }; }
		constexpr FVec3d operator-() const { return { -X, -Y, -Z }; }
		constexpr FVec3d operator*(double S) const { return { X * S, Y * S, Z * S }; }
		constexpr FVec3d operator/(double S) const { return { X / S, Y / S, Z / S }; }
		constexpr bool operator==(const FVec3d& O) const = default;

		constexpr double Dot(const FVec3d& O) const { return X * O.X + Y * O.Y + Z * O.Z; }
		constexpr FVec3d Cross(const FVec3d& O) const
		{
			return { Y * O.Z - Z * O.Y, Z * O.X - X * O.Z, X * O.Y - Y * O.X };
		}
		double Length() const { return std::sqrt(Dot(*this)); }
	};

	constexpr FVec3d operator*(double S, const FVec3d& V) { return V * S; }

	// Unit quaternion, Hamilton convention, W is the scalar part.
	struct FQuatd
	{
		double X = 0.0;
		double Y = 0.0;
		double Z = 0.0;
		double W = 1.0;

		static FQuatd FromAxisAngle(const FVec3d& UnitAxis, double AngleRad)
		{
			const double S = std::sin(0.5 * AngleRad);
			return { UnitAxis.X * S, UnitAxis.Y * S, UnitAxis.Z * S, std::cos(0.5 * AngleRad) };
		}

		constexpr FQuatd operator*(const FQuatd& Q) const
		{
			return {
				W * Q.X + X * Q.W + Y * Q.Z - Z * Q.Y,
				W * Q.Y - X * Q.Z + Y * Q.W + Z * Q.X,
				W * Q.Z + X * Q.Y - Y * Q.X + Z * Q.W,
				W * Q.W - X * Q.X - Y * Q.Y - Z * Q.Z
			};
		}
		constexpr bool operator==(const FQuatd& O) const = default;

		constexpr FQuatd Conjugate() const { return { -X, -Y, -Z, W }; }

		// v' = v + 2w(q x v) + 2 q x (q x v)
		constexpr FVec3d Rotate(const FVec3d& V) const
		{
			const FVec3d Q { X, Y, Z };
			const FVec3d T = Q.Cross(V) * 2.0;
			return V + T * W + Q.Cross(T);
		}
		constexpr FVec3d UnrotateVector(const FVec3d& V) const { return Conjugate().Rotate(V); }
	};

	// Row-major 3x3 matrix.
	struct FMat3d
	{
		double M[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };

		static constexpr FMat3d Diagonal(const FVec3d& D)
		{
			FMat3d R;
			R.M[0][0] = D.X;
			R.M[1][1] = D.Y;
			R.M[2][2] = D.Z;
			return R;
		}

		constexpr FVec3d operator*(const FVec3d& V) const
		{
			return {
				M[0][0] * V.X + M[0][1] * V.Y + M[0][2] * V.Z,
				M[1][0] * V.X + M[1][1] * V.Y + M[1][2] * V.Z,
				M[2][0] * V.X + M[2][1] * V.Y + M[2][2] * V.Z
			};
		}

		constexpr FMat3d operator*(const FMat3d& O) const
		{
			FMat3d R;
			for (int I = 0; I < 3; ++I)
			{
				for (int J = 0; J < 3; ++J)
				{
					R.M[I][J] = M[I][0] * O.M[0][J] + M[I][1] * O.M[1][J] + M[I][2] * O.M[2][J];
				}
			}
			return R;
		}

		constexpr FMat3d Transposed() const
		{
			FMat3d R;
			for (int I = 0; I < 3; ++I)
			{
				for (int J = 0; J < 3; ++J)
				{
					R.M[I][J] = M[J][I];
				}
			}
			return R;
		}
	};

	static_assert(std::is_trivially_copyable_v<FVec2d>);
	static_assert(std::is_trivially_copyable_v<FVec3d>);
	static_assert(std::is_trivially_copyable_v<FQuatd>);
	static_assert(std::is_trivially_copyable_v<FMat3d>);
}
