// Main game loop and initialization
// QuakeCloneWASM - Main entry point

#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "renderer.h"
#include "player.h"
#include "world.h"
#include "input.h"
#include "space.h"

// Include GL headers for GL enum types and functions
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <GLES3/gl3.h>
#endif

// External reference to GL state ready flag
extern int g_gl_state_ready;

// Global game state
static int g_window_width = 800;
static int g_window_height = 600;
static double g_last_time = 0.0;
static double g_delta_time = 0.0;
static int g_fps = 0;
static double g_fps_counter_time = 0.0;
static int g_fps_frame_count = 0;

// Game state structure
typedef struct {
    int running;
    double current_time;
} GameState;

static GameState g_game_state = {1, 0.0};

// Forward declarations
void game_loop(void* user_data);
void update_game(double delta_time);
void render_game(void);
void update_fps_counter(double delta_time);

// Main game loop callback for Emscripten
void game_loop(void* user_data) {
    // Calculate delta time
    double current_time = emscripten_get_now() / 1000.0;
    if (g_last_time == 0.0) {
        g_last_time = current_time;
    }
    g_delta_time = current_time - g_last_time;
    g_last_time = current_time;
    
    // Cap delta time to prevent large jumps
    if (g_delta_time > 0.1) {
        g_delta_time = 0.1;
    }
    
    // Update FPS counter
    update_fps_counter(g_delta_time);
    
    // Update game state
    update_game(g_delta_time);
    
    // Render frame
    render_game();
    
    // Update input state
    input_update();
}

// Update game logic
void update_game(double delta_time) {
    if (!g_game_state.running) {
        return;
    }
    
    // Check for beam up key (B key)
    if (input_is_key_pressed('B') || input_is_key_pressed('b')) {
        if (space_get_location_type() == LOCATION_PLANET) {
            space_beam_to_spaceship();
        }
    }
    
    // Update player movement and rotation
    player_update(delta_time);
    
    // Update world interactions
    world_update(delta_time);
    
    // Update space system
    space_update(delta_time);
}

// Render the game frame
void render_game(void) {
    if (!g_gl_state_ready) {
        return;
    }

    renderer_clear();
    world_render();
    player_render();
    renderer_present();
}

// Update FPS counter
void update_fps_counter(double delta_time) {
    g_fps_counter_time += delta_time;
    g_fps_frame_count++;
    
    if (g_fps_counter_time >= 1.0) {
        g_fps = g_fps_frame_count;
        g_fps_frame_count = 0;
        g_fps_counter_time = 0.0;
    }
}

// Get current FPS
EMSCRIPTEN_KEEPALIVE
int get_fps(void) {
    return g_fps;
}

// Window resize callback
EMSCRIPTEN_KEEPALIVE
void resize_window(int width, int height) {
    g_window_width = width;
    g_window_height = height;
    renderer_resize(width, height);
}

// Beam up to spaceship (called from JavaScript)
EMSCRIPTEN_KEEPALIVE
void beam_up(void) {
    if (space_get_location_type() == LOCATION_PLANET) {
        space_beam_to_spaceship();
    }
}

// Beam to pilot seat (called from JavaScript - triggers C# transition)
EMSCRIPTEN_KEEPALIVE
void beam_to_pilot_seat(void) {
    space_beam_to_pilot_seat();
}

// Get current location name (for JavaScript display)
EMSCRIPTEN_KEEPALIVE
const char* get_current_location_name(void) {
    if (space_get_location_type() == LOCATION_SPACESHIP) {
        return "Spaceship";
    }
    
    int planet_idx = space_get_current_planet();
    if (planet_idx >= 0) {
        PlanetData* planet = space_get_planet(planet_idx);
        if (planet) {
            return planet->name;
        }
    }
    
    return "Unknown";
}

// Get planet count (for JavaScript)
EMSCRIPTEN_KEEPALIVE
int get_planet_count(void) {
    return space_get_planet_count();
}

// Get planet name by index (for JavaScript)
EMSCRIPTEN_KEEPALIVE
const char* get_planet_name(int index) {
    PlanetData* planet = space_get_planet(index);
    if (planet) {
        return planet->name;
    }
    return "Unknown";
}

// Get planet info as formatted string (for JavaScript)
// Uses static buffer to avoid malloc/free issues
EMSCRIPTEN_KEEPALIVE
const char* get_planet_info(int index) {
    static char buffer[512];
    PlanetData* planet = space_get_planet(index);
    if (!planet) {
        buffer[0] = '\0';
        return buffer;
    }
    
    snprintf(buffer, sizeof(buffer),
        "%s\nDistance: %.2f AU\nTemp: %.1fK (%.1fC)\nGravity: %.2fg\nAtmosphere: %s\nResources: %d%%",
        planet->name,
        planet->distance_au,
        planet->surface_temp_k,
        planet->surface_temp_k - 273.15f,
        planet->gravity_g,
        planet->atmosphere_type == 0 ? "None" :
        planet->atmosphere_type == 1 ? "Thin" :
        planet->atmosphere_type == 2 ? "Breathable" : "Toxic",
        planet->resource_richness);
    
    return buffer;
}

// Beam to planet (called from JavaScript)
EMSCRIPTEN_KEEPALIVE
void beam_to_planet(int planet_index) {
    space_beam_to_planet(planet_index);
}

// Check if on spaceship (for JavaScript)
EMSCRIPTEN_KEEPALIVE
int is_on_spaceship(void) {
    return space_get_location_type() == LOCATION_SPACESHIP;
}

// Initialize the game
int main(void) {
    printf("Initializing QuakeCloneWASM...\n");
    
    // Initialize renderer (doesn't create GL context - GL emulation will handle it)
    if (!renderer_init(g_window_width, g_window_height)) {
        printf("ERROR: Failed to initialize renderer\n");
        return 1;
    }
    
    // Initialize input system
    if (!input_init()) {
        printf("ERROR: Failed to initialize input system\n");
        return 1;
    }
    
    // Initialize space exploration system
    if (!space_init()) {
        printf("ERROR: Failed to initialize space system\n");
        return 1;
    }
    
    // Initialize world
    if (!world_init()) {
        printf("ERROR: Failed to initialize world\n");
        return 1;
    }
    
    // Initialize player on first planet
    int current_planet = space_get_current_planet();
    if (current_planet >= 0) {
        PlanetData* planet = space_get_planet(current_planet);
        if (planet) {
            player_init(planet->map_offset_x, planet->map_offset_z);
            printf("Starting on planet: %s\n", planet->name);
        } else {
            // Fallback position
            player_init(16.0f, 16.0f);
        }
    } else {
        // On spaceship
        player_init(4.0f, 4.0f);
    }
    
    printf("Game initialized successfully!\n");
    printf("Starting main loop...\n");
    emscripten_set_main_loop_arg(game_loop, NULL, 0, 1);
    
    return 0;
}

