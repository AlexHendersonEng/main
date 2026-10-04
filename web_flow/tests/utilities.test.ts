import { describe, expect, it } from 'vitest'
import { alignTo } from '../src/gpu/core/resources'
import { UniformWriter } from '../src/gpu/core/uniforms'
import { samplePalette } from '../src/interaction/palettes'

describe('GPU support utilities', () => {
  it('aligns resource sizes to the requested boundary', () => {
    expect(alignTo(17, 16)).toBe(32)
    expect(alignTo(32, 16)).toBe(32)
  })

  it('packs little-endian uniform values at explicit byte offsets', () => {
    const writer = new UniformWriter(16)
      .setFloat32(0, 1.5)
      .setUint32(4, 42)
      .setFloat32Array(8, [2.5, 3.5])

    expect(writer.view.getFloat32(0, true)).toBe(1.5)
    expect(writer.view.getUint32(4, true)).toBe(42)
    expect(writer.view.getFloat32(8, true)).toBe(2.5)
    expect(writer.view.getFloat32(12, true)).toBe(3.5)
  })

  it('wraps palette phases and keeps channels normalized', () => {
    expect(samplePalette('aurora', 0)).toEqual(
      samplePalette('aurora', 1),
    )
    expect(
      samplePalette('solar', 0.37).every(
        (channel) => channel >= 0 && channel <= 1,
      ),
    ).toBe(true)
  })
})
