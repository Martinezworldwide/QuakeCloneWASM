// Player header - Player movement and camera controls
// QuakeCloneWASM - Player system

#ifndef PLAYER_H
#define PLAYER_H

// Initialize player at position
void player_init(float start_x, float start_y);

// Update player state
void player_update(double delta_time);

// Render player view (first-person)
void player_render(void);

// Get player position
void player_get_position(float* x, float* y, float* z);

// Get player rotation
float player_get_yaw(void);
float player_get_pitch(void);

// Set player rotation (for mouse look)
void player_set_rotation(float yaw, float pitch);

// Move player (relative to current position)
void player_move(float forward, float right, float up);

#endif // PLAYER_H

