export const testedSlangVersion = '2026.8'

export const shaderEntries = [
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'clearMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/clear.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'splatVelocityMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/splat-velocity.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'splatDyeMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/splat-dye.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'advectVelocityMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/advect-velocity.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'divergenceMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/divergence.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'pressureJacobiMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/pressure-jacobi.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'subtractGradientMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/subtract-gradient.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'advectDyeMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/advect-dye.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'fullScreenVertex',
    stage: 'vertex',
    output: 'src/gpu/shaders/generated/full-screen-vertex.wgsl',
    expectedAttribute: '@vertex',
  },
  {
    source: 'src/gpu/shaders/fluid.slang',
    entry: 'displayFragment',
    stage: 'fragment',
    output: 'src/gpu/shaders/generated/display-fragment.wgsl',
    expectedAttribute: '@fragment',
  },
]
