import { useEffect, useRef, useState, type ChangeEvent } from 'react'
import './App.css'
import {
  FluidSimulation,
  type FluidSplat,
} from './gpu/simulation/fluid-simulation'
import {
  defaultFluidSettings,
  type FluidSettings,
} from './gpu/simulation/settings'
import { WebGpuError } from './gpu/core/types'
import { palettes, type PaletteId } from './interaction/palettes'
import { PointerSplatController } from './interaction/pointer-splats'

type SimulationStatus = 'loading' | 'ready' | 'error'

interface RangeControlProps {
  readonly label: string
  readonly value: number
  readonly minimum: number
  readonly maximum: number
  readonly step: number
  readonly displayValue?: string
  readonly onChange: (value: number) => void
}

function RangeControl({
  label,
  value,
  minimum,
  maximum,
  step,
  displayValue = value.toFixed(2),
  onChange,
}: RangeControlProps) {
  return (
    <label className="range-control">
      <span>
        {label}
        <output>{displayValue}</output>
      </span>
      <input
        type="range"
        min={minimum}
        max={maximum}
        step={step}
        value={value}
        onChange={(event) => onChange(event.currentTarget.valueAsNumber)}
      />
    </label>
  )
}

function initialSplats(): FluidSplat[] {
  return [
    {
      position: [0.35, 0.52],
      velocity: [0.35, -0.08],
      color: [0.1, 0.8, 1],
      radius: 0.085,
    },
    {
      position: [0.65, 0.48],
      velocity: [-0.35, 0.08],
      color: [1, 0.12, 0.72],
      radius: 0.085,
    },
  ]
}

function App() {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const simulationRef = useRef<FluidSimulation>(null)
  const pausedRef = useRef(false)
  const paletteRef = useRef<PaletteId>('aurora')
  const brushRadiusRef = useRef(0.055)
  const [status, setStatus] = useState<SimulationStatus>('loading')
  const [errorMessage, setErrorMessage] = useState('')
  const [paused, setPaused] = useState(false)
  const [palette, setPalette] = useState<PaletteId>('aurora')
  const [brushRadius, setBrushRadius] = useState(0.055)
  const [settings, setSettings] = useState<FluidSettings>({
    ...defaultFluidSettings,
  })

  useEffect(() => {
    pausedRef.current = paused
  }, [paused])

  useEffect(() => {
    paletteRef.current = palette
  }, [palette])

  useEffect(() => {
    brushRadiusRef.current = brushRadius
  }, [brushRadius])

  useEffect(() => {
    simulationRef.current?.setSettings(settings)
  }, [settings])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) {
      return
    }

    let disposed = false
    let failed = false
    let animationFrame = 0
    let lastFrameTime = performance.now()
    let input: PointerSplatController | undefined
    let resizeObserver: ResizeObserver | undefined

    const fail = (error: unknown) => {
      if (disposed || failed) {
        return
      }
      failed = true
      cancelAnimationFrame(animationFrame)
      const message =
        error instanceof WebGpuError
          ? error.message
          : error instanceof Error
            ? error.message
            : 'The fluid simulation could not start.'
      setErrorMessage(message)
      setStatus('error')
    }

    const simulation = new FluidSimulation(canvas, {
      onError: fail,
    })
    simulationRef.current = simulation

    const animate = (time: number) => {
      if (disposed || failed) {
        return
      }
      const elapsedSeconds = Math.min(
        Math.max((time - lastFrameTime) / 1000, 0),
        0.1,
      )
      lastFrameTime = time

      try {
        if (!pausedRef.current && document.visibilityState === 'visible') {
          simulation.advance(elapsedSeconds, input?.drain() ?? [])
        } else {
          input?.drain()
        }
        animationFrame = requestAnimationFrame(animate)
      } catch (error) {
        fail(error)
      }
    }

    const initialize = async () => {
      try {
        await simulation.initialize()
        if (disposed) {
          simulation.destroy()
          return
        }

        input = new PointerSplatController(canvas, {
          brushRadius: () => brushRadiusRef.current,
          palette: () => paletteRef.current,
        })
        resizeObserver = new ResizeObserver(([entry]) => {
          if (!entry || disposed) {
            return
          }
          simulation.resize(
            entry.contentRect.width,
            entry.contentRect.height,
            window.devicePixelRatio,
          )
        })
        resizeObserver.observe(canvas)
        simulation.step(initialSplats())
        simulation.render()
        setStatus('ready')
        animationFrame = requestAnimationFrame(animate)
      } catch (error) {
        fail(error)
        simulation.destroy()
      }
    }

    const handleVisibilityChange = () => {
      lastFrameTime = performance.now()
    }
    document.addEventListener('visibilitychange', handleVisibilityChange)
    void initialize()

    return () => {
      disposed = true
      cancelAnimationFrame(animationFrame)
      document.removeEventListener(
        'visibilitychange',
        handleVisibilityChange,
      )
      resizeObserver?.disconnect()
      input?.destroy()
      simulation.destroy()
      if (simulationRef.current === simulation) {
        simulationRef.current = null
      }
    }
  }, [])

  const updateSetting = <Key extends keyof FluidSettings>(
    key: Key,
    value: FluidSettings[Key],
  ) => {
    setSettings((current) => ({ ...current, [key]: value }))
  }

  const handlePaletteChange = (event: ChangeEvent<HTMLSelectElement>) => {
    setPalette(event.currentTarget.value as PaletteId)
  }

  const reset = () => {
    const simulation = simulationRef.current
    if (!simulation) {
      return
    }
    simulation.reset()
    simulation.step(initialSplats())
    simulation.render()
  }

  return (
    <main className="fluid-app">
      <canvas
        ref={canvasRef}
        className="fluid-canvas"
        aria-label="Interactive colourful fluid simulation"
      />

      <header className="app-header">
        <p className="eyebrow">WebGPU experiment</p>
        <h1>Chromaflow</h1>
        <p>Drag across the canvas to stir colour into the fluid.</p>
      </header>

      {status === 'loading' && (
        <div className="status-card" role="status" aria-live="polite">
          Initialising WebGPU...
        </div>
      )}

      {status === 'error' && (
        <div className="status-card error-card" role="alert">
          <strong>WebGPU unavailable</strong>
          <span>{errorMessage}</span>
          <small>
            Try a current browser with hardware acceleration enabled over HTTPS
            or localhost.
          </small>
        </div>
      )}

      <aside
        className="control-panel"
        aria-label="Fluid controls"
        data-disabled={status !== 'ready'}
      >
        <div className="control-actions">
          <button
            type="button"
            onClick={() => setPaused((current) => !current)}
            disabled={status !== 'ready'}
          >
            {paused ? 'Resume' : 'Pause'}
          </button>
          <button
            type="button"
            onClick={reset}
            disabled={status !== 'ready'}
          >
            Reset
          </button>
        </div>

        <label className="select-control">
          <span>Palette</span>
          <select value={palette} onChange={handlePaletteChange}>
            {palettes.map(({ id, label }) => (
              <option key={id} value={id}>
                {label}
              </option>
            ))}
          </select>
        </label>

        <RangeControl
          label="Brush"
          value={brushRadius}
          minimum={0.015}
          maximum={0.14}
          step={0.005}
          displayValue={`${Math.round(brushRadius * 100)}%`}
          onChange={setBrushRadius}
        />
        <RangeControl
          label="Force"
          value={settings.force}
          minimum={0.1}
          maximum={4}
          step={0.1}
          onChange={(value) => updateSetting('force', value)}
        />
        <RangeControl
          label="Viscosity"
          value={settings.viscosity}
          minimum={0}
          maximum={3}
          step={0.05}
          onChange={(value) => updateSetting('viscosity', value)}
        />
        <RangeControl
          label="Velocity fade"
          value={settings.velocityDissipation}
          minimum={0}
          maximum={2}
          step={0.05}
          onChange={(value) => updateSetting('velocityDissipation', value)}
        />
        <RangeControl
          label="Colour fade"
          value={settings.dyeDissipation}
          minimum={0}
          maximum={2}
          step={0.05}
          onChange={(value) => updateSetting('dyeDissipation', value)}
        />
        <RangeControl
          label="Pressure"
          value={settings.pressureIterations}
          minimum={8}
          maximum={64}
          step={4}
          displayValue={String(settings.pressureIterations)}
          onChange={(value) => updateSetting('pressureIterations', value)}
        />
      </aside>
    </main>
  )
}

export default App
