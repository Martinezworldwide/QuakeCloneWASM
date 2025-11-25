// Space exploration system - Planets and beaming
// QuakeCloneWASM - Space exploration

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "space.h"
#include "player.h"
#include "world.h"

// Current location state
static LocationType g_current_location = LOCATION_PLANET;
static int g_current_planet = 0; // Start on first planet

// Realistic planet database - Based on real exoplanet characteristics
static PlanetData g_planets[] = {
    // Planet 0: Earth-like (Terran)
    {
        .name = "Terra Nova",
        .distance_au = 1.0f,
        .radius_km = 6371.0f,
        .surface_temp_k = 288.0f,
        .gravity_g = 1.0f,
        .atmosphere_type = 2,
        .has_water = 1,
        .has_life = 1,
        .rotation_period_h = 24.0f,
        .resource_richness = 85,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 1: Mars-like (Desert)
    {
        .name = "Aridus Prime",
        .distance_au = 1.5f,
        .radius_km = 3396.0f,
        .surface_temp_k = 210.0f,
        .gravity_g = 0.38f,
        .atmosphere_type = 1,
        .has_water = 0,
        .has_life = 0,
        .rotation_period_h = 24.6f,
        .resource_richness = 60,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 2: Venus-like (Toxic)
    {
        .name = "Vulcanis",
        .distance_au = 0.7f,
        .radius_km = 6051.0f,
        .surface_temp_k = 737.0f,
        .gravity_g = 0.91f,
        .atmosphere_type = 3,
        .has_water = 0,
        .has_life = 0,
        .rotation_period_h = 5832.0f,
        .resource_richness = 40,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 3: Gas Giant Moon (Ice)
    {
        .name = "Glacius",
        .distance_au = 5.2f,
        .radius_km = 2634.0f,
        .surface_temp_k = 110.0f,
        .gravity_g = 0.13f,
        .atmosphere_type = 0,
        .has_water = 1,
        .has_life = 0,
        .rotation_period_h = 84.0f,
        .resource_richness = 75,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 4: Ocean World
    {
        .name = "Aquarius",
        .distance_au = 1.2f,
        .radius_km = 8000.0f,
        .surface_temp_k = 280.0f,
        .gravity_g = 1.2f,
        .atmosphere_type = 2,
        .has_water = 1,
        .has_life = 1,
        .rotation_period_h = 18.0f,
        .resource_richness = 90,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 5: Rocky Desert
    {
        .name = "Cimmeria",
        .distance_au = 2.8f,
        .radius_km = 4500.0f,
        .surface_temp_k = 180.0f,
        .gravity_g = 0.55f,
        .atmosphere_type = 0,
        .has_water = 0,
        .has_life = 0,
        .rotation_period_h = 36.0f,
        .resource_richness = 70,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 6: Lava World
    {
        .name = "Inferno",
        .distance_au = 0.3f,
        .radius_km = 6000.0f,
        .surface_temp_k = 1500.0f,
        .gravity_g = 0.95f,
        .atmosphere_type = 3,
        .has_water = 0,
        .has_life = 0,
        .rotation_period_h = 12.0f,
        .resource_richness = 95,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    },
    // Planet 7: Gas Giant with Space Station
    {
        .name = "Neptunus Station",
        .distance_au = 30.0f,
        .radius_km = 24622.0f,
        .surface_temp_k = 55.0f,
        .gravity_g = 1.14f,
        .atmosphere_type = 2,
        .has_water = 0,
        .has_life = 0,
        .rotation_period_h = 16.0f,
        .resource_richness = 50,
        .map_offset_x = 16.0f,
        .map_offset_z = 16.0f
    }
};

static const int g_planet_count = sizeof(g_planets) / sizeof(g_planets[0]);
static int g_space_initialized = 0;

// Spaceship interior state (simple room)
static float g_spaceship_player_x = 0.0f;
static float g_spaceship_player_z = 0.0f;

// Initialize space system
int space_init(void) {
    if (g_space_initialized) {
        return 1;
    }
    
    g_current_location = LOCATION_PLANET;
    g_current_planet = 0; // Start on first planet
    
    g_space_initialized = 1;
    printf("Space exploration system initialized with %d planets\n", g_planet_count);
    
    return 1;
}

// Get current location type
LocationType space_get_location_type(void) {
    return g_current_location;
}

// Get current planet index
int space_get_current_planet(void) {
    if (g_current_location == LOCATION_PLANET) {
        return g_current_planet;
    }
    return -1;
}

// Beam player to spaceship
void space_beam_to_spaceship(void) {
    if (g_current_location == LOCATION_SPACESHIP) {
        return; // Already on spaceship
    }
    
    printf("Beaming up to spaceship...\n");
    
    // Save current planet position
    float player_x, player_y, player_z;
    player_get_position(&player_x, &player_y, &player_z);
    
    // Teleport to spaceship interior
    g_current_location = LOCATION_SPACESHIP;
    
    // Switch to spaceship map
    extern void world_set_spaceship_map(void);
    world_set_spaceship_map();
    
    player_init(4.0f, 4.0f); // Spaceship interior position
    player_set_rotation(0.0f, 0.0f);
    
    printf("Arrived on spaceship\n");
}

// Beam player to pilot seat (triggers C# transition)
void space_beam_to_pilot_seat(void) {
    printf("Beaming to pilot seat...\n");
    printf("Initializing C# game engine transition...\n");
    
    // This will trigger JavaScript to show loading screen and initialize C# runtime
    // The actual C# initialization will be handled by JavaScript
}

// Beam player to planet
void space_beam_to_planet(int planet_index) {
    if (planet_index < 0 || planet_index >= g_planet_count) {
        printf("ERROR: Invalid planet index %d\n", planet_index);
        return;
    }
    
    PlanetData* planet = &g_planets[planet_index];
    
    printf("Beaming down to %s...\n", planet->name);
    printf("  Distance: %.2f AU\n", planet->distance_au);
    printf("  Temperature: %.1f K (%.1f C)\n", planet->surface_temp_k, planet->surface_temp_k - 273.15f);
    printf("  Gravity: %.2fg\n", planet->gravity_g);
    printf("  Atmosphere: %s\n", 
           planet->atmosphere_type == 0 ? "None" :
           planet->atmosphere_type == 1 ? "Thin" :
           planet->atmosphere_type == 2 ? "Breathable" : "Toxic");
    
    g_current_location = LOCATION_PLANET;
    g_current_planet = planet_index;
    
    // Set planet-specific map
    world_set_planet_map(planet_index);
    
    // Teleport to planet surface
    player_init(planet->map_offset_x, planet->map_offset_z);
    player_set_rotation(0.0f, 0.0f);
    
    printf("Arrived on %s surface\n", planet->name);
}

// Get planet data
PlanetData* space_get_planet(int index) {
    if (index < 0 || index >= g_planet_count) {
        return NULL;
    }
    return &g_planets[index];
}

// Get number of planets
int space_get_planet_count(void) {
    return g_planet_count;
}

// Update space system
void space_update(double delta_time) {
    // Future: Add space environment updates (stars, nebulas, etc.)
}

// Render space environment (if on spaceship)
void space_render(void) {
    // Future: Render space viewscreen, stars, planets in distance
    // For now, spaceship interior uses same world rendering
}

