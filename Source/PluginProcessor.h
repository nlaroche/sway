#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <random>
#include <array>

#if HAS_PROJECT_DATA
#include "ProjectData.h"
#endif

#if BEATCONNECT_ACTIVATION_ENABLED
#include <beatconnect/Activation.h>
#endif

class SwayAudioProcessor : public juce::AudioProcessor
{
public:
    SwayAudioProcessor();
    ~SwayAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Visualizer data
    float getCurrentRMS() const { return currentRMS.load(); }
    float getLfoPhase() const { return lfoPhaseVis.load(); }
    float getModulationAmount() const { return modulationAmount.load(); }
    int getCurrentMode() const { return currentMode.load(); }
    bool isBypassed() const { return bypassed.load(); }

    // BeatConnect integration
    bool hasActivationEnabled() const;
    juce::String getPluginId() const { return pluginId; }
    juce::String getApiBaseUrl() const { return apiBaseUrl; }
    juce::String getSupabaseKey() const { return supabaseKey; }

#if BEATCONNECT_ACTIVATION_ENABLED
    beatconnect::Activation* getActivation() { return activation.get(); }
#endif

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void loadProjectData();

    // LFO shape generators
    float getSineLFO(float phase);
    float getTriangleLFO(float phase);
    float getSquareLFO(float phase);
    float getRandomLFO(float phase, int channel);

    juce::AudioProcessorValueTreeState apvts;

    // Delay lines for chorus/flanger (per voice, stereo)
    static constexpr int kMaxVoices = 8;
    static constexpr int kMaxDelaySize = 4096;  // ~90ms at 44.1kHz
    std::array<std::array<float, kMaxDelaySize>, kMaxVoices * 2> delayLines {};
    int writePos = 0;

    // Allpass filters for phaser (12 stages max, stereo)
    struct AllpassStage {
        float z1 = 0.0f;
        float process(float input, float coeff) {
            float output = -input * coeff + z1;
            z1 = output * coeff + input;
            return output;
        }
    };
    std::array<std::array<AllpassStage, 12>, 2> phaserStages {};

    // LFO state
    float lfoPhase[2] = { 0.0f, 0.0f };
    std::mt19937 rng;
    float randomLfoValue[2] = { 0.0f, 0.0f };
    float randomLfoTarget[2] = { 0.0f, 0.0f };
    float lastRandomPhase = 0.0f;

    // Feedback state
    float feedbackSample[2] = { 0.0f, 0.0f };

    // Parameter smoothing
    juce::SmoothedValue<float> rateSmoothed;
    juce::SmoothedValue<float> depthSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> mixSmoothed;

    double currentSampleRate = 44100.0;

    // Visualizer data
    std::atomic<float> currentRMS { 0.0f };
    std::atomic<float> lfoPhaseVis { 0.0f };
    std::atomic<float> modulationAmount { 0.0f };
    std::atomic<int> currentMode { 0 };
    std::atomic<bool> bypassed { false };

    // BeatConnect data
    juce::String pluginId;
    juce::String apiBaseUrl;
    juce::String supabaseKey;
    juce::var buildFlags;

#if BEATCONNECT_ACTIVATION_ENABLED
    std::unique_ptr<beatconnect::Activation> activation;
#endif

    static constexpr int kStateVersion = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwayAudioProcessor)
};
