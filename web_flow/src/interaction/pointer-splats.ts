import type { FluidSplat } from '../gpu/simulation/fluid-simulation'
import type { PaletteId } from './palettes'
import { samplePalette } from './palettes'

interface PointerState {
  readonly position: readonly [number, number]
  readonly time: number
}

export interface InterpolationInput {
  readonly previousPosition: readonly [number, number]
  readonly position: readonly [number, number]
  readonly elapsedSeconds: number
  readonly radius: number
  readonly color: readonly [number, number, number]
}

export interface PointerSplatOptions {
  readonly brushRadius: () => number
  readonly palette: () => PaletteId
}

const MAXIMUM_QUEUE_LENGTH = 128
const MAXIMUM_POINTER_SPEED = 2.5
const MAXIMUM_INTERPOLATION_STEPS = 16

function clamp(value: number, minimum: number, maximum: number): number {
  return Math.min(Math.max(value, minimum), maximum)
}

function clampVelocity(
  x: number,
  y: number,
): readonly [number, number] {
  const speed = Math.hypot(x, y)
  if (speed <= MAXIMUM_POINTER_SPEED || speed === 0) {
    return [x, y]
  }
  const scale = MAXIMUM_POINTER_SPEED / speed
  return [x * scale, y * scale]
}

export function interpolateSplats({
  previousPosition,
  position,
  elapsedSeconds,
  radius,
  color,
}: InterpolationInput): FluidSplat[] {
  const safeElapsedSeconds = Math.max(elapsedSeconds, 1 / 240)
  const velocity = clampVelocity(
    (position[0] - previousPosition[0]) / safeElapsedSeconds,
    (position[1] - previousPosition[1]) / safeElapsedSeconds,
  )
  const distance = Math.hypot(
    position[0] - previousPosition[0],
    position[1] - previousPosition[1],
  )
  const steps = clamp(
    Math.ceil(distance / Math.max(radius * 0.35, 0.005)),
    1,
    MAXIMUM_INTERPOLATION_STEPS,
  )

  return Array.from({ length: steps }, (_, index) => {
    const fraction = (index + 1) / steps
    return {
      position: [
        previousPosition[0] +
          (position[0] - previousPosition[0]) * fraction,
        previousPosition[1] +
          (position[1] - previousPosition[1]) * fraction,
      ],
      velocity,
      color,
      radius,
    }
  })
}

export class PointerSplatController {
  readonly #element: HTMLElement
  readonly #options: PointerSplatOptions
  readonly #pointers = new Map<number, PointerState>()
  readonly #queue: FluidSplat[] = []

  constructor(element: HTMLElement, options: PointerSplatOptions) {
    this.#element = element
    this.#options = options
    element.addEventListener('pointerdown', this.#onPointerDown)
    element.addEventListener('pointermove', this.#onPointerMove)
    element.addEventListener('pointerup', this.#onPointerEnd)
    element.addEventListener('pointercancel', this.#onPointerEnd)
    element.addEventListener('lostpointercapture', this.#onPointerEnd)
  }

  drain(maximumCount = 32): FluidSplat[] {
    return this.#queue.splice(0, maximumCount)
  }

  destroy(): void {
    this.#element.removeEventListener('pointerdown', this.#onPointerDown)
    this.#element.removeEventListener('pointermove', this.#onPointerMove)
    this.#element.removeEventListener('pointerup', this.#onPointerEnd)
    this.#element.removeEventListener('pointercancel', this.#onPointerEnd)
    this.#element.removeEventListener(
      'lostpointercapture',
      this.#onPointerEnd,
    )
    this.#pointers.clear()
    this.#queue.length = 0
  }

  readonly #onPointerDown = (event: PointerEvent): void => {
    if (event.pointerType === 'mouse' && event.button !== 0) {
      return
    }
    event.preventDefault()
    this.#element.setPointerCapture(event.pointerId)
    const position = this.#normalizePosition(event)
    this.#pointers.set(event.pointerId, {
      position,
      time: event.timeStamp,
    })
    this.#enqueue({
      position,
      velocity: [0, 0],
      color: this.#colorFor(event.pointerId, event.timeStamp),
      radius: this.#options.brushRadius(),
    })
  }

  readonly #onPointerMove = (event: PointerEvent): void => {
    const pointer = this.#pointers.get(event.pointerId)
    if (!pointer) {
      return
    }
    event.preventDefault()
    const events = event.getCoalescedEvents?.() ?? [event]
    let previous = pointer

    for (const sample of events) {
      const position = this.#normalizePosition(sample)
      const elapsedSeconds = Math.max(
        (sample.timeStamp - previous.time) / 1000,
        1 / 240,
      )
      const radius = this.#options.brushRadius()
      const splats = interpolateSplats({
        previousPosition: previous.position,
        position,
        elapsedSeconds,
        radius,
        color: this.#colorFor(event.pointerId, sample.timeStamp),
      })

      for (const splat of splats) {
        this.#enqueue(splat)
      }

      previous = {
        position,
        time: sample.timeStamp,
      }
    }

    this.#pointers.set(event.pointerId, previous)
  }

  readonly #onPointerEnd = (event: PointerEvent): void => {
    this.#pointers.delete(event.pointerId)
  }

  #normalizePosition(event: PointerEvent): readonly [number, number] {
    const bounds = this.#element.getBoundingClientRect()
    return [
      clamp((event.clientX - bounds.left) / Math.max(bounds.width, 1), 0, 1),
      clamp((event.clientY - bounds.top) / Math.max(bounds.height, 1), 0, 1),
    ]
  }

  #colorFor(
    pointerId: number,
    timeStamp: number,
  ): readonly [number, number, number] {
    return samplePalette(
      this.#options.palette(),
      timeStamp * 0.00008 + pointerId * 0.173,
    )
  }

  #enqueue(splat: FluidSplat): void {
    this.#queue.push(splat)
    if (this.#queue.length > MAXIMUM_QUEUE_LENGTH) {
      this.#queue.splice(0, this.#queue.length - MAXIMUM_QUEUE_LENGTH)
    }
  }
}
