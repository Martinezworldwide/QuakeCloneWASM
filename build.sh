#!/bin/bash
# Build script for Linux/Mac
# QuakeCloneWASM - Compile C to WebAssembly

echo "============================================"
echo "Building QuakeCloneWASM for WebAssembly"
echo "============================================"
echo ""

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "ERROR: Emscripten not found in PATH"
    echo "Please activate Emscripten environment first"
    echo "source ~/emsdk/emsdk_env.sh"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p site/wasm

echo "Compiling C source files..."
echo ""

# Compile with Emscripten
emcc \
    src/main.c \
    src/renderer.c \
    src/player.c \
    src/world.c \
    src/input.c \
    src/space.c \
    -o site/wasm/game.js \
    -O3 \
    -s WASM=1 \
    -s USE_WEBGL2=1 \
    -s USE_GLFW=0 \
    -s MIN_WEBGL_VERSION=2 \
    -s MAX_WEBGL_VERSION=2 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap","UTF8ToString","_malloc","_free"] \
    -s EXPORTED_FUNCTIONS=["_main","_get_fps","_resize_window","_set_key_state","_set_mouse_delta","_beam_up","_get_current_location_name","_get_planet_count","_get_planet_name","_get_planet_info","_beam_to_planet","_is_on_spaceship"] \
    -s ASSERTIONS=0 \
    -s SINGLE_FILE=0 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="GameModule" \
    -I src \
    -lm

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Compilation failed!"
    exit 1
fi

echo ""
echo "============================================"
echo "Build completed successfully!"
echo "============================================"
echo "Output files:"
echo "  - site/wasm/game.js"
echo "  - site/wasm/game.wasm"
echo ""
echo "To test the game, run: ./start.sh"
echo ""

