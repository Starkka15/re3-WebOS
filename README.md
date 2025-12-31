<img src="https://github.com/Jai-JAP/re-GTA/blob/re3/res/images/logo_1024.png?raw=true" alt="re3 logo" width="200">

# re3 - WebOS Port (HP TouchPad)

## ⚠️ **WORK IN PROGRESS - NOT PLAYABLE YET**

This is a **webOS port** of re3 (GTA III reverse engineered) for the **HP webOS TouchPad (2011)**, NOT for Nintendo Switch or other platforms.

**Current Status:** Proof of Concept - Partial Functionality ✅
**Last Updated:** December 31, 2025

---

## 🎯 What Works

- ✅ Game launches and initializes
- ✅ SDL2 window and OpenGL ES 2.0 context creation
- ✅ Touch input system with virtual controls
- ✅ File I/O and case-insensitive file loading
- ✅ Texture loading (1,408+ textures successfully loaded)
- ✅ Cutscene playback with skip functionality
- ✅ 2D rendering (HUD, menus, sprites)
- ✅ Asset streaming system (CD streaming operational)
- ✅ Game loop and timer systems

## ❌ Known Issues & Limitations

### **CRITICAL - Game-Breaking:**
1. **Animation System Crash** - Game crashes when trying to initialize character animations (~20 seconds after launch). This prevents gameplay from starting.
2. **Basic 3D Rendering** - Caused by shader compilation failures on Adreno 220 GPU (GLSL ES 1.00 limitations).

### **Major Issues:**
3. **No Audio** - Cutscenes and gameplay have no sound
4. **OpenGL Errors** - GL_INVALID_ENUM errors (non-critical but present)

### **Hardware Limitations:**
- **GPU:** Adreno 220 doesn't support advanced GLSL features (dynamic array indexing, complex shaders)
- **Memory:** 1 GB RAM limits asset loading
- **Performance:** Target 30 FPS, older hardware may struggle

**For detailed technical information, see:** [PROGRESS_DEC31_2025.md](../PROGRESS_DEC31_2025.md)

---

## 🖥️ Target Hardware

- **Device:** HP webOS TouchPad (2011)
- **CPU:** Qualcomm Snapdragon APQ8060 (Dual-core 1.2 GHz)
- **GPU:** Adreno 220 (OpenGL ES 2.0 / GLSL ES 1.00)
- **RAM:** 1 GB
- **Resolution:** 1024×768
- **OS:** webOS 3.0.5

---

## 📚 About re3

re3 is a fully reversed source code for GTA III. The original re3 project works on Windows, Linux, MacOS, and FreeBSD. It has been ported to [Nintendo Switch](https://github.com/AGraber/re3-nx/), [Playstation Vita](https://github.com/Rinnegatamante/re3), and [Nintendo Wii U](https://github.com/GaryOderNichts/re3-wiiu/).

**This repository is specifically for the webOS port only.**

## Installation (WebOS)

**Requirements:**
- re3 requires PC game assets to work, so you **must** own [a copy of GTA III](https://store.steampowered.com/app/12100/Grand_Theft_Auto_III/)
- HP webOS TouchPad with webOS 3.0.5
- webOS SDK installed on development machine
- GTA III game files copied to device at `/media/internal/.gta3/`

**Current State:** No stable builds available yet. You must build from source (see below).

---

## Improvements

We have implemented a number of changes and improvements to the original game.
They can be configured in `core/config.h`.
Some of them can be toggled at runtime, some cannot.

* Fixed a lot of smaller and bigger bugs
* User files (saves and settings) stored in GTA root directory
* Settings stored in re3.ini file instead of gta3.set
* Debug menu to do and change various things (Ctrl-M to open)
* Debug camera (Ctrl-B to toggle)
* Rotatable camera
* XInput controller support (Windows)
* No loading screens between islands ("map memory usage" in menu)
* Skinned ped support (models from Xbox or Mobile)
* Rendering
  * Widescreen support (properly scaled HUD, Menu and FOV)
  * PS2 MatFX (vehicle reflections)
  * PS2 alpha test (better rendering of transparency)
  * PS2 particles
  * Xbox vehicle rendering
  * Xbox world lightmap rendering (needs Xbox map)
  * Xbox ped rim light
  * Xbox screen rain droplets
  * More customizable colourfilter
* Menu
  * Map
  * More options
  * Controller configuration menu
  * ...
* Can load DFFs and TXDs from other platforms, possibly with a performance penalty
* ...

## To-Do

The following things would be nice to have/do:

* Fix physics for high FPS
* Improve performance on lower end devices, especially the OpenGL layer on the Raspberry Pi (if you have experience with this, please get in touch)
* Compare code with PS2 code (tedious, no good decompiler)
* [PS2 port](https://github.com/GTAmodding/re3/wiki/PS2-port)
* Xbox port (not quite as important)
* reverse remaining unused/debug functions
* compare CodeWarrior build with original binary for more accurate code (very tedious)

## Modding

Asset modifications (models, texture, handling, script, ...) should work the same way as with original GTA for the most part.

CLEO scripts work with [CLEO Redux](https://github.com/cleolibrary/CLEO-Redux).

Mods that make changes to the code (dll/asi, limit adjusters) will *not* work.
Some things these mods do are already implemented in re3 (much of SkyGFX, GInput, SilentPatch, Widescreen fix),
others can easily be achieved (increasing limis, see `config.h`),
others will simply have to be rewritten and integrated into the code directly.
Sorry for the inconvenience.

## Building from Source (WebOS)

**Prerequisites:**
- webOS SDK 3.0.5
- CMake 3.10+
- WSL (Windows Subsystem for Linux) or Linux development machine
- webOS Toolchain

**Build Configuration:**
```cmake
LIBRW_PLATFORM=GL3               # GLES2+ with shaders
LIBRW_GL3_GFXLIB=SDL2           # SDL2 for window/input
CMAKE_BUILD_TYPE=Release
WEBOS_TOUCHPAD=ON
CMAKE_TOOLCHAIN_FILE=cmake/webos/WebOSToolchain.cmake
```

**Build Steps:**
```bash
# Clone with submodules
git clone --recursive https://github.com/Starkka15/re3-WebOS.git
cd re3-WebOS

# Create build directory
mkdir build-webos-wsl
cd build-webos-wsl

# Configure with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/webos/WebOSToolchain.cmake \
         -DLIBRW_PLATFORM=GL3 \
         -DLIBRW_GL3_GFXLIB=SDL2 \
         -DWEBOS_TOUCHPAD=ON \
         -DCMAKE_BUILD_TYPE=Release

# Build
make -j4
```

**Output:** `build-webos-wsl/src/re3` (~7.5 MB)

**Packaging for WebOS:**
```bash
./package_webos.sh
```

> :information_source: Various webOS-specific settings can be found in `src/core/config.h` under `#ifdef WEBOS_TOUCHPAD`

> :information_source: This port uses [librw](https://github.com/aap/librw/) (GL3 backend) for rendering, and custom SDL2 compatibility layers for webOS.

## Contributing (WebOS Port)

This is a platform-specific port. Contributions welcome for:

- **Bug fixes** for webOS-specific issues (wrapped in `#ifdef WEBOS_TOUCHPAD`)
- **Performance optimizations** for Adreno 220 GPU and older ARM hardware
- **Shader fixes** for GLSL ES 1.00 compatibility
- **Touch control improvements**
- **Documentation** of webOS-specific quirks and fixes

All webOS-specific code should be wrapped in preprocessor conditions (`#ifdef WEBOS_TOUCHPAD`).

See [CODING_STYLE.md](CODING_STYLE.md) for style guidelines. Avoid C++11 or later features.


## History

### Original re3 Project
re3 was started in spring 2018 as a reverse engineering project for GTA III. After years of community effort, a standalone executable was achieved in April 2020. The project has since expanded to include Vice City (reVC) and Liberty City Stories (reLCS).

### WebOS Port (This Repository)
The webOS port was started in December 2025, targeting the HP webOS TouchPad (2011). This port focuses on:
- ARM architecture compatibility (Snapdragon APQ8060)
- OpenGL ES 2.0 / GLSL ES 1.00 rendering
- Touch-based controls for tablet interface
- Low-end hardware optimization (1 GB RAM, Adreno 220 GPU)

**Development Status:**
- **Dec 27-31, 2025:** Initial port development, achieved proof-of-concept with partial functionality
- **Current:** Working on animation system crash and 3D rendering issues

See [PROGRESS_DEC31_2025.md](../PROGRESS_DEC31_2025.md) for detailed development log.


## License

We don't feel like we're in a position to give this code a license.\
The code should only be used for educational, documentation and modding purposes.\
We do not encourage piracy or commercial use.\
Please keep derivative work open source and give proper credit.

---

## Credits

### Original re3 Project
- **re3 Team:** aap, Fire_Head, shfil, erorcun, Nick007J, Serge, and many contributors
- **LibRW:** aap - RenderWare reimplementation

### WebOS Port
- **Port Developer:** Starkka15
- **Reference Ports:** Vita port (TheOfficialFloW/Rinnegatamante) - ARM reference implementation
- **Tools:** Claude AI - Debugging assistance and code analysis

### Resources
- Original re3: [GTAmodding/re3](https://github.com/GTAmodding/re3)
- LibRW: [aap/librw](https://github.com/aap/librw)
- Vita Port: [TheOfficialFloW/re3](https://github.com/TheOfficialFloW/re3)
- webOS SDK: [HP webOS SDK 3.0.5](https://sdk.webosarchive.org/)

---

**Repository:** https://github.com/Starkka15/re3-WebOS
**Issues:** Please report webOS-specific issues in this repository's issue tracker
**Documentation:** See [WEBOS-PORTING-STATUS.md](WEBOS-PORTING-STATUS.md) and [WEBOS_ISSUES_AND_FIXES.md](WEBOS_ISSUES_AND_FIXES.md) for technical details
