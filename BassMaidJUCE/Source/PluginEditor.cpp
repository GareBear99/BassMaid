#include "PluginEditor.h"

BassEnhancerAudioProcessorEditor::BassEnhancerAudioProcessorEditor (BassEnhancerAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (760, 420);

    setupKnob(mix, "Mix");
    setupKnob(sub, "Sub");
    setupKnob(harm, "Harm");
    setupKnob(tighten, "Tighten");

    setupSmall(xover, "Xover");
    setupSmall(lowLevel, "LowLvl");
    setupSmall(drive, "Drive");
    setupSmall(tone, "Tone");
    setupSmall(subTune, "SubHz");
    setupSmall(subTight, "SubT");
    setupSmall(tightSpeed, "Spd");
    setupSmall(ceiling, "Ceil");
    setupSmall(inGain, "In");
    setupSmall(outGain, "Out");

    limiterOn.setButtonText("Limiter");
    addAndMakeVisible(limiterOn);

    outMeterLbl.setText("Out: -inf dB", juce::dontSendNotification);
    grLbl.setText("GR: 0.0 dB", juce::dontSendNotification);
    outMeterLbl.setJustificationType(juce::Justification::centredLeft);
    grLbl.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(outMeterLbl);
    addAndMakeVisible(grLbl);

    auto& apvts = audioProcessor.apvts;

    aMix = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::mix, mix);
    aSub = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::subAmount, sub);
    aHarm= std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::harmAmount, harm);
    aTight=std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::tightAmt, tighten);

    aXover = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::xover, xover);
    aLowLevel = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::lowLevel, lowLevel);
    aDrive = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::drive, drive);
    aTone  = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::harmTone, tone);
    aSubTune = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::subTune, subTune);
    aSubTight= std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::subTight, subTight);
    aTightSpeed=std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::tightSpd, tightSpeed);
    aCeil  = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::ceiling, ceiling);

    aIn  = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::inputGain, inGain);
    aOut = std::make_unique<Attach>(apvts, BassEnhancerAudioProcessor::IDs::outputGain, outGain);

    aLim = std::make_unique<BAttach>(apvts, BassEnhancerAudioProcessor::IDs::limiterOn, limiterOn);

    startTimerHz(15);
}

BassEnhancerAudioProcessorEditor::~BassEnhancerAudioProcessorEditor() {}

void BassEnhancerAudioProcessorEditor::setupKnob(juce::Slider& s, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 18);
    s.setName(name);
    addAndMakeVisible(s);
}

void BassEnhancerAudioProcessorEditor::setupSmall(juce::Slider& s, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    s.setName(name);
    s.setMouseDragSensitivity(160);
    addAndMakeVisible(s);
}

void BassEnhancerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff0b0b10));

    g.setColour(juce::Colour(0xffe8e8ff));
    g.setFont(18.0f);
    g.drawText("BassMaid", 16, 10, 300, 26, juce::Justification::left);

    g.setFont(12.0f);
    g.setColour(juce::Colour(0xffa8a8c8));
    g.drawText("Housekeeping for Your Low End", 16, 34, 520, 18, juce::Justification::left);

    auto label = [&](juce::Slider& s)
    {
        auto r = s.getBounds();
        g.setColour(juce::Colour(0xffc8c8e8));
        g.drawFittedText(s.getName(), r.withY(r.getY()-18).withHeight(18), juce::Justification::centred, 1);
    };

    label(mix); label(sub); label(harm); label(tighten);
    for (auto* s : { &xover,&lowLevel,&drive,&tone,&subTune,&subTight,&tightSpeed,&ceiling,&inGain,&outGain })
        label(*s);
}

void BassEnhancerAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced(14);
    r.removeFromTop(60);

    auto top = r.removeFromTop(210);
    auto bigW = 160;

    mix.setBounds(top.removeFromLeft(bigW).reduced(10));
    sub.setBounds(top.removeFromLeft(bigW).reduced(10));
    harm.setBounds(top.removeFromLeft(bigW).reduced(10));
    tighten.setBounds(top.removeFromLeft(bigW).reduced(10));

    auto row1 = r.removeFromTop(120);
    auto small = 96;

    xover.setBounds(row1.removeFromLeft(small).reduced(8));
    lowLevel.setBounds(row1.removeFromLeft(small).reduced(8));
    drive.setBounds(row1.removeFromLeft(small).reduced(8));
    tone.setBounds(row1.removeFromLeft(small).reduced(8));
    subTune.setBounds(row1.removeFromLeft(small).reduced(8));
    subTight.setBounds(row1.removeFromLeft(small).reduced(8));
    tightSpeed.setBounds(row1.removeFromLeft(small).reduced(8));

    auto row2 = r.removeFromTop(120);
    ceiling.setBounds(row2.removeFromLeft(small).reduced(8));
    inGain.setBounds(row2.removeFromLeft(small).reduced(8));
    outGain.setBounds(row2.removeFromLeft(small).reduced(8));

    limiterOn.setBounds(row2.removeFromLeft(120).reduced(8).withHeight(26));

    auto meters = r.removeFromTop(50);
    outMeterLbl.setBounds(meters.removeFromTop(22));
    grLbl.setBounds(meters.removeFromTop(22));
}

void BassEnhancerAudioProcessorEditor::timerCallback()
{
    const float outRms = audioProcessor.meters.outRms.load(std::memory_order_relaxed);
    const float grDb   = audioProcessor.meters.grDb.load(std::memory_order_relaxed);

    auto toDb = [](float rms){ return 20.0f * std::log10(std::max(rms, 1.0e-12f)); };

    outMeterLbl.setText("Out: " + juce::String(toDb(outRms), 1) + " dB", juce::dontSendNotification);
    grLbl.setText("GR:  " + juce::String(grDb, 2) + " dB", juce::dontSendNotification);
}
