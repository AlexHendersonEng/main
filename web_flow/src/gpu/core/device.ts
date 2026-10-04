import {
  WebGpuError,
  type WebGpuCallbacks,
  type WebGpuContext,
} from './types'

export async function createWebGpuContext(
  canvas: HTMLCanvasElement,
  callbacks: WebGpuCallbacks = {},
): Promise<WebGpuContext> {
  if (!navigator.gpu) {
    throw new WebGpuError(
      'unsupported',
      'WebGPU is unavailable. Use a compatible browser and a secure context.',
    )
  }

  const adapter = await navigator.gpu.requestAdapter({
    powerPreference: 'high-performance',
  })
  if (!adapter) {
    throw new WebGpuError(
      'adapter',
      'WebGPU is available, but no suitable GPU adapter was found.',
    )
  }

  let device: GPUDevice
  try {
    device = await adapter.requestDevice()
  } catch (error) {
    throw new WebGpuError(
      'device',
      'The WebGPU device could not be created.',
      error,
    )
  }

  const canvasContext = canvas.getContext('webgpu')
  if (!canvasContext) {
    device.destroy()
    throw new WebGpuError(
      'context',
      'The canvas could not create a WebGPU rendering context.',
    )
  }

  device.addEventListener('uncapturederror', (event) => {
    callbacks.onUncapturedError?.(event.error)
  })
  void device.lost.then((info) => {
    callbacks.onDeviceLost?.(
      new WebGpuError(
        'device-lost',
        `The WebGPU device was lost: ${info.message || info.reason}.`,
      ),
    )
  })

  const canvasFormat = navigator.gpu.getPreferredCanvasFormat()
  canvasContext.configure({
    device,
    format: canvasFormat,
    alphaMode: 'opaque',
  })

  return {
    adapter,
    device,
    canvasContext,
    canvasFormat,
  }
}
