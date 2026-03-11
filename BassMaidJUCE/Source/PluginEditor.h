#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class BassEnhancerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit BassEnhancerAudioProcessorEditor (BassEnhancerAudioProcessor&);
    ~BassEnhancerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BassEnhancerAudioProcessor& audioProcessor;

    juce::Slider mix, sub, harm, tighten;
    juce::Slider xover, drive, ceiling, inGain, outGain, lowLevel, tone, subTune, subTight, tightSpeed;
    juce::ToggleButton limiterOn;

    juce::Label outMeterLbl, grLbl;

    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<Attach> aMix, aSub, aHarm, aTight, aXover, aDrive, aCeil, aIn, aOut, aLowLevel, aTone, aSubTune, aSubTight, aTightSpeed;
    std::unique_ptr<BAttach> aLim;

    void setupKnob(juce::Slider& s, const juce::String& name);
    void setupSmall(juce::Slider& s, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassEnhancerAudioProcessorEditor)
};
