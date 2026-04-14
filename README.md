# 🚗 The Infinite Drive

> A cinematic, real-time animated scene viewer built entirely with OpenGL and GLUT — every pixel drawn using custom DDA line and Midpoint Circle algorithms, with no texture or image assets.

---

## 📖 Description

**The Infinite Drive** is a real-time 2D animated graphics demo written in C. It renders five distinct hand-crafted cinematic scenes — from a rain-soaked neon city at night to a coastal river at sunset — using only fundamental drawing primitives (DDA line algorithm and Midpoint Circle Algorithm).

The project demonstrates how far classical computer graphics techniques can go: atmospheric scattering, volumetric fog, bloom glow, Fresnel water effects, wet-surface reflections, particle systems, and parallax camera movement — all implemented from scratch in OpenGL immediate mode.

---

## ✨ Features

- **5 fully animated scenes**, each with a unique time-of-day and environment:
  - 🌃 Scene 1 — City Night (rain-wet neon city, traffic, buildings)
  - 🌄 Scene 2 — Mountain Road at Dawn (god rays, atmospheric mist, pine trees)
  - 🏡 Scene 3 — Village Afternoon (stone cottages, windmill, chimney smoke physics)
  - 🌾 Scene 4 — Harvest Crop Field (golden hour, corn rows, animated bird flock)
  - 🌊 Scene 5 — River & Sea at Sunset (lighthouse beam, sailboats, Gerstner waves)

- **All rendering done with two primitives only:**
  - DDA (Digital Differential Analyzer) line algorithm
  - Midpoint Circle Algorithm (MCA)

- **Cinematic visual effects:**
  - Multi-stop sky gradient rendering (scan-line accurate)
  - Bloom/glow halos around light sources
  - Atmospheric perspective (fog blending by distance)
  - Alpha-blended volumetric effects (god rays, dust, haze)
  - Animated smoke particle system
  - Wet-surface reflections and rain puddles
  - Fresnel-style ocean and river with Gerstner waves
  - Rotating lighthouse beam (volumetric cone)
  - Animated walking person, flocking birds, moving vehicles

- **Scene management:**
  - Auto-advance every 12 seconds with fade in/out transitions
  - Manual navigation (next / previous)
  - HUD with scene name, progress bar, and dot indicators

- **Cinematic camera:** Subtle sinusoidal horizontal pan per scene

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| Language | C (C11 standard) |
| Graphics API | OpenGL (immediate mode) |
| Windowing | GLUT / freeglut |
| Math | Standard C `<math.h>` |
| Build System | GNU Make (`Makefile`) |
| Platform Support | macOS (GLUT framework) and Linux (freeglut) |

> **Note:** No external image assets, no shaders, no textures. All visuals are computed purely through geometry and color.

---

## 💻 Installation

### Prerequisites

**macOS:**
- Xcode Command Line Tools (includes `cc`, OpenGL, and GLUT frameworks)
```bash
xcode-select --install
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install freeglut3-dev libglu1-mesa-dev gcc make
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install freeglut-devel mesa-libGLU-devel gcc make
```

### Build

Clone or download the repository, then build with:

**macOS:**
```bash
make
```

**Linux:**
```bash
make CC=cc LDLIBS='-lglut -lGLU -lGL -lm'
```

This compiles all `.c` source files and links them into the `infinite_drive` executable.

To clean build artifacts:
```bash
make clean
```

---

## 🚀 Usage

Run the compiled binary directly:

```bash
./infinite_drive
```

Or use the Makefile shortcut:

```bash
make run
```

### Controls

| Key | Action |
|---|---|
| `SPACE` | Advance to next scene |
| `D` | Go back to previous scene |
| `ESC` | Quit the application |

Scenes also auto-advance every **12 seconds** with a smooth fade transition.

---
## Screenshots

### Scene 1
![Screenshot 2](https://i.ibb.co.com/DDjftbK9/Screenshot-2026-04-15-at-2-23-10-AM.png)

### Scene 2
![Screenshot 1](https://i.ibb.co.com/ycfkpFZ8/Screenshot-2026-04-15-at-2-23-19-AM.png)

### Scene 3
![Screenshot 3](https://i.ibb.co.com/WWS718jQ/Screenshot-2026-04-15-at-2-23-28-AM.png)

### Scene 4
![Screenshot 4](https://i.ibb.co.com/VWkNJjJB/Screenshot-2026-04-15-at-2-23-36-AM.png)

### Scene 5
![Screenshot 5](https://i.ibb.co.com/xq73SmHm/Screenshot-2026-04-15-at-2-23-44-AM.png)

---

## 📁 Project Structure

```
infinite_drive/
├── drive.h                      # Central header — extern declarations, Stop type, all prototypes
├── globals.c                    # All global state: window size, scene/timer state, animation variables
├── primitives.c                 # Core drawing engine: DDA, MCA, glow, skyGrad, drawStars, drawPerson
├── scene1.c                     # City Night — buildings, neon signs, traffic lights, wet road, cars
├── scene2.c                     # Mountain Dawn — mountain fill, snow caps, pine trees, god rays, fog
├── scene3.c                     # Village Afternoon — cottages, windmill, smoke particles, cobblestones
├── scene4.c                     # Harvest Field — cinematic corn, scarecrows, bird flock, camera pan
├── scene5.c                     # River & Sea — lighthouse, sailboats, waves, caustics, bank vegetation
├── main_app.c                   # GLUT entry point, display loop, HUD, fade system, keyboard, timer
├── code_monolithic_backup.c     # Original single-file version (reference only, not compiled)
├── Makefile                     # Build rules for macOS and Linux
└── .gitignore                   # Excludes binaries and media files
```

### Key Files Explained

| File | Role |
|---|---|
| `drive.h` | The single shared header tying all modules together. Declares all globals as `extern`, defines the `Stop` struct for sky gradients, and prototypes every public function. |
| `primitives.c` | The graphics foundation. All scenes call these routines — nothing in a scene file touches OpenGL directly except through these helpers. |
| `globals.c` | Owns all mutable global state. Animation positions (`c1a`, `c5beam`, `bx[]`, etc.), window size (`W`, `H`), timing (`T`, `sceneTimer`), and the star field arrays live here. |
| `main_app.c` | The application shell: GLUT setup, the `timerCB` running at 60 Hz advancing all animations, `display`, `keyboard`, and the HUD/fade overlay. |
| `scene[1-5].c` | Self-contained scene renderers. Each exports one function (`scene1()` … `scene5()`) that draws its entire frame back-to-front. |

---

## 🎨 Rendering Architecture

The project uses a **layered painter's algorithm** — elements are drawn back-to-front within each scene:

1. Sky gradient (full-screen scan lines)
2. Sun / moon / stars
3. Distant background (mountains, buildings, ocean)
4. Mid-ground (trees, cornfields, road)
5. Foreground objects (vehicles, people, boats)
6. Atmospheric overlay (fog, dust, haze)
7. HUD and fade overlay (always drawn last, on top)

All transparency uses standard OpenGL `GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA` blending.

---

## ⚙️ Configuration

There are no configuration files or environment variables. All tuneable constants are defined directly in source:

| Constant | Location | Default | Description |
|---|---|---|---|
| `SCENE_DUR` | `globals.c` | `12.0f` s | How long each scene displays before auto-advancing |
| `N_SCENES` | `globals.c` | `5` | Total number of scenes |
| Window size | `main_app.c` | `1920 × 1080` | Initial window dimensions (resizable) |
| `glPointSize` | `main_app.c` | `1.5f` | Base pixel size for all drawn points |

---

## 📸 Screenshots / Demo

*No screenshots are included in the repository (image files are excluded by `.gitignore`).*

Run the program to see the live animated output. Each scene runs for 12 seconds and transitions automatically with a fade effect.

---

## 🤝 Contributing

Contributions are welcome. Suggested areas for improvement:

- Additional scenes (desert, arctic, space)
- Higher-resolution rendering modes
- Audio synchronization
- Scene duration/sequencing via a config file
- Port to modern OpenGL (VBOs, shaders) while preserving the algorithmic aesthetic

**To contribute:**

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-scene`
3. Commit your changes with clear messages
4. Open a pull request describing what you added and why

Please keep all rendering within the DDA / MCA primitive constraint — that is the defining spirit of the project.

---

## 📄 License

This project is licensed under the MIT License.
See the LICENSE file for details.

---

## 🔗 Additional Notes

- `code_monolithic_backup.c` is a **single-file reference copy** of the entire program and is **not compiled** by the Makefile. It exists for historical/archival reference only.
- The project is deliberately self-contained — no external assets, no third-party libraries beyond a standard OpenGL/GLUT installation.
- On Apple Silicon Macs, the `Makefile` automatically links the `Cocoa` framework, which is required for GLUT windowing on arm64.
