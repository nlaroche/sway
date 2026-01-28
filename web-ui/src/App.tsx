import { useSliderParam, useToggleParam, useChoiceParam } from './hooks/useJuceParam';
import { SwayVisualizer } from './components/SwayVisualizer';
import './index.css';

// Mode names
const modeNames = ['Chorus', 'Flanger', 'Phaser', 'Ensemble'];
const shapeNames = ['Sine', 'Triangle', 'Square', 'Random'];

function App() {
  // Parameters
  const mode = useChoiceParam('mode', 4, 0);
  const rate = useSliderParam('rate', 1.0);
  const depth = useSliderParam('depth', 50.0);
  const shape = useChoiceParam('shape', 4, 0);
  const stereoPhase = useSliderParam('stereoPhase', 90.0);
  const feedback = useSliderParam('feedback', 0.0);
  const voices = useSliderParam('voices', 2.0);
  const spread = useSliderParam('spread', 50.0);
  const warmth = useSliderParam('warmth', 0.0);
  const stages = useSliderParam('stages', 4.0);
  const color = useSliderParam('color', 50.0);
  const mix = useSliderParam('mix', 50.0);
  const width = useSliderParam('width', 100.0);
  const bypass = useToggleParam('bypass', false);

  // Show different controls based on mode
  const isChorus = mode.value === 0;
  const isFlanger = mode.value === 1;
  const isPhaser = mode.value === 2;
  const isEnsemble = mode.value === 3;

  return (
    <div className={`app ${bypass.value ? 'bypassed' : ''}`}>
      <header className="header">
        <h1 className="title">SWAY</h1>
        <span className="subtitle">Modulation Suite</span>
        <button
          className={`bypass-btn ${bypass.value ? 'active' : ''}`}
          onClick={bypass.toggle}
        >
          {bypass.value ? 'BYPASSED' : 'ACTIVE'}
        </button>
      </header>

      <div className="visualizer-section">
        <SwayVisualizer
          mode={mode.value}
          rate={rate.value}
          depth={depth.value / 100}
          voices={voices.value}
          stereoPhase={stereoPhase.value}
        />
      </div>

      {/* Mode Selector */}
      <div className="mode-section">
        <div className="mode-selector">
          {modeNames.map((name, i) => (
            <button
              key={name}
              className={`mode-btn ${mode.value === i ? 'active' : ''}`}
              onClick={() => mode.setChoice(i)}
            >
              {name}
            </button>
          ))}
        </div>
      </div>

      <div className="controls-grid">
        {/* LFO Section */}
        <div className="section lfo-section">
          <h2 className="section-title">LFO</h2>
          <div className="controls-row">
            <Knob
              label="Rate"
              value={rate.value}
              onChange={rate.setValue}
              onDragStart={rate.dragStart}
              onDragEnd={rate.dragEnd}
              min={0.01}
              max={20}
              unit="Hz"
              decimals={2}
            />
            <Knob
              label="Depth"
              value={depth.value}
              onChange={depth.setValue}
              onDragStart={depth.dragStart}
              onDragEnd={depth.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
            <div className="shape-selector">
              <label>Shape</label>
              <div className="shape-buttons">
                {shapeNames.map((name, i) => (
                  <button
                    key={name}
                    className={`shape-btn ${shape.value === i ? 'active' : ''}`}
                    onClick={() => shape.setChoice(i)}
                    title={name}
                  >
                    <ShapeIcon shape={i} />
                  </button>
                ))}
              </div>
            </div>
          </div>
        </div>

        {/* Mode-specific Section */}
        <div className="section mode-params-section">
          <h2 className="section-title">{modeNames[mode.value]}</h2>
          <div className="controls-row">
            {(isChorus || isEnsemble) && (
              <>
                <Knob
                  label="Voices"
                  value={voices.value}
                  onChange={voices.setValue}
                  onDragStart={voices.dragStart}
                  onDragEnd={voices.dragEnd}
                  min={1}
                  max={4}
                  decimals={0}
                />
                <Knob
                  label="Spread"
                  value={spread.value}
                  onChange={spread.setValue}
                  onDragStart={spread.dragStart}
                  onDragEnd={spread.dragEnd}
                  min={0}
                  max={100}
                  unit="%"
                />
              </>
            )}
            {isFlanger && (
              <>
                <Knob
                  label="Feedback"
                  value={feedback.value}
                  onChange={feedback.setValue}
                  onDragStart={feedback.dragStart}
                  onDragEnd={feedback.dragEnd}
                  min={-99}
                  max={99}
                  unit="%"
                />
                <Knob
                  label="Color"
                  value={color.value}
                  onChange={color.setValue}
                  onDragStart={color.dragStart}
                  onDragEnd={color.dragEnd}
                  min={0}
                  max={100}
                  unit="%"
                />
              </>
            )}
            {isPhaser && (
              <>
                <Knob
                  label="Stages"
                  value={stages.value}
                  onChange={stages.setValue}
                  onDragStart={stages.dragStart}
                  onDragEnd={stages.dragEnd}
                  min={2}
                  max={12}
                  decimals={0}
                />
                <Knob
                  label="Feedback"
                  value={feedback.value}
                  onChange={feedback.setValue}
                  onDragStart={feedback.dragStart}
                  onDragEnd={feedback.dragEnd}
                  min={-99}
                  max={99}
                  unit="%"
                />
              </>
            )}
            <Knob
              label="Warmth"
              value={warmth.value}
              onChange={warmth.setValue}
              onDragStart={warmth.dragStart}
              onDragEnd={warmth.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
          </div>
        </div>

        {/* Output Section */}
        <div className="section output-section">
          <h2 className="section-title">Output</h2>
          <div className="controls-row">
            <Knob
              label="Stereo"
              value={stereoPhase.value}
              onChange={stereoPhase.setValue}
              onDragStart={stereoPhase.dragStart}
              onDragEnd={stereoPhase.dragEnd}
              min={0}
              max={180}
              unit="°"
            />
            <Knob
              label="Width"
              value={width.value}
              onChange={width.setValue}
              onDragStart={width.dragStart}
              onDragEnd={width.dragEnd}
              min={0}
              max={200}
              unit="%"
            />
            <Knob
              label="Mix"
              value={mix.value}
              onChange={mix.setValue}
              onDragStart={mix.dragStart}
              onDragEnd={mix.dragEnd}
              min={0}
              max={100}
              unit="%"
              highlight
            />
          </div>
        </div>
      </div>
    </div>
  );
}

// Knob Component
interface KnobProps {
  label: string;
  value: number;
  onChange: (value: number) => void;
  onDragStart?: () => void;
  onDragEnd?: () => void;
  min: number;
  max: number;
  unit?: string;
  decimals?: number;
  highlight?: boolean;
}

function Knob({ label, value, onChange, onDragStart, onDragEnd, min, max, unit = '', decimals = 1, highlight }: KnobProps) {
  const normalized = (value - min) / (max - min);
  const angle = -135 + normalized * 270;

  const handleMouseDown = (e: React.MouseEvent) => {
    e.preventDefault();
    onDragStart?.();

    const startY = e.clientY;
    const startValue = value;
    const range = max - min;

    const handleMouseMove = (e: MouseEvent) => {
      const delta = (startY - e.clientY) / 150;
      const newValue = Math.max(min, Math.min(max, startValue + delta * range));
      onChange(newValue);
    };

    const handleMouseUp = () => {
      onDragEnd?.();
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
  };

  return (
    <div className={`knob-container ${highlight ? 'highlight' : ''}`}>
      <div className="knob" onMouseDown={handleMouseDown}>
        <svg viewBox="0 0 100 100" className="knob-svg">
          {/* Track background */}
          <circle
            cx="50"
            cy="50"
            r="40"
            fill="none"
            stroke="#333"
            strokeWidth="4"
            strokeDasharray="188.5 251.3"
            strokeDashoffset="-31.4"
            strokeLinecap="round"
          />
          {/* Active track */}
          <circle
            cx="50"
            cy="50"
            r="40"
            fill="none"
            stroke="var(--accent-color)"
            strokeWidth="4"
            strokeDasharray={`${normalized * 188.5} 251.3`}
            strokeDashoffset="-31.4"
            strokeLinecap="round"
            className="knob-track-active"
          />
          {/* Indicator */}
          <g transform={`rotate(${angle} 50 50)`}>
            <line
              x1="50"
              y1="20"
              x2="50"
              y2="30"
              stroke="var(--accent-color)"
              strokeWidth="3"
              strokeLinecap="round"
            />
          </g>
        </svg>
      </div>
      <div className="knob-value">
        {decimals === 0 ? Math.round(value) : value.toFixed(decimals)}{unit}
      </div>
      <div className="knob-label">{label}</div>
    </div>
  );
}

// Shape icons for LFO
function ShapeIcon({ shape }: { shape: number }) {
  switch (shape) {
    case 0: // Sine
      return (
        <svg viewBox="0 0 24 24" className="shape-icon">
          <path d="M2 12 Q6 4, 12 12 Q18 20, 22 12" fill="none" stroke="currentColor" strokeWidth="2" />
        </svg>
      );
    case 1: // Triangle
      return (
        <svg viewBox="0 0 24 24" className="shape-icon">
          <path d="M2 12 L8 4 L16 20 L22 12" fill="none" stroke="currentColor" strokeWidth="2" />
        </svg>
      );
    case 2: // Square
      return (
        <svg viewBox="0 0 24 24" className="shape-icon">
          <path d="M2 16 L2 8 L12 8 L12 16 L22 16 L22 8" fill="none" stroke="currentColor" strokeWidth="2" />
        </svg>
      );
    case 3: // Random
      return (
        <svg viewBox="0 0 24 24" className="shape-icon">
          <path d="M2 14 L6 8 L10 16 L14 6 L18 18 L22 10" fill="none" stroke="currentColor" strokeWidth="2" />
        </svg>
      );
    default:
      return null;
  }
}

export default App;
