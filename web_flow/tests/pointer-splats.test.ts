import { describe, expect, it } from 'vitest'
import { interpolateSplats } from '../src/interaction/pointer-splats'

describe('pointer splat interpolation', () => {
  it('interpolates sparse movement through the final position', () => {
    const splats = interpolateSplats({
      previousPosition: [0.1, 0.2],
      position: [0.5, 0.6],
      elapsedSeconds: 0.2,
      radius: 0.05,
      color: [1, 0, 0.5],
    })

    expect(splats.length).toBeGreaterThan(1)
    expect(splats.at(-1)?.position).toEqual([0.5, 0.6])
    expect(splats.every(({ radius }) => radius === 0.05)).toBe(true)
  })

  it('caps interpolation work and pointer velocity', () => {
    const splats = interpolateSplats({
      previousPosition: [0, 0],
      position: [1, 1],
      elapsedSeconds: 0.001,
      radius: 0.001,
      color: [0, 1, 1],
    })

    expect(splats).toHaveLength(16)
    expect(Math.hypot(...splats[0].velocity)).toBeCloseTo(2.5)
  })

  it('emits one stationary splat for a repeated position', () => {
    const splats = interpolateSplats({
      previousPosition: [0.4, 0.4],
      position: [0.4, 0.4],
      elapsedSeconds: 0.016,
      radius: 0.05,
      color: [0.2, 0.3, 0.4],
    })

    expect(splats).toHaveLength(1)
    expect(splats[0].velocity).toEqual([0, 0])
  })
})
