import { WebGpuError } from './types'

export async function createCheckedShaderModule(
  device: GPUDevice,
  label: string,
  code: string,
): Promise<GPUShaderModule> {
  const module = device.createShaderModule({ label, code })
  const compilationInfo = await module.getCompilationInfo()
  const errors = compilationInfo.messages.filter(
    (message) => message.type === 'error',
  )

  if (errors.length > 0) {
    const diagnostics = errors
      .map(
        (message) =>
          `${message.lineNum}:${message.linePos} ${message.message}`,
      )
      .join('\n')
    throw new WebGpuError(
      'shader',
      `WebGPU rejected shader "${label}":\n${diagnostics}`,
    )
  }

  return module
}
