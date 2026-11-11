# SURGE

**Genre:** Procedural Tower Defence  
**Engine:** Unreal Engine 5  
**Development Team:** Latita Mvunelo (PCG, Shaders, VFX, UI, C++) & Sandile Dube (PCG Map, Progression Architecture, C++, UI)  

## 📖 Overview

**SURGE** is a dynamic, cyberpunk-themed tower defence game built around procedural generation and adaptive strategy. Rather than fighting static waves of identical instances, players must defend against a continuously mutating threat. Every enemy is mathematically rolled at spawn, forcing players to adapt their static grid defences to highly variable enemy compositions.

## 🔁 Core Gameplay Loop

1. **Defend:** Place specialized defenders (Gunner, Frost, Mortar) along procedurally generated lanes to stop enemy leaks.
2. **Adapt:** Analyze enemy mutations in real-time via UI health bar broadcasts (e.g., identifying heavily armoured slow units vs. fragile, highly volatile units).
3. **Expand:** Earn gold from eliminations to purchase structural upgrades, intensifying defender VFX and dynamically unlocking new tactical build slots.

## ⚙️ Key Features & Architecture

### 1. Procedural Enemy Mutation (PCG)

Enemies are generated at spawn using a deterministic, seeded `FRandomStream` (hashed via instance name and wave number) ensuring network-safe reproducibility. Mutations are designed strictly as **trades** rather than raw buffs to protect the wave director's threat budget:

* **Continuous Rolls:** Determines subtle hue shifts and physical body size.
* **Armoured:** Significant health increase / Severe movement speed penalty.
* **Volatile:** Massive damage output / Drastic maximum health reduction.
* **Pacing Curve:** Mutation probability scales dynamically from 15% (Wave 1) to a 65% cap in later stages.

### 2. C++ Progression & Economy

* **Upgrades:** Defenders and the Central Tower feature a two-tier upgrade system. C++ logic handles budget verification and stat scaling, broadcasting an `OnUpgraded` event to safely decouple logic from the visual layer.
* **Dynamic Expansion:** Hitting specific round-number economy thresholds automatically unlocks and activates new build slots across the grid.
* **Scalable UI:** UMG widget layouts are strictly anchored and structured using Box containers and translucent borders to ensure high legibility and zero clipping across varying monitor resolutions.

### 3. Custom Shaders & Cyberpunk Aesthetics

* **Vertex Displacement Terrain:** The map environment breathes dynamically using a time-driven, clamped Sine wave fed into the World Position Offset pin, safely contained to prevent mesh explosions.
* **CRT Post-Process:** A fully custom post-process material overlays holographic CRT scanlines across the camera, cementing the retro-futuristic aesthetic.
* **Dynamic Emissives:** Unit materials utilize `BaseTint` and `UpgradeLevel` parameters exposed to C++ to overdrive their emissive output upon leveling up.

### 4. Niagara VFX

* **Dynamic Lasers (`NS_Tracer`):** Defenders fire absolute-world-space beam emitters that read exact `HitLocation` vector coordinates from the C++ `OnFired` event. 
* **Upgrade Auras (`NS_UpgradeAura`):** A surface-only particle sphere scales in spawn-rate intensity based on the unit's current upgrade level.
