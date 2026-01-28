#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

#if HAS_WEB_UI_DATA
#include "WebUIData.h"
#endif

SwayAudioProcessorEditor::SwayAudioProcessorEditor(SwayAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(850, 550);
    setResizable(false, false);

    // Create relays
    modeRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::mode);
    rateRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::rate);
    depthRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::depth);
    shapeRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::shape);
    stereoPhaseRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::stereoPhase);
    feedbackRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::feedback);
    voicesRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::voices);
    spreadRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::spread);
    warmthRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::warmth);
    stagesRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::stages);
    colorRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::color);
    mixRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::mix);
    widthRelay = std::make_unique<juce::WebSliderRelay>(ParameterIDs::width);
    bypassRelay = std::make_unique<juce::WebToggleButtonRelay>(ParameterIDs::bypass);

    setupWebView();

    // Create attachments
    auto& apvts = processorRef.getAPVTS();
    modeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::mode), *modeRelay, nullptr);
    rateAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::rate), *rateRelay, nullptr);
    depthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::depth), *depthRelay, nullptr);
    shapeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::shape), *shapeRelay, nullptr);
    stereoPhaseAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::stereoPhase), *stereoPhaseRelay, nullptr);
    feedbackAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::feedback), *feedbackRelay, nullptr);
    voicesAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::voices), *voicesRelay, nullptr);
    spreadAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::spread), *spreadRelay, nullptr);
    warmthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::warmth), *warmthRelay, nullptr);
    stagesAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::stages), *stagesRelay, nullptr);
    colorAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::color), *colorRelay, nullptr);
    mixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::mix), *mixRelay, nullptr);
    widthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(*apvts.getParameter(ParameterIDs::width), *widthRelay, nullptr);
    bypassAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(*apvts.getParameter(ParameterIDs::bypass), *bypassRelay, nullptr);

    visualizerTimer.startTimerHz(60);
}

SwayAudioProcessorEditor::~SwayAudioProcessorEditor()
{
    visualizerTimer.stopTimer();
}

void SwayAudioProcessorEditor::setupWebView()
{
    auto options = juce::WebBrowserComponent::Options{}
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(
            juce::WebBrowserComponent::Options::WinWebView2{}
                .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Sway"))
        )
        .withOptionsFrom(*modeRelay)
        .withOptionsFrom(*rateRelay)
        .withOptionsFrom(*depthRelay)
        .withOptionsFrom(*shapeRelay)
        .withOptionsFrom(*stereoPhaseRelay)
        .withOptionsFrom(*feedbackRelay)
        .withOptionsFrom(*voicesRelay)
        .withOptionsFrom(*spreadRelay)
        .withOptionsFrom(*warmthRelay)
        .withOptionsFrom(*stagesRelay)
        .withOptionsFrom(*colorRelay)
        .withOptionsFrom(*mixRelay)
        .withOptionsFrom(*widthRelay)
        .withOptionsFrom(*bypassRelay)
        .withNativeIntegrationEnabled()
#if BEATCONNECT_ACTIVATION_ENABLED
        .withEventListener("activateLicense", [this](const juce::var& data) { handleActivateLicense(data); })
        .withEventListener("deactivateLicense", [this](const juce::var& data) { handleDeactivateLicense(data); })
        .withEventListener("getActivationStatus", [this](const juce::var&) { handleGetActivationStatus(); })
#endif
        .withEventListener("getPluginInfo", [this](const juce::var&) {
            juce::DynamicObject::Ptr info = new juce::DynamicObject();
            info->setProperty("hasActivation", processorRef.hasActivationEnabled());
            webView->emitEventIfBrowserIsVisible("pluginInfo", juce::var(info.get()));
        });

    webView = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webView);

#if SWAY_DEV_MODE
    webView->goToURL("http://localhost:5173");
#elif HAS_WEB_UI_DATA
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
}

void SwayAudioProcessorEditor::VisualizerTimer::timerCallback()
{
    if (editor.webView == nullptr) return;

    juce::DynamicObject::Ptr data = new juce::DynamicObject();
    data->setProperty("rms", editor.processorRef.getCurrentRMS());
    data->setProperty("lfoPhase", editor.processorRef.getLfoPhase());
    data->setProperty("modulation", editor.processorRef.getModulationAmount());
    data->setProperty("mode", editor.processorRef.getCurrentMode());
    data->setProperty("bypassed", editor.processorRef.isBypassed());

    editor.webView->emitEventIfBrowserIsVisible("visualizerData", juce::var(data.get()));
}

#if BEATCONNECT_ACTIVATION_ENABLED
void SwayAudioProcessorEditor::sendActivationState()
{
    auto* activation = processorRef.getActivation();
    if (!activation) return;

    juce::DynamicObject::Ptr data = new juce::DynamicObject();
    data->setProperty("isConfigured", activation->isConfigured());
    data->setProperty("isActivated", activation->isActivated());
    webView->emitEventIfBrowserIsVisible("activationState", juce::var(data.get()));
}

void SwayAudioProcessorEditor::handleActivateLicense(const juce::var& data)
{
    auto* activation = processorRef.getActivation();
    if (!activation) return;

    auto code = data.getProperty("code", "").toString().toStdString();
    activation->activate(code,
        [this](beatconnect::ActivationStatus status, const beatconnect::ActivationInfo&) {
            juce::MessageManager::callAsync([this, status]() {
                juce::DynamicObject::Ptr result = new juce::DynamicObject();
                result->setProperty("status", juce::String(beatconnect::statusToString(status)));
                webView->emitEventIfBrowserIsVisible("activationResult", juce::var(result.get()));
            });
        });
}

void SwayAudioProcessorEditor::handleDeactivateLicense(const juce::var&)
{
    auto* activation = processorRef.getActivation();
    if (!activation) return;

    activation->deactivate([this](beatconnect::ActivationStatus status) {
        juce::MessageManager::callAsync([this, status]() {
            juce::DynamicObject::Ptr result = new juce::DynamicObject();
            result->setProperty("status", juce::String(beatconnect::statusToString(status)));
            webView->emitEventIfBrowserIsVisible("deactivationResult", juce::var(result.get()));
        });
    });
}

void SwayAudioProcessorEditor::handleGetActivationStatus() { sendActivationState(); }
#endif

void SwayAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff0a0a0c)); }
void SwayAudioProcessorEditor::resized() { if (webView) webView->setBounds(getLocalBounds()); }
