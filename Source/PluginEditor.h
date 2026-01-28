#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class SwayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SwayAudioProcessorEditor(SwayAudioProcessor&);
    ~SwayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void setupWebView();

#if BEATCONNECT_ACTIVATION_ENABLED
    void sendActivationState();
    void handleActivateLicense(const juce::var& data);
    void handleDeactivateLicense(const juce::var& data);
    void handleGetActivationStatus();
#endif

    SwayAudioProcessor& processorRef;

    // Relays
    std::unique_ptr<juce::WebSliderRelay> modeRelay;
    std::unique_ptr<juce::WebSliderRelay> rateRelay;
    std::unique_ptr<juce::WebSliderRelay> depthRelay;
    std::unique_ptr<juce::WebSliderRelay> shapeRelay;
    std::unique_ptr<juce::WebSliderRelay> stereoPhaseRelay;
    std::unique_ptr<juce::WebSliderRelay> feedbackRelay;
    std::unique_ptr<juce::WebSliderRelay> voicesRelay;
    std::unique_ptr<juce::WebSliderRelay> spreadRelay;
    std::unique_ptr<juce::WebSliderRelay> warmthRelay;
    std::unique_ptr<juce::WebSliderRelay> stagesRelay;
    std::unique_ptr<juce::WebSliderRelay> colorRelay;
    std::unique_ptr<juce::WebSliderRelay> mixRelay;
    std::unique_ptr<juce::WebSliderRelay> widthRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassRelay;

    // Attachments
    std::unique_ptr<juce::WebSliderParameterAttachment> modeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> rateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> depthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> shapeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> stereoPhaseAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> feedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> voicesAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> spreadAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> warmthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> stagesAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> colorAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> mixAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> widthAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassAttachment;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    class VisualizerTimer : public juce::Timer
    {
    public:
        VisualizerTimer(SwayAudioProcessorEditor& e) : editor(e) {}
        void timerCallback() override;
    private:
        SwayAudioProcessorEditor& editor;
    };
    VisualizerTimer visualizerTimer { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwayAudioProcessorEditor)
};
