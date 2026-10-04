export function alignTo(value: number, alignment: number): number {
  return Math.ceil(value / alignment) * alignment
}

export function createGpuBuffer(
  device: GPUDevice,
  label: string,
  size: number,
  usage: GPUBufferUsageFlags,
): GPUBuffer {
  return device.createBuffer({
    label,
    size: alignTo(size, 4),
    usage,
  })
}

export function writeBuffer(
  device: GPUDevice,
  buffer: GPUBuffer,
  data: AllowSharedBufferSource,
): void {
  device.queue.writeBuffer(buffer, 0, data)
}
