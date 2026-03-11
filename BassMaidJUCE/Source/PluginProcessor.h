#pragma once
#include <JuceHeader.h>

class BassEnhancerAudioProcessor final : public juce::AudioProcessor,
                                         private juce::AudioProcessorValueTreeState::Listener
{
public:
    BassEnhancerAudioProcessor();
    ~BassEnhancerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    struct IDs
    {
        static constexpr const char* inputGain  = "inputGain";
        static constexpr const char* outputGain = "outputGain";
        static constexpr const char* mix        = "mix";

        static constexpr const char* xover      = "xover";
        static constexpr const char* lowLevel   = "lowLevel";

        static constexpr const char* subAmount  = "subAmount";
        static constexpr const char* subTune    = "subTune";
        static constexpr const char* subTight   = "subTight";

        static constexpr const char* harmAmount = "harmAmount";
        static constexpr const char* harmTone   = "harmTone";
        static constexpr const char* drive      = "drive";

        static constexpr const char* tightAmt   = "tightAmt";
        static constexpr const char* tightSpd   = "tightSpd";

        static constexpr const char* limiterOn  = "limiterOn";
        static constexpr const char* ceiling    = "ceiling";
    };

    struct MeterState
    {
        std::atomic<float> outRms { 0.0f };
        std::atomic<float> grDb   { 0.0f };
    } meters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    static inline float dbToLin(float db) { return std::pow(10.0f, db / 20.0f); }
    static inline float linToDb(float lin) { return 20.0f * std::log10(std::max(lin, 1.0e-12f)); }
    static inline float clamp01(float x) { return std::min(1.0f, std::max(0.0f, x)); }

    static inline float envFollow(float xAbs, float& env, float attackCoeff, float releaseCoeff)
    {
        const float coeff = (xAbs > env) ? attackCoeff : releaseCoeff;
        env = xAbs + coeff * (env - xAbs);
        return env;
    }

    void updateDSPIfNeeded(double sr);

    double sr = 44100.0;
    int maxBlock = 512;
    juce::dsp::ProcessSpec spec;

    juce::dsp::LinkwitzRileyFilter<float> xoverLow[2];
    juce::dsp::LinkwitzRileyFilter<float> xoverHigh[2];

    juce::AudioBuffer<float> lowBuf, highBuf, wetBuf;

    juce::dsp::IIR::Filter<float> subBP[2];
    juce::dsp::Oscillator<float> subOsc[2];
    float subEnv[2] = {0.0f, 0.0f};

    juce::dsp::IIR::Filter<float> harmPreTilt[2];
    juce::dsp::IIR::Filter<float> harmLPF[2];
    juce::dsp::WaveShaper<float> shaper;

    float compEnv[2] = {0.0f, 0.0f};

    juce::dsp::Gain<float> inGain, outGain;
    juce::dsp::Limiter<float> limiter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, lowLevelSm, subAmtSm, harmAmtSm, driveSm, tightAmtSm;

    std::atomic<bool> dspDirty { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassEnhancerAudioProcessor)
};
