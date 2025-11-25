// Pilot seat controller - C# game engine
// QuakeCloneWASM - Game Engine

namespace GameEngine;

/// <summary>
/// Controls the pilot seat interface and flight mechanics
/// </summary>
public class PilotSeatController
{
    private SpaceShip _ship;
    private List<Planet> _planets;
    private bool _isActive = false;
    
    // Input state
    private float _thrusterPower = 0.0f;
    private float _pitchInput = 0.0f;
    private float _yawInput = 0.0f;
    private float _rollInput = 0.0f;
    
    // UI state
    public bool ScannerActive { get; set; } = false;
    public Planet? SelectedPlanet { get; set; } = null;
    
    public PilotSeatController(SpaceShip ship, List<Planet> planets)
    {
        _ship = ship;
        _planets = planets;
    }
    
    /// <summary>
    /// Initialize pilot seat mode
    /// </summary>
    public void Activate()
    {
        _isActive = true;
        ScannerActive = false;
        SelectedPlanet = null;
        
        // Reset ship controls
        _thrusterPower = 0.0f;
        _pitchInput = 0.0f;
        _yawInput = 0.0f;
        _rollInput = 0.0f;
    }
    
    /// <summary>
    /// Deactivate pilot seat mode
    /// </summary>
    public void Deactivate()
    {
        _isActive = false;
    }
    
    /// <summary>
    /// Update pilot seat controller (called each frame)
    /// </summary>
    public void Update(float deltaTime)
    {
        if (!_isActive) return;
        
        // Update ship physics
        _ship.Update(deltaTime);
        
        // Apply flight controls
        _ship.ApplyThruster(_thrusterPower, deltaTime);
        _ship.ApplyRotation(_pitchInput, _yawInput, _rollInput, deltaTime);
        
        // Update scanner if active
        if (ScannerActive)
        {
            _ship.ScanForPlanets(_planets);
        }
    }
    
    // Input handlers
    public void SetThrusterPower(float power) => _thrusterPower = Math.Clamp(power, 0.0f, 1.0f);
    public void SetPitchInput(float pitch) => _pitchInput = Math.Clamp(pitch, -1.0f, 1.0f);
    public void SetYawInput(float yaw) => _yawInput = Math.Clamp(yaw, -1.0f, 1.0f);
    public void SetRollInput(float roll) => _rollInput = Math.Clamp(roll, -1.0f, 1.0f);
    
    // Ship status getters
    public float GetSpeed() => _ship.Velocity.Length();
    public float GetFuel() => _ship.Fuel;
    public float GetHullIntegrity() => _ship.HullIntegrity;
    public float GetShieldPower() => _ship.ShieldPower;
    public float GetEnergy() => _ship.Energy;
    
    // Scanner methods
    public void ToggleScanner() => ScannerActive = !ScannerActive;
    public List<Planet> GetNearbyPlanets() => _ship.NearbyPlanets;
    public List<Planet> GetScannedPlanets() => _ship.ScannedPlanets;
    
    // Planet selection
    public void SelectPlanet(Planet planet) => SelectedPlanet = planet;
    public void DeselectPlanet() => SelectedPlanet = null;
}

