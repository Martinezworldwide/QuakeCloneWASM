@echo off
REM Setup script - Activate Emscripten environment
REM QuakeCloneWASM - Environment setup

echo ============================================
echo Activating Emscripten Environment
echo ============================================
echo.

REM Try common Emscripten installation paths
set EMSDK_PATH=
if "%EMSDK%" NEQ "" set EMSDK_PATH=%EMSDK%
if "%EMSDK_PATH%"=="" if exist "%USERPROFILE%\emsdk" set EMSDK_PATH=%USERPROFILE%\emsdk
if "%EMSDK_PATH%"=="" if exist "C:\emsdk" set EMSDK_PATH=C:\emsdk
if "%EMSDK_PATH%"=="" if exist "%LOCALAPPDATA%\emsdk" set EMSDK_PATH=%LOCALAPPDATA%\emsdk

if "%EMSDK_PATH%"=="" (
    echo ERROR: Could not find Emscripten SDK
    echo Please set EMSDK_PATH environment variable or install Emscripten
    echo.
    echo Expected locations:
    echo   %%USERPROFILE%%\emsdk
    echo   C:\emsdk
    echo   %%LOCALAPPDATA%%\emsdk
    echo.
    pause
    exit /b 1
)

echo Found Emscripten at: %EMSDK_PATH%
echo.

REM Activate Emscripten
call "%EMSDK_PATH%\emsdk_env.bat"

if %errorlevel% neq 0 (
    echo ERROR: Failed to activate Emscripten
    pause
    exit /b 1
)

echo.
echo Emscripten activated successfully!
echo.
echo You can now run: build.bat
echo.

REM Keep the environment active
cmd /k

