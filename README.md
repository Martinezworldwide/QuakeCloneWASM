https://martinezworldwide.github.io/QuakeCloneWASM/

# QuakeCloneWASM - Space Exploration Game

A Quake-style first-person shooter game compiled to WebAssembly, featuring space exploration, planet beaming, and a transition to C# for pilot seat mechanics.

## Project Overview

This project is a 3D first-person space exploration game built with C and WebAssembly. It features:
- Real-time 3D raycasting rendering (CPU-based software renderer)
- WebGL2 compositing pipeline for GPU acceleration
- First-person movement controls (WASD + mouse look)
- Space exploration system with 8 realistic planets
- Beaming mechanics between spaceship and planets
- Pilot seat interface (transitioning to C#)

## Technology Stack

### Core Technologies

#### **C Programming Language (C99)**
- **Purpose**: Core game logic, rendering, physics, and game systems
- **Standard**: C99 with GCC/Clang extensions
- **Files**: All `.c` and `.h` files in `src/`
- **Key Features Used**:
  - Standard library: `stdio.h`, `stdlib.h`, `math.h`, `string.h`
  - Static arrays for map data
  - Function pointers for map switching
  - Struct-based data organization

#### **WebAssembly (WASM)**
- **Compiler**: Emscripten SDK (clang-based)
- **Target**: WASM 32-bit
- **Output**: `site/wasm/game.wasm` + `site/wasm/game.js`
- **Purpose**: Compile C code to run in browser at near-native performance
- **Features Used**:
  - WASM module exports
  - JavaScript interop via `ccall`/`cwrap`
  - Memory management (growable heap)
  - Direct memory access from JavaScript

#### **Emscripten SDK**
- **Version**: Latest stable (configured via `emsdk`)
- **Purpose**: C-to-WASM compilation toolchain
- **Key Flags**:
  - `-s WASM=1`: Enable WebAssembly output
  - `-s USE_WEBGL2=1`: Enable WebGL2 support
  - `-s MIN_WEBGL_VERSION=2`: Target WebGL2 minimum
  - `-s MAX_WEBGL_VERSION=2`: Target WebGL2 maximum
  - `-s ALLOW_MEMORY_GROWTH=1`: Allow dynamic memory growth
  - `-s MODULARIZE=1`: Create modular JavaScript wrapper
  - `-s EXPORT_NAME="GameModule"`: Export as GameModule
  - `-O3`: Maximum optimization level

#### **WebGL2 (OpenGL ES 3.0)**
- **API**: OpenGL ES 3.0 via WebGL2
- **Purpose**: GPU-accelerated compositing of software-rendered framebuffer
- **Implementation**:
  - Context creation via `emscripten_webgl_create_context`
  - Vertex/Fragment shaders (GLSL ES 3.0)
  - Vertex Array Objects (VAO) and Vertex Buffer Objects (VBO)
  - 2D texture uploads for software framebuffer
  - Fullscreen quad rendering
- **Shaders**:
  - Vertex shader: Fullscreen quad with texture coordinates
  - Fragment shader: Simple texture sampling

#### **JavaScript (ES6+)**
- **Purpose**: Browser interface, WASM loading, input handling, UI
- **Key Technologies**:
  - ES6 Modules (for Emscripten module loading)
  - Pointer Lock API (for mouse look)
  - Keyboard/Mouse event handling
  - DOM manipulation
  - `requestAnimationFrame` for game loop
  - `performance.now()` for timing

#### **HTML5 Canvas**
- **Purpose**: Rendering target for WebGL2 context
- **API**: Standard HTML5 `<canvas>` element
- **Context**: WebGL2 via Emscripten

### Rendering Architecture

#### **Software Raycasting Renderer**
- **Type**: CPU-based 3D raycasting engine
- **Algorithm**: DDA (Digital Differential Analyzer) raycasting
- **Features**:
  - Perspective-correct wall rendering
  - Distance-based shading
  - Wall side differentiation (north/south vs east/west)
  - Ceiling/floor gradients
  - Environment-specific color palettes (spaceship vs planet)
- **Framebuffer**: `uint32_t*` array (RGBA, 32-bit per pixel)
- **Performance**: ~60 FPS at 800x600 on modern CPUs

#### **WebGL2 Compositing Pipeline**
- **Pipeline**:
  1. CPU renders to software framebuffer (uint32_t array)
  2. Framebuffer uploaded to GPU texture via `glTexSubImage2D`
  3. Fullscreen quad rendered with texture shader
  4. GPU handles final display to canvas
- **Benefits**:
  - Maintains software rendering flexibility
  - GPU acceleration for final compositing
  - Avoids GL emulation timing issues
  - Predictable initialization

### Game Systems

#### **Input System (`src/input.c`)**
- **Keyboard**: Direct key code mapping via JavaScript events
- **Mouse**: Pointer Lock API for first-person look
- **Functions**:
  - `input_is_key_down(key_code)`: Check if key is pressed
  - `input_is_key_pressed(key_code)`: Check if key was just pressed
  - `input_get_mouse_delta(dx, dy)`: Get mouse movement delta
- **JavaScript Bridge**: `set_key_state()`, `set_mouse_delta()` called from JS

#### **Player System (`src/player.c`)**
- **Movement**: First-person WASD controls
- **Rotation**: Mouse-based yaw (horizontal) and pitch (vertical)
- **Physics**:
  - Speed: 5.0 units/second
  - Mouse sensitivity: 0.1 degrees per pixel
  - Collision detection via world system
  - World bounds clamping
- **Position**: 3D coordinates (x, y, z) with yaw/pitch rotation

#### **World System (`src/world.c`)**
- **Map Format**: 16x16 grid-based map (1 = wall, 0 = empty)
- **Map Scale**: 2.0 units per cell
- **Maps**:
  - `g_planet_map`: Maze-style planet surface
  - `g_spaceship_map`: Corridor-style spaceship interior
- **Raycasting**: DDA algorithm for wall detection
- **Rendering**:
  - Environment-specific colors (spaceship vs planet)
  - Distance-based shading
  - Perspective correction
- **Collision**: Radius-based collision with map cells

#### **Space Exploration System (`src/space.c`)**
- **Planets**: 8 realistic planets with scientific data:
  - Distance from star (AU)
  - Radius (km)
  - Surface temperature (Kelvin)
  - Gravity (g units)
  - Atmosphere type (None/Thin/Breathable/Toxic)
  - Resource richness (0-100%)
- **Beaming**:
  - `space_beam_to_spaceship()`: Teleport to spaceship interior
  - `space_beam_to_planet(index)`: Teleport to planet surface
  - `space_beam_to_pilot_seat()`: Trigger C# transition (future)
- **Location Tracking**: Enum-based location state (SPACESHIP/PLANET)

#### **Renderer System (`src/renderer.c`)**
- **Initialization**:
  - WebGL2 context creation
  - Shader compilation (vertex + fragment)
  - Fullscreen quad setup (VAO/VBO)
  - Texture allocation for framebuffer
- **Rendering**:
  - `renderer_clear()`: Clear software framebuffer
  - `renderer_present()`: Upload framebuffer to GPU and draw
- **Memory**: Dynamic framebuffer allocation with capacity tracking

### Build System

#### **Emscripten Build Scripts**
- **Windows**: `build.bat` (batch script)
- **Linux/Mac**: `build.sh` (bash script)
- **Process**:
  1. Check for Emscripten in PATH
  2. Auto-activate Emscripten if not found (`emsdk_env.bat`)
  3. Compile all C source files to WASM
  4. Output: `site/wasm/game.js` + `site/wasm/game.wasm`

#### **Source Files Compiled**
```
src/main.c      - Game loop, initialization, Emscripten integration
src/renderer.c  - WebGL2 rendering, software framebuffer management
src/player.c    - Player movement, rotation, physics
src/world.c     - Map data, raycasting, collision detection
src/input.c     - Input state management
src/space.c     - Planet data, beaming mechanics
```

#### **Emscripten Export Configuration**
- **Exported Functions** (callable from JavaScript):
  - `_main`: Entry point
  - `_get_fps`: Get current FPS
  - `_resize_window`: Handle window resize
  - `_set_key_state`: Update keyboard state
  - `_set_mouse_delta`: Update mouse movement
  - `_beam_up`: Beam to spaceship
  - `_beam_to_pilot_seat`: Beam to pilot seat (C# transition)
  - `_get_current_location_name`: Get current location
  - `_get_planet_count`: Get number of planets
  - `_get_planet_name`: Get planet name by index
  - `_get_planet_info`: Get formatted planet info
  - `_beam_to_planet`: Beam to planet surface
  - `_is_on_spaceship`: Check if on spaceship

- **Exported Runtime Methods**:
  - `ccall`: Call C functions from JavaScript
  - `cwrap`: Wrap C functions for easier calling

### File Structure

```
QuakeCloneWASM/
├── src/                    # C source code
│   ├── main.c             # Main game loop, initialization
│   ├── renderer.c         # WebGL2 rendering system
│   ├── renderer.h          # Renderer API
│   ├── player.c            # Player movement and controls
│   ├── player.h            # Player API
│   ├── world.c             # Map data, raycasting, collision
│   ├── world.h             # World API
│   ├── input.c             # Input state management
│   ├── input.h             # Input API
│   ├── space.c              # Space exploration system
│   └── space.h              # Space API
│
├── site/                   # Web deployment files
│   ├── index.html          # Main HTML page
│   ├── main.js             # JavaScript bridge, UI, input handling
│   └── wasm/               # WebAssembly output
│       ├── game.js         # Emscripten-generated JavaScript wrapper
│       └── game.wasm        # Compiled WebAssembly binary
│
├── build.bat               # Windows build script
├── build.sh                # Linux/Mac build script
├── start.bat               # Windows development server
├── start.sh                # Linux/Mac development server
├── CSharpIntegration.md    # C# integration plan
└── README.md               # This file
```

### Development Setup

#### **Prerequisites**
1. **Emscripten SDK** (required)
   - Install from: https://emscripten.org/docs/getting_started/downloads.html
   - Or use: `emsdk install latest` + `emsdk activate latest`
   - Location: `%USERPROFILE%\emsdk\` (Windows) or `~/emsdk/` (Linux/Mac)

2. **Python 3.x** (required for Emscripten)
   - Used by Emscripten for toolchain scripts

3. **Web Browser** (for testing)
   - Chrome, Firefox, Edge (all support WebGL2)
   - Enable WebAssembly and WebGL2

#### **Build Instructions**

**Windows:**
```cmd
cd QuakeCloneWASM
build.bat
```

**Linux/Mac:**
```bash
cd QuakeCloneWASM
chmod +x build.sh
./build.sh
```

The build script will:
1. Check for Emscripten in PATH
2. Auto-activate Emscripten if needed
3. Compile all C files to WASM
4. Output files to `site/wasm/`

#### **Run Development Server**

**Windows:**
```cmd
start.bat
```
Then open: `http://localhost:8000`

**Linux/Mac:**
```bash
./start.sh
```
Then open: `http://localhost:8000`

### Key Design Decisions

#### **Why Software Raycasting + WebGL2 Compositing?**
- **Problem Solved**: Emscripten's `LEGACY_GL_EMULATION` had unreliable initialization timing
- **Solution**: CPU-side rendering to framebuffer, GPU compositing for display
- **Benefits**:
  - Predictable initialization (no GL emulation delays)
  - Full control over rendering pipeline
  - Easy to add effects (just modify framebuffer)
  - GPU acceleration for final display

#### **Why Separate Maps for Spaceship and Planets?**
- **Architecture**: Function pointer to map array (`g_map` points to active map)
- **Benefits**:
  - Different layouts for different environments
  - Easy to add more planet-specific maps
  - Clean separation of concerns

#### **Why C for Core Game Logic?**
- **Performance**: Near-native performance via WASM
- **Portability**: C code compiles to WASM for any platform
- **Control**: Direct memory management, low-level optimizations
- **Integration**: Easy to call from JavaScript

#### **Why Transition to C# for Pilot Seat?**
- **Rapid Prototyping**: C# offers better tooling and faster development
- **Game Development**: Rich ecosystem (Unity, Godot, etc. use C#)
- **Modern Features**: LINQ, async/await, generics, reflection
- **Integration**: Blazor WebAssembly allows C# to run in browser

### Current Limitations & Future Work

#### **Current Limitations**
1. Single map per environment (spaceship/planet)
2. No texture mapping (walls are solid colors)
3. No lighting beyond distance shading
4. No enemies or AI
5. Simple collision detection (radius-based)
6. No save/load system

#### **Planned C# Integration**
1. **Blazor WebAssembly Setup**
   - Create C# game engine library
   - Implement pilot seat interface
   - Space flight mechanics
   - Planet scanner UI

2. **C# Game Engine Prototypes**
   - Pilot seat controls (thruster, rotation)
   - Navigation system
   - Planet scanning interface
   - Mission system
   - Save/load functionality

3. **Hybrid Architecture**
   - C: Rendering, collision, physics (existing)
   - C#: UI, game logic, pilot seat (new)
   - JavaScript: Bridge between C and C#

### Performance Considerations

#### **Rendering Performance**
- **Target**: 60 FPS at 800x600 resolution
- **Optimizations**:
  - `-O3` compiler optimization
  - Single-pass raycasting per column
  - Efficient framebuffer operations
  - GPU compositing for final display

#### **Memory Usage**
- **Framebuffer**: ~2MB for 800x600 (800 * 600 * 4 bytes)
- **Map Data**: ~1KB (16x16 grid)
- **Planet Data**: ~2KB (8 planets with metadata)
- **Total**: ~5-10MB typical usage
- **Growth**: `ALLOW_MEMORY_GROWTH=1` enables dynamic expansion

### Browser Compatibility

#### **Required Features**
- WebAssembly support (WASM 1.0)
- WebGL2 support (OpenGL ES 3.0)
- Pointer Lock API
- ES6 JavaScript features

#### **Tested Browsers**
- Chrome/Edge (Chromium): ✅ Full support
- Firefox: ✅ Full support
- Safari: ✅ Full support (with WebGL2)

### Controls

| Action | Input |
|--------|-------|
| Move Forward | W |
| Move Backward | S |
| Strafe Left | A |
| Strafe Right | D |
| Look Around | Mouse (with pointer lock) |
| Lock Cursor | Click Canvas |
| Release Cursor | ESC |
| Beam Up | B (when on planet) |
| Planet Selector | P (when on spaceship) |
| Pilot Seat | Click "PILOT SEAT" button |

### Known Issues

1. **Favicon 404**: Browser requests favicon.ico (harmless)
2. **Initial Load**: First frame may be black (GL initialization)
3. **Resize**: Canvas resize requires brief delay for GL context

### Credits

- **Rendering**: DDA raycasting algorithm (Wolfenstein 3D style)
- **Planet Data**: Based on real exoplanet characteristics
- **WebGL2**: OpenGL ES 3.0 specification
- **Emscripten**: C-to-WASM compilation toolchain

### License

[Specify your license here]

---

## Quick Reference

### Build Command
```bash
# Windows
build.bat

# Linux/Mac
./build.sh
```

### Run Server
```bash
# Windows
start.bat

# Linux/Mac
./start.sh
```

### Key Files
- `src/main.c`: Game loop and initialization
- `src/renderer.c`: Rendering system (WebGL2 + software framebuffer)
- `src/world.c`: Map data and raycasting
- `src/space.c`: Planet data and beaming
- `site/main.js`: JavaScript bridge and UI
- `site/index.html`: Main HTML page

### Next Steps
1. Review `CSharpIntegration.md` for C# setup
2. Set up Blazor WebAssembly project
3. Implement pilot seat interface in C#
4. Add space flight mechanics
5. Create planet scanner UI
