@echo off
REM Clean server start
REM QuakeCloneWASM - Start development server on port 8080

cd /d %~dp0site

echo ============================================
echo Starting QuakeCloneWASM Development Server
echo ============================================
echo.
echo Server URL: http://localhost:8080
echo Serving from: %CD%
echo.
echo Press Ctrl+C to stop the server
echo.

python -m http.server 8080


