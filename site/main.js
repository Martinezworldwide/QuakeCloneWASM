// Main JavaScript - WASM loader and game initialization
// QuakeCloneWASM - JavaScript bridge

let gameModule = null;
let gameInitialized = false;
let pointerLocked = false;
let canvas = null;
let ctx = null;

// FPS counter
let fpsDisplay = null;
let lastFpsUpdate = 0;
let fpsFrameCount = 0;
let fpsLastTime = performance.now();

// Input state
const keys = {};
let mouseDeltaX = 0;
let mouseDeltaY = 0;
let lastMouseX = 0;
let lastMouseY = 0;

// Initialize the game
async function initGame() {
    const loadingText = document.getElementById('loading-text');
    const loadingBar = document.getElementById('loading-bar');
    const canvas = document.getElementById('canvas');
    
    try {
        // Update loading text
        loadingText.textContent = 'Loading WebAssembly module...';
        loadingBar.style.width = '30%';
        
        // Wait for script to load if not already available
        loadingBar.style.width = '60%';
        loadingText.textContent = 'Initializing game...';
        
        // Check if GameModule is available (loaded via script tag)
        // With MODULARIZE=1 and EXPORT_NAME="GameModule", it should be a global function
        let GameModuleConstructor;
        
        if (typeof GameModule !== 'undefined') {
            // GameModule is available globally from script tag
            GameModuleConstructor = GameModule;
            console.log('Using global GameModule');
        } else {
            // Try ES6 import as fallback
            const GameModuleClass = await import('./wasm/game.js');
            
            // Check what we got
            if (typeof GameModuleClass === 'function') {
                GameModuleConstructor = GameModuleClass;
            } else if (typeof GameModuleClass.default === 'function') {
                GameModuleConstructor = GameModuleClass.default;
            } else if (typeof GameModuleClass.GameModule === 'function') {
                GameModuleConstructor = GameModuleClass.GameModule;
            } else {
                throw new Error('Could not find GameModule. Check console for details.');
            }
            console.log('Using ES6 import GameModule');
        }
        
        // Ensure canvas is ready and has proper dimensions
        if (!canvas || !canvas.getContext) {
            throw new Error('Canvas element not found or not ready');
        }
        
        // Set canvas size if not already set
        if (canvas.width === 0 || canvas.height === 0) {
            canvas.width = 800;
            canvas.height = 600;
        }
        
        // The renderer manages its own WebGL context; no manual setup needed here
        
        // Initialize Emscripten module
        gameModule = await GameModuleConstructor({
            canvas: canvas,
            locateFile: (path) => {
                if (path.endsWith('.wasm')) {
                    return './wasm/' + path;
                }
                return path;
            },
            noInitialRun: false,
            print: console.log,
            printErr: console.error
        });
        
        loadingBar.style.width = '90%';
        loadingText.textContent = 'Setting up input...';
        
        // Setup input handlers
        setupInputHandlers();
        
        // Setup beam up button
        setupBeamUpButton();
        
        // Setup planet selector
        setupPlanetSelector();
        
        // Initialize resize handler
        setupResizeHandler();
        
        loadingBar.style.width = '100%';
        loadingText.textContent = 'Starting game...';
        
        // Brief pause to allow rendering resources to settle
        await new Promise(resolve => setTimeout(resolve, 200));
        
        // Hide loading screen
        document.getElementById('loading').classList.add('hidden');
        
        // Start game loop
        gameInitialized = true;
        startGameLoop();
        
        console.log('Game initialized successfully!');
        
    } catch (error) {
        console.error('Error initializing game:', error);
        showError('Failed to initialize game: ' + error.message);
    }
}

// Setup input handlers
function setupInputHandlers() {
    canvas = document.getElementById('canvas');
    
    // Keyboard input
    document.addEventListener('keydown', (e) => {
        const keyCode = e.keyCode || e.which;
        keys[keyCode] = true;
        
        // Update C key state
        if (gameModule) {
            gameModule.ccall('set_key_state', null, ['number', 'number'], [keyCode, 1]);
        }
        
        // ESC to unlock pointer
        if (keyCode === 27) { // ESC
            if (pointerLocked) {
                document.exitPointerLock();
            }
        }
    });
    
    document.addEventListener('keyup', (e) => {
        const keyCode = e.keyCode || e.which;
        keys[keyCode] = false;
        
        // Update C key state
        if (gameModule) {
            gameModule.ccall('set_key_state', null, ['number', 'number'], [keyCode, 0]);
        }
    });
    
    // Mouse movement (for pointer lock)
    canvas.addEventListener('mousedown', (e) => {
        if (!pointerLocked && canvas) {
            try {
                canvas.requestPointerLock = canvas.requestPointerLock || 
                                           canvas.mozRequestPointerLock || 
                                           canvas.webkitRequestPointerLock;
                // Request pointer lock with error handling
                const promise = canvas.requestPointerLock();
                if (promise && promise.catch) {
                    promise.catch((err) => {
                        // Ignore errors if user cancelled or already locked
                        if (err.name !== 'SecurityError' && err.name !== 'NotAllowedError') {
                            console.warn('Pointer lock request failed:', err);
                        }
                    });
                }
            } catch (err) {
                // Ignore errors if user cancelled or already locked
                if (err.name !== 'SecurityError' && err.name !== 'NotAllowedError') {
                    console.warn('Pointer lock request failed:', err);
                }
            }
        }
    });
    
    // Pointer lock events
    document.addEventListener('pointerlockchange', handlePointerLockChange);
    document.addEventListener('mozpointerlockchange', handlePointerLockChange);
    document.addEventListener('webkitpointerlockchange', handlePointerLockChange);
    
    // Mouse movement tracking
    document.addEventListener('mousemove', (e) => {
        if (pointerLocked) {
            const movementX = e.movementX || e.mozMovementX || e.webkitMovementX || 0;
            const movementY = e.movementY || e.mozMovementY || e.webkitMovementY || 0;
            
            mouseDeltaX = movementX;
            mouseDeltaY = movementY;
            
            // Update C mouse delta
            if (gameModule) {
                gameModule.ccall('set_mouse_delta', null, ['number', 'number'], 
                               [mouseDeltaX, mouseDeltaY]);
            }
        }
    });
    
    // Prevent context menu on right click
    canvas.addEventListener('contextmenu', (e) => {
        e.preventDefault();
    });
}

// Handle pointer lock changes
function handlePointerLockChange() {
    const isLocked = document.pointerLockElement === canvas ||
                     document.mozPointerLockElement === canvas ||
                     document.webkitPointerLockElement === canvas;
    
    pointerLocked = isLocked;
    
    if (pointerLocked) {
        canvas.style.cursor = 'none';
    } else {
        canvas.style.cursor = 'default';
    }
}

// Setup window resize handler
function setupResizeHandler() {
    window.addEventListener('resize', () => {
        resizeCanvas();
    });
    
    // Initial resize
    resizeCanvas();
}

// Resize canvas to fit viewport
function resizeCanvas() {
    if (!canvas || !gameModule) return;
    
    const container = document.getElementById('container');
    const maxWidth = Math.min(1280, window.innerWidth);
    const maxHeight = Math.min(720, window.innerHeight - 20);
    
    canvas.width = maxWidth;
    canvas.height = maxHeight;
    
    // NOTIFY C code of resize (but ONLY after game is initialized AND GL is ready)
    // DON'T call resize_window immediately - it will call glViewport which requires GL emulation to be ready
    // The C code will check if GL is ready before calling glViewport
    if (gameInitialized && gameModule) {
        // Use setTimeout to defer the resize call until after GL is initialized
        // Wait longer to ensure GL emulation is fully ready
        setTimeout(() => {
            try {
                // This will eventually call glViewport, but only after GL is ready (150+ frames)
                // renderer_resize() now just stores dimensions, and renderer_clear() will call glViewport
                // when GL is ready, so this should be safe
                gameModule.ccall('resize_window', null, ['number', 'number'], [maxWidth, maxHeight]);
            } catch (e) {
                console.warn('Resize failed, GL context may not be ready:', e);
            }
        }, 200); // Wait 200ms before calling resize
    }
}

// Game loop (FPS display and location updates)
function startGameLoop() {
    function update() {
        if (!gameInitialized || !gameModule) return;
        
        // Update FPS counter
        const now = performance.now();
        fpsFrameCount++;
        
        if (now - fpsLastTime >= 1000) {
            const fps = gameModule.ccall('get_fps', 'number');
            if (fpsDisplay) {
                fpsDisplay.textContent = fps;
            }
            fpsLastTime = now;
            fpsFrameCount = 0;
        }
        
        // Update location display
        try {
            const locationName = gameModule.ccall('get_current_location_name', 'string');
            const locationSpan = document.getElementById('location');
            if (locationSpan) {
                locationSpan.textContent = locationName;
            }
            
            // Show/hide beam up button based on location
            const beamUpBtn = document.getElementById('beam-up-btn');
            const pilotSeatBtn = document.getElementById('pilot-seat-btn');
            const isOnSpaceship = gameModule.ccall('is_on_spaceship', 'number');
            
            if (beamUpBtn) {
                if (isOnSpaceship) {
                    beamUpBtn.style.display = 'none';
                } else {
                    beamUpBtn.style.display = 'block';
                }
            }
            
            // Show pilot seat button when on spaceship
            if (pilotSeatBtn) {
                if (isOnSpaceship) {
                    pilotSeatBtn.style.display = 'block';
                } else {
                    pilotSeatBtn.style.display = 'none';
                }
            }
        } catch (e) {
            // Ignore errors if functions not ready yet
        }
        
        requestAnimationFrame(update);
    }
    
    fpsDisplay = document.getElementById('fps');
    requestAnimationFrame(update);
}

// Setup beam up button
function setupBeamUpButton() {
    const beamUpBtn = document.getElementById('beam-up-btn');
    const pilotSeatBtn = document.getElementById('pilot-seat-btn');
    
    if (beamUpBtn) {
        beamUpBtn.addEventListener('click', () => {
            if (gameModule) {
                try {
                    gameModule.ccall('beam_up', null);
                    console.log('Beaming up to spaceship...');
                } catch (e) {
                    console.error('Beam up failed:', e);
                }
            }
        });
    }
    
    // Setup pilot seat button
    if (pilotSeatBtn) {
        pilotSeatBtn.addEventListener('click', () => {
            if (gameModule) {
                try {
                    // Show loading screen
                    showPilotSeatLoading();
                    // Trigger C# transition
                    gameModule.ccall('beam_to_pilot_seat', null);
                    console.log('Beaming to pilot seat - transitioning to C#...');
                } catch (e) {
                    console.error('Pilot seat transition failed:', e);
                }
            }
        });
    }
    
    // Also handle B key in JavaScript
    document.addEventListener('keydown', (e) => {
        if (e.key === 'B' || e.key === 'b') {
            if (gameModule && !document.getElementById('planet-selector').classList.contains('hidden')) {
                return; // Don't beam if planet selector is open
            }
            if (gameModule) {
                try {
                    const isOnSpaceship = gameModule.ccall('is_on_spaceship', 'number');
                    if (!isOnSpaceship) {
                        gameModule.ccall('beam_up', null);
                        console.log('Beaming up to spaceship...');
                    }
                } catch (e) {
                    console.error('Beam up failed:', e);
                }
            }
        }
    });
}

// Setup planet selector UI
function setupPlanetSelector() {
    const planetSelector = document.getElementById('planet-selector');
    const planetList = document.getElementById('planet-list');
    const closeBtn = document.getElementById('close-selector');
    
    if (!planetSelector || !planetList || !closeBtn) {
        return;
    }
    
    // Close selector
    closeBtn.addEventListener('click', () => {
        planetSelector.classList.add('hidden');
    });
    
    // Update planet list when shown
    function updatePlanetList() {
        if (!gameModule) return;
        
        try {
            const planetCount = gameModule.ccall('get_planet_count', 'number');
            planetList.innerHTML = '';
            
            for (let i = 0; i < planetCount; i++) {
                const planetBtn = document.createElement('button');
                planetBtn.style.cssText = 'display: block; width: 100%; margin: 10px 0; padding: 15px; background: rgba(0, 255, 0, 0.1); border: 1px solid #0f0; color: #0f0; cursor: pointer; text-align: left; font-family: "Courier New", monospace;';
                
                const planetName = gameModule.ccall('get_planet_name', 'string', ['number'], [i]);
                const planetInfo = gameModule.ccall('get_planet_info', 'string', ['number'], [i]);
                
                planetBtn.innerHTML = `<strong>${planetName}</strong><br><small style="font-size: 10px;">${planetInfo.replace(/\n/g, '<br>')}</small>`;
                
                planetBtn.addEventListener('click', () => {
                    if (gameModule) {
                        try {
                            gameModule.ccall('beam_to_planet', null, ['number'], [i]);
                            console.log(`Beaming to planet ${i}: ${planetName}`);
                            planetSelector.classList.add('hidden');
                        } catch (e) {
                            console.error('Beam to planet failed:', e);
                        }
                    }
                });
                
                planetList.appendChild(planetBtn);
            }
        } catch (e) {
            console.error('Failed to update planet list:', e);
        }
    }
    
    // Show planet selector when on spaceship (P key)
    document.addEventListener('keydown', (e) => {
        if (e.key === 'P' || e.key === 'p') {
            if (gameModule) {
                try {
                    const isOnSpaceship = gameModule.ccall('is_on_spaceship', 'number');
                    if (isOnSpaceship) {
                        updatePlanetList();
                        planetSelector.classList.remove('hidden');
                    }
                } catch (e) {
                    console.error('Failed to show planet selector:', e);
                }
            }
        }
    });
}

// Show pilot seat loading screen and initialize C# runtime
function showPilotSeatLoading() {
    const loadingScreen = document.getElementById('loading-screen');
    const loadingBar = document.getElementById('loading-bar-pilot');
    const loadingStatus = document.getElementById('loading-status');
    
    if (!loadingScreen) return;
    
    // Show loading screen
    loadingScreen.classList.remove('hidden');
    
    // Simulate C# initialization progress
    const steps = [
        { percent: 10, text: 'Loading C# runtime...' },
        { percent: 25, text: 'Compiling C# game engine...' },
        { percent: 40, text: 'Initializing WASM bridge...' },
        { percent: 60, text: 'Loading game assets...' },
        { percent: 80, text: 'Setting up pilot seat interface...' },
        { percent: 95, text: 'Finalizing C# prototypes...' },
        { percent: 100, text: 'Ready for flight!' }
    ];
    
    let currentStep = 0;
    const interval = setInterval(() => {
        if (currentStep < steps.length) {
            const progress = steps[currentStep].percent;
            if (loadingBar) {
                loadingBar.style.width = progress + '%';
            }
            if (loadingStatus) {
                loadingStatus.textContent = steps[currentStep].text;
            }
            currentStep++;
        } else {
            clearInterval(interval);
            // After loading, initialize C# game engine
            setTimeout(() => {
                initializeCSharpGameEngine();
            }, 500);
        }
    }, 800);
}

// Global flag to prevent multiple Blazor initializations
let blazorInitialized = false;
let blazorAppHost = null;

// Initialize C# game engine (Blazor WebAssembly)
function initializeCSharpGameEngine() {
    // Prevent multiple initializations
    if (blazorInitialized) {
        console.log('[Blazor] Already initialized, skipping...');
        const pilotSeatApp = document.getElementById('pilot-seat-app');
        const canvas = document.getElementById('canvas');
        if (pilotSeatApp) pilotSeatApp.style.display = 'block';
        if (canvas) canvas.style.display = 'none';
        return;
    }
    
    console.log('Initializing C# game engine (Blazor WebAssembly)...');
    
    const loadingScreen = document.getElementById('loading-screen');
    const loadingStatus = document.getElementById('loading-status');
    
    if (loadingStatus) {
        loadingStatus.textContent = 'Loading Blazor WebAssembly runtime...';
    }
    
    // Load Blazor WebAssembly runtime from published output
    // Blazor files are in site/pilot-seat/wwwroot/_framework/
    // Configure Blazor base path BEFORE loading any scripts
    const blazorBasePath = './pilot-seat/wwwroot/';
    const frameworkPath = blazorBasePath + '_framework/';
    
    // Intercept fetch requests BEFORE Blazor loads
    // This catches ALL resource requests including dotnet.js
    const originalFetch = window.fetch;
    window.fetch = function(...args) {
        const url = args[0];
        if (typeof url === 'string') {
            // Fix any _framework/ paths to use our base path
            if (url.includes('_framework/')) {
                let fixedUrl = url;
                if (url.startsWith('/_framework/')) {
                    fixedUrl = blazorBasePath + url.substring(1);
                } else if (url.startsWith('_framework/')) {
                    fixedUrl = blazorBasePath + url;
                } else {
                    // Contains _framework/ somewhere in the path
                    const parts = url.split('_framework/');
                    fixedUrl = blazorBasePath + '_framework/' + parts[parts.length - 1];
                }
                args[0] = fixedUrl;
                console.log(`[Fetch Intercept] Fixed: ${url} -> ${fixedUrl}`);
            }
        }
        return originalFetch.apply(this, args);
    };
    
    // Intercept dynamic imports (import() calls) - these bypass fetch()
    // Save original import function if it exists
    const originalImport = window.__import || function() {};
    
    // Override import() function
    const originalImportFunc = window.import || function() {};
    window.import = function(moduleSpecifier) {
        if (typeof moduleSpecifier === 'string' && moduleSpecifier.includes('_framework/')) {
            let fixedSpecifier = moduleSpecifier;
            if (moduleSpecifier.startsWith('/_framework/')) {
                fixedSpecifier = blazorBasePath + moduleSpecifier.substring(1);
            } else if (moduleSpecifier.startsWith('_framework/')) {
                fixedSpecifier = blazorBasePath + moduleSpecifier;
            } else {
                const parts = moduleSpecifier.split('_framework/');
                fixedSpecifier = blazorBasePath + '_framework/' + parts[parts.length - 1];
            }
            console.log(`[Import Intercept] Fixed: ${moduleSpecifier} -> ${fixedSpecifier}`);
            // Use dynamic import with fixed path
            return import(fixedSpecifier);
        }
        // Fallback to original import
        return originalImportFunc.call(this, moduleSpecifier);
    };
    
    // Set global Blazor configuration BEFORE loading blazor.webassembly.js
    // This must be set up before the script loads to intercept resource loading
    if (!window.Blazor) {
        window.Blazor = {};
    }
    if (!window.Blazor.start) {
        window.Blazor.start = {};
    }
    
    // Configure runtime with custom resource loader
    window.Blazor.start.configureRuntime = function (config) {
        console.log('[Blazor] Configuring runtime with base path:', blazorBasePath);
        config.baseUrl = blazorBasePath;
        
        // Override loadBootResource to fix ALL resource paths
        // This intercepts all resource loading including dotnet.js
        const originalLoadBootResource = config.loadBootResource || function(type, name, defaultUri, integrity) {
            return defaultUri;
        };
        
        config.loadBootResource = function (type, name, defaultUri, integrity) {
            console.log(`[Blazor] Loading resource: type=${type}, name=${name || 'none'}, defaultUri=${defaultUri || 'none'}`);
            
            // All Blazor resources are in _framework/ directory
            let resourcePath = null;
            
            // Handle different URI formats
            if (defaultUri) {
                if (defaultUri.startsWith('_framework/')) {
                    resourcePath = blazorBasePath + defaultUri;
                } else if (defaultUri.startsWith('/_framework/')) {
                    resourcePath = blazorBasePath + defaultUri.substring(1);
                } else if (defaultUri.startsWith('./_framework/')) {
                    resourcePath = blazorBasePath + defaultUri.substring(2);
                } else if (defaultUri === '_framework/dotnet.js' || defaultUri === '/_framework/dotnet.js') {
                    // Explicitly handle dotnet.js (the problematic one)
                    resourcePath = frameworkPath + 'dotnet.js';
                } else if (defaultUri.startsWith('./')) {
                    resourcePath = blazorBasePath + defaultUri.substring(2);
                } else {
                    // Try to fix absolute paths
                    const cleanUri = defaultUri.startsWith('/') ? defaultUri.substring(1) : defaultUri;
                    if (cleanUri.startsWith('_framework/')) {
                        resourcePath = blazorBasePath + cleanUri;
                    } else {
                        resourcePath = frameworkPath + cleanUri;
                    }
                }
            } else if (name) {
                // No defaultUri, use name directly
                if (name === 'dotnet.js') {
                    resourcePath = frameworkPath + 'dotnet.js';
                } else if (name.startsWith('_framework/')) {
                    resourcePath = blazorBasePath + name;
                } else {
                    resourcePath = frameworkPath + name;
                }
            }
            
            // Return the corrected path
            if (resourcePath) {
                console.log(`[Blazor] Resolved to: ${resourcePath}`);
                return resourcePath;
            }
            
            // Fallback
            const result = originalLoadBootResource(type, name, defaultUri, integrity);
            if (result && typeof result === 'string') {
                // Fix result if it's a string
                if (result.startsWith('_framework/')) {
                    return blazorBasePath + result;
                } else if (result.startsWith('/_framework/')) {
                    return blazorBasePath + result.substring(1);
                }
                return result;
            }
            
            // Last resort fallback
            return result || defaultUri || (name ? frameworkPath + name : null);
        };
    };
    
    // Load Blazor script AFTER configuration is set up
    // Check if script already exists AND Blazor is already initialized
    const existingScript = document.querySelector('script[src*="blazor.webassembly.js"]');
    if (existingScript && blazorInitialized) {
        console.log('[Blazor] Already initialized, reusing existing instance');
        // Just show the app container
        const pilotSeatApp = document.getElementById('pilot-seat-app');
        const canvas = document.getElementById('canvas');
        if (pilotSeatApp) {
            pilotSeatApp.style.display = 'block';
            // Clear any previous content to prevent duplicate mounting
            if (pilotSeatApp.children.length > 0) {
                console.log('[Blazor] Clearing previous app content');
            }
        }
        if (canvas) canvas.style.display = 'none';
        if (loadingScreen) loadingScreen.classList.add('hidden');
        return;
    }
    
    // If script exists but Blazor not initialized, wait for it to finish loading
    if (existingScript && !blazorInitialized) {
        console.log('[Blazor] Script loaded but not initialized yet, waiting...');
        // Wait for script to finish loading
        existingScript.addEventListener('load', () => {
            setTimeout(() => {
                blazorInitialized = true;
                const pilotSeatApp = document.getElementById('pilot-seat-app');
                const canvas = document.getElementById('canvas');
                if (pilotSeatApp) pilotSeatApp.style.display = 'block';
                if (canvas) canvas.style.display = 'none';
                if (loadingScreen) loadingScreen.classList.add('hidden');
            }, 2000);
        });
        return;
    }
    
    const blazorScript = document.createElement('script');
    blazorScript.src = frameworkPath + 'blazor.webassembly.js';
    
    // If the script fails, try loading from root _framework/ directory
    blazorScript.onerror = function() {
        console.log('[Blazor] Failed to load from subdirectory, trying root _framework/');
        const fallbackScript = document.createElement('script');
        fallbackScript.src = './_framework/blazor.webassembly.js';
        document.head.appendChild(fallbackScript);
    };
    
    blazorScript.onload = () => {
        console.log('Blazor WebAssembly runtime script loaded successfully');
        if (loadingStatus) {
            loadingStatus.textContent = 'Initializing C# runtime...';
        }
        
        // Mark as initialized to prevent multiple loads
        blazorInitialized = true;
        
        // Wait a bit for Blazor to start initializing
        // Blazor will mount to #pilot-seat-app automatically
        setTimeout(() => {
            // Show Blazor app container
            const pilotSeatApp = document.getElementById('pilot-seat-app');
            if (pilotSeatApp) {
                pilotSeatApp.style.display = 'block';
            }
            
            // Hide C game canvas
            const canvas = document.getElementById('canvas');
            if (canvas) {
                canvas.style.display = 'none';
            }
            
            if (loadingStatus) {
                loadingStatus.textContent = 'C# runtime ready! Loading pilot seat interface...';
            }
            
            // Hide loading screen after Blazor fully initializes
            setTimeout(() => {
                if (loadingScreen) {
                    loadingScreen.classList.add('hidden');
                    console.log('C# game engine initialized - Pilot seat interface ready');
                }
            }, 2000);
        }, 1000);
    };
    
    blazorScript.onerror = (error) => {
        console.error('Failed to load Blazor WebAssembly runtime:', error);
        if (loadingStatus) {
            loadingStatus.textContent = 'Error: Failed to load C# runtime';
        }
        
        // Fallback message
        setTimeout(() => {
            if (loadingScreen) {
                loadingScreen.classList.add('hidden');
            }
            alert('Failed to load C# runtime.\n\nBlazor files should be in:\nsite/pilot-seat/wwwroot/_framework/\n\nRun: cd PilotSeatEngine && dotnet publish -c Release -o ../site/pilot-seat');
        }, 2000);
    };
    
    document.head.appendChild(blazorScript);
}

// JavaScript interop functions for C# to call
window.beamToPlanetFromCSharp = function(planetIndex) {
    if (gameModule) {
        try {
            gameModule.ccall('beam_to_planet', null, ['number'], [planetIndex]);
            console.log(`Beaming to planet ${planetIndex} from C#`);
        } catch (e) {
            console.error('Beam to planet failed:', e);
        }
    }
};

window.exitPilotSeat = function() {
    console.log('Exiting pilot seat - returning to spaceship interior');
    
    // Hide C# Blazor app (but don't dispose - keep it initialized for next time)
    const pilotSeatApp = document.getElementById('pilot-seat-app');
    if (pilotSeatApp) {
        pilotSeatApp.style.display = 'none';
    }
    
    // Show C game canvas
    const canvas = document.getElementById('canvas');
    if (canvas) {
        canvas.style.display = 'block';
    }
    
    // Return to spaceship (already on spaceship from beaming)
    console.log('Back to spaceship interior');
};

// Show error message
function showError(message) {
    const errorDiv = document.getElementById('error');
    errorDiv.textContent = message;
    errorDiv.classList.remove('hidden');
}

// Initialize when page loads
window.addEventListener('load', () => {
    initGame();
});

