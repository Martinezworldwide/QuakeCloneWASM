// Planet data structure - C# game engine
// QuakeCloneWASM - Game Engine

namespace GameEngine;

/// <summary>
/// Represents a planet with realistic astronomical data
/// </summary>
public class Planet
{
    public string Name { get; set; } = string.Empty;
    public float DistanceAu { get; set; } // Distance from star in AU
    public float RadiusKm { get; set; } // Planet radius in km
    public float SurfaceTempK { get; set; } // Surface temperature in Kelvin
    public float GravityG { get; set; } // Gravity in g units
    public AtmosphereType Atmosphere { get; set; }
    public bool HasWater { get; set; }
    public bool HasLife { get; set; }
    public float RotationPeriodHours { get; set; }
    public int ResourceRichness { get; set; } // 0-100
    
    public float SurfaceTempCelsius => SurfaceTempK - 273.15f;
    
    public string AtmosphereDescription => Atmosphere switch
    {
        AtmosphereType.None => "None",
        AtmosphereType.Thin => "Thin",
        AtmosphereType.Breathable => "Breathable",
        AtmosphereType.Toxic => "Toxic",
        _ => "Unknown"
    };
}

/// <summary>
/// Atmosphere types for planets
/// </summary>
public enum AtmosphereType
{
    None = 0,
    Thin = 1,
    Breathable = 2,
    Toxic = 3
}

