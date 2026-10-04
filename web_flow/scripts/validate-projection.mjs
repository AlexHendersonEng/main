const width = 32
const height = 32
const cellCount = width * height
const index = (x, y) => y * width + x
const clamp = (value, maximum) => Math.max(0, Math.min(maximum - 1, value))

function createVelocityField() {
  const velocity = new Float64Array(cellCount * 2)
  const centerX = (width - 1) * 0.5
  const centerY = (height - 1) * 0.5

  for (let y = 1; y < height - 1; y += 1) {
    for (let x = 1; x < width - 1; x += 1) {
      const offset = index(x, y) * 2
      const dx = x - centerX
      const dy = y - centerY
      const envelope = Math.exp(-(dx * dx + dy * dy) / 80)
      velocity[offset] = (-dy * 0.08 + dx * 0.035) * envelope
      velocity[offset + 1] = (dx * 0.08 + dy * 0.035) * envelope
    }
  }

  return velocity
}

function computeDivergence(velocity) {
  const divergence = new Float64Array(cellCount)
  for (let y = 0; y < height; y += 1) {
    for (let x = 0; x < width; x += 1) {
      const left = velocity[index(clamp(x - 1, width), y) * 2]
      const right = velocity[index(clamp(x + 1, width), y) * 2]
      const bottom = velocity[index(x, clamp(y - 1, height)) * 2 + 1]
      const top = velocity[index(x, clamp(y + 1, height)) * 2 + 1]
      divergence[index(x, y)] = 0.5 * (right - left + top - bottom)
    }
  }
  return divergence
}

function solvePressure(divergence, iterations) {
  let read = new Float64Array(cellCount)
  let write = new Float64Array(cellCount)

  for (let iteration = 0; iteration < iterations; iteration += 1) {
    for (let y = 0; y < height; y += 1) {
      for (let x = 0; x < width; x += 1) {
        const left = read[index(clamp(x - 1, width), y)]
        const right = read[index(clamp(x + 1, width), y)]
        const bottom = read[index(x, clamp(y - 1, height))]
        const top = read[index(x, clamp(y + 1, height))]
        write[index(x, y)] =
          0.25 * (left + right + bottom + top - divergence[index(x, y)])
      }
    }
    ;[read, write] = [write, read]
  }

  return read
}

function project(velocity, pressure) {
  const projected = velocity.slice()
  for (let y = 1; y < height - 1; y += 1) {
    for (let x = 1; x < width - 1; x += 1) {
      const offset = index(x, y) * 2
      projected[offset] -=
        0.5 * (pressure[index(x + 1, y)] - pressure[index(x - 1, y)])
      projected[offset + 1] -=
        0.5 * (pressure[index(x, y + 1)] - pressure[index(x, y - 1)])
    }
  }
  return projected
}

function rootMeanSquare(values) {
  const meanSquare =
    values.reduce((sum, value) => sum + value * value, 0) / values.length
  return Math.sqrt(meanSquare)
}

const velocity = createVelocityField()
const divergenceBefore = computeDivergence(velocity)
const pressure = solvePressure(divergenceBefore, 80)
const projectedVelocity = project(velocity, pressure)
const divergenceAfter = computeDivergence(projectedVelocity)
const before = rootMeanSquare(divergenceBefore)
const after = rootMeanSquare(divergenceAfter)
const reduction = 1 - after / before

if (!Number.isFinite(reduction) || reduction < 0.6) {
  throw new Error(
    `Projection reduced RMS divergence by only ${(reduction * 100).toFixed(1)}%.`,
  )
}

console.log(
  `Projection reduced RMS divergence by ${(reduction * 100).toFixed(1)}% (${before.toExponential(3)} to ${after.toExponential(3)}).`,
)
