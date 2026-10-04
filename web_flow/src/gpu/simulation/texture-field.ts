import { PingPong } from '../core/ping-pong'

export interface TextureResource {
  readonly texture: GPUTexture
  readonly view: GPUTextureView
}

export function createTextureResource(
  device: GPUDevice,
  label: string,
  width: number,
  height: number,
): TextureResource {
  const texture = device.createTexture({
    label,
    size: { width, height },
    format: 'rgba16float',
    usage:
      GPUTextureUsage.TEXTURE_BINDING |
      GPUTextureUsage.STORAGE_BINDING |
      GPUTextureUsage.COPY_SRC,
  })
  return {
    texture,
    view: texture.createView({ label: `${label} view` }),
  }
}

export class TextureField {
  readonly resources: PingPong<TextureResource>

  constructor(
    device: GPUDevice,
    label: string,
    width: number,
    height: number,
  ) {
    this.resources = new PingPong(
      createTextureResource(device, `${label} A`, width, height),
      createTextureResource(device, `${label} B`, width, height),
    )
  }

  get read(): TextureResource {
    return this.resources.read
  }

  get write(): TextureResource {
    return this.resources.write
  }

  swap(): void {
    this.resources.swap()
  }

  destroy(): void {
    this.read.texture.destroy()
    this.write.texture.destroy()
  }
}
