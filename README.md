# Gravity Defied C++ PSP (AI Homebrew Port)

**Gravity Defied** is a legendary mototrial racing mobile game. It was originally developed by Codebrew Software in 2004 for J2ME platform.

This project is an experimental native PlayStation Portable (PSP) homebrew port based on the C++ / SDL2 codebase, brought to the platform with the assistance of AI development tools (Google Jules). It aims to deliver all features and authentic physics of the original game directly on real PSP hardware and emulators.

***Disclaimer  
This project is an unofficial fan homebrew and is not associated with Codebrew Software in any fashion. All rights to the original Gravity Defied, including its name, logotype, brand, assets, and original codebase, belong to Codebrew Software.***

## Custom Level Packs

The release build includes the original track set along with **5 top community level packs from [gdtr.net](https://gdtr.net)** ready to play.

### Adding Your Own Tracks
Put any `.mrg` level pack files into the `levels/` folder inside the game directory:  
`ms0:/PSP/GAME/<GravityDefied_Folder>/levels/your_pack.mrg`

Select your pack in **Menu > Play > Level pack:**. Each pack maintains its own independent track progress and saves.

---

## Controls

### Menu Navigation

| Button | Action |
| :--- | :--- |
| **D-Pad / Analog Stick** | Navigate menu options |
| **✕ (Cross) / START** | Confirm / Select |
| **○ (Circle)** | Back / Cancel |
| **HOME / PS** | Open standard PSP exit dialog |
| **△, □, SELECT, L, R** | Disabled in menus |

---

### In-Game (Race)

Controls replicate the authentic J2ME mobile phone keypad layout:

| Action | D-Pad / Analog Stick | Face Buttons | J2ME Equivalent |
| :--- | :--- | :--- | :--- |
| **Accelerate (Gas)** | **Up** | **△ (Triangle)** | `KEY_NUM2` / `UP` |
| **Brake / Reverse** | **Down** | **✕ (Cross)** | `KEY_NUM8` / `DOWN` |
| **Lean Backward** | **Left** | **□ (Square)** | `KEY_NUM4` / `LEFT` |
| **Lean Forward** | **Right** | **○ (Circle)** | `KEY_NUM6` / `RIGHT` |

#### Shortcuts & System

* **L Trigger:** Gas + Lean Backward (`KEY_NUM1`)
* **R Trigger:** Gas + Lean Forward (`KEY_NUM3`)

---

## Acknowledgements

* **[antim0118](https://github.com/antim0118)** — For the gLib2D over sceGu rendering approach and libraries.
