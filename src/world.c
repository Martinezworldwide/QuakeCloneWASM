// World implementation - Simple raycasting-based map
// QuakeCloneWASM - Level geometry and rendering

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "player.h"
#include "renderer.h"
#include "space.h"  // For LocationType and space_get_location_type

// Simple map definition (grid-based)
#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define MAP_SCALE 2.0f

// Planet map data (maze/outdoor environment)
static int g_planet_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Spaceship interior map (futuristic ship corridors)
static int g_spaceship_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Current active map (points to planet or spaceship map)
static int (*g_map)[MAP_WIDTH] = g_planet_map;

static int g_world_initialized = 0;

// Helper: Get map cell value
static int get_map_cell(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 1; // Out of bounds = wall
    }
    return g_map[y][x];
}

// Get current location type for rendering (wraps space system)
int world_get_location_type(void) {
    LocationType loc = space_get_location_type();
    return (int)loc;
}

// Convert world position to map coordinates
static void world_to_map(float wx, float wz, int* mx, int* mz) {
    *mx = (int)(wx / MAP_SCALE);
    *mz = (int)(wz / MAP_SCALE);
}

// Raycasting for wall rendering
static void cast_ray(float ray_angle, float max_dist, float* hit_dist, int* hit_wall) {
    float player_x, player_y, player_z;
    player_get_position(&player_x, &player_y, &player_z);
    
    float ray_dir_x = sinf(ray_angle);
    float ray_dir_z = -cosf(ray_angle);
    
    float pos_x = player_x;
    float pos_z = player_z;
    
    float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabsf(1.0f / ray_dir_x);
    float delta_dist_z = (ray_dir_z == 0) ? 1e30 : fabsf(1.0f / ray_dir_z);
    
    int map_x = (int)(pos_x / MAP_SCALE);
    int map_z = (int)(pos_z / MAP_SCALE);
    
    int step_x, step_z;
    float side_dist_x, side_dist_z;
    
    if (ray_dir_x < 0) {
        step_x = -1;
        side_dist_x = (pos_x / MAP_SCALE - map_x) * delta_dist_x;
    } else {
        step_x = 1;
        side_dist_x = (map_x + 1.0f - pos_x / MAP_SCALE) * delta_dist_x;
    }
    
    if (ray_dir_z < 0) {
        step_z = -1;
        side_dist_z = (pos_z / MAP_SCALE - map_z) * delta_dist_z;
    } else {
        step_z = 1;
        side_dist_z = (map_z + 1.0f - pos_z / MAP_SCALE) * delta_dist_z;
    }
    
    int hit = 0;
    int side = 0;
    
    while (!hit) {
        if (side_dist_x < side_dist_z) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        } else {
            side_dist_z += delta_dist_z;
            map_z += step_z;
            side = 1;
        }
        
        if (map_x < 0 || map_x >= MAP_WIDTH || map_z < 0 || map_z >= MAP_HEIGHT) {
            *hit_dist = max_dist;
            *hit_wall = 0;
            return;
        }
        
        if (get_map_cell(map_x, map_z) == 1) {
            hit = 1;
        }
    }
    
    if (side == 0) {
        *hit_dist = (map_x - pos_x / MAP_SCALE + (1 - step_x) / 2.0f) / ray_dir_x * MAP_SCALE;
    } else {
        *hit_dist = (map_z - pos_z / MAP_SCALE + (1 - step_z) / 2.0f) / ray_dir_z * MAP_SCALE;
    }
    
    *hit_wall = side;
}

// Initialize world
int world_init(void) {
    if (g_world_initialized) {
        return 1;
    }
    
    g_world_initialized = 1;
    printf("World initialized: %dx%d map\n", MAP_WIDTH, MAP_HEIGHT);
    
    return 1;
}

// Update world state
void world_update(double delta_time) {
    // World updates (enemies, triggers, etc.)
}

// Helper to safely pack RGB values into framebuffer format
static inline uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b) {
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

// Render world using raycasting into the software framebuffer
void world_render(void) {
    if (!g_world_initialized) {
        return;
    }

    uint32_t* framebuffer = renderer_get_framebuffer();
    if (!framebuffer) {
        return;
    }

    int viewport_width, viewport_height;
    renderer_get_viewport(&viewport_width, &viewport_height);

    float player_x, player_y, player_z;
    player_get_position(&player_x, &player_y, &player_z);

    float player_yaw = player_get_yaw();
    float player_pitch = player_get_pitch();

    // Establish horizon based on pitch for simple look up/down effect
    int horizon = viewport_height / 2 - (int)(player_pitch * (viewport_height / 180.0f));
    if (horizon < 0) horizon = 0;
    if (horizon > viewport_height) horizon = viewport_height;

    // Determine if we're on spaceship or planet for different rendering
    LocationType location_type = space_get_location_type();
    int is_spaceship = (location_type == LOCATION_SPACESHIP);

    // Fill ceiling and floor with environment-specific colors
    for (int y = 0; y < viewport_height; ++y) {
        uint8_t r, g, b;
        if (y < horizon) {
            // Ceiling
            float t = (horizon > 0) ? (float)y / (float)horizon : 0.0f;
            if (is_spaceship) {
                // Spaceship ceiling - dark metallic with blue glow
                r = (uint8_t)(15.0f + 10.0f * t);
                g = (uint8_t)(20.0f + 15.0f * t);
                b = (uint8_t)(30.0f + 25.0f * t);
            } else {
                // Planet sky - natural gradient
                r = (uint8_t)(50.0f + 40.0f * t);
                g = (uint8_t)(80.0f + 50.0f * t);
                b = (uint8_t)(120.0f + 70.0f * t);
            }
        } else {
            // Floor
            int denom = viewport_height - horizon;
            float t = (denom > 0) ? (float)(y - horizon) / (float)denom : 0.0f;
            if (is_spaceship) {
                // Spaceship floor - metallic with subtle glow
                r = (uint8_t)(25.0f + 20.0f * t);
                g = (uint8_t)(30.0f + 25.0f * t);
                b = (uint8_t)(40.0f + 35.0f * t);
            } else {
                // Planet ground - warmer earthy tones
                r = (uint8_t)(60.0f + 80.0f * t);
                g = (uint8_t)(50.0f + 60.0f * t);
                b = (uint8_t)(35.0f + 40.0f * t);
            }
        }

        uint32_t color = pack_rgba(r, g, b);
        uint32_t* row = framebuffer + (size_t)y * (size_t)viewport_width;
        for (int x = 0; x < viewport_width; ++x) {
            row[x] = color;
        }
    }

    // Raycasting - render walls column by column
    const float max_dist = 50.0f;
    const float wall_height_world = 2.0f;
    const float fov = 66.0f;
    const float fov_radians = fov * (M_PI / 180.0f);

    float yaw_rad = player_yaw * (M_PI / 180.0f);
    float start_angle = yaw_rad - (fov_radians * 0.5f);
    float ray_angle_step = fov_radians / (float)viewport_width;

    // Render every column for better visual quality
    for (int x = 0; x < viewport_width; ++x) {
        float ray_angle = start_angle + x * ray_angle_step;
        float hit_dist;
        int hit_wall;
        cast_ray(ray_angle, max_dist, &hit_dist, &hit_wall);

        // Skip if ray didn't hit anything (hit distance is max_dist)
        if (hit_dist >= max_dist) {
            continue;
        }

        // Calculate perspective-corrected distance for wall height
        float corrected_dist = hit_dist * cosf(ray_angle - yaw_rad);
        if (corrected_dist < 0.001f) {
            corrected_dist = 0.001f;
        }

        // Calculate wall height on screen
        float line_height = ((float)viewport_height / corrected_dist) * wall_height_world;
        int draw_start = horizon - (int)(line_height * 0.5f);
        int draw_end = horizon + (int)(line_height * 0.5f);

        // Clamp to screen bounds
        if (draw_start < 0) draw_start = 0;
        if (draw_end >= viewport_height) draw_end = viewport_height - 1;
        if (draw_start > draw_end) continue; // Skip if wall is off-screen

        // Calculate distance-based shading (farther = darker)
        float shade = 1.0f - (hit_dist / max_dist) * 0.65f;
        if (shade < 0.25f) shade = 0.25f; // Minimum brightness
        if (shade > 1.0f) shade = 1.0f;
        
        // Different shade for different wall sides (north/south vs east/west)
        if (hit_wall) {
            shade *= 0.82f; // Slightly darker for one side
        }

        // Environment-specific wall colors
        uint8_t base_r, base_g, base_b;
        if (is_spaceship) {
            // Spaceship walls - metallic blue-gray with highlights
            base_r = hit_wall ? 120 : 140;
            base_g = hit_wall ? 145 : 165;
            base_b = hit_wall ? 180 : 200;
            
            // Add subtle blue glow
            if (!hit_wall) {
                base_b = (uint8_t)(base_b * 1.1f);
                if (base_b > 220) base_b = 220;
            }
        } else {
            // Planet walls - warmer stone/rock colors
            base_r = hit_wall ? 180 : 200;
            base_g = hit_wall ? 150 : 170;
            base_b = hit_wall ? 120 : 140;
        }

        uint8_t r = (uint8_t)(base_r * shade);
        uint8_t g = (uint8_t)(base_g * shade);
        uint8_t b = (uint8_t)(base_b * shade);
        
        // Add depth-based color variation for more visual interest
        float depth_factor = hit_dist / max_dist;
        if (depth_factor > 0.7f) {
            // Fade to darker at distance
            float fade = (depth_factor - 0.7f) / 0.3f;
            r = (uint8_t)(r * (1.0f - fade * 0.3f));
            g = (uint8_t)(g * (1.0f - fade * 0.3f));
            b = (uint8_t)(b * (1.0f - fade * 0.3f));
        }
        
        uint32_t wall_color = pack_rgba(r, g, b);

        // Draw the wall column
        for (int y = draw_start; y <= draw_end; ++y) {
            framebuffer[(size_t)y * (size_t)viewport_width + x] = wall_color;
        }
    }
}

// Check collision with world
int world_check_collision(float x, float y, float z, float radius) {
    int map_x = (int)(x / MAP_SCALE);
    int map_z = (int)(z / MAP_SCALE);
    
    // Check current cell and neighboring cells
    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (get_map_cell(map_x + dx, map_z + dz) == 1) {
                float cell_x = (map_x + dx) * MAP_SCALE;
                float cell_z = (map_z + dz) * MAP_SCALE;
                
                float dist_x = fabsf(x - cell_x);
                float dist_z = fabsf(z - cell_z);
                
                if (dist_x < radius + MAP_SCALE / 2.0f && 
                    dist_z < radius + MAP_SCALE / 2.0f) {
                    return 1; // Collision
                }
            }
        }
    }
    
    return 0; // No collision
}

// Get world bounds
void world_get_bounds(float* min_x, float* max_x, float* min_z, float* max_z) {
    if (min_x) *min_x = 0.0f;
    if (max_x) *max_x = MAP_WIDTH * MAP_SCALE;
    if (min_z) *min_z = 0.0f;
    if (max_z) *max_z = MAP_HEIGHT * MAP_SCALE;
}

// Set planet-specific map (for different planets)
void world_set_planet_map(int planet_type) {
    if (planet_type < 0) {
        return; // Invalid planet type
    }
    
    // Switch to planet map
    g_map = g_planet_map;
    printf("Map set for planet type %d\n", planet_type);
}

// Set spaceship map
void world_set_spaceship_map(void) {
    g_map = g_spaceship_map;
    printf("Map set for spaceship interior\n");
}

// Shutdown world
void world_shutdown(void) {
    g_world_initialized = 0;
}

