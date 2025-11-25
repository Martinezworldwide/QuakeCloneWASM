@echo off
REM Simple server starter
REM QuakeCloneWASM - Start development server

cd /d %~dp0site

echo ============================================
echo QuakeCloneWASM Development Server
echo ============================================
echo.
echo Serving files from: %CD%
echo Server will start on: http://localhost:8080
echo.
echo Press Ctrl+C to stop the server
echo.

python -m http.server 8080

pause


