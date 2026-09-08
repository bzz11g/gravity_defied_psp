# Gravity Defied (PSP Port) Agent Guidelines

## PSP Build Instructions

This project targets the PlayStation Portable (PSP) hardware. It requires the PSPDEV toolchain to build the target artifact (`EBOOT.PBP`).

### Toolchain Prerequisites
- A working PSPDEV toolchain installation.
- Ensure the `psp-cmake` and `psp-g++` binaries are available in your `$PATH`.
- Typically, the toolchain is located at `/usr/local/pspdev`.

Before building, set up the environment:
```bash
export PATH=/usr/local/pspdev/bin:$PATH
```

### Build Commands
To compile and produce `EBOOT.PBP` from scratch:
```bash
# 1. Generate build files with psp-cmake
psp-cmake -B build-psp

# 2. Compile and package the binary
make -C build-psp
```

The resulting PSP executable will be generated at `build-psp/EBOOT.PBP`.

### Project Structure & PSP Hardware Dependencies
- **C++ Source Files**: Located in `src/`.
- **Game Assets**: Located in `assets/`.
- **PSP Specific Macros**: Located in `src/main.cpp` (using `PSP_MODULE_INFO` and `<pspkernel.h>`).
- **Dependencies**: The project requires `SDL2`, `SDL2_ttf`, and `SDL2_image` ports for PSP, linked automatically via standard CMake `pkg-config` and `target_link_libraries`.
- **CMake config**: The primary configuration is in `CMakeLists.txt`. For PSP builds, the toolchain provides a `create_pbp_file` macro to package the EBOOT.PBP.

### Asset Packing Rules
When the `EBOOT.PBP` is generated, it bundles the executable along with the following UI assets from the `assets/` directory:
- **Title (SFO metadata)**: `Gravity Defied: Trial Racing`
- **XMB Icon**: `assets/ICON0.png`
- **XMB Background**: `assets/PIC1.png`

These are defined inside `CMakeLists.txt` using the `create_pbp_file` command. The PSP DEV script `pack-pbp` uses these arguments to assemble the `EBOOT.PBP` output.

### Guidelines for Testing
- The output `EBOOT.PBP` can be tested using the **PPSSPP** emulator or executed on **real PSP hardware** running Custom Firmware (CFW).
- For real hardware, copy the output `EBOOT.PBP` to `ms0:/PSP/GAME/GravityDefied/EBOOT.PBP`.
- Automated testing (`ctest`) is currently unconfigured for this target. Verification must be done either manually (via emulator/hardware).

### File Boundaries
- Do not modify or commit files in the `build-psp/` directory (these are generated outputs, such as intermediate `.o` / `.obj` files, and raw ELF binaries).
- Do not manually construct or commit `PARAM.SFO`. It is generated automatically during the build process via `mksfo` or `mksfoex`.
- Do not add Windows or other platform-specific metadata files (e.g., MSVC projects) as this build focuses on PSP hardware.
