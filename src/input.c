// Input implementation - Keyboard and mouse handling via JavaScript
// QuakeCloneWASM - Input system

#include <emscripten.h>
#include <string.h>
#include <stdio.h>
#include "input.h"

// Input state
#define MAX_KEYS 256
static int g_keys[MAX_KEYS];
static int g_keys_pressed[MAX_KEYS];
static float g_mouse_dx = 0.0f;
static float g_mouse_dy = 0.0f;
static int g_input_initialized = 0;

// External JavaScript functions (to be called from JS)
EMSCRIPTEN_KEEPALIVE
void set_key_state(int key_code, int is_down) {
    if (key_code >= 0 && key_code < MAX_KEYS) {
        int was_down = g_keys[key_code];
        g_keys[key_code] = is_down;
        g_keys_pressed[key_code] = (is_down && !was_down) ? 1 : 0;
    }
}

// External function to set mouse delta
EMSCRIPTEN_KEEPALIVE
void set_mouse_delta(float dx, float dy) {
    g_mouse_dx = dx;
    g_mouse_dy = dy;
}

// Initialize input system
int input_init(void) {
    if (g_input_initialized) {
        return 1;
    }
    
    memset(g_keys, 0, sizeof(g_keys));
    memset(g_keys_pressed, 0, sizeof(g_keys_pressed));
    g_mouse_dx = 0.0f;
    g_mouse_dy = 0.0f;
    
    g_input_initialized = 1;
    printf("Input system initialized\n");
    
    return 1;
}

// Update input state (call each frame)
void input_update(void) {
    // Reset pressed keys
    memset(g_keys_pressed, 0, sizeof(g_keys_pressed));
    
    // Mouse delta is reset by JavaScript after reading
    // So we keep it until next frame
}

// Check if key is currently down
int input_is_key_down(int key_code) {
    if (key_code >= 0 && key_code < MAX_KEYS) {
        return g_keys[key_code];
    }
    return 0;
}

// Check if key was just pressed this frame
int input_is_key_pressed(int key_code) {
    if (key_code >= 0 && key_code < MAX_KEYS) {
        return g_keys_pressed[key_code];
    }
    return 0;
}

// Get mouse delta (movement since last frame)
void input_get_mouse_delta(float* dx, float* dy) {
    if (dx) *dx = g_mouse_dx;
    if (dy) *dy = g_mouse_dy;
    
    // Reset after reading (JavaScript will update on next movement)
    g_mouse_dx = 0.0f;
    g_mouse_dy = 0.0f;
}

// Set mouse position (for centering)
void input_set_mouse_position(int x, int y) {
    // This would be handled by JavaScript
}

// Shutdown input system
void input_shutdown(void) {
    memset(g_keys, 0, sizeof(g_keys));
    memset(g_keys_pressed, 0, sizeof(g_keys_pressed));
    g_input_initialized = 0;
}

