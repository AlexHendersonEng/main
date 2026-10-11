# Helios

Helios is an Unreal Engine 5.8 C++ scenario playback and camera-rendering
prototype. A JSON scenario describes runtime-loaded OBJ/STL vehicle meshes,
CSV position/attitude trajectories, cameras, playback defaults, and optional
video or image-sequence exports. Supported vehicle domains are aircraft,
ground vehicles, boats, and underwater vessels.

## Quick start

1. Open `helios.uproject` with Unreal Engine 5.8 and build the `heliosEditor`
   target.
2. Start Play-in-Editor. The default scenario is
   `Config/Scenarios/sample_scenario.json`.
3. Use the on-screen panel to play/pause, scrub time, adjust speed (0.1x-4x),
   toggle vehicle visibility, or render the configured cameras.

To select another scenario, add `-HeliosConfig=<absolute-or-project-relative-path>`
to the editor/game command line. A smaller submarine-only example is
`Config/Scenarios/underwater_scenario.json`.

## Build from PowerShell

Set `$UE` to your Unreal Engine 5.8 installation, then run from the project
directory:

```powershell
$UE = 'C:\Program Files\Epic Games\UE_5.8'
& "$UE\Engine\Build\BatchFiles\Build.bat" heliosEditor Win64 Development "$PWD\helios.uproject" -WaitMutex
```

To build a standalone Windows game target, replace `heliosEditor` with `helios`.
Open the project in the editor to preview scenarios and use Movie Render Queue.

## Sample content and configuration

- `Config/Scenarios/sample_scenario.json`: all four vehicle domains, two
  enabled export cameras, and an underwater preview camera.
- `Config/Scenarios/underwater_scenario.json`: one underwater vehicle and an
  image-sequence camera export.
- `SampleAssets/Models/`: small geometry-only OBJ and STL proxy meshes.
- `SampleAssets/Trajectories/`: short CSV trajectories for each vehicle.

See [scenario configuration](Config/Scenarios/README.md) for the JSON schema,
units, loader limitations, camera exports, and command-line options.

## Rendering

The preview panel's **Render configured cameras** button queues one render job
per enabled camera. For automated runs, launch the built game with a map, then
the config and render switches; for example:

```text
helios.exe /Engine/Maps/Templates/OpenWorld -log -HeliosConfig="C:\path\to\helios\Config\Scenarios\sample_scenario.json" -HeliosRender
```

PNG sequences and MP4 files go to the output directories in the camera entries.
Video encoding uses Unreal's built-in Movie Render Pipeline MP4 encoder and no
external codec library. Check platform codec licensing requirements before
redistributing encoded video.

## Implementation notes

Configuration, CSV parsing, and OBJ/STL geometry parsing use first-party
Helios C++ and Unreal Engine modules. The Water plugin is used for the sample
ocean surface; sky, lighting, fog, and basic environment planes are created at
runtime. The visuals are simple environment backdrops, not terrain, buoyancy,
or underwater physics simulations. The project is a prototype and its sample
proxy meshes are intended to be replaced with your own models.

The project adds no third-party or paid runtime libraries. The Water plugin is
Epic-provided and enabled in the project; confirm its availability in your UE
installation. Review the Unreal Engine EULA, any model-asset licenses, and
platform-specific codec terms before commercial distribution.
