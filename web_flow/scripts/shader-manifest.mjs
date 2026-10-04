export const testedSlangVersion = '2026.8'

export const shaderEntries = [
  {
    source: 'src/gpu/shaders/foundation.slang',
    entry: 'clearMain',
    stage: 'compute',
    output: 'src/gpu/shaders/generated/clear.wgsl',
    expectedAttribute: '@compute',
  },
  {
    source: 'src/gpu/shaders/foundation.slang',
    entry: 'fullScreenVertex',
    stage: 'vertex',
    output: 'src/gpu/shaders/generated/full-screen-vertex.wgsl',
    expectedAttribute: '@vertex',
  },
  {
    source: 'src/gpu/shaders/foundation.slang',
    entry: 'displayFragment',
    stage: 'fragment',
    output: 'src/gpu/shaders/generated/display-fragment.wgsl',
    expectedAttribute: '@fragment',
  },
]
