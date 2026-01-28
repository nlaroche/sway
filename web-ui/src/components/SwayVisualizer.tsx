import { useRef, useEffect } from 'react';
import { useVisualizerData } from '../hooks/useVisualizerData';

interface SwayVisualizerProps {
  mode: number;
  rate: number;
  depth: number;
  voices: number;
  stereoPhase: number;
}

// Mode colors
const modeColors = {
  0: { primary: '#00d4ff', secondary: '#0088aa', glow: 'rgba(0, 212, 255, 0.3)' },   // Chorus - cyan
  1: { primary: '#ff6b00', secondary: '#aa4400', glow: 'rgba(255, 107, 0, 0.3)' },   // Flanger - orange
  2: { primary: '#aa00ff', secondary: '#6600aa', glow: 'rgba(170, 0, 255, 0.3)' },   // Phaser - purple
  3: { primary: '#00ff88', secondary: '#00aa55', glow: 'rgba(0, 255, 136, 0.3)' },   // Ensemble - green
};

export function SwayVisualizer({ mode, rate, depth, voices, stereoPhase }: SwayVisualizerProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const data = useVisualizerData();

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const width = canvas.width;
    const height = canvas.height;
    const centerX = width / 2;
    const centerY = height / 2;

    // Clear with gradient background
    const bgGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, Math.max(width, height) / 2);
    bgGradient.addColorStop(0, '#1a1a2e');
    bgGradient.addColorStop(1, '#0d0d1a');
    ctx.fillStyle = bgGradient;
    ctx.fillRect(0, 0, width, height);

    const colors = modeColors[mode as keyof typeof modeColors] || modeColors[0];
    const numVoices = Math.round(voices);
    const maxRadius = Math.min(width, height) * 0.4;

    // Draw based on mode
    if (mode === 0 || mode === 3) {
      // Chorus / Ensemble - multiple orbiting circles
      drawChorusVisualization(ctx, centerX, centerY, maxRadius, colors, data, numVoices, depth, stereoPhase);
    } else if (mode === 1) {
      // Flanger - comb filter visualization
      drawFlangerVisualization(ctx, centerX, centerY, maxRadius, colors, data, depth);
    } else if (mode === 2) {
      // Phaser - phase notches visualization
      drawPhaserVisualization(ctx, centerX, centerY, maxRadius, colors, data, depth);
    }

    // Draw LFO indicator
    drawLfoIndicator(ctx, width - 50, height - 50, 30, colors, data.lfoValue, rate);

    // Draw stereo phase indicator
    if (stereoPhase > 0) {
      drawStereoIndicator(ctx, 50, height - 50, 30, colors, data.stereoPhaseL, data.stereoPhaseR);
    }

  }, [data, mode, rate, depth, voices, stereoPhase]);

  return (
    <div className="visualizer-container">
      <canvas
        ref={canvasRef}
        width={400}
        height={200}
        className="visualizer-canvas"
      />
    </div>
  );
}

function drawChorusVisualization(
  ctx: CanvasRenderingContext2D,
  centerX: number,
  centerY: number,
  maxRadius: number,
  colors: { primary: string; secondary: string; glow: string },
  data: ReturnType<typeof useVisualizerData>,
  numVoices: number,
  depth: number,
  stereoPhase: number
) {
  // Draw center circle
  ctx.beginPath();
  ctx.arc(centerX, centerY, 8, 0, Math.PI * 2);
  ctx.fillStyle = colors.secondary;
  ctx.fill();

  // Draw voice orbits
  for (let i = 0; i < numVoices; i++) {
    const voicePhase = data.voicePhases[i] || 0;
    const orbitRadius = maxRadius * (0.4 + (i / numVoices) * 0.6) * depth;

    // Draw orbit path
    ctx.beginPath();
    ctx.arc(centerX, centerY, orbitRadius, 0, Math.PI * 2);
    ctx.strokeStyle = `${colors.secondary}40`;
    ctx.lineWidth = 1;
    ctx.stroke();

    // Left channel voice
    const angleL = voicePhase * Math.PI * 2;
    const xL = centerX + Math.cos(angleL) * orbitRadius * 0.8;
    const yL = centerY + Math.sin(angleL) * orbitRadius;

    // Right channel voice (offset by stereo phase)
    const angleR = (voicePhase + stereoPhase / 360) * Math.PI * 2;
    const xR = centerX + Math.cos(angleR) * orbitRadius * 1.2;
    const yR = centerY + Math.sin(angleR) * orbitRadius;

    // Draw connecting line
    ctx.beginPath();
    ctx.moveTo(xL, yL);
    ctx.lineTo(xR, yR);
    ctx.strokeStyle = `${colors.primary}60`;
    ctx.lineWidth = 2;
    ctx.stroke();

    // Draw voice circles with glow
    const glowSize = 12 + data.modDepthL * 8;

    // Left voice
    ctx.beginPath();
    ctx.arc(xL, yL, glowSize, 0, Math.PI * 2);
    ctx.fillStyle = colors.glow;
    ctx.fill();

    ctx.beginPath();
    ctx.arc(xL, yL, 6, 0, Math.PI * 2);
    ctx.fillStyle = colors.primary;
    ctx.fill();

    // Right voice
    ctx.beginPath();
    ctx.arc(xR, yR, glowSize, 0, Math.PI * 2);
    ctx.fillStyle = colors.glow;
    ctx.fill();

    ctx.beginPath();
    ctx.arc(xR, yR, 6, 0, Math.PI * 2);
    ctx.fillStyle = colors.secondary;
    ctx.fill();
  }
}

function drawFlangerVisualization(
  ctx: CanvasRenderingContext2D,
  centerX: number,
  centerY: number,
  maxRadius: number,
  colors: { primary: string; secondary: string; glow: string },
  data: ReturnType<typeof useVisualizerData>,
  depth: number
) {
  const numLines = 16;
  const baseSpacing = maxRadius * 2 / numLines;

  for (let i = 0; i < numLines; i++) {
    const x = centerX - maxRadius + i * baseSpacing;
    const offset = Math.sin(data.lfoPhase * Math.PI * 2 + i * 0.3) * depth * 30;
    const height = (maxRadius * 1.5) * (1 - Math.abs(i - numLines / 2) / (numLines / 2) * 0.5);

    // Comb teeth
    ctx.beginPath();
    ctx.moveTo(x + offset, centerY - height / 2);
    ctx.lineTo(x + offset, centerY + height / 2);

    const alpha = 0.3 + Math.abs(Math.sin(data.lfoPhase * Math.PI * 2 + i * 0.3)) * 0.7;
    ctx.strokeStyle = `${colors.primary}${Math.floor(alpha * 255).toString(16).padStart(2, '0')}`;
    ctx.lineWidth = 3;
    ctx.lineCap = 'round';
    ctx.stroke();
  }

  // Sweep indicator
  const sweepX = centerX + data.lfoValue * maxRadius * 0.8;
  ctx.beginPath();
  ctx.moveTo(sweepX, centerY - maxRadius * 0.8);
  ctx.lineTo(sweepX, centerY + maxRadius * 0.8);
  ctx.strokeStyle = colors.primary;
  ctx.lineWidth = 2;
  ctx.stroke();

  // Glow at sweep position
  const gradient = ctx.createRadialGradient(sweepX, centerY, 0, sweepX, centerY, 40);
  gradient.addColorStop(0, colors.glow);
  gradient.addColorStop(1, 'transparent');
  ctx.fillStyle = gradient;
  ctx.fillRect(sweepX - 40, centerY - 40, 80, 80);
}

function drawPhaserVisualization(
  ctx: CanvasRenderingContext2D,
  centerX: number,
  centerY: number,
  maxRadius: number,
  colors: { primary: string; secondary: string; glow: string },
  data: ReturnType<typeof useVisualizerData>,
  depth: number
) {
  const numNotches = 6;

  // Draw frequency response curve
  ctx.beginPath();
  for (let x = 0; x <= maxRadius * 2; x++) {
    const freq = x / (maxRadius * 2);
    let amplitude = 1;

    // Create notches based on LFO position
    for (let n = 0; n < numNotches; n++) {
      const notchFreq = (data.lfoPhase + n / numNotches) % 1;
      const distance = Math.abs(freq - notchFreq);
      const notchDepth = depth * 0.8 * Math.exp(-distance * 20);
      amplitude *= (1 - notchDepth);
    }

    const y = centerY - amplitude * maxRadius * 0.6 + maxRadius * 0.3;

    if (x === 0) {
      ctx.moveTo(centerX - maxRadius + x, y);
    } else {
      ctx.lineTo(centerX - maxRadius + x, y);
    }
  }

  ctx.strokeStyle = colors.primary;
  ctx.lineWidth = 2;
  ctx.stroke();

  // Draw notch markers
  for (let n = 0; n < numNotches; n++) {
    const notchFreq = (data.lfoPhase + n / numNotches) % 1;
    const x = centerX - maxRadius + notchFreq * maxRadius * 2;

    ctx.beginPath();
    ctx.arc(x, centerY + maxRadius * 0.3, 4, 0, Math.PI * 2);
    ctx.fillStyle = colors.secondary;
    ctx.fill();

    // Vertical line to show notch
    ctx.beginPath();
    ctx.moveTo(x, centerY - maxRadius * 0.3);
    ctx.lineTo(x, centerY + maxRadius * 0.3);
    ctx.strokeStyle = `${colors.primary}40`;
    ctx.lineWidth = 1;
    ctx.stroke();
  }

  // Phase rotation indicator
  const rotation = data.lfoValue * Math.PI;
  ctx.save();
  ctx.translate(centerX, centerY);
  ctx.rotate(rotation);

  ctx.beginPath();
  ctx.moveTo(-20, 0);
  ctx.lineTo(20, 0);
  ctx.moveTo(15, -5);
  ctx.lineTo(20, 0);
  ctx.lineTo(15, 5);
  ctx.strokeStyle = colors.primary;
  ctx.lineWidth = 2;
  ctx.stroke();

  ctx.restore();
}

function drawLfoIndicator(
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  radius: number,
  colors: { primary: string; secondary: string; glow: string },
  lfoValue: number,
  rate: number
) {
  // Background circle
  ctx.beginPath();
  ctx.arc(x, y, radius, 0, Math.PI * 2);
  ctx.fillStyle = '#00000040';
  ctx.fill();
  ctx.strokeStyle = colors.secondary;
  ctx.lineWidth = 1;
  ctx.stroke();

  // LFO position indicator
  const indicatorY = y - lfoValue * (radius - 4);
  ctx.beginPath();
  ctx.arc(x, indicatorY, 4, 0, Math.PI * 2);
  ctx.fillStyle = colors.primary;
  ctx.fill();

  // Rate label
  ctx.fillStyle = '#ffffff80';
  ctx.font = '9px monospace';
  ctx.textAlign = 'center';
  ctx.fillText(`${rate.toFixed(1)}Hz`, x, y + radius + 12);
}

function drawStereoIndicator(
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  radius: number,
  colors: { primary: string; secondary: string; glow: string },
  phaseL: number,
  phaseR: number
) {
  // Background
  ctx.beginPath();
  ctx.arc(x, y, radius, 0, Math.PI * 2);
  ctx.fillStyle = '#00000040';
  ctx.fill();

  // Left channel arc
  ctx.beginPath();
  ctx.arc(x, y, radius - 4, -Math.PI / 2, -Math.PI / 2 + phaseL * Math.PI * 2, false);
  ctx.strokeStyle = colors.primary;
  ctx.lineWidth = 3;
  ctx.stroke();

  // Right channel arc
  ctx.beginPath();
  ctx.arc(x, y, radius - 8, -Math.PI / 2, -Math.PI / 2 + phaseR * Math.PI * 2, false);
  ctx.strokeStyle = colors.secondary;
  ctx.lineWidth = 3;
  ctx.stroke();

  // Label
  ctx.fillStyle = '#ffffff80';
  ctx.font = '9px monospace';
  ctx.textAlign = 'center';
  ctx.fillText('L/R', x, y + radius + 12);
}
