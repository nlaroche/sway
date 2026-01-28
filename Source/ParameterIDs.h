#pragma once

namespace ParameterIDs
{
    // === MODULATION TYPE ===
    inline constexpr const char* mode         = "mode";         // 0=Chorus, 1=Flanger, 2=Phaser, 3=Ensemble

    // === LFO SECTION ===
    inline constexpr const char* rate         = "rate";         // LFO rate (0.01 - 20 Hz)
    inline constexpr const char* depth        = "depth";        // Modulation depth (0-100%)
    inline constexpr const char* shape        = "shape";        // LFO shape (0=Sine, 1=Triangle, 2=Square, 3=Random)
    inline constexpr const char* stereoPhase  = "stereoPhase";  // L/R phase offset (0-180 degrees)

    // === CHARACTER ===
    inline constexpr const char* feedback     = "feedback";     // Feedback amount (0-100%)
    inline constexpr const char* voices       = "voices";       // Number of voices (1-8)
    inline constexpr const char* spread       = "spread";       // Voice spread/detune (0-100%)
    inline constexpr const char* warmth       = "warmth";       // Analog warmth/saturation (0-100%)

    // === FILTER (for phaser) ===
    inline constexpr const char* stages       = "stages";       // Phaser stages (2-12)
    inline constexpr const char* color        = "color";        // Filter color/resonance (0-100%)

    // === OUTPUT ===
    inline constexpr const char* mix          = "mix";          // Dry/wet mix (0-100%)
    inline constexpr const char* width        = "width";        // Stereo width (0-200%)
    inline constexpr const char* bypass       = "bypass";       // Master bypass

    namespace Ranges
    {
        // Rate: 0.01 - 20 Hz (normalized 0-100)
        inline constexpr float rateMin = 0.0f;
        inline constexpr float rateMax = 100.0f;
        inline constexpr float rateDefault = 30.0f;

        // Depth: 0-100%
        inline constexpr float depthMin = 0.0f;
        inline constexpr float depthMax = 100.0f;
        inline constexpr float depthDefault = 50.0f;

        // Shape: 0-3
        inline constexpr int shapeDefault = 0;

        // Stereo phase: 0-180 degrees (normalized 0-100)
        inline constexpr float stereoPhaseMin = 0.0f;
        inline constexpr float stereoPhaseMax = 100.0f;
        inline constexpr float stereoPhaseDefault = 50.0f;

        // Feedback: 0-100%
        inline constexpr float feedbackMin = 0.0f;
        inline constexpr float feedbackMax = 100.0f;
        inline constexpr float feedbackDefault = 30.0f;

        // Voices: 1-8
        inline constexpr float voicesMin = 1.0f;
        inline constexpr float voicesMax = 8.0f;
        inline constexpr float voicesDefault = 3.0f;

        // Spread: 0-100%
        inline constexpr float spreadMin = 0.0f;
        inline constexpr float spreadMax = 100.0f;
        inline constexpr float spreadDefault = 50.0f;

        // Warmth: 0-100%
        inline constexpr float warmthMin = 0.0f;
        inline constexpr float warmthMax = 100.0f;
        inline constexpr float warmthDefault = 20.0f;

        // Stages: 2-12
        inline constexpr float stagesMin = 2.0f;
        inline constexpr float stagesMax = 12.0f;
        inline constexpr float stagesDefault = 6.0f;

        // Color: 0-100%
        inline constexpr float colorMin = 0.0f;
        inline constexpr float colorMax = 100.0f;
        inline constexpr float colorDefault = 50.0f;

        // Mode: 0=Chorus, 1=Flanger, 2=Phaser, 3=Ensemble
        inline constexpr int modeDefault = 0;

        // Mix: 0-100%
        inline constexpr float mixMin = 0.0f;
        inline constexpr float mixMax = 100.0f;
        inline constexpr float mixDefault = 50.0f;

        // Width: 0-200%
        inline constexpr float widthMin = 0.0f;
        inline constexpr float widthMax = 200.0f;
        inline constexpr float widthDefault = 100.0f;
    }
}
