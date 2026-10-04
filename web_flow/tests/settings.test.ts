import { describe, expect, it } from 'vitest'
import {
  defaultFluidSettings,
  normalizeFluidSettings,
} from '../src/gpu/simulation/settings'

describe('fluid settings', () => {
  it('preserves defaults inside supported bounds', () => {
    expect(normalizeFluidSettings(defaultFluidSettings)).toEqual(
      defaultFluidSettings,
    )
  })

  it('clamps unsafe values and rounds discrete settings', () => {
    expect(
      normalizeFluidSettings({
        fixedTimeStep: 1,
        maximumSubsteps: 99,
        pressureIterations: 17.7,
        velocityDissipation: -1,
        dyeDissipation: 12,
        viscosity: 20,
        force: -4,
        exposure: 0,
      }),
    ).toEqual({
      fixedTimeStep: 1 / 20,
      maximumSubsteps: 8,
      pressureIterations: 18,
      velocityDissipation: 0,
      dyeDissipation: 5,
      viscosity: 10,
      force: 0,
      exposure: 0.1,
    })
  })
})
