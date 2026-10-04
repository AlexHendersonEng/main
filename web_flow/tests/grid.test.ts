import { describe, expect, it } from 'vitest'
import {
  calculateCanvasSize,
  calculateGridSize,
} from '../src/gpu/simulation/grid'

describe('simulation sizing', () => {
  it('uses the desktop long-edge tier and workgroup-aligned dimensions', () => {
    expect(calculateGridSize(1920, 1080, 8192)).toEqual({
      width: 384,
      height: 216,
    })
  })

  it('uses the mobile tier while preserving portrait aspect ratio', () => {
    expect(calculateGridSize(390, 844, 8192)).toEqual({
      width: 120,
      height: 256,
    })
  })

  it('never exceeds the adapter texture limit', () => {
    const size = calculateGridSize(2000, 1000, 252)
    expect(size.width).toBeLessThanOrEqual(252)
    expect(size.height).toBeLessThanOrEqual(252)
    expect(size.width % 8).toBe(0)
    expect(size.height % 8).toBe(0)
  })

  it('clamps canvas pixel ratio and dimensions', () => {
    expect(calculateCanvasSize(800, 600, 3, 4096)).toEqual({
      width: 1600,
      height: 1200,
    })
    expect(calculateCanvasSize(3000, 3000, 2, 4096)).toEqual({
      width: 4096,
      height: 4096,
    })
  })
})
