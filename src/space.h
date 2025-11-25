// Space exploration system - Planets and beaming
// QuakeCloneWASM - Space exploration

#ifndef SPACE_H
#define SPACE_H

// Location types
typedef enum {
    LOCATION_SPACESHIP = 0,
    LOCATION_PLANET = 1
} LocationType;

// Planet data structure
typedef struct {
    const char* name;
    float distance_au;        // Distance from star in AU
    float radius_km;          // Planet radius in km
    float surface_temp_k;     // Surface temperature in Kelvin
    float gravity_g;          // Gravity in g units
    int atmosphere_type;      // 0=none, 1=thin, 2=thick, 3=toxic
    int has_water;           // 0=no, 1=yes
    int has_life;            // 0=no, 1=yes
    float rotation_period_h;  // Rotation period in hours
    int resource_richness;    // 0-100
    float map_offset_x;       // Map spawn position X
    float map_offset_z;       // Map spawn position Z
} PlanetData;

// Get current location type
LocationType space_get_location_type(void);

// Get current planet index (-1 if on spaceship)
int space_get_current_planet(void);

// Beam player to spaceship
void space_beam_to_spaceship(void);

// Beam player to pilot seat (triggers C# transition)
void space_beam_to_pilot_seat(void);

// Beam player to planet
void space_beam_to_planet(int planet_index);

// Get planet data
PlanetData* space_get_planet(int index);

// Get number of available planets
int space_get_planet_count(void);

// Initialize space system
int space_init(void);

// Update space system
void space_update(double delta_time);

// Render space environment (if on spaceship)
void space_render(void);

#endif // SPACE_H

