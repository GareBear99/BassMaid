#include "PluginProcessor.h"
#include "PluginEditor.h"

BassEnhancerAudioProcessor::BassEnhancerAudioProcessor()
: AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
  apvts(*this, nullptr, "PARAMS", createParams())
{
    for (auto* id : { IDs::xover, IDs::subTune, IDs::subTight, IDs::harmTone, IDs::tightSpd, IDs::ceiling,
                      IDs::inputGain, IDs::outputGain, IDs::mix, IDs::lowLevel,
                      IDs::subAmount, IDs::harmAmount, IDs::drive, IDs::tightAmt, IDs::limiterOn })
        apvts.addParameterListener(id, this);

    shaper.functionToUse = [] (float x)
    {
        const float a = 1.5f;
        return std::tanh(a * x) / std::tanh(a);
    };
}

BassEnhancerAudioProcessor::~BassEnhancerAudioProcessor()
{
    for (auto* id : { IDs::xover, IDs::subTune, IDs::subTight, IDs::harmTone, IDs::tightSpd, IDs::ceiling,
                      IDs::inputGain, IDs::outputGain, IDs::mix, IDs::lowLevel,
                      IDs::subAmount, IDs::harmAmount, IDs::drive, IDs::tightAmt, IDs::limiterOn })
        apvts.removeParameterListener (id, this);
}

const juce::String BassEnhancerAudioProcessor::getName() const { return "BassMaid"; }
bool BassEnhancerAudioProcessor::acceptsMidi() const { return false; }
bool BassEnhancerAudioProcessor::producesMidi() const { return false; }
bool BassEnhancerAudioProcessor::isMidiEffect() const { return false; }
double BassEnhancerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int BassEnhancerAudioProcessor::getNumPrograms() { return 1; }
int BassEnhancerAudioProcessor::getCurrentProgram() { return 0; }
void BassEnhancerAudioProcessor::setCurrentProgram (int) {}
const juce::String BassEnhancerAudioProcessor::getProgramName (int) { return {}; }
void BassEnhancerAudioProcessor::changeProgramName (int, const juce::String&) {}

bool BassEnhancerAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* BassEnhancerAudioProcessor::createEditor() { return new BassEnhancerAudioProcessorEditor (*this); }

bool BassEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getChannelSet(true, 0)  == juce::AudioChannelSet::stereo()
        && layouts.getChannelSet(false, 0) == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorValueTreeState::ParameterLayout BassEnhancerAudioProcessor::createParams()
{
    using P = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<P>(IDs::inputGain,  "Input",  juce::NormalisableRange<float>(-24.f, 24.f, 0.01f), 0.f));
    layout.add(std::make_unique<P>(IDs::outputGain, "Output", juce::NormalisableRange<float>(-24.f, 24.f, 0.01f), 0.f));
    layout.add(std::make_unique<P>(IDs::mix,        "Mix",    juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 50.f));

    layout.add(std::make_unique<P>(IDs::xover,    "Xover Hz", juce::NormalisableRange<float>(50.f, 250.f, 0.01f, 0.35f), 120.f));
    layout.add(std::make_unique<P>(IDs::lowLevel, "Low Level", juce::NormalisableRange<float>(-12.f, 12.f, 0.01f), 0.f));

    layout.add(std::make_unique<P>(IDs::subAmount, "Sub",      juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 0.f));
    layout.add(std::make_unique<P>(IDs::subTune,   "Sub Tune", juce::NormalisableRange<float>(30.f, 90.f, 0.01f, 0.35f), 50.f));
    layout.add(std::make_unique<P>(IDs::subTight,  "Sub Tight",juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 50.f));

    layout.add(std::make_unique<P>(IDs::harmAmount,"Harm",     juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 20.f));
    layout.add(std::make_unique<P>(IDs::harmTone,  "Harm Tone",juce::NormalisableRange<float>(-1.f, 1.f, 0.001f), 0.f));
    layout.add(std::make_unique<P>(IDs::drive,     "Drive",    juce::NormalisableRange<float>(0.f, 24.f, 0.01f), 6.f));

    layout.add(std::make_unique<P>(IDs::tightAmt,  "Tighten",  juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 0.f));
    layout.add(std::make_unique<P>(IDs::tightSpd,  "Tighten Speed", juce::NormalisableRange<float>(0.f, 100.f, 0.01f), 50.f));

    layout.add(std::make_unique<B>(IDs::limiterOn, "Limiter", true));
    layout.add(std::make_unique<P>(IDs::ceiling,   "Ceiling", juce::NormalisableRange<float>(-12.f, 0.f, 0.01f), -0.5f));

    return layout;
}

void BassEnhancerAudioProcessor::parameterChanged (const juce::String&, float)
{
    dspDirty.store(true, std::memory_order_release);
}

void BassEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    maxBlock = std::max(64, samplesPerBlock);

    spec.sampleRate = sr;
    spec.maximumBlockSize = (juce::uint32) maxBlock;
    spec.numChannels = 2;

    for (int ch=0; ch<2; ++ch)
    {
        xoverLow[ch].reset();  xoverLow[ch].prepare(spec);
        xoverHigh[ch].reset(); xoverHigh[ch].prepare(spec);

        subBP[ch].reset();
        harmPreTilt[ch].reset();
        harmLPF[ch].reset();

        subOsc[ch].initialise([](float x){ return std::sin(x); }, 128);
        subOsc[ch].prepare(spec);
        subOsc[ch].reset();
    }

    inGain.prepare(spec);
    outGain.prepare(spec);
    limiter.prepare(spec);
    limiter.reset();

    lowBuf.setSize(2, maxBlock, false, false, true);
    highBuf.setSize(2, maxBlock, false, false, true);
    wetBuf.setSize(2, maxBlock, false, false, true);

    mixSm.reset(sr, 0.02);
    lowLevelSm.reset(sr, 0.02);
    subAmtSm.reset(sr, 0.02);
    harmAmtSm.reset(sr, 0.02);
    driveSm.reset(sr, 0.02);
    tightAmtSm.reset(sr, 0.02);

    dspDirty.store(true, std::memory_order_release);
}

void BassEnhancerAudioProcessor::releaseResources() {}

void BassEnhancerAudioProcessor::updateDSPIfNeeded(double sampleRate)
{
    if (!dspDirty.exchange(false, std::memory_order_acq_rel))
        return;

    const float xoverHz     = apvts.getRawParameterValue(IDs::xover)->load();
    const float lowLevelDb  = apvts.getRawParameterValue(IDs::lowLevel)->load();
    const float subTuneHz   = apvts.getRawParameterValue(IDs::subTune)->load();
    const float harmTone    = apvts.getRawParameterValue(IDs::harmTone)->load();

    for (int ch=0; ch<2; ++ch)
    {
        xoverLow[ch].setCutoffFrequency(xoverHz);
        xoverHigh[ch].setCutoffFrequency(xoverHz);
        subOsc[ch].setFrequency(subTuneHz);
    }

    const float q = 1.0f;
    auto bp = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, subTuneHz, q);
    for (int ch=0; ch<2; ++ch)
        *subBP[ch].coefficients = *bp;

    const float tone = juce::jlimit(-1.0f, 1.0f, harmTone);
    const float shelfDb = 6.0f * tone;
    const float shelfFreq = 200.0f;

    juce::dsp::IIR::Coefficients<float>::Ptr shelf;
    if (tone >= 0.0f)
        shelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, shelfFreq, 0.707f, dbToLin(shelfDb));
    else
        shelf = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, shelfFreq, 0.707f, dbToLin(-shelfDb));

    for (int ch=0; ch<2; ++ch)
        *harmPreTilt[ch].coefficients = *shelf;

    auto lpf = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 2500.0f, 0.707f);
    for (int ch=0; ch<2; ++ch)
        *harmLPF[ch].coefficients = *lpf;

    lowLevelSm.setTargetValue(dbToLin(lowLevelDb));
}

void BassEnhancerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    updateDSPIfNeeded(sr);

    const float inDb      = apvts.getRawParameterValue(IDs::inputGain)->load();
    const float outDb     = apvts.getRawParameterValue(IDs::outputGain)->load();
    const float mixPct    = apvts.getRawParameterValue(IDs::mix)->load();
    const float subPct    = apvts.getRawParameterValue(IDs::subAmount)->load();
    const float harmPct   = apvts.getRawParameterValue(IDs::harmAmount)->load();
    const float driveDb   = apvts.getRawParameterValue(IDs::drive)->load();
    const float tightPct  = apvts.getRawParameterValue(IDs::tightAmt)->load();
    const float tightSpd  = apvts.getRawParameterValue(IDs::tightSpd)->load();
    const bool  limOn     = apvts.getRawParameterValue(IDs::limiterOn)->load() > 0.5f;
    const float ceilingDb = apvts.getRawParameterValue(IDs::ceiling)->load();

    mixSm.setTargetValue(clamp01(mixPct / 100.0f));
    subAmtSm.setTargetValue(clamp01(subPct / 100.0f));
    harmAmtSm.setTargetValue(clamp01(harmPct / 100.0f));
    driveSm.setTargetValue(dbToLin(driveDb));
    tightAmtSm.setTargetValue(clamp01(tightPct / 100.0f));

    inGain.setGainDecibels(inDb);
    outGain.setGainDecibels(outDb);

    wetBuf.makeCopyOf(buffer, true);

    {
        juce::dsp::AudioBlock<float> blk(wetBuf);
        juce::dsp::ProcessContextReplacing<float> ctx(blk);
        inGain.process(ctx);
    }

    for (int ch=0; ch<2; ++ch)
    {
        auto* in  = wetBuf.getWritePointer(ch);
        auto* low = lowBuf.getWritePointer(ch);
        auto* hi  = highBuf.getWritePointer(ch);

        memcpy(low, in, sizeof(float) * (size_t)numSamples);
        memcpy(hi,  in, sizeof(float) * (size_t)numSamples);

        juce::dsp::AudioBlock<float> lowBlock(&low, 1, (size_t)numSamples);
        juce::dsp::AudioBlock<float> hiBlock(&hi,  1, (size_t)numSamples);

        xoverLow[ch].process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
        xoverHigh[ch].process(juce::dsp::ProcessContextReplacing<float>(hiBlock));
    }

    float grDbAcc = 0.0f;

    const float sp01 = clamp01(tightSpd / 100.0f);
    const float attackMs  = 5.0f  + (1.0f - sp01) * 35.0f;
    const float releaseMs = 40.0f + (1.0f - sp01) * 160.0f;
    const float atkCoeff = expf(-1.0f / (0.001f * attackMs  * (float)sr));
    const float relCoeff = expf(-1.0f / (0.001f * releaseMs * (float)sr));

    const float compThresh = 0.18f;
    const float compRatio  = 4.0f;

    const float ceilingLin = dbToLin(ceilingDb);

    const float subTight01 = clamp01(apvts.getRawParameterValue(IDs::subTight)->load() / 100.0f);
    const float subAtkMs = 6.0f - 4.0f * subTight01;
    const float subRelMs = 120.0f - 80.0f * subTight01;
    const float subAtkC = expf(-1.0f / (0.001f * subAtkMs * (float)sr));
    const float subRelC = expf(-1.0f / (0.001f * subRelMs * (float)sr));

    for (int ch=0; ch<2; ++ch)
    {
        auto* low = lowBuf.getWritePointer(ch);
        auto* hi  = highBuf.getWritePointer(ch);

        float env  = subEnv[ch];
        float comp = compEnv[ch];

        for (int i=0; i<numSamples; ++i)
        {
            const float lowLevel = lowLevelSm.getNextValue();

            const float subAmt = subAmtSm.getNextValue();
            float subSample = 0.0f;
            if (subAmt > 1.0e-6f)
            {
                const float bp = subBP[ch].processSample(low[i]);
                const float e = envFollow(fabsf(bp), env, subAtkC, subRelC);
                const float osc = subOsc[ch].processSample(0.0f);
                subSample = tanhf(1.2f * (osc * e * 2.5f)) * subAmt;
            }

            const float harmAmt = harmAmtSm.getNextValue();
            float harmSample = 0.0f;
            if (harmAmt > 1.0e-6f)
            {
                float x = harmPreTilt[ch].processSample(low[i]);
                x *= driveSm.getNextValue();
                x = shaper.processSample (x);
                x = harmLPF[ch].processSample(x);
                harmSample = x * harmAmt;
            }

            const float tAmt = tightAmtSm.getNextValue();
            float gr = 1.0f;
            if (tAmt > 1.0e-6f)
            {
                const float det = envFollow(fabsf(low[i]), comp, atkCoeff, relCoeff);
                float gain = 1.0f;
                if (det > compThresh)
                {
                    const float over = det / compThresh;
                    const float compressed = powf(over, (1.0f / compRatio));
                    gain = compressed / over;
                }
                gr = 1.0f + tAmt * (gain - 1.0f);
                grDbAcc += linToDb(gr);
            }

            low[i] = (low[i] * lowLevel * gr) + subSample + harmSample;
        }

        subEnv[ch] = env;
        compEnv[ch] = comp;

        auto* wet = wetBuf.getWritePointer(ch);
        for (int i=0; i<numSamples; ++i)
            wet[i] = (low[i] + hi[i]) * ceilingLin;
    }

    const float mix = mixSm.getCurrentValue();
    for (int ch=0; ch<2; ++ch)
    {
        auto* dry = buffer.getReadPointer(ch);
        auto* wet = wetBuf.getWritePointer(ch);
        for (int i=0; i<numSamples; ++i)
            wet[i] = wet[i] * mix + dry[i] * (1.0f - mix);
    }

    {
        juce::dsp::AudioBlock<float> blk(wetBuf);
        juce::dsp::ProcessContextReplacing<float> ctx(blk);
        if (limOn) limiter.process(ctx);
        outGain.process(ctx);
    }

    buffer.makeCopyOf(wetBuf, true);

    double outSq = 0.0;
    for (int ch=0; ch<2; ++ch)
    {
        const float* outPtr = buffer.getReadPointer(ch);
        for (int i=0; i<numSamples; ++i)
            outSq += (double)outPtr[i] * (double)outPtr[i];
    }
    const float outRms = sqrtf((float)(outSq / (double)(numSamples * 2)));
    meters.outRms.store(outRms, std::memory_order_relaxed);
    meters.grDb.store(grDbAcc / std::max(1, numSamples * 2), std::memory_order_relaxed);
}

void BassEnhancerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BassEnhancerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    dspDirty.store(true, std::memory_order_release);
}

// Standard JUCE factory (required by juce_add_plugin).
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassEnhancerAudioProcessor();
}
