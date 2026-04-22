<p align="center">
  <img src="assets/project-thumbnail.png" alt="MaXangle XR Banner" width="100%"/>
</p>

<h1 align="center">MaXangle XR</h1>
<h3 align="center">Precision STEM Engineering in Mixed Reality</h3>

<p align="center">
  <em>Don't just solve the equation — <strong>become</strong> the equation.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Engine-Unreal_Engine_5-313131?style=for-the-badge&logo=unrealengine&logoColor=white" alt="Unreal Engine 5"/>
  <img src="https://img.shields.io/badge/Hardware-Meta_Quest_3-1877F2?style=for-the-badge&logo=meta&logoColor=white" alt="Meta Quest 3"/>
  <img src="https://img.shields.io/badge/Stylus-Logitech_MX_Ink-00B956?style=for-the-badge&logo=logitech&logoColor=white" alt="Logitech MX Ink"/>
  <img src="https://img.shields.io/badge/Backend-C++_Math_Library-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++ Backend"/>
</p>

<p align="center">
  <a href="https://drive.google.com/drive/folders/1xYn6ykOEiC_hHFcao6NhQzIz542DSHHP?usp=sharing">
    <img src="https://img.shields.io/badge/⬇_DOWNLOAD-Sideload_APK_v1.0-brightgreen?style=for-the-badge" alt="Download Build"/>
  </a>
  &nbsp;
  <a href="https://youtu.be/qAW34viKivw">
    <img src="https://img.shields.io/badge/▶_WATCH-Gameplay_Demo-FF0000?style=for-the-badge&logo=youtube&logoColor=white" alt="Watch Demo"/>
  </a>
</p>

---

## 💡 Inspiration

Traditional STEM education is stuck in 2D. We learn complex 3D concepts like trigonometry, kinematics, and spatial geometry on flat whiteboards. A student learning projectile motion stares at a textbook equation, memorizes it, passes the test, and forgets it three weeks later because they never *felt* it.

With the launch of the **Logitech MX Ink**, our team — **Squirtle Squad** — saw an opportunity to shatter that paradigm. While others looked at the stylus and saw a paintbrush, we saw a **micrometer**. We wanted to turn the physical world into a mathematically precise laboratory, building a Mixed Reality experience that feels like a professional CAD tool but plays like a fast-paced arcade game.

> **We didn't just want students to solve equations — we wanted them to *become* the equation.**

---

## 🎯 What It Does

**MaXangle XR** is a gamified Mixed Reality platform where players solve complex math and physics challenges using the Logitech MX Ink as a **precision scientific instrument**.

### Module A — Angle Sniper *(Tactical Kinematics)*

Floating holographic drone targets spawn across your physical room. The UI provides the exact distance to the target. Players must use the kinematic formula to calculate their required launch angle:

$$\theta = \frac{1}{2} \arcsin\left(\frac{g \cdot d}{v^2}\right)$$

You press the MX Ink nib against your real desk — light pressure equals a slow lob; a hard press calculates a high-speed sniper laser. Tilt your wrist until the **Holographic Protractor** floating on the pen tip reads your calculated angle, pull the trigger, and watch the physics projectile bounce off your real walls via Meta's **MRUK room mesh** to obliterate the target.

> **This is not a simulation — the ball obeys the exact equation you just solved.**

### Module B — Geometry Builder *(Procedural Construction)*

Construct 3D shapes in mid-air using the stylus as a **spatial compass**. Our custom tolerance engine enforces a strict **1° validation system**. The target angle is computed from the polygon's interior angle formula:

$$\text{Interior Angle} = \frac{(n - 2) \times 180°}{n}$$

The exact millisecond the user closes a geometrically perfect shape, the UI triggers a **"Snap" color shift**, a haptic click fires in the stylus, and a holographic measurement spawns locked to the shape's true geometric centroid. No rounding. No hand-holding.

### Module C — The "Juice" *(Haptics & Audio)*

We mapped **custom Haptic Effect Curves** directly to the stylus:

| Event | Haptic Response |
|---|---|
| Pen press down | Micro-haptic charge rumble |
| Angle validation success | Heavy, satisfying recoil kick |
| UI interaction | Sharp precision click |

High-tech UI hums and success chimes are spawned at the pen's exact 3D coordinates for flawless **spatial audio panning**.

---

## 🏗️ How We Built It — The Hybrid Architecture

VR demands a flawless **90 FPS** on the Quest 3's Snapdragon XR2 Gen 2 processor. Real-time 6DOF spatial math at that framerate is a non-trivial engineering problem. We engineered a strict **Hybrid execution model** to ensure absolute stability:

### The Heavy Lifter — C++ Math Backend

We offloaded **100%** of the intense 6DOF tracking math, tolerance validation, and Newell's Method polygon calculations into a custom `UMaXangleMathLibrary`:

- Hardware sensor drift causes native `Acos` functions to receive values outside `[-1, 1]` and silently return `NaN`, which **crashes the physics engine**. Our backend applies a strict `FMath::Clamp` before every angle calculation.
- We reimplemented **signed angle detection** using the Cross Product against the World Up Axis, allowing the engine to detect clockwise vs. counter-clockwise stylus torque.

### The Fast Iteration Layer — Blueprint Spline Engine

Because the heavy math is safely isolated in C++, the Blueprint layer simply polls the backend. We drive visual projectile arcs using UE5's `Predict Projectile Path By Trace Channel` node, dynamically stretching **Spline Meshes** to create glowing laser beams that bend with gravity in real-time.

```
┌─────────────────────────────────────────────────────────┐
│                    BLUEPRINT LAYER                       │
│   Spline Visuals  ·  UI Widgets  ·  Game State Machine  │
│─────────────────────────────────────────────────────────│
│                        ▲  polls                         │
│                        │                                │
│─────────────────────────────────────────────────────────│
│                     C++ BACKEND                         │
│   UMaXangleMathLibrary                                  │
│   6DOF Math · Tolerance Validation · Newell's Method    │
│   Clamped Acos · Signed Angle Detection                 │
└─────────────────────────────────────────────────────────┘
```

---

## 🧱 Challenges We Ran Into — The Hardware Hack

We built our entire architecture in **Unreal Engine 5**, only to discover midway through development that there was **no official UE5 integration** for the MX Ink. We faced a binary choice: abandon our C++ math backend and rebuild in Unity from zero, or reverse-engineer the hardware ourselves.

**We chose to fight.**

Through systematic testing, we mapped the MX Ink's legacy inputs directly into UE5's Enhanced Input System:

| MX Ink Hardware | UE5 Mapping |
|---|---|
| Tip Pressure *(analog)* | Thumbstick X-Axis |
| Front Button | Grip Axis |
| Middle Cluster | Trigger *(digital, truncated)* |

> 📣 We shared our mapping documentation back with the Logitech development team to assist future UE5 developers. **We turned a critical blocker into a contribution to the ecosystem.**

---

## 🏆 Accomplishments We're Proud Of

- **True Tip-Origin Tracking** — We ripped out standard VR controller logic (whose pivot point sits *inside the palm*) and wired every module through `GetTrueTipLocation`. Every projectile, spline, and geometric edge originates exactly at the **physical plastic nib**.
- **Wrist-as-Variable** — The physical torque of the user's wrist and the analog pressure against a real desk now dictate the exact variables of an active physics engine.
- **Visceral STEM** — We transformed intimidating 3D math into a hardware-driven, gamified experience that you can *feel*.

---

## 📚 What We Learned

We learned that **hardware limitations are design opportunities**. The absence of an official plugin forced us to understand the hardware at a lower level than any SDK would have allowed.

We also learned that STEM education's real failure is not the curriculum — it's the **embodiment**:

> *When a student presses a nib against a desk and watches a parabola form in their living room, they are not learning projectile motion. They are **experiencing** it. The equation becomes memory because it was first a tactile sensation.*

---

## 🔮 What's Next for MaXangle XR

This prototype is just the foundation of a **Comprehensive Spatial STEM Ecosystem**. Our roadmap:

| Initiative | Description |
|---|---|
| 🤖 **Spatial AI Co-Pilot** | Multimodal LLM assistant that *sees* what the player is building in MR, providing real-time diegetic voice guidance and holographic visual hints when a tolerance fails. |
| 🔌 **Native OpenXR Plugin** | Custom C++ OpenXR plugin to manually expose the native `XR_LOGITECH_mx_ink_stylus_interaction` profile inside Unreal. |
| 🔧 **Precision Engineering** | Industrial module where players repair virtual CAD structures with sub-millimeter error margins. |
| 🌀 **The Equation Portal** | Algebraic sandbox where players physically graph functions (like $y = x^2$) in 3D space to unlock portals. |
| 🌍 **Global Multiplayer Workshops** | Students across the globe collaborate on the same 3D geometric blueprint in a shared Mixed Reality space. |

---

## ⚙️ Installation & Setup

1. **Download** the `.apk` from the [Releases](https://drive.google.com/drive/folders/1xYn6ykOEiC_hHFcao6NhQzIz542DSHHP?usp=sharing) page.
2. **Sideload** using [SideQuest](https://sidequestvr.com/) or Meta Quest Developer Hub.
3. **Pair** your Logitech MX Ink stylus via Bluetooth.
4. **Launch** MaXangle XR and define your passthrough boundary.

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| Engine | Unreal Engine 5.x |
| Platform | Meta Quest 3 (Snapdragon XR2 Gen 2) |
| Stylus | Logitech MX Ink (6DOF) |
| Math Backend | Custom C++ `UBlueprintFunctionLibrary` |
| Scene Understanding | Meta MRUK (Mixed Reality Utility Kit) |
| UI / UX | Frosted glass + neon hologram 3D widgets |
| Haptics | Custom curve-based feedback engine |
| Audio | Spatial 3D audio at pen coordinates |

---

<p align="center">
  <strong>Developed for the Logitech × Devpost MX Ink Hackathon 2026</strong><br/>
  <sub>Team <strong>Squirtle Squad</strong> by Collision Mob</sub>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Hackathon-Logitech_DevStudio_2026-7B68EE?style=flat-square" alt="Hackathon Badge"/>
</p>
