export interface FluidSettings {
  readonly fixedTimeStep: number
  readonly maximumSubsteps: number
  readonly pressureIterations: number
  readonly velocityDissipation: number
  readonly dyeDissipation: number
  readonly viscosity: number
  readonly force: number
  readonly exposure: number
}

export const defaultFluidSettings: FluidSettings = {
  fixedTimeStep: 1 / 60,
  maximumSubsteps: 4,
  pressureIterations: 32,
  velocityDissipation: 0.12,
  dyeDissipation: 0.08,
  viscosity: 0.08,
  force: 1,
  exposure: 1.35,
}

export function normalizeFluidSettings(
  settings: FluidSettings,
): FluidSettings {
  return {
    fixedTimeStep: Math.min(Math.max(settings.fixedTimeStep, 1 / 240), 1 / 20),
    maximumSubsteps: Math.round(
      Math.min(Math.max(settings.maximumSubsteps, 1), 8),
    ),
    pressureIterations: Math.round(
      Math.min(Math.max(settings.pressureIterations, 4), 80),
    ),
    velocityDissipation: Math.min(
      Math.max(settings.velocityDissipation, 0),
      5,
    ),
    dyeDissipation: Math.min(Math.max(settings.dyeDissipation, 0), 5),
    viscosity: Math.min(Math.max(settings.viscosity, 0), 10),
    force: Math.min(Math.max(settings.force, 0), 10),
    exposure: Math.min(Math.max(settings.exposure, 0.1), 5),
  }
}
