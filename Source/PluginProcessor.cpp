/*
  ==============================================================================
    SWAY - Modulation Suite
    Chorus, Flanger, Phaser, Ensemble with analog character
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

SwayAudioProcessor::SwayAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      rng(std::random_device{}())
{
    loadProjectData();
}

SwayAudioProcessor::~SwayAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SwayAudioProcessor::createParameterLayout()
{
    using namespace ParameterIDs;
    using namespace ParameterIDs::Ranges;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Mode
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { mode, 1 }, "Mode",
        juce::StringArray { "Chorus", "Flanger", "Phaser", "Ensemble" },
        modeDefault
    ));

    // LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { rate, 1 }, "Rate",
        juce::NormalisableRange<float>(rateMin, rateMax, 0.1f),
        rateDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { depth, 1 }, "Depth",
        juce::NormalisableRange<float>(depthMin, depthMax, 0.1f),
        depthDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { shape, 1 }, "Shape",
        juce::StringArray { "Sine", "Triangle", "Square", "Random" },
        shapeDefault
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { stereoPhase, 1 }, "Stereo",
        juce::NormalisableRange<float>(stereoPhaseMin, stereoPhaseMax, 0.1f),
        stereoPhaseDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    // Character
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { feedback, 1 }, "Feedback",
        juce::NormalisableRange<float>(feedbackMin, feedbackMax, 0.1f),
        feedbackDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { voices, 1 }, "Voices",
        juce::NormalisableRange<float>(voicesMin, voicesMax, 1.0f),
        voicesDefault
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { spread, 1 }, "Spread",
        juce::NormalisableRange<float>(spreadMin, spreadMax, 0.1f),
        spreadDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { warmth, 1 }, "Warmth",
        juce::NormalisableRange<float>(warmthMin, warmthMax, 0.1f),
        warmthDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    // Phaser specific
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { stages, 1 }, "Stages",
        juce::NormalisableRange<float>(stagesMin, stagesMax, 1.0f),
        stagesDefault
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { color, 1 }, "Color",
        juce::NormalisableRange<float>(colorMin, colorMax, 0.1f),
        colorDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    // Output
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { mix, 1 }, "Mix",
        juce::NormalisableRange<float>(mixMin, mixMax, 0.1f),
        mixDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { width, 1 }, "Width",
        juce::NormalisableRange<float>(widthMin, widthMax, 1.0f),
        widthDefault, juce::AudioParameterFloatAttributes().withLabel("%")
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { bypass, 1 }, "Bypass", false
    ));

    return { params.begin(), params.end() };
}

void SwayAudioProcessor::loadProjectData()
{
#if HAS_PROJECT_DATA
    int dataSize = 0;
    const char* data = ProjectData::getNamedResource("project_data_json", dataSize);

    if (data == nullptr || dataSize == 0) return;

    auto parsed = juce::JSON::parse(juce::String::fromUTF8(data, dataSize));
    if (parsed.isVoid()) return;

    pluginId = parsed.getProperty("pluginId", "").toString();
    apiBaseUrl = parsed.getProperty("apiBaseUrl", "").toString();
    supabaseKey = parsed.getProperty("supabasePublishableKey", "").toString();
    buildFlags = parsed.getProperty("flags", juce::var());

#if BEATCONNECT_ACTIVATION_ENABLED
    bool enableActivation = static_cast<bool>(buildFlags.getProperty("enableActivationKeys", false));
    if (enableActivation && pluginId.isNotEmpty())
    {
        beatconnect::ActivationConfig config;
        config.apiBaseUrl = apiBaseUrl.toStdString();
        config.pluginId = pluginId.toStdString();
        config.supabaseKey = supabaseKey.toStdString();
        activation = beatconnect::Activation::create(config);
    }
#endif
#endif
}

bool SwayAudioProcessor::hasActivationEnabled() const
{
#if HAS_PROJECT_DATA && BEATCONNECT_ACTIVATION_ENABLED
    return static_cast<bool>(buildFlags.getProperty("enableActivationKeys", false));
#else
    return false;
#endif
}

float SwayAudioProcessor::getSineLFO(float phase)
{
    return std::sin(phase * juce::MathConstants<float>::twoPi);
}

float SwayAudioProcessor::getTriangleLFO(float phase)
{
    return 4.0f * std::abs(phase - 0.5f) - 1.0f;
}

float SwayAudioProcessor::getSquareLFO(float phase)
{
    return phase < 0.5f ? 1.0f : -1.0f;
}

float SwayAudioProcessor::getRandomLFO(float phase, int channel)
{
    // Smoothed random - new target each cycle
    if (phase < lastRandomPhase)
    {
        randomLfoTarget[channel] = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);
    }
    lastRandomPhase = phase;

    // Smooth interpolation towards target
    randomLfoValue[channel] += (randomLfoTarget[channel] - randomLfoValue[channel]) * 0.01f;
    return randomLfoValue[channel];
}

void SwayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Clear delay lines
    for (auto& dl : delayLines)
        dl.fill(0.0f);
    writePos = 0;

    // Reset phaser allpasses
    for (auto& ch : phaserStages)
        for (auto& stage : ch)
            stage.z1 = 0.0f;

    // Reset LFO
    lfoPhase[0] = lfoPhase[1] = 0.0f;
    randomLfoValue[0] = randomLfoValue[1] = 0.0f;
    randomLfoTarget[0] = randomLfoTarget[1] = 0.0f;
    lastRandomPhase = 0.0f;

    feedbackSample[0] = feedbackSample[1] = 0.0f;

    // Smoothing
    rateSmoothed.reset(sampleRate, 0.05);
    depthSmoothed.reset(sampleRate, 0.02);
    feedbackSmoothed.reset(sampleRate, 0.02);
    mixSmoothed.reset(sampleRate, 0.02);

    rateSmoothed.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::rate)->load());
    depthSmoothed.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::depth)->load() / 100.0f);
    feedbackSmoothed.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::feedback)->load() / 100.0f);
    mixSmoothed.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::mix)->load() / 100.0f);
}

void SwayAudioProcessor::releaseResources()
{
}

bool SwayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void SwayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const float sampleRate = static_cast<float>(currentSampleRate);

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, numSamples);

    // Get parameters
    const int modeVal = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::mode)->load());
    const float rateVal = apvts.getRawParameterValue(ParameterIDs::rate)->load();
    const float depthVal = apvts.getRawParameterValue(ParameterIDs::depth)->load() / 100.0f;
    const int shapeVal = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::shape)->load());
    const float stereoPhaseVal = apvts.getRawParameterValue(ParameterIDs::stereoPhase)->load() / 100.0f * 0.5f;
    const float feedbackVal = apvts.getRawParameterValue(ParameterIDs::feedback)->load() / 100.0f;
    const int voicesVal = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::voices)->load());
    const float spreadVal = apvts.getRawParameterValue(ParameterIDs::spread)->load() / 100.0f;
    const float warmthVal = apvts.getRawParameterValue(ParameterIDs::warmth)->load() / 100.0f;
    const int stagesVal = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::stages)->load());
    const float colorVal = apvts.getRawParameterValue(ParameterIDs::color)->load() / 100.0f;
    const float mixVal = apvts.getRawParameterValue(ParameterIDs::mix)->load() / 100.0f;
    const float widthVal = apvts.getRawParameterValue(ParameterIDs::width)->load() / 100.0f;
    const bool bypassVal = apvts.getRawParameterValue(ParameterIDs::bypass)->load() > 0.5f;

    currentMode.store(modeVal);
    bypassed.store(bypassVal);

    // Update smoothed values
    rateSmoothed.setTargetValue(rateVal);
    depthSmoothed.setTargetValue(depthVal);
    feedbackSmoothed.setTargetValue(feedbackVal);
    mixSmoothed.setTargetValue(mixVal);

    // Visualizer data
    float inputRms = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        inputRms += buffer.getRMSLevel(ch, 0, numSamples);
    currentRMS.store(inputRms / static_cast<float>(numChannels));

    if (bypassVal) return;

    // Mode-specific delay ranges
    float minDelay, maxDelay;
    switch (modeVal) {
        case 0:  // Chorus: 7-30ms
            minDelay = 7.0f;
            maxDelay = 30.0f;
            break;
        case 1:  // Flanger: 0.1-10ms
            minDelay = 0.1f;
            maxDelay = 10.0f;
            break;
        case 3:  // Ensemble: 5-25ms (multiple detuned voices)
            minDelay = 5.0f;
            maxDelay = 25.0f;
            break;
        default:
            minDelay = 1.0f;
            maxDelay = 10.0f;
    }

    const float* inputL = buffer.getReadPointer(0);
    const float* inputR = numChannels > 1 ? buffer.getReadPointer(1) : inputL;
    float* outputL = buffer.getWritePointer(0);
    float* outputR = numChannels > 1 ? buffer.getWritePointer(1) : outputL;

    for (int i = 0; i < numSamples; ++i)
    {
        const float curRate = rateSmoothed.getNextValue();
        const float curDepth = depthSmoothed.getNextValue();
        const float curFeedback = feedbackSmoothed.getNextValue();
        const float curMix = mixSmoothed.getNextValue();

        // LFO rate: 0.01 to 20 Hz (exponential mapping)
        const float lfoFreq = 0.01f * std::pow(2000.0f, curRate / 100.0f);
        const float lfoInc = lfoFreq / sampleRate;

        // Get LFO values for both channels
        float lfoL, lfoR;
        switch (shapeVal) {
            case 0: lfoL = getSineLFO(lfoPhase[0]); lfoR = getSineLFO(lfoPhase[1]); break;
            case 1: lfoL = getTriangleLFO(lfoPhase[0]); lfoR = getTriangleLFO(lfoPhase[1]); break;
            case 2: lfoL = getSquareLFO(lfoPhase[0]); lfoR = getSquareLFO(lfoPhase[1]); break;
            case 3: lfoL = getRandomLFO(lfoPhase[0], 0); lfoR = getRandomLFO(lfoPhase[1], 1); break;
            default: lfoL = lfoR = getSineLFO(lfoPhase[0]);
        }

        // Update LFO phases with stereo offset
        lfoPhase[0] += lfoInc;
        if (lfoPhase[0] >= 1.0f) lfoPhase[0] -= 1.0f;
        lfoPhase[1] = lfoPhase[0] + stereoPhaseVal;
        if (lfoPhase[1] >= 1.0f) lfoPhase[1] -= 1.0f;

        float wetL = 0.0f, wetR = 0.0f;

        if (modeVal == 2)  // Phaser
        {
            // Phaser: allpass cascade with modulated coefficients
            const float minFreq = 200.0f;
            const float maxFreq = 4000.0f + colorVal * 4000.0f;

            float inL = inputL[i] + feedbackSample[0] * curFeedback * 0.7f;
            float inR = inputR[i] + feedbackSample[1] * curFeedback * 0.7f;

            for (int s = 0; s < stagesVal; ++s)
            {
                // Each stage modulated with phase offset
                const float stagePhase = static_cast<float>(s) / static_cast<float>(stagesVal);
                float modL = lfoL * std::sin(stagePhase * juce::MathConstants<float>::pi);
                float modR = lfoR * std::sin(stagePhase * juce::MathConstants<float>::pi);

                const float freqL = minFreq + (maxFreq - minFreq) * (0.5f + modL * curDepth * 0.5f);
                const float freqR = minFreq + (maxFreq - minFreq) * (0.5f + modR * curDepth * 0.5f);

                // Allpass coefficient from frequency
                const float coeffL = (std::tan(juce::MathConstants<float>::pi * freqL / sampleRate) - 1.0f) /
                                     (std::tan(juce::MathConstants<float>::pi * freqL / sampleRate) + 1.0f);
                const float coeffR = (std::tan(juce::MathConstants<float>::pi * freqR / sampleRate) - 1.0f) /
                                     (std::tan(juce::MathConstants<float>::pi * freqR / sampleRate) + 1.0f);

                inL = phaserStages[0][s].process(inL, coeffL);
                inR = phaserStages[1][s].process(inR, coeffR);
            }

            wetL = inL;
            wetR = inR;
            feedbackSample[0] = wetL;
            feedbackSample[1] = wetR;
        }
        else  // Chorus, Flanger, Ensemble
        {
            // Write to delay lines
            for (int v = 0; v < voicesVal; ++v)
            {
                delayLines[v * 2][writePos] = inputL[i] + feedbackSample[0] * curFeedback;
                delayLines[v * 2 + 1][writePos] = inputR[i] + feedbackSample[1] * curFeedback;
            }

            // Read from delay lines with modulation
            for (int v = 0; v < voicesVal; ++v)
            {
                // Voice-specific LFO offset for richer sound
                const float voiceOffset = static_cast<float>(v) / static_cast<float>(voicesVal);
                float voiceLfoL = getSineLFO(std::fmod(lfoPhase[0] + voiceOffset * spreadVal, 1.0f));
                float voiceLfoR = getSineLFO(std::fmod(lfoPhase[1] + voiceOffset * spreadVal, 1.0f));

                // Calculate delay time
                const float delayMsL = minDelay + (maxDelay - minDelay) * (0.5f + voiceLfoL * curDepth * 0.5f);
                const float delayMsR = minDelay + (maxDelay - minDelay) * (0.5f + voiceLfoR * curDepth * 0.5f);

                const float delaySamplesL = delayMsL * sampleRate / 1000.0f;
                const float delaySamplesR = delayMsR * sampleRate / 1000.0f;

                // Interpolated read
                float readPosL = static_cast<float>(writePos) - delaySamplesL;
                float readPosR = static_cast<float>(writePos) - delaySamplesR;
                while (readPosL < 0) readPosL += kMaxDelaySize;
                while (readPosR < 0) readPosR += kMaxDelaySize;

                const int idxL = static_cast<int>(readPosL) % kMaxDelaySize;
                const int idxL1 = (idxL + 1) % kMaxDelaySize;
                const float fracL = readPosL - std::floor(readPosL);

                const int idxR = static_cast<int>(readPosR) % kMaxDelaySize;
                const int idxR1 = (idxR + 1) % kMaxDelaySize;
                const float fracR = readPosR - std::floor(readPosR);

                wetL += (delayLines[v * 2][idxL] * (1.0f - fracL) + delayLines[v * 2][idxL1] * fracL);
                wetR += (delayLines[v * 2 + 1][idxR] * (1.0f - fracR) + delayLines[v * 2 + 1][idxR1] * fracR);
            }

            // Normalize by voice count
            wetL /= static_cast<float>(voicesVal);
            wetR /= static_cast<float>(voicesVal);

            feedbackSample[0] = wetL;
            feedbackSample[1] = wetR;
        }

        // Apply warmth (soft saturation)
        if (warmthVal > 0.01f)
        {
            const float drive = 1.0f + warmthVal * 3.0f;
            wetL = std::tanh(wetL * drive) / drive;
            wetR = std::tanh(wetR * drive) / drive;
        }

        // Stereo width
        if (numChannels == 2 && std::abs(widthVal - 1.0f) > 0.01f)
        {
            const float mid = (wetL + wetR) * 0.5f;
            const float side = (wetL - wetR) * 0.5f * widthVal;
            wetL = mid + side;
            wetR = mid - side;
        }

        // Mix
        outputL[i] = inputL[i] * (1.0f - curMix) + wetL * curMix;
        outputR[i] = inputR[i] * (1.0f - curMix) + wetR * curMix;

        writePos = (writePos + 1) % kMaxDelaySize;
    }

    lfoPhaseVis.store(lfoPhase[0]);
    modulationAmount.store(depthVal);
}

juce::AudioProcessorEditor* SwayAudioProcessor::createEditor()
{
    return new SwayAudioProcessorEditor(*this);
}

void SwayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("stateVersion", kStateVersion, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SwayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SwayAudioProcessor();
}
