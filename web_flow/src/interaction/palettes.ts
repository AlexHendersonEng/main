export type PaletteId = 'aurora' | 'solar' | 'ultraviolet' | 'tropical'

export interface PaletteDefinition {
  readonly id: PaletteId
  readonly label: string
  readonly colors: readonly (readonly [number, number, number])[]
}

export const palettes: readonly PaletteDefinition[] = [
  {
    id: 'aurora',
    label: 'Aurora',
    colors: [
      [0.15, 1, 0.72],
      [0.1, 0.55, 1],
      [0.72, 0.2, 1],
      [1, 0.18, 0.62],
    ],
  },
  {
    id: 'solar',
    label: 'Solar flare',
    colors: [
      [1, 0.15, 0.04],
      [1, 0.52, 0.02],
      [1, 0.95, 0.18],
      [1, 0.2, 0.5],
    ],
  },
  {
    id: 'ultraviolet',
    label: 'Ultraviolet',
    colors: [
      [0.18, 0.05, 1],
      [0.52, 0.08, 1],
      [1, 0.12, 0.92],
      [0.12, 0.72, 1],
    ],
  },
  {
    id: 'tropical',
    label: 'Tropical',
    colors: [
      [0.02, 0.92, 0.78],
      [0.1, 0.62, 1],
      [1, 0.82, 0.08],
      [1, 0.22, 0.34],
    ],
  },
]

export function samplePalette(
  paletteId: PaletteId,
  phase: number,
): readonly [number, number, number] {
  const palette = palettes.find(({ id }) => id === paletteId) ?? palettes[0]
  const wrappedPhase = ((phase % 1) + 1) % 1
  const scaled = wrappedPhase * palette.colors.length
  const startIndex = Math.floor(scaled) % palette.colors.length
  const endIndex = (startIndex + 1) % palette.colors.length
  const blend = scaled - Math.floor(scaled)
  const start = palette.colors[startIndex]
  const end = palette.colors[endIndex]

  return [
    start[0] + (end[0] - start[0]) * blend,
    start[1] + (end[1] - start[1]) * blend,
    start[2] + (end[2] - start[2]) * blend,
  ]
}
