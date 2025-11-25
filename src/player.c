// Player implementation - First-person movement and camera
// QuakeCloneWASM - Player controls system

#include <math.h>
#include <stdio.h>
#include "player.h"
#include "input.h"
#include "world.h"

// Player state
typedef struct {
    float pos_x, pos_y, pos_z;  // Position
    float yaw, pitch;           // Rotation (degrees)
    float speed;                // Movement speed
    float mouse_sensitivity;    // Mouse look sensitivity
} Player;

static Player g_player = {
    .pos_x = 0.0f,
    .pos_y = 0.0f,
    .pos_z = 0.0f,
    .yaw = 0.0f,
    .pitch = 0.0f,
    .speed = 5.0f,
    .mouse_sensitivity = 0.5f
};

// Initialize player at starting position
void player_init(float start_x, float start_y) {
    g_player.pos_x = start_x;
    g_player.pos_y = 0.0f;
    g_player.pos_z = start_y;
    g_player.yaw = 0.0f;
    g_player.pitch = 0.0f;
    printf("Player initialized at (%.2f, %.2f, %.2f)\n", 
           g_player.pos_x, g_player.pos_y, g_player.pos_z);
}

// Update player state
void player_update(double delta_time) {
    // Get mouse delta for look rotation
    float mouse_dx, mouse_dy;
    input_get_mouse_delta(&mouse_dx, &mouse_dy);
    
    // Update rotation based on mouse movement
    g_player.yaw += mouse_dx * g_player.mouse_sensitivity;
    g_player.pitch -= mouse_dy * g_player.mouse_sensitivity;
    
    // Clamp pitch to prevent gimbal lock
    if (g_player.pitch > 89.0f) g_player.pitch = 89.0f;
    if (g_player.pitch < -89.0f) g_player.pitch = -89.0f;
    
    // Normalize yaw
    while (g_player.yaw < 0.0f) g_player.yaw += 360.0f;
    while (g_player.yaw >= 360.0f) g_player.yaw -= 360.0f;
    
    // Get input state
    int move_forward = input_is_key_down('W') || input_is_key_down('w');
    int move_backward = input_is_key_down('S') || input_is_key_down('s');
    int move_left = input_is_key_down('A') || input_is_key_down('a');
    int move_right = input_is_key_down('D') || input_is_key_down('d');
    
    // Calculate movement direction
    float move_forward_amount = 0.0f;
    float move_right_amount = 0.0f;
    
    if (move_forward) move_forward_amount += 1.0f;
    if (move_backward) move_forward_amount -= 1.0f;
    if (move_left) move_right_amount -= 1.0f;
    if (move_right) move_right_amount += 1.0f;
    
    // Normalize diagonal movement
    if (move_forward_amount != 0.0f && move_right_amount != 0.0f) {
        float len = sqrtf(move_forward_amount * move_forward_amount + 
                         move_right_amount * move_right_amount);
        move_forward_amount /= len;
        move_right_amount /= len;
    }
    
    // Convert yaw to radians for calculations
    float yaw_rad = g_player.yaw * (M_PI / 180.0f);
    
    // Calculate forward and right vectors
    float forward_x = sinf(yaw_rad);
    float forward_z = -cosf(yaw_rad);
    float right_x = cosf(yaw_rad);
    float right_z = sinf(yaw_rad);
    
    // Apply movement
    float move_x = (forward_x * move_forward_amount + right_x * move_right_amount) * 
                   g_player.speed * delta_time;
    float move_z = (forward_z * move_forward_amount + right_z * move_right_amount) * 
                   g_player.speed * delta_time;
    
    // Update position
    float new_x = g_player.pos_x + move_x;
    float new_z = g_player.pos_z + move_z;
    
    // Check collision before updating position (simple radius check)
    // Player radius is about 0.3 units
    float player_radius = 0.3f;
    extern int world_check_collision(float x, float y, float z, float radius);
    
    // Only update position if no collision
    if (!world_check_collision(new_x, 0.0f, new_z, player_radius)) {
        g_player.pos_x = new_x;
        g_player.pos_z = new_z;
    }
    
    // Clamp player to world bounds (prevent going outside map)
    float min_x, max_x, min_z, max_z;
    extern void world_get_bounds(float* min_x, float* max_x, float* min_z, float* max_z);
    world_get_bounds(&min_x, &max_x, &min_z, &max_z);
    
    if (g_player.pos_x < min_x + player_radius) g_player.pos_x = min_x + player_radius;
    if (g_player.pos_x > max_x - player_radius) g_player.pos_x = max_x - player_radius;
    if (g_player.pos_z < min_z + player_radius) g_player.pos_z = min_z + player_radius;
    if (g_player.pos_z > max_z - player_radius) g_player.pos_z = max_z - player_radius;
    
    // Simple floor collision (keep player at y=0 for now)
    g_player.pos_y = 0.0f;
}

// Render player view (first-person camera)
void player_render(void) {
    // Camera setup is done in world_render() using player position/rotation
    // This function can be used for HUD rendering or other player-specific visuals
}

// Get player position
void player_get_position(float* x, float* y, float* z) {
    if (x) *x = g_player.pos_x;
    if (y) *y = g_player.pos_y;
    if (z) *z = g_player.pos_z;
}

// Get player rotation
float player_get_yaw(void) {
    return g_player.yaw;
}

float player_get_pitch(void) {
    return g_player.pitch;
}

// Set player rotation (for mouse look)
void player_set_rotation(float yaw, float pitch) {
    g_player.yaw = yaw;
    g_player.pitch = pitch;
}

// Move player (relative to current position)
void player_move(float forward, float right, float up) {
    float yaw_rad = g_player.yaw * (M_PI / 180.0f);
    float forward_x = sinf(yaw_rad);
    float forward_z = -cosf(yaw_rad);
    float right_x = cosf(yaw_rad);
    float right_z = sinf(yaw_rad);
    
    g_player.pos_x += forward_x * forward + right_x * right;
    g_player.pos_y += up;
    g_player.pos_z += forward_z * forward + right_z * right;
}

