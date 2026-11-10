# SURGE (Tower Defence) 

## 📖 Project Overview
This project is a dynamic, procedurally generated Tower Defence game built in Unreal Engine 5. Set against a dark, neon-lit cyberpunk aesthetic, the game challenges players to defend a Central Tower against endless waves of mutating enemies. The core design philosophy revolves around procedural unpredictability balanced by strict economic and tactical rules.

## 🔁 Core Gameplay Loop
1. **Defend:** Enemies spawn and follow three procedurally carved lanes toward the Central Tower.
2. **Earn:** Defeating enemies yields Gold.
3. **Expand:** Reaching specific Gold thresholds dynamically unlocks new build slots on the map.
4. **Upgrade:** Players spend Gold to deploy distinct defender classes and upgrade them, enhancing both their mechanical stats and visual intensity.

---

## ⚙️ Core Mechanics & Systems

### Procedural Enemy Mutations (PCG)
Rather than using static enemy templates, every unit is procedurally generated at spawn using a seeded `FRandomStream` (hashed from the enemy's instance name and the wave number) to ensure network-safe determinism. 
* **Continuous Rolls:** Determines subtle hue shifts and physical body size (which derives base health and speed).
* **Discrete Mutations:** Enemies have a chance to spawn with one of five distinct mutations (e.g., *Armoured*, *Volatile*). 
* **The "Trade" Philosophy:** Mutations are strictly stat *trades*, not raw buffs. *Armoured* units gain health but suffer movement speed penalties; *Volatile* units deal massive damage but have brittle health. This protects the mathematical integrity of the Wave Director's threat budget.
* **Pacing Curve:** Mutation probability scales from 15% in Wave 1 up to a hard cap of 65% in later waves, demanding constant tactical adaptation.

### Economy and Wave Director
The Wave Director utilizes a strict threat budget to construct waves. The player's economy is deeply tied to the map itself: as the player hoards Gold, the map physically responds by unlocking new grid slots, forcing the player to decide between upgrading existing towers or expanding their defensive perimeter.

### Defender Classes & Upgrades
Players have access to three primary turrets:
* **Gunner:** High fire rate, single-target damage.
* **Frost:** Crowd control and enemy slowdown.
* **Mortar:** Heavy area-of-effect (Splash) damage.
* **Upgrades:** Every turret (and the Central Tower) can be upgraded twice. Upgrades increase health and damage, while instantly triggering visual feedback (intensified emissive materials and expanding Niagara auras).

---

## 🎨 Aesthetics & Visual Effects (VFX)

The game utilizes a striking, high-contrast neon palette against a void-like background to ensure tactical readability.

* **Procedural Map Scatter:** The map generator carves violet lanes through dark blue buildable terrain, using a secondary PCG pass to scatter neon props (cubes, cylinders) across the map without blocking gameplay paths.
* **Custom Shaders:**
  * **Vertex Displacement:** The terrain uses a time-driven, clamped Sine wave on the World Position Offset to give the map a gentle, digital "breathing" effect.
  * **Post-Process:** A custom CRT scanline material overlays the camera to cement the retro-futuristic hacker aesthetic.
* **Niagara Particle Systems:**
  * **Dynamic Lasers:** Projectiles are handled via instantaneous, dynamic Niagara beam emitters colored specifically to match the firing defender (Cyan, Blue, etc.).
  * **Upgrade Auras:** Upgraded units project a looping "Goku-style" energy sphere that grows denser and brighter with each level.

---

## 💻 Technical Architecture
* **Engine:** Unreal Engine 5 (C++ & Blueprints).
* **UI/UX:** Scalable UMG widgets anchored for varied aspect ratios, featuring dynamic casting to display real-time PCG mutation names over enemy health bars.
* **Event-Driven:** Heavy use of delegates and event dispatchers (e.g., `OnFired`, `OnUpgraded`) to keep the C++ backend decoupled from the Blueprint visual layer.

---

## 🛠️ Team & Credits
* **Latita:** Procedural Generation Architecture (Enemies & Map Scatter), Custom Shaders (Vertex Displacement, CRT Post-Process), Niagara VFX (Lasers, Auras).
* **Sandile:** Progression & Upgrade Architecture, Economic Systems, UI/HUD Layout, Blueprint UI Integration.

---

## 🚀 Build & Installation
*(Build instructions and executable links will be added here prior to final release).*
