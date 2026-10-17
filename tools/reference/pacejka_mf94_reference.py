"""Independent reference (oracle) for the T-0101 tyre model.

Implements docs/TECHNICAL_PHYSICS_AND_GIS_SPEC.md section 2 directly from the spec formulas and
writes Tests/Headless/Dynamics/golden/pacejka_mf94_reference.csv. Python 3.12 standard library
only; run with:  python -I tools/reference/pacejka_mf94_reference.py

Independence from the C++ code: peak slips are found with Newton's method (C++ uses bisection),
and grids are generated from integer indices. If this script and a hand value in T-0101 disagree,
stop and escalate; never tune either side to agree.
"""

import math
import os
import sys

FZ0 = 3200.0
PDY2 = -0.144
SIGMA_Y = 0.55
MIN_RELAX_SPEED = 1.5

# Spec section 2.1. Longitudinal coefficients are not in the spec (ADR 0005 item 5): the tests
# set Bx = By, Cx = Cy, Ex = Ey.
SURFACES = {
    "dry": (1.00, 10.00, 1.30, -1.00),
    "wet": (0.75, 10.25, 1.30, -1.40),
    "gravel": (0.50, 8.50, 1.25, -0.30),
    "ice": (0.20, 9.00, 1.20, -2.00),
}
FZ_SET = [0.0, 800.0, 1600.0, 2997.0, 3200.0, 3379.5, 6400.0, 9000.0]


def peak_force(fz, mu):
    if fz <= 0.0:
        return 0.0
    d = fz * mu * (1.0 + PDY2 * (fz - FZ0) / FZ0)
    return max(d, 0.0)


def magic(x, d, b, c, e):
    bx = b * x
    return d * math.sin(c * math.atan(bx - e * (bx - math.atan(bx))))


def peak_slip(b, c, e):
    """Newton on g(u) = (1 - E) u + E atan(u) - tan(pi / 2C), u = B x."""
    target = math.tan(math.pi / (2.0 * c))
    u = target / max(1.0 - e, 1e-9)
    for _ in range(100):
        g = (1.0 - e) * u + e * math.atan(u) - target
        dg = (1.0 - e) + e / (1.0 + u * u)
        step = g / dg
        u -= step
        if abs(step) < 1e-16 * max(1.0, abs(u)):
            break
    return u / b


def combined(kappa, alpha, fz, mu, b, c, e, kp, ap):
    sx = kappa / kp
    sy = alpha / ap
    rho = math.hypot(sx, sy)
    if rho < 1e-12:
        return 0.0, 0.0
    d = peak_force(fz, mu)
    fx0 = magic(rho * kp, d, b, c, e)
    fy0 = magic(rho * ap, d, b, c, e)
    return (sx / rho) * fx0, (sy / rho) * fy0


def relax(fy_prev, fss, vx, dt):
    if dt <= 0.0:
        return fy_prev
    return fss + (fy_prev - fss) * math.exp(-max(abs(vx), MIN_RELAX_SPEED) * dt / SIGMA_Y)


def main():
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    out = os.path.join(root, "Tests", "Headless", "Dynamics", "golden", "pacejka_mf94_reference.csv")
    rows = []

    for name, (mu, b, c, e) in SURFACES.items():
        ap = peak_slip(b, c, e)
        rows.append(f"peak,{name},0,0,0,{ap!r},{ap!r}")
        for fz in FZ_SET:
            d = peak_force(fz, mu)
            rows.append(f"D,{name},{fz!r},0,0,{d!r},0.0")
            for i in range(-60, 61):  # alpha in [-0.6, 0.6] step 0.01
                alpha = i / 100.0
                rows.append(f"fy,{name},{fz!r},{alpha!r},0,{magic(alpha, d, b, c, e)!r},0.0")
            for i in range(-50, 51):  # kappa in [-1, 1] step 0.02
                kappa = i / 50.0
                rows.append(f"fx,{name},{fz!r},0,{kappa!r},{magic(kappa, d, b, c, e)!r},0.0")
            for i in range(-10, 11):  # kappa step 0.1
                kappa = i / 10.0
                for j in range(-12, 13):  # alpha step 0.05
                    alpha = j / 20.0
                    fx, fy = combined(kappa, alpha, fz, mu, b, c, e, ap, ap)
                    rows.append(f"comb,{name},{fz!r},{alpha!r},{kappa!r},{fx!r},{fy!r}")

    # Relaxation sequences: Fy_prev = 0 toward 3200 N, dt = 1/120, 120 steps. Columns: vx in fz slot, step in kappa slot.
    for vx in (0.0, 0.5, 20.0, -20.0):
        fy = 0.0
        for step in range(1, 121):
            fy = relax(fy, 3200.0, vx, 1.0 / 120.0)
            rows.append(f"relax,seq,{vx!r},0,{step},{fy!r},0.0")

    with open(out, "w", encoding="ascii", newline="\n") as f:
        f.write("kind,surface,fz_or_vx,alpha,kappa_or_step,v1,v2\n")
        f.write("\n".join(rows))
        f.write("\n")

    # Hand values from T-0101, checked here so a disagreement stops the task.
    dry = SURFACES["dry"]
    checks = [
        ("D(3200)", peak_force(3200.0, 1.0), 3200.0, 0.01),
        ("D(1600)", peak_force(1600.0, 1.0), 1715.2, 0.01),
        ("D(6400)", peak_force(6400.0, 1.0), 5478.4, 0.01),
        ("D(3379.5)", peak_force(3379.5, 1.0), 3352.20, 0.01),
        ("D(2997)", peak_force(2997.0, 1.0), 3024.38, 0.01),
        ("dry alpha_peak", peak_slip(*dry[1:]), 0.18568, 0.0005),
        ("dry Fy(0.05)", magic(0.05, 3200.0, *dry[1:]), 1911.0, 1911.0 * 0.005),
        ("relax Vx=0 step1", relax(0.0, 3200.0, 0.0, 1.0 / 120.0), 71.91, 0.01),
        ("relax Vx=20 step1", relax(0.0, 3200.0, 20.0, 1.0 / 120.0), 836.55, 0.01),
    ]
    fy = 0.0
    for _ in range(120):
        fy = relax(fy, 3200.0, 0.0, 1.0 / 120.0)
    checks.append(("relax Vx=0 step120", fy, 2990.73, 0.05))

    bad = 0
    for label, got, want, tol in checks:
        ok = abs(got - want) <= tol
        bad += 0 if ok else 1
        print(f"{'OK ' if ok else 'BAD'} {label}: {got:.6f} (hand {want} +/- {tol})")
    print(f"wrote {len(rows)} rows to {out}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
