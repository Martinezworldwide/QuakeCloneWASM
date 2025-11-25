@echo off
REM Start development server
REM QuakeCloneWASM - Local testing server

echo ============================================
echo Starting QuakeCloneWASM Development Server
echo ============================================
echo.

REM Check if build files exist
if not exist "site\wasm\game.js" (
    echo ERROR: Build files not found!
    echo Please run build.bat first to compile the game
    pause
    exit /b 1
)

if not exist "site\index.html" (
    echo ERROR: index.html not found!
    pause
    exit /b 1
)

REM Change to script directory to ensure correct paths
cd /d %~dp0

REM Check if port 8000 is available (use 8000 by default to avoid conflicts)
netstat -ano | findstr :8000 | findstr LISTENING >nul 2>&1
if %errorlevel% equ 0 (
    echo WARNING: Port 8000 is already in use!
    echo Attempting to use port 8001 instead...
    set SERVER_PORT=8001
) else (
    set SERVER_PORT=8000
)

REM Start Python HTTP server
echo ============================================
echo Starting QuakeCloneWASM Development Server
echo ============================================
echo.
echo Server URL: http://localhost:%SERVER_PORT%
echo Serving from: %CD%\site
echo.
echo Press Ctrl+C to stop the server
echo.

REM Change to site directory before starting server
cd site

REM Try Python 3 first, then Python 2
echo Starting server...
python -m http.server %SERVER_PORT%
if %errorlevel% neq 0 (
    python -m SimpleHTTPServer %SERVER_PORT%
)

cd ..

