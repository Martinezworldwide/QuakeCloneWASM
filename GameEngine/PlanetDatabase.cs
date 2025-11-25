// Planet database - C# game engine
// QuakeCloneWASM - Game Engine

namespace GameEngine;

/// <summary>
/// Database of all planets in the game
/// </summary>
public static class PlanetDatabase
{
    /// <summary>
    /// Get all available planets with realistic astronomical data
    /// </summary>
    public static List<Planet> GetAllPlanets()
    {
        return new List<Planet>
        {
            // Planet 0: Earth-like (Terran)
            new Planet
            {
                Name = "Terra Nova",
                DistanceAu = 1.0f,
                RadiusKm = 6371.0f,
                SurfaceTempK = 288.0f,
                GravityG = 1.0f,
                Atmosphere = AtmosphereType.Breathable,
                HasWater = true,
                HasLife = true,
                RotationPeriodHours = 24.0f,
                ResourceRichness = 85
            },
            
            // Planet 1: Mars-like (Desert)
            new Planet
            {
                Name = "Aridus Prime",
                DistanceAu = 1.5f,
                RadiusKm = 3396.0f,
                SurfaceTempK = 210.0f,
                GravityG = 0.38f,
                Atmosphere = AtmosphereType.Thin,
                HasWater = false,
                HasLife = false,
                RotationPeriodHours = 24.6f,
                ResourceRichness = 60
            },
            
            // Planet 2: Venus-like (Toxic)
            new Planet
            {
                Name = "Vulcanis",
                DistanceAu = 0.7f,
                RadiusKm = 6051.0f,
                SurfaceTempK = 737.0f,
                GravityG = 0.91f,
                Atmosphere = AtmosphereType.Toxic,
                HasWater = false,
                HasLife = false,
                RotationPeriodHours = 5832.0f,
                ResourceRichness = 40
            },
            
            // Planet 3: Gas Giant Moon (Ice)
            new Planet
            {
                Name = "Glacius",
                DistanceAu = 5.2f,
                RadiusKm = 2634.0f,
                SurfaceTempK = 110.0f,
                GravityG = 0.13f,
                Atmosphere = AtmosphereType.None,
                HasWater = true,
                HasLife = false,
                RotationPeriodHours = 84.0f,
                ResourceRichness = 75
            },
            
            // Planet 4: Ocean World
            new Planet
            {
                Name = "Aquarius",
                DistanceAu = 1.2f,
                RadiusKm = 8000.0f,
                SurfaceTempK = 280.0f,
                GravityG = 1.2f,
                Atmosphere = AtmosphereType.Breathable,
                HasWater = true,
                HasLife = true,
                RotationPeriodHours = 18.0f,
                ResourceRichness = 90
            },
            
            // Planet 5: Rocky Desert
            new Planet
            {
                Name = "Cimmeria",
                DistanceAu = 2.8f,
                RadiusKm = 4500.0f,
                SurfaceTempK = 180.0f,
                GravityG = 0.55f,
                Atmosphere = AtmosphereType.None,
                HasWater = false,
                HasLife = false,
                RotationPeriodHours = 36.0f,
                ResourceRichness = 70
            },
            
            // Planet 6: Lava World
            new Planet
            {
                Name = "Inferno",
                DistanceAu = 0.3f,
                RadiusKm = 6000.0f,
                SurfaceTempK = 1500.0f,
                GravityG = 0.95f,
                Atmosphere = AtmosphereType.Toxic,
                HasWater = false,
                HasLife = false,
                RotationPeriodHours = 12.0f,
                ResourceRichness = 95
            },
            
            // Planet 7: Gas Giant with Space Station
            new Planet
            {
                Name = "Neptunus Station",
                DistanceAu = 30.0f,
                RadiusKm = 24622.0f,
                SurfaceTempK = 55.0f,
                GravityG = 1.14f,
                Atmosphere = AtmosphereType.Breathable,
                HasWater = false,
                HasLife = false,
                RotationPeriodHours = 16.0f,
                ResourceRichness = 50
            }
        };
    }
}

