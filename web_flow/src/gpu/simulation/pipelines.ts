import clearShader from '../shaders/generated/clear.wgsl?raw'
import splatVelocityShader from '../shaders/generated/splat-velocity.wgsl?raw'
import splatDyeShader from '../shaders/generated/splat-dye.wgsl?raw'
import advectVelocityShader from '../shaders/generated/advect-velocity.wgsl?raw'
import divergenceShader from '../shaders/generated/divergence.wgsl?raw'
import pressureJacobiShader from '../shaders/generated/pressure-jacobi.wgsl?raw'
import subtractGradientShader from '../shaders/generated/subtract-gradient.wgsl?raw'
import advectDyeShader from '../shaders/generated/advect-dye.wgsl?raw'
import fullScreenVertexShader from '../shaders/generated/full-screen-vertex.wgsl?raw'
import displayFragmentShader from '../shaders/generated/display-fragment.wgsl?raw'
import { createCheckedShaderModule } from '../core/shader'

export interface FluidPipelines {
  readonly clear: GPUComputePipeline
  readonly splatVelocity: GPUComputePipeline
  readonly splatDye: GPUComputePipeline
  readonly advectVelocity: GPUComputePipeline
  readonly divergence: GPUComputePipeline
  readonly pressureJacobi: GPUComputePipeline
  readonly subtractGradient: GPUComputePipeline
  readonly advectDye: GPUComputePipeline
  readonly display: GPURenderPipeline
}

async function createComputePipeline(
  device: GPUDevice,
  label: string,
  code: string,
  entryPoint: string,
): Promise<GPUComputePipeline> {
  const module = await createCheckedShaderModule(device, label, code)
  return device.createComputePipelineAsync({
    label,
    layout: 'auto',
    compute: { module, entryPoint },
  })
}

export async function createFluidPipelines(
  device: GPUDevice,
  canvasFormat: GPUTextureFormat,
): Promise<FluidPipelines> {
  const [
    clear,
    splatVelocity,
    splatDye,
    advectVelocity,
    divergence,
    pressureJacobi,
    subtractGradient,
    advectDye,
    vertexModule,
    fragmentModule,
  ] = await Promise.all([
    createComputePipeline(device, 'Clear field', clearShader, 'clearMain'),
    createComputePipeline(
      device,
      'Splat velocity',
      splatVelocityShader,
      'splatVelocityMain',
    ),
    createComputePipeline(
      device,
      'Splat dye',
      splatDyeShader,
      'splatDyeMain',
    ),
    createComputePipeline(
      device,
      'Advect velocity',
      advectVelocityShader,
      'advectVelocityMain',
    ),
    createComputePipeline(
      device,
      'Compute divergence',
      divergenceShader,
      'divergenceMain',
    ),
    createComputePipeline(
      device,
      'Solve pressure',
      pressureJacobiShader,
      'pressureJacobiMain',
    ),
    createComputePipeline(
      device,
      'Subtract pressure gradient',
      subtractGradientShader,
      'subtractGradientMain',
    ),
    createComputePipeline(
      device,
      'Advect dye',
      advectDyeShader,
      'advectDyeMain',
    ),
    createCheckedShaderModule(
      device,
      'Full-screen vertex',
      fullScreenVertexShader,
    ),
    createCheckedShaderModule(
      device,
      'Display fragment',
      displayFragmentShader,
    ),
  ])

  const display = await device.createRenderPipelineAsync({
    label: 'Fluid display',
    layout: 'auto',
    vertex: {
      module: vertexModule,
      entryPoint: 'fullScreenVertex',
    },
    fragment: {
      module: fragmentModule,
      entryPoint: 'displayFragment',
      targets: [{ format: canvasFormat }],
    },
    primitive: {
      topology: 'triangle-list',
    },
  })

  return {
    clear,
    splatVelocity,
    splatDye,
    advectVelocity,
    divergence,
    pressureJacobi,
    subtractGradient,
    advectDye,
    display,
  }
}
