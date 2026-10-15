# CODEX — Canonical Physics, 120 Hz Chaos C++ & GIS Specification

**Status:** Reconciled Mathematical & Engineering Reference  
**Modules Covered:** `CodexCore`, `CodexVehicleDynamics`, `CodexVehicles`, `CodexWorld`

---

## 1. Baseline Vehicle Parameterization (1985–1990 RWD Sedan)

Models a longitudinal front-engine, rear-wheel-drive compact executive sedan (E30 / W201 / R31 class) with front MacPherson struts and rear semi-trailing arm independent suspension.

| Parameter | Symbol | Canonical Value | Unit | Implementation Notes |
| :--- | :--- | :--- | :--- | :--- |
| **Total Mass (Curb + Driver)** | $m$ | `1300.0` | $\text{kg}$ | Sprung mass $m_s = 1140.0\text{ kg}$ |
| **Unsprung Mass (F / R per corner)** | $m_{u,f} / m_{u,r}$ | `38.0 / 42.0` | $\text{kg}$ | Total unsprung $= 160.0\text{ kg}$ |
| **Weight Distribution (F / R)** | $W_f / W_r$ | `53.0 / 47.0` | $\%$ | Static axle loads: $F_{z,f} = 6759\text{ N}$ ($3379.5\text{ N/wheel}$), $F_{z,r} = 5994\text{ N}$ ($2997.0\text{ N/wheel}$) |
| **Center of Gravity Height** | $h_{\text{CG}}$ | `0.510` | $\text{m}$ | Above ground plane at static ride height |
| **Wheelbase** | $L$ | `2.620` | $\text{m}$ | CG to front axle $a = 1.231\text{ m}$; CG to rear axle $b = 1.389\text{ m}$ |
| **Track Width (F / R)** | $t_f / t_r$ | `1.440 / 1.450` | $\text{m}$ | Static Stability Factor $\text{SSF} = 1.417$ |
| **Inertia Tensor (Roll / Pitch / Yaw)** | $I_{xx} / I_{yy} / I_{zz}$ | `440.0 / 2150.0 / 2280.0` | $\text{kg}\cdot\text{m}^2$ | Dynamic index $k^2 / (a \cdot b) = 1.03$ |
| **Effective Wheel Spring Rate (F / R)** | $k_{w,f} / k_{w,r}$ | `26000.0 / 29000.0` | $\text{N/m}$ | Ride frequencies: $1.42\text{ Hz}$ (F), $1.60\text{ Hz}$ (R) |
| **Split Damper Rate — Bump (F / R)** | $c_{\text{bump},f/r}$ | `2200.0 / 2400.0` | $\text{N}\cdot\text{s/m}$ | $\zeta_{\text{bump}} \approx 0.28$ (prevents harshness over sharp joints) |
| **Split Damper Rate — Rebound (F / R)** | $c_{\text{reb},f/r}$ | `3400.0 / 3800.0` | $\text{N}\cdot\text{s/m}$ | $\zeta_{\text{rebound}} \approx 0.44$ (prevents jacking & controls roll transient) |

---

## 2. Reconciled Pacejka MF94 Tire Model & Combined Slip

### 2.1 Pure Lateral Force ($195/65\text{ R15}$ at Nominal $F_{z0} = 3200\text{ N}$)
Evaluated with slip angle $\alpha$ in **radians**:
$$F_{y0}(\alpha) = D \sin\left(C \arctan\left(B\alpha - E(B\alpha - \arctan(B\alpha))\right)\right)$$

| Surface Regime | Peak Friction $\mu_y$ | Stiffness $B$ ($\text{rad}^{-1}$) | Shape $C$ | Peak $D$ ($\text{N}$) | Curvature $E$ | Cornering Stiffness $C_{F\alpha}$ ($\text{N/rad}$) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Dry Clean Asphalt** | `1.00` | `10.00` | `1.30` | `3200.0` | `-1.00` | `41600.0` |
| **Wet Tarmac** | `0.75` | `10.25` | `1.30` | `2400.0` | `-1.40` | `31980.0` |
| **Loose Gravel** | `0.50` | `8.50` | `1.25` | `1600.0` | `-0.30` | `17000.0` |
| **Hard Snow / Ice** | `0.20` | `9.00` | `1.20` | `640.0` | `-2.00` | `6912.0` |

### 2.2 Load Sensitivity, Relaxation Length & Friction Ellipse
1. **Degressive Load Sensitivity $D(F_z)$:**
   $$D(F_z) = F_z \cdot \mu_{\text{surface}} \cdot \left[ 1.0 + p_{Dy2} \left( \frac{F_z - F_{z0}}{F_{z0}} \right) \right], \quad p_{Dy2} = -0.144, \quad F_{z0} = 3200\text{ N}$$
2. **First-Order Transient Tire Force Relaxation:**
   $$\frac{dF_y}{dt} = \frac{\max(\vert{}V_x\vert{}, 1.5\text{ m/s})}{\sigma_y} \left( F_{y,\text{ss}}(\alpha) - F_y \right), \quad \sigma_y = \frac{C_{F\alpha}}{K_y} \approx 0.55\text{ m} \quad (K_y = 75000\text{ N/m})$$
3. **Combined Lateral + Longitudinal Slip (Friction Circle Enforcement):**
   Compute pure longitudinal force $F_{x0}(\kappa)$ and pure lateral force $F_{y0}(\alpha)$, then normalize via combined slip magnitude $\sigma_{\text{tot}} = \sqrt{(\kappa / \kappa_{\text{peak}})^2 + (\alpha / \alpha_{\text{peak}})^2}$ so total horizontal force $\sqrt{F_x^2 + F_y^2} \le D(F_z)$.

### 2.3 Altitude Barometric & Thermal Power Loss
Given orthometric world elevation $h$ (in meters above NAVD88 sea level):
$$P(h) = P_0 \left(1 - 0.0000225577 \cdot h\right)^{5.25588}$$
Naturally aspirated engine torque scales by air density ratio $\rho(h)/\rho_0$. For carbureted engines without manual altitude mixture adjustment, fuel flow remains fixed while oxygen mass drops, enriching AFR and reducing combustion efficiency above $2,500\text{ m}$.

---

## 3. Deterministic State Struct (`FCodexVehicleSimulationState`)

Includes forward-compatible hooks for **Tire Relaxation Persistence**, **Dynamic Cargo Inertia**, **Fuel Sloshing**, and **Part Degradation**:

```cpp
USTRUCT(BlueprintType)
struct CODEXVEHICLEDYNAMICS_API FCodexVehicleSimulationState
{
    GENERATED_BODY()

    UPROPERTY() uint32 SimulationFrame = 0;
    UPROPERTY() double SimulationTimeSeconds = 0.0;

    // Rigid Body State (SI Units internally: m, m/s, rad/s)
    UPROPERTY() FVector WorldLocation = FVector::ZeroVector;
    UPROPERTY() FQuat WorldRotation = FQuat::Identity;
    UPROPERTY() FVector LinearVelocity = FVector::ZeroVector;
    UPROPERTY() FVector AngularVelocity = FVector::ZeroVector;

    // Powertrain State
    UPROPERTY() float EngineRPM = 800.0f;
    UPROPERTY() float ThrottlePosition = 0.0f;
    UPROPERTY() float ClutchEngagement = 1.0f;
    UPROPERTY() int32 CurrentGear = 1;
    UPROPERTY() float DriveshaftRPM = 0.0f;
    UPROPERTY() float CarburetorMixtureTrim = 0.0f; // Altitude AFR screw adjustment

    // Corner States [0:FL, 1:FR, 2:RL, 3:RR]
    UPROPERTY() float SuspensionCompression[4] = {0.f, 0.f, 0.f, 0.f};
    UPROPERTY() float SuspensionVelocity[4] = {0.f, 0.f, 0.f, 0.f};
    UPROPERTY() float WheelAngularVelocity[4] = {0.f, 0.f, 0.f, 0.f};
    UPROPERTY() float TireSlipAngle[4] = {0.f, 0.f, 0.f, 0.f};
    UPROPERTY() float TireSlipRatio[4] = {0.f, 0.f, 0.f, 0.f};
    UPROPERTY() FVector2f TireRelaxedForce[4]; // [Fx, Fy] persisted for deterministic replay
    UPROPERTY() float TirePressurePSI[4] = {32.0f, 32.0f, 32.0f, 32.0f};
    UPROPERTY() float BrakeThermalStateC[4] = {20.0f, 20.0f, 20.0f, 20.0f};

    // Dynamic Mass, Slosh & Condition State
    UPROPERTY() float FuelRemainingLiters = 50.0f;
    UPROPERTY() FVector2f FuelSloshOffset = FVector2f::ZeroVector; // 2D pendulum state
    UPROPERTY() FVector2f FuelSloshVelocity = FVector2f::ZeroVector;
    UPROPERTY() float AddedCargoMassKg = 0.0f;
    UPROPERTY() FVector AddedCargoLocalCoM = FVector::ZeroVector; // Parallel-axis inertia shift
    UPROPERTY() float EngineCoolantTempC = 85.0f;
    UPROPERTY() float BatteryChargeAh = 55.0f;
    UPROPERTY() double MechanicalOdometerMeters = 184320000.0; // Driven by wheel rotation
};