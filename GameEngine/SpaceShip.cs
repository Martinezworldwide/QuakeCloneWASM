// Space ship system - C# game engine
// QuakeCloneWASM - Game Engine

using System.Numerics;

namespace GameEngine;

/// <summary>
/// Represents the player's spaceship with flight mechanics
/// </summary>
public class SpaceShip
{
    public Vector3 Position { get; set; } = Vector3.Zero;
    public Vector3 Velocity { get; set; } = Vector3.Zero;
    public Vector3 Rotation { get; set; } = Vector3.Zero; // Pitch, Yaw, Roll in degrees
    
    // Flight parameters
    public float MaxSpeed { get; set; } = 50.0f; // Units per second
    public float Acceleration { get; set; } = 10.0f;
    public float RotationSpeed { get; set; } = 90.0f; // Degrees per second
    
    // Ship status
    public float Fuel { get; set; } = 100.0f; // 0-100%
    public float HullIntegrity { get; set; } = 100.0f; // 0-100%
    public float ShieldPower { get; set; } = 100.0f; // 0-100%
    
    // Energy systems
    public float Energy { get; set; } = 100.0f; // 0-100%
    public float EnergyRegenRate { get; set; } = 5.0f; // Per second
    
    // Scanning system
    public float ScannerRange { get; set; } = 1000.0f; // Units
    public List<Planet> ScannedPlanets { get; set; } = new();
    public List<Planet> NearbyPlanets { get; set; } = new();
    
    /// <summary>
    /// Update ship physics and systems
    /// </summary>
    public void Update(float deltaTime)
    {
        // Regenerate energy
        Energy = Math.Min(100.0f, Energy + EnergyRegenRate * deltaTime);
        
        // Update position based on velocity
        Position += Velocity * deltaTime;
        
        // Apply drag/friction
        Velocity *= 0.98f; // Gradual slowdown
    }
    
    /// <summary>
    /// Apply thruster input in the forward direction
    /// </summary>
    public void ApplyThruster(float power, float deltaTime)
    {
        if (Energy < 1.0f) return; // Need energy to thrust
        
        power = Math.Clamp(power, 0.0f, 1.0f);
        float energyCost = power * 5.0f * deltaTime;
        
        if (Energy >= energyCost)
        {
            // Calculate forward direction from rotation
            float yawRad = Rotation.Y * MathF.PI / 180.0f;
            float pitchRad = Rotation.X * MathF.PI / 180.0f;
            
            Vector3 forward = new(
                MathF.Sin(yawRad) * MathF.Cos(pitchRad),
                -MathF.Sin(pitchRad),
                -MathF.Cos(yawRad) * MathF.Cos(pitchRad)
            );
            
            Velocity = Velocity + forward * Acceleration * power * deltaTime;
            
            // Clamp velocity to max speed
            if (Velocity.Length() > MaxSpeed)
            {
                Velocity = Vector3.Normalize(Velocity) * MaxSpeed;
            }
            
            Energy -= energyCost;
        }
    }
    
    /// <summary>
    /// Apply rotation input
    /// </summary>
    public void ApplyRotation(float pitch, float yaw, float roll, float deltaTime)
    {
        float newPitch = Rotation.X + pitch * RotationSpeed * deltaTime;
        float newYaw = Rotation.Y + yaw * RotationSpeed * deltaTime;
        float newRoll = Rotation.Z + roll * RotationSpeed * deltaTime;
        
        // Clamp pitch
        newPitch = Math.Clamp(newPitch, -90.0f, 90.0f);
        
        // Normalize yaw
        while (newYaw < 0) newYaw += 360.0f;
        while (newYaw >= 360.0f) newYaw -= 360.0f;
        
        // Create new rotation vector
        Rotation = new Vector3(newPitch, newYaw, newRoll);
    }
    
    /// <summary>
    /// Scan for nearby planets
    /// </summary>
    public void ScanForPlanets(List<Planet> allPlanets)
    {
        NearbyPlanets.Clear();
        
        foreach (var planet in allPlanets)
        {
            // Simple distance check (in real implementation, would use 3D distance)
            float distance = planet.DistanceAu * 149.6f; // Convert AU to millions of km (simplified)
            
            if (distance <= ScannerRange)
            {
                NearbyPlanets.Add(planet);
                
                // Add to scanned planets if not already there
                if (!ScannedPlanets.Contains(planet))
                {
                    ScannedPlanets.Add(planet);
                }
            }
        }
    }
}

