# WebGPU Fluid

A React and Vite application for an interactive 2D incompressible-fluid simulation. The simulation shaders are authored in [Slang](https://shader-slang.org/) and compiled to WGSL for WebGPU.

## Prerequisites

- Node.js and npm
- A WebGPU-capable browser
- [`slangc` 2026.8](https://github.com/shader-slang/slang/releases) available on `PATH`

The tested compiler is also included in Vulkan SDK 1.4.350.0. To use a compiler outside `PATH`, set `SLANGC_PATH` to the full executable path. The build intentionally rejects other Slang versions because WGSL support is still evolving and generated output can change between releases.

WebGPU requires a secure context in production. Localhost is treated as secure for development.

## Commands

```text
npm install
npm run dev
npm run build
npm run lint
npm run shaders
npm run validate:solver
```

`npm run dev` and `npm run build` compile every shader declared in `scripts/shader-manifest.mjs` before starting Vite. Generated WGSL is written to `src/gpu/shaders/generated/`, is ignored by Git, and must not be edited directly.

If shader compilation fails, confirm that this command prints `2026.8`:

```text
slangc -version
```

## Project structure

```text
scripts/
  compile-shaders.mjs     Slang compiler integration
  shader-manifest.mjs     Shader entry points and tested compiler version
src/gpu/
  core/                   Typed WebGPU setup and resource helpers
  shaders/                Slang source
  shaders/generated/      Build-generated WGSL
```

The React page will own controls and status UI. The WebGPU simulation engine will own GPU resources, compute/render pipelines, and the animation lifecycle.

The solver uses ping-pong `rgba16float` textures for velocity, pressure, and dye. Each fixed timestep advects velocity, computes divergence, solves pressure with Jacobi iterations, subtracts the pressure gradient, and advects dye. `npm run validate:solver` runs a deterministic CPU reference check that confirms the projection step reduces RMS divergence.
