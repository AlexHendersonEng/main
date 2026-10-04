export class UniformWriter {
  readonly buffer: ArrayBuffer
  readonly view: DataView

  constructor(byteLength: number) {
    this.buffer = new ArrayBuffer(byteLength)
    this.view = new DataView(this.buffer)
  }

  setFloat32(byteOffset: number, value: number): this {
    this.view.setFloat32(byteOffset, value, true)
    return this
  }

  setUint32(byteOffset: number, value: number): this {
    this.view.setUint32(byteOffset, value, true)
    return this
  }

  setFloat32Array(byteOffset: number, values: readonly number[]): this {
    values.forEach((value, index) => {
      this.setFloat32(byteOffset + index * Float32Array.BYTES_PER_ELEMENT, value)
    })
    return this
  }
}
