# Chromaflow

Chromaflow is a one-page React application that runs a colourful 2D incompressible-fluid simulation on WebGPU. Compute and render shaders are authored in [Slang](https://shader-slang.org/) and compiled ahead of time to WGSL with `slangc`.

## Requirements

- Node.js and npm
- A browser and GPU/driver combination that supports WebGPU
- A secure context for deployment (`https://`); `localhost` is accepted for development
- [`slangc` 2026.8](https://github.com/shader-slang/slang/releases) available on `PATH`

The tested compiler is also included in Vulkan SDK 1.4.350.0. To use another installation location, set `SLANGC_PATH` to the full compiler executable path:

```powershell
$env:SLANGC_PATH = 'C:\tools\slang\bin\slangc.exe'
npm run shaders
```

The build intentionally rejects other Slang versions. Slang's WGSL backend is still evolving, so pinning the tested compiler prevents silent shader-output changes.

## Development

```text
npm install
npm run dev
```

Open the local URL printed by Vite. `npm run dev` compiles all Slang entry points before starting the development server.

Available commands:

| Command | Purpose |
| --- | --- |
| `npm run dev` | Compile shaders and start Vite with HMR |
| `npm run shaders` | Compile the shader manifest to generated WGSL |
| `npm run test` | Run focused Vitest tests |
| `npm run validate:solver` | Verify that pressure projection reduces RMS divergence |
| `npm run lint` | Run ESLint |
| `npm run build` | Compile shaders, type-check, and create the production bundle |
| `npm run preview` | Preview the production bundle |
| `npm run check` | Run tests, solver validation, lint, and the production build |

Generated WGSL is written to `src/gpu/shaders/generated/`, ignored by Git, and regenerated for every development or production build. Do not edit generated shader files.

## Controls

- **Pointer or touch drag:** inject momentum and palette-driven dye
- **Pause / Resume:** suspend or continue simulation stepping
- **Reset:** clear the fields and restore the opening colour splats
- **Palette:** choose the colour sequence used for new dye
- **Brush:** change the injection radius
- **Force:** scale pointer momentum
- **Viscosity:** smooth velocity through local diffusion
- **Velocity fade:** control momentum dissipation
- **Colour fade:** control dye dissipation
- **Pressure:** change Jacobi iterations; higher values improve projection quality but cost more GPU time

The input controller uses pointer capture, coalesced events where available, bounded velocity, interpolation between sparse events, and a capped queue. Mouse, pen, and multiple simultaneous touch pointers use the same path.

## Architecture

```text
scripts/
  compile-shaders.mjs       Version check and atomic Slang compilation
  shader-manifest.mjs       Shader entry points and output declarations
  validate-projection.mjs   Deterministic CPU projection invariant
src/
  interaction/              Palette sampling and pointer-to-splat input
  gpu/core/                 WebGPU setup, errors, buffers, and uniforms
  gpu/shaders/              Slang source
  gpu/shaders/generated/    Build-generated WGSL
  gpu/simulation/           Pipelines, textures, sizing, settings, and runtime
  App.tsx                   React lifecycle, controls, and status UI
tests/                      Focused pure-logic tests
```

`FluidSimulation` owns the WebGPU device, resources, pipelines, command encoding, fixed-step accumulator, resize behavior, and cleanup. React owns only page state and controls; high-frequency pointer input stays outside React.

The solver uses ping-pong `rgba16float` textures for velocity, pressure, and dye plus a divergence texture. Each fixed timestep:

1. Applies queued force and dye splats.
2. Backtraces and advects velocity.
3. Applies velocity dissipation and viscosity smoothing.
4. Computes velocity divergence.
5. Clears and solves pressure with Jacobi relaxation.
6. Subtracts the pressure gradient to project velocity.
7. Advects and dissipates dye.
8. Tone-maps the dye field to a full-screen render pass.

The simulation grid is independent from canvas resolution. It uses a lower long-edge target on narrow/mobile viewports, aligns dimensions to the compute workgroup size, caps device pixel ratio, and limits fixed-timestep catch-up work.

## Browser and device behavior

The application requests a high-performance WebGPU adapter but does not require optional GPU features. Shader outputs use write-only `rgba16float` storage textures so the solver does not depend on read/write storage-texture support.

When WebGPU, an adapter, the canvas context, shader compilation, or the device is unavailable, the app shows an accessible compatibility message instead of silently switching to a CPU solver. Device loss and uncaptured WebGPU errors are also surfaced to the page.

## Troubleshooting

### `slangc` cannot be found

Run:

```text
slangc -version
```

The output must contain `2026.8`. Add the compiler to `PATH` or set `SLANGC_PATH`.

### The page says no suitable GPU adapter was found

- Use a current WebGPU-capable browser.
- Enable hardware acceleration.
- Update the GPU driver.
- Test over HTTPS or localhost.
- Check browser GPU diagnostics for blocked or disabled adapters.

### Shader compilation fails

Run `npm run shaders` directly to see the Slang diagnostic. Confirm the compiler version and avoid editing `src/gpu/shaders/generated/`.

### The simulation is slow on a mobile device

Reduce **Pressure**, **Force**, or **Brush**. The app already selects a lower mobile grid and caps frame catch-up, but pressure iterations remain the primary quality/performance trade-off.

## Verification scope

Automated checks cover adaptive dimensions, settings bounds, pointer interpolation and velocity limits, palette sampling, uniform packing, shader compilation, and a deterministic pressure-projection invariant. A supported physical WebGPU browser is still required to verify device-specific rendering, touch behavior, and performance.
