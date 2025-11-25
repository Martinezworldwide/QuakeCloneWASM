#!/bin/bash
# Start development server
# QuakeCloneWASM - Local testing server

echo "============================================"
echo "Starting QuakeCloneWASM Development Server"
echo "============================================"
echo ""

# Check if build files exist
if [ ! -f "site/wasm/game.js" ]; then
    echo "ERROR: Build files not found!"
    echo "Please run ./build.sh first to compile the game"
    exit 1
fi

if [ ! -f "site/index.html" ]; then
    echo "ERROR: index.html not found!"
    exit 1
fi

# Start Python HTTP server
echo "Starting HTTP server on http://localhost:8080"
echo "Press Ctrl+C to stop the server"
echo ""

cd site

# Try Python 3 first, then Python 2
python3 -m http.server 8080 2>/dev/null || python -m SimpleHTTPServer 8080

cd ..

