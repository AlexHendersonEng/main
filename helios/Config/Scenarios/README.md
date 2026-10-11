# Scenario configuration

Helios reads **JSON**, not YAML. At startup it loads
`Config/Scenarios/sample_scenario.json`; pass `-HeliosConfig=<path>` to select a
different file. Paths in the config may be absolute or relative to the project
directory.

`sample_scenario.json` is a runnable example with aircraft, ground vehicle,
boat, and underwater vessel, plus vehicle-attached and world-fixed cameras.
`underwater_scenario.json` is a smaller example focused on the underwater view.
Both use the lightweight OBJ/STL proxy meshes and CSV trajectories in
`SampleAssets/`.

## Schema

```jsonc
{
  "scenario_name": "string",
  "vehicles": [
    {
      "id": "string (unique)",
      "type": "Aircraft | GroundVehicle | Boat | UnderwaterVessel",
      "model_path": "path to a .obj or .stl file",
      "trajectory_csv": "path to a .csv file",
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
        "resolution_x": 640,
        "resolution_y": 360,
        "frame_rate": 24,
        "output_directory": "Renders/camera_id",
        "output_name": "camera_id"
      }
    }
  ],
  "playback": {
    "default_speed": 1.0,
    "loop": false
  }
}
```

### Units and behavior

- CSV columns are `time,x,y,z,roll,pitch,yaw`; time is seconds, position is
  meters, and attitude is degrees. Position is converted to Unreal centimeters.
  A header row is optional; samples are sorted by time and interpolated between
  rows.
- Model scale is applied after the loader's meters-to-centimeters conversion.
  OBJ faces should use positive vertex indices; STL may be ASCII or binary.
  These are geometry-only runtime loaders, not full DCC importers (materials,
  textures, and skeletal animation are not read).
- Camera locations and attached-camera offsets use Unreal centimeters;
  rotations and FOV are degrees.
- Empty or unrecognized vehicle type defaults to `Aircraft`; an unrecognized
  export mode disables that camera's export. Invalid entries missing an `id`
  are skipped with a warning.
- The ground plane is added when the scenario includes a ground vehicle. An
  ocean plane with the Water plugin's built-in material is added when it
  includes a boat or underwater vessel. An underwater-attached camera gets a
  blue grade and vignette; this is a visual treatment, not a fluid/pressure
  simulation.
- The generated sequence drives the configured trajectory; no vehicle physics
  simulation overrides it.

### Camera exports

Set `export.enabled` to `true` and choose `video` for the built-in Unreal MP4
encoder or `image_sequence` for PNG frames. Each enabled camera gets its own
Movie Render Queue job, output path, resolution, and frame rate. Relative output
directories are project-relative. The default sample writes beneath `Renders/`.

Start preview, then use **Render configured cameras** in the playback panel.
For a non-interactive game run, add `-HeliosRender`. That option starts every
enabled camera export at startup; it does not change the scenario config.
Review applicable H.264/platform codec terms before distributing MP4 output.
