# Helios

Helios scenarios are plain **JSON** files (no YAML). By default the game loads
`<ProjectDir>/Config/Scenarios/sample_scenario.json` on startup; override this with the
`-HeliosConfig=<path>` command line switch.

See `Config/Scenarios/sample_scenario.json` for a complete example covering all four
supported vehicle domains (aircraft, ground vehicle, boat, underwater vessel) and two
camera setups (a vehicle-attached chase camera exporting video, and a world-fixed
overview camera exporting an image sequence).

## Schema

```jsonc
{
  "scenario_name": "string",
  "vehicles": [
    {
      "id": "string (unique)",
      "type": "Aircraft | GroundVehicle | Boat | UnderwaterVessel",
      "model_path": "path to a .obj or .stl file, loaded at runtime",
      "trajectory_csv": "path to a .csv file with columns: time,x,y,z,roll,pitch,yaw",
      "scale": 1.0,
      "visible": true
    }
  ],
  "cameras": [
    {
      "id": "string (unique)",
      "attached_to": "vehicle id, or empty string for a world-fixed camera",
      "location": { "x": 0.0, "y": 0.0, "z": 0.0 },
      "rotation": { "pitch": 0.0, "yaw": 0.0, "roll": 0.0 },
      "fov": 90.0,
      "export": {
        "enabled": true,
        "mode": "video | image_sequence",
        "resolution_x": 1920,
        "resolution_y": 1080,
        "frame_rate": 30,
        "output_directory": "path",
        "output_name": "string"
      }
    }
  ],
  "playback": {
    "default_speed": 1.0,
    "loop": false
  }
}
```

### Notes
- `time` in the trajectory CSV is in seconds; positions are meters; `roll`/`pitch`/`yaw` are degrees.
  Samples are linearly interpolated between timestamps during playback.
- Model/trajectory paths may be absolute, or relative to the project directory.
- This file is parsed by `UHeliosScenarioSubsystem` (`Source/helios/Config/HeliosScenarioSubsystem.h`)
  into the `FHeliosScenarioConfig` struct (`Source/helios/Config/HeliosScenarioTypes.h`).
