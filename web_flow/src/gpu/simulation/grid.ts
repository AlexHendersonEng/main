export interface GridSize {
  readonly width: number
  readonly height: number
}

const WORKGROUP_SIZE = 8

function alignToWorkgroup(value: number, maximum: number): number {
  return Math.max(
    WORKGROUP_SIZE,
    Math.min(
      Math.floor(maximum / WORKGROUP_SIZE) * WORKGROUP_SIZE,
      Math.round(value / WORKGROUP_SIZE) * WORKGROUP_SIZE,
    ),
  )
}

export function calculateGridSize(
  displayWidth: number,
  displayHeight: number,
  maximumTextureDimension: number,
): GridSize {
  const safeWidth = Math.max(1, displayWidth)
  const safeHeight = Math.max(1, displayHeight)
  const longEdgeTarget = Math.min(
    safeWidth < 768 ? 256 : 384,
    maximumTextureDimension,
  )
  const aspectRatio = safeWidth / safeHeight

  if (aspectRatio >= 1) {
    return {
      width: alignToWorkgroup(longEdgeTarget, maximumTextureDimension),
      height: alignToWorkgroup(
        longEdgeTarget / aspectRatio,
        maximumTextureDimension,
      ),
    }
  }

  return {
    width: alignToWorkgroup(
      longEdgeTarget * aspectRatio,
      maximumTextureDimension,
    ),
    height: alignToWorkgroup(longEdgeTarget, maximumTextureDimension),
  }
}

export function calculateCanvasSize(
  cssWidth: number,
  cssHeight: number,
  devicePixelRatio: number,
  maximumTextureDimension: number,
): GridSize {
  const scale = Math.min(Math.max(devicePixelRatio, 1), 2)
  return {
    width: Math.min(
      maximumTextureDimension,
      Math.max(1, Math.round(cssWidth * scale)),
    ),
    height: Math.min(
      maximumTextureDimension,
      Math.max(1, Math.round(cssHeight * scale)),
    ),
  }
}
