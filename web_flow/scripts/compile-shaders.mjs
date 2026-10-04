import { spawnSync } from 'node:child_process'
import {
  copyFileSync,
  mkdirSync,
  mkdtempSync,
  readFileSync,
  rmSync,
} from 'node:fs'
import { tmpdir } from 'node:os'
import { basename, dirname, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { shaderEntries, testedSlangVersion } from './shader-manifest.mjs'

const projectRoot = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const compiler = process.env.SLANGC_PATH?.trim() || 'slangc'

function runCompiler(argumentsList) {
  const result = spawnSync(compiler, argumentsList, {
    cwd: projectRoot,
    encoding: 'utf8',
    shell: false,
  })

  if (result.error) {
    const detail =
      result.error.code === 'ENOENT'
        ? `Set SLANGC_PATH or add slangc ${testedSlangVersion} to PATH.`
        : result.error.message
    throw new Error(`Unable to run "${compiler}". ${detail}`)
  }

  if (result.status !== 0) {
    const diagnostics = [result.stdout, result.stderr]
      .map((output) => output.trim())
      .filter(Boolean)
      .join('\n')
    throw new Error(
      `slangc exited with code ${result.status}.${diagnostics ? `\n${diagnostics}` : ''}`,
    )
  }

  return `${result.stdout}${result.stderr}`.trim()
}

function readCompilerVersion() {
  const output = runCompiler(['-version'])
  const match = output.match(/\d{4}\.\d+(?:\.\d+)?/)

  if (!match) {
    throw new Error(`Could not parse the slangc version from: ${output}`)
  }

  return match[0]
}

function compileShaders() {
  const version = readCompilerVersion()
  if (version !== testedSlangVersion) {
    throw new Error(
      `Unsupported slangc version ${version}; install the tested version ${testedSlangVersion}.`,
    )
  }

  const temporaryDirectory = mkdtempSync(
    resolve(tmpdir(), 'web-flow-slang-'),
  )

  try {
    const compiledOutputs = shaderEntries.map((shader) => {
      const temporaryOutput = resolve(
        temporaryDirectory,
        `${shader.entry}-${basename(shader.output)}`,
      )
      runCompiler([
        resolve(projectRoot, shader.source),
        '-target',
        'wgsl',
        '-entry',
        shader.entry,
        '-stage',
        shader.stage,
        '-warnings-as-errors',
        'all',
        '-o',
        temporaryOutput,
      ])

      const source = readFileSync(temporaryOutput, 'utf8')
      if (!source.includes(shader.expectedAttribute)) {
        throw new Error(
          `${shader.entry} did not produce a ${shader.stage} WGSL entry point.`,
        )
      }

      return {
        temporaryOutput,
        finalOutput: resolve(projectRoot, shader.output),
      }
    })

    for (const { temporaryOutput, finalOutput } of compiledOutputs) {
      mkdirSync(dirname(finalOutput), { recursive: true })
      copyFileSync(temporaryOutput, finalOutput)
    }

    console.log(
      `Compiled ${compiledOutputs.length} WGSL shaders with slangc ${version}.`,
    )
  } finally {
    rmSync(temporaryDirectory, { recursive: true, force: true })
  }
}

try {
  compileShaders()
} catch (error) {
  console.error(
    `Shader compilation failed: ${error instanceof Error ? error.message : String(error)}`,
  )
  process.exitCode = 1
}
