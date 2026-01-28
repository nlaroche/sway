import { useState, useEffect, useCallback } from 'react';
import { isInJuceWebView, addEventListener, removeEventListener } from '../lib/juce-bridge';

export interface SwayVisualizerData {
  lfoPhase: number;
  lfoValue: number;
  stereoPhaseL: number;
  stereoPhaseR: number;
  modDepthL: number;
  modDepthR: number;
  voicePhases: number[];
  mode: number;
}

const defaultData: SwayVisualizerData = {
  lfoPhase: 0,
  lfoValue: 0,
  stereoPhaseL: 0,
  stereoPhaseR: 0,
  modDepthL: 0,
  modDepthR: 0,
  voicePhases: [0, 0, 0, 0],
  mode: 0,
};

export function useVisualizerData(): SwayVisualizerData {
  const [data, setData] = useState<SwayVisualizerData>(defaultData);

  const handleVisualizerData = useCallback((eventData: any) => {
    if (eventData && typeof eventData === 'object') {
      setData({
        lfoPhase: eventData.lfoPhase ?? 0,
        lfoValue: eventData.lfoValue ?? 0,
        stereoPhaseL: eventData.stereoPhaseL ?? 0,
        stereoPhaseR: eventData.stereoPhaseR ?? 0,
        modDepthL: eventData.modDepthL ?? 0,
        modDepthR: eventData.modDepthR ?? 0,
        voicePhases: eventData.voicePhases ?? [0, 0, 0, 0],
        mode: eventData.mode ?? 0,
      });
    }
  }, []);

  useEffect(() => {
    if (!isInJuceWebView()) {
      // Demo mode animation
      let animationFrame: number;
      let phase = 0;

      const animate = () => {
        phase += 0.02;
        const lfoValue = Math.sin(phase);

        setData({
          lfoPhase: (phase % (Math.PI * 2)) / (Math.PI * 2),
          lfoValue,
          stereoPhaseL: Math.sin(phase) * 0.5 + 0.5,
          stereoPhaseR: Math.sin(phase + Math.PI * 0.25) * 0.5 + 0.5,
          modDepthL: Math.abs(Math.sin(phase)) * 0.7,
          modDepthR: Math.abs(Math.sin(phase + Math.PI * 0.25)) * 0.7,
          voicePhases: [
            (phase % (Math.PI * 2)) / (Math.PI * 2),
            ((phase + Math.PI * 0.5) % (Math.PI * 2)) / (Math.PI * 2),
            ((phase + Math.PI) % (Math.PI * 2)) / (Math.PI * 2),
            ((phase + Math.PI * 1.5) % (Math.PI * 2)) / (Math.PI * 2),
          ],
          mode: 0,
        });

        animationFrame = requestAnimationFrame(animate);
      };

      animate();
      return () => cancelAnimationFrame(animationFrame);
    }

    addEventListener('visualizerData', handleVisualizerData);
    return () => removeEventListener('visualizerData', handleVisualizerData);
  }, [handleVisualizerData]);

  return data;
}
