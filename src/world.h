// World header - Level geometry and map rendering
// QuakeCloneWASM - World system

#ifndef WORLD_H
#define WORLD_H

// Initialize world
int world_init(void);

// Update world state
void world_update(double delta_time);

// Render world geometry
void world_render(void);

// Get current location type (for rendering different environments)
int world_get_location_type(void);

// Check collision with world
int world_check_collision(float x, float y, float z, float radius);

// Get world bounds
void world_get_bounds(float* min_x, float* max_x, float* min_z, float* max_z);

// Set planet-specific map (for different planets)
void world_set_planet_map(int planet_type);

// Shutdown world
void world_shutdown(void);

#endif // WORLD_H

