@echo off
REM Build script for Windows
REM QuakeCloneWASM - Compile C to WebAssembly

echo ============================================
echo Building QuakeCloneWASM for WebAssembly
echo ============================================
echo.

REM Check if Emscripten is activated
where emcc >nul 2>&1
if %errorlevel% neq 0 (
    echo Emscripten not found in PATH, attempting to activate...
    REM Try to find and activate Emscripten
    set EMSDK_PATH=
    if "%EMSDK%" NEQ "" set EMSDK_PATH=%EMSDK%
    if "%EMSDK_PATH%"=="" if exist "%USERPROFILE%\emsdk\emsdk_env.bat" set EMSDK_PATH=%USERPROFILE%\emsdk
    if "%EMSDK_PATH%"=="" if exist "C:\emsdk\emsdk_env.bat" set EMSDK_PATH=C:\emsdk
    
    if "%EMSDK_PATH%"=="" (
        echo ERROR: Emscripten not found in PATH
        echo Please run setup.bat first to activate Emscripten
        pause
        exit /b 1
    )
    
    echo Activating Emscripten from: %EMSDK_PATH%
    call "%EMSDK_PATH%\emsdk_env.bat"
)

REM Create output directory if it doesn't exist
if not exist "site\wasm" mkdir "site\wasm"

echo Compiling C source files...
echo.

REM Compile with Emscripten
emcc ^
    src/main.c ^
    src/renderer.c ^
    src/player.c ^
    src/world.c ^
    src/input.c ^
    src/space.c ^
    -o site/wasm/game.js ^
    -O3 ^
    -s WASM=1 ^
    -s USE_WEBGL2=1 ^
    -s USE_GLFW=0 ^
    -s MIN_WEBGL_VERSION=2 ^
    -s MAX_WEBGL_VERSION=2 ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] ^
    -s EXPORTED_FUNCTIONS=["_main","_get_fps","_resize_window","_set_key_state","_set_mouse_delta","_beam_up","_beam_to_pilot_seat","_get_current_location_name","_get_planet_count","_get_planet_name","_get_planet_info","_beam_to_planet","_is_on_spaceship"] ^
    -s ASSERTIONS=0 ^
    -s SINGLE_FILE=0 ^
    -s MODULARIZE=1 ^
    -s EXPORT_NAME="GameModule" ^
    -I src ^
    -lm

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo Output files:
echo   - site/wasm/game.js
echo   - site/wasm/game.wasm
echo.
echo To test the game, run: start.bat
echo.

