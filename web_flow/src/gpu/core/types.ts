export type WebGpuFailureStage =
  | 'unsupported'
  | 'adapter'
  | 'device'
  | 'context'
  | 'shader'
  | 'device-lost'

export class WebGpuError extends Error {
  readonly stage: WebGpuFailureStage

  constructor(stage: WebGpuFailureStage, message: string, cause?: unknown) {
    super(message, { cause })
    this.name = 'WebGpuError'
    this.stage = stage
  }
}

export interface WebGpuContext {
  readonly adapter: GPUAdapter
  readonly device: GPUDevice
  readonly canvasContext: GPUCanvasContext
  readonly canvasFormat: GPUTextureFormat
}

export interface WebGpuCallbacks {
  onDeviceLost?: (error: WebGpuError) => void
  onUncapturedError?: (error: GPUError) => void
}
