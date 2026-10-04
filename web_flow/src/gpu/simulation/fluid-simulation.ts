import { createWebGpuContext } from '../core/device'
import { createGpuBuffer, writeBuffer } from '../core/resources'
import { UniformWriter } from '../core/uniforms'
import {
  WebGpuError,
  type WebGpuCallbacks,
  type WebGpuContext,
} from '../core/types'
import { calculateCanvasSize, calculateGridSize, type GridSize } from './grid'
import { createFluidPipelines, type FluidPipelines } from './pipelines'
import {
  defaultFluidSettings,
  normalizeFluidSettings,
  type FluidSettings,
} from './settings'
import {
  createTextureResource,
  TextureField,
  type TextureResource,
} from './texture-field'

const UNIFORM_BYTE_LENGTH = 6 * 4 * Float32Array.BYTES_PER_ELEMENT
const WORKGROUP_SIZE = 8

export interface FluidSplat {
  readonly position: readonly [number, number]
  readonly velocity: readonly [number, number]
  readonly color: readonly [number, number, number]
  readonly radius: number
}

export interface FluidSimulationCallbacks extends WebGpuCallbacks {
  onError?: (error: WebGpuError) => void
}

interface SimulationResources {
  readonly velocity: TextureField
  readonly pressure: TextureField
  readonly dye: TextureField
  readonly divergence: TextureResource
}

interface BindGroupTextures {
  readonly source?: GPUTextureView
  readonly auxiliary?: GPUTextureView
  readonly output?: GPUTextureView
}

export class FluidSimulation {
  readonly #canvas: HTMLCanvasElement
  readonly #callbacks: FluidSimulationCallbacks
  readonly #uniformWriter = new UniformWriter(UNIFORM_BYTE_LENGTH)
  readonly #viewIds = new WeakMap<GPUTextureView, number>()
  readonly #bindGroups = new Map<string, GPUBindGroup>()

  #context?: WebGpuContext
  #pipelines?: FluidPipelines
  #uniformBuffer?: GPUBuffer
  #resources?: SimulationResources
  #gridSize?: GridSize
  #settings = defaultFluidSettings
  #accumulator = 0
  #pendingSplats: FluidSplat[] = []
  #nextViewId = 1
  #destroyed = false

  constructor(
    canvas: HTMLCanvasElement,
    callbacks: FluidSimulationCallbacks = {},
  ) {
    this.#canvas = canvas
    this.#callbacks = callbacks
  }

  get initialized(): boolean {
    return Boolean(this.#context && this.#pipelines && this.#resources)
  }

  get settings(): FluidSettings {
    return this.#settings
  }

  async initialize(): Promise<void> {
    if (this.#destroyed) {
      throw new WebGpuError(
        'device',
        'A destroyed fluid simulation cannot be initialized.',
      )
    }
    if (this.#context) {
      return
    }

    const context = await createWebGpuContext(this.#canvas, {
      onDeviceLost: (error) => {
        if (!this.#destroyed) {
          this.#callbacks.onDeviceLost?.(error)
          this.#callbacks.onError?.(error)
        }
      },
      onUncapturedError: (error) => {
        this.#callbacks.onUncapturedError?.(error)
        this.#callbacks.onError?.(
          new WebGpuError('device', `WebGPU validation error: ${error.message}`),
        )
      },
    })

    try {
      const pipelines = await createFluidPipelines(
        context.device,
        context.canvasFormat,
      )
      const uniformBuffer = createGpuBuffer(
        context.device,
        'Simulation uniforms',
        UNIFORM_BYTE_LENGTH,
        GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
      )
      this.#context = context
      this.#pipelines = pipelines
      this.#uniformBuffer = uniformBuffer

      const bounds = this.#canvas.getBoundingClientRect()
      this.resize(
        Math.max(bounds.width, 1),
        Math.max(bounds.height, 1),
        window.devicePixelRatio,
      )
    } catch (error) {
      context.device.destroy()
      throw error instanceof WebGpuError
        ? error
        : new WebGpuError(
            'shader',
            'The fluid GPU pipelines could not be created.',
            error,
          )
    }
  }

  setSettings(settings: Partial<FluidSettings>): void {
    this.#settings = normalizeFluidSettings({
      ...this.#settings,
      ...settings,
    })
  }

  resize(
    cssWidth: number,
    cssHeight: number,
    devicePixelRatio = window.devicePixelRatio,
  ): void {
    const context = this.#requireContext()
    const canvasSize = calculateCanvasSize(
      cssWidth,
      cssHeight,
      devicePixelRatio,
      context.device.limits.maxTextureDimension2D,
    )
    if (
      this.#canvas.width !== canvasSize.width ||
      this.#canvas.height !== canvasSize.height
    ) {
      this.#canvas.width = canvasSize.width
      this.#canvas.height = canvasSize.height
    }

    const gridSize = calculateGridSize(
      cssWidth,
      cssHeight,
      context.device.limits.maxTextureDimension2D,
    )
    if (
      this.#gridSize?.width === gridSize.width &&
      this.#gridSize.height === gridSize.height
    ) {
      return
    }

    this.#destroyResources()
    this.#gridSize = gridSize
    this.#resources = {
      velocity: new TextureField(
        context.device,
        'Velocity',
        gridSize.width,
        gridSize.height,
      ),
      pressure: new TextureField(
        context.device,
        'Pressure',
        gridSize.width,
        gridSize.height,
      ),
      dye: new TextureField(
        context.device,
        'Dye',
        gridSize.width,
        gridSize.height,
      ),
      divergence: createTextureResource(
        context.device,
        'Divergence',
        gridSize.width,
        gridSize.height,
      ),
    }
    this.#bindGroups.clear()
    this.#accumulator = 0
    this.reset()
  }

  advance(elapsedSeconds: number, splats: readonly FluidSplat[] = []): void {
    this.#pendingSplats.push(...splats)
    if (this.#pendingSplats.length > 32) {
      this.#pendingSplats.splice(0, this.#pendingSplats.length - 32)
    }
    this.#accumulator = Math.min(
      this.#accumulator + Math.max(elapsedSeconds, 0),
      this.#settings.fixedTimeStep * this.#settings.maximumSubsteps,
    )

    let substeps = 0
    while (
      this.#accumulator >= this.#settings.fixedTimeStep &&
      substeps < this.#settings.maximumSubsteps
    ) {
      this.step(substeps === 0 ? this.#pendingSplats : [])
      if (substeps === 0) {
        this.#pendingSplats = []
      }
      this.#accumulator -= this.#settings.fixedTimeStep
      substeps += 1
    }

    this.render()
  }

  step(splats: readonly FluidSplat[] = []): void {
    const context = this.#requireContext()
    const pipelines = this.#requirePipelines()
    const resources = this.#requireResources()

    for (const splat of splats.slice(0, 32)) {
      this.#writeUniforms(splat)
      const splatEncoder = context.device.createCommandEncoder({
        label: 'Fluid splat commands',
      })
      this.#encodeCompute(
        splatEncoder,
        'Splat velocity',
        pipelines.splatVelocity,
        {
          source: resources.velocity.read.view,
          output: resources.velocity.write.view,
        },
      )
      resources.velocity.swap()
      this.#encodeCompute(splatEncoder, 'Splat dye', pipelines.splatDye, {
        source: resources.dye.read.view,
        output: resources.dye.write.view,
      })
      resources.dye.swap()
      context.device.queue.submit([splatEncoder.finish()])
    }

    this.#writeUniforms()
    const encoder = context.device.createCommandEncoder({
      label: 'Fluid simulation step',
    })

    this.#encodeCompute(
      encoder,
      'Advect velocity',
      pipelines.advectVelocity,
      {
        source: resources.velocity.read.view,
        output: resources.velocity.write.view,
      },
    )
    resources.velocity.swap()

    this.#encodeCompute(encoder, 'Compute divergence', pipelines.divergence, {
      source: resources.velocity.read.view,
      output: resources.divergence.view,
    })

    this.#encodeCompute(encoder, 'Clear pressure A', pipelines.clear, {
      output: resources.pressure.read.view,
    })
    this.#encodeCompute(encoder, 'Clear pressure B', pipelines.clear, {
      output: resources.pressure.write.view,
    })

    for (
      let iteration = 0;
      iteration < this.#settings.pressureIterations;
      iteration += 1
    ) {
      this.#encodeCompute(
        encoder,
        `Pressure iteration ${iteration + 1}`,
        pipelines.pressureJacobi,
        {
          source: resources.pressure.read.view,
          auxiliary: resources.divergence.view,
          output: resources.pressure.write.view,
        },
      )
      resources.pressure.swap()
    }

    this.#encodeCompute(
      encoder,
      'Subtract pressure gradient',
      pipelines.subtractGradient,
      {
        source: resources.velocity.read.view,
        auxiliary: resources.pressure.read.view,
        output: resources.velocity.write.view,
      },
    )
    resources.velocity.swap()

    this.#encodeCompute(encoder, 'Advect dye', pipelines.advectDye, {
      source: resources.dye.read.view,
      auxiliary: resources.velocity.read.view,
      output: resources.dye.write.view,
    })
    resources.dye.swap()

    context.device.queue.submit([encoder.finish()])
  }

  render(): void {
    const context = this.#requireContext()
    const pipelines = this.#requirePipelines()
    const resources = this.#requireResources()
    this.#writeUniforms()

    const encoder = context.device.createCommandEncoder({
      label: 'Fluid display commands',
    })
    const pass = encoder.beginRenderPass({
      label: 'Fluid display pass',
      colorAttachments: [
        {
          view: context.canvasContext.getCurrentTexture().createView(),
          loadOp: 'clear',
          clearValue: { r: 0.004, g: 0.006, b: 0.015, a: 1 },
          storeOp: 'store',
        },
      ],
    })
    pass.setPipeline(pipelines.display)
    pass.setBindGroup(
      0,
      this.#getBindGroup('display', pipelines.display, {
        source: resources.dye.read.view,
      }),
    )
    pass.draw(3)
    pass.end()
    context.device.queue.submit([encoder.finish()])
  }

  reset(): void {
    if (!this.#resources || !this.#context || !this.#pipelines) {
      return
    }
    this.#writeUniforms()
    const encoder = this.#context.device.createCommandEncoder({
      label: 'Reset fluid fields',
    })
    const outputs = [
      this.#resources.velocity.read.view,
      this.#resources.velocity.write.view,
      this.#resources.pressure.read.view,
      this.#resources.pressure.write.view,
      this.#resources.dye.read.view,
      this.#resources.dye.write.view,
      this.#resources.divergence.view,
    ]
    outputs.forEach((output, index) => {
      this.#encodeCompute(
        encoder,
        `Clear field ${index + 1}`,
        this.#pipelines!.clear,
        { output },
      )
    })
    this.#context.device.queue.submit([encoder.finish()])
    this.#accumulator = 0
    this.#pendingSplats = []
  }

  destroy(): void {
    if (this.#destroyed) {
      return
    }
    this.#destroyed = true
    this.#destroyResources()
    this.#uniformBuffer?.destroy()
    this.#context?.canvasContext.unconfigure()
    this.#context?.device.destroy()
    this.#context = undefined
    this.#pipelines = undefined
    this.#uniformBuffer = undefined
    this.#gridSize = undefined
    this.#pendingSplats = []
  }

  #encodeCompute(
    encoder: GPUCommandEncoder,
    label: string,
    pipeline: GPUComputePipeline,
    textures: BindGroupTextures,
  ): void {
    const gridSize = this.#requireGridSize()
    const pass = encoder.beginComputePass({ label })
    pass.setPipeline(pipeline)
    pass.setBindGroup(0, this.#getBindGroup(label, pipeline, textures))
    pass.dispatchWorkgroups(
      Math.ceil(gridSize.width / WORKGROUP_SIZE),
      Math.ceil(gridSize.height / WORKGROUP_SIZE),
    )
    pass.end()
  }

  #getBindGroup(
    label: string,
    pipeline: GPUPipelineBase,
    textures: BindGroupTextures,
  ): GPUBindGroup {
    const uniformBuffer = this.#uniformBuffer
    if (!uniformBuffer) {
      throw new WebGpuError('device', 'Simulation uniforms are unavailable.')
    }

    const textureEntries: GPUBindGroupEntry[] = []
    if (textures.source) {
      textureEntries.push({
        binding: 1,
        resource: textures.source,
      })
    }
    if (textures.auxiliary) {
      textureEntries.push({
        binding: 2,
        resource: textures.auxiliary,
      })
    }
    if (textures.output) {
      textureEntries.push({
        binding: 3,
        resource: textures.output,
      })
    }
    const key = [
      label.replace(/\d+$/, ''),
      ...textureEntries.map(
        (entry) =>
          `${entry.binding}:${this.#getViewId(entry.resource as GPUTextureView)}`,
      ),
    ].join('|')
    const cached = this.#bindGroups.get(key)
    if (cached) {
      return cached
    }

    const bindGroup = this.#requireContext().device.createBindGroup({
      label: `${label} bindings`,
      layout: pipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: { buffer: uniformBuffer },
        },
        ...textureEntries,
      ],
    })
    this.#bindGroups.set(key, bindGroup)
    return bindGroup
  }

  #getViewId(view: GPUTextureView): number {
    const existing = this.#viewIds.get(view)
    if (existing) {
      return existing
    }
    const id = this.#nextViewId
    this.#nextViewId += 1
    this.#viewIds.set(view, id)
    return id
  }

  #writeUniforms(splat?: FluidSplat): void {
    const context = this.#requireContext()
    const uniformBuffer = this.#uniformBuffer
    const gridSize = this.#requireGridSize()
    if (!uniformBuffer) {
      throw new WebGpuError('device', 'Simulation uniforms are unavailable.')
    }

    const timeStep = this.#settings.fixedTimeStep
    const velocityRetention = Math.exp(
      -this.#settings.velocityDissipation * timeStep,
    )
    const dyeRetention = Math.exp(-this.#settings.dyeDissipation * timeStep)
    this.#uniformWriter
      .setFloat32Array(0, [
        gridSize.width,
        gridSize.height,
        1 / gridSize.width,
        1 / gridSize.height,
      ])
      .setFloat32Array(16, [
        timeStep,
        velocityRetention,
        dyeRetention,
        this.#settings.viscosity,
      ])
      .setFloat32Array(32, [
        splat?.position[0] ?? 0,
        splat?.position[1] ?? 0,
        splat?.radius ?? 0,
        0,
      ])
      .setFloat32Array(48, [
        (splat?.velocity[0] ?? 0) *
          gridSize.width *
          this.#settings.force,
        (splat?.velocity[1] ?? 0) *
          gridSize.height *
          this.#settings.force,
        0,
        0,
      ])
      .setFloat32Array(64, [
        splat?.color[0] ?? 0,
        splat?.color[1] ?? 0,
        splat?.color[2] ?? 0,
        1,
      ])
      .setFloat32Array(80, [this.#settings.exposure, 0, 0, 0])

    writeBuffer(context.device, uniformBuffer, this.#uniformWriter.buffer)
  }

  #destroyResources(): void {
    this.#resources?.velocity.destroy()
    this.#resources?.pressure.destroy()
    this.#resources?.dye.destroy()
    this.#resources?.divergence.texture.destroy()
    this.#resources = undefined
    this.#bindGroups.clear()
  }

  #requireContext(): WebGpuContext {
    if (!this.#context) {
      throw new WebGpuError(
        'device',
        'Initialize the fluid simulation before using it.',
      )
    }
    return this.#context
  }

  #requirePipelines(): FluidPipelines {
    if (!this.#pipelines) {
      throw new WebGpuError('shader', 'Fluid pipelines are unavailable.')
    }
    return this.#pipelines
  }

  #requireResources(): SimulationResources {
    if (!this.#resources) {
      throw new WebGpuError('device', 'Fluid textures are unavailable.')
    }
    return this.#resources
  }

  #requireGridSize(): GridSize {
    if (!this.#gridSize) {
      throw new WebGpuError('device', 'The simulation grid is unavailable.')
    }
    return this.#gridSize
  }
}
