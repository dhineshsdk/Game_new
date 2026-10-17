#include "CodexTest.h"

#include "CodexMath.h"

using namespace Codex;

CODEX_TEST(Math_Vec3Basics)
{
	const FVec3d A { 1.0, 2.0, 3.0 };
	const FVec3d B { -4.0, 5.0, 0.5 };
	CODEX_CHECK_NEAR(A.Dot(B), 7.5, 1e-15);
	const FVec3d C = A.Cross(B);
	CODEX_CHECK_NEAR(C.X, 2.0 * 0.5 - 3.0 * 5.0, 1e-15);
	CODEX_CHECK_NEAR(C.Y, 3.0 * -4.0 - 1.0 * 0.5, 1e-15);
	CODEX_CHECK_NEAR(C.Z, 1.0 * 5.0 - 2.0 * -4.0, 1e-15);
	CODEX_CHECK_NEAR(C.Dot(A), 0.0, 1e-12);
	CODEX_CHECK_NEAR((FVec3d { 3.0, 4.0, 12.0 }).Length(), 13.0, 1e-15);
}

CODEX_TEST(Math_QuatRotation)
{
	const FQuatd Yaw90 = FQuatd::FromAxisAngle({ 0.0, 0.0, 1.0 }, 0.5 * Pi);
	const FVec3d R = Yaw90.Rotate({ 1.0, 0.0, 0.0 });
	CODEX_CHECK_NEAR(R.X, 0.0, 1e-15);
	CODEX_CHECK_NEAR(R.Y, 1.0, 1e-15);
	CODEX_CHECK_NEAR(R.Z, 0.0, 1e-15);

	const FVec3d Back = Yaw90.UnrotateVector(R);
	CODEX_CHECK_NEAR(Back.X, 1.0, 1e-15);
	CODEX_CHECK_NEAR(Back.Y, 0.0, 1e-15);

	// Composition: two 90 degree yaws equal one 180 degree yaw.
	const FVec3d Twice = (Yaw90 * Yaw90).Rotate({ 1.0, 0.0, 0.0 });
	CODEX_CHECK_NEAR(Twice.X, -1.0, 1e-15);
	CODEX_CHECK_NEAR(Twice.Y, 0.0, 1e-15);
}

CODEX_TEST(Math_Mat3)
{
	const FMat3d D = FMat3d::Diagonal({ 2.0, 3.0, 4.0 });
	const FVec3d V = D * FVec3d { 1.0, 1.0, 1.0 };
	CODEX_CHECK(V == (FVec3d { 2.0, 3.0, 4.0 }));
	FMat3d A;
	A.M[0][1] = 5.0;
	const FMat3d At = A.Transposed();
	CODEX_CHECK(At.M[1][0] == 5.0 && At.M[0][1] == 0.0);
	const FMat3d P = A * At;
	CODEX_CHECK_NEAR(P.M[0][0], 26.0, 1e-15);
}

static_assert(GravityMps2 == 9.80665);
static_assert(ForceToChaos == 100.0);
static_assert(TorqueToChaos == 10000.0);

CODEX_SELFTEST(SelfTest_DeliberateFailure)
{
	volatile int Two = 2;
	CODEX_CHECK(Two == 3);
}
