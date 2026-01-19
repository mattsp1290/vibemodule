#include "PluginProcessor.h"
#include "PluginEditor.h"

// Factory presets
static const std::vector<ReverbPreset> kFactoryPresets = {
    {"Default",       0.50f, 0.50f, 0.50f, 0.625f, 0.70f},
    {"Small Room",    0.30f, 0.40f, 0.20f, 0.50f,  0.50f},
    {"Large Hall",    0.50f, 0.50f, 0.60f, 0.625f, 0.60f},
    {"Cathedral",     0.60f, 0.45f, 0.80f, 0.70f,  0.50f},
    {"Ambient Pad",   0.80f, 0.50f, 0.85f, 0.80f,  0.40f},
    {"Shimmer",       0.60f, 0.60f, 0.75f, 0.70f,  0.90f},
    {"Vintage Plate", 0.40f, 0.60f, 0.40f, 0.70f,  0.35f},
    {"Tight Ambience",0.25f, 0.45f, 0.15f, 0.55f,  0.65f},
    {"Dark Space",    0.55f, 0.50f, 0.70f, 0.65f,  0.25f},
    {"Infinite",      0.70f, 0.40f, 0.95f, 0.75f,  0.45f},
};

const std::vector<ReverbPreset>& CloudsReverbProcessor::getFactoryPresets()
{
    return kFactoryPresets;
}

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amount", 1},
        "Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"input_gain", 1},
        "Input Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"time", 1},
        "Time",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"diffusion", 1},
        "Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.625f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lp", 1},
        "LP Filter",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.7f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; },
        nullptr));

    return { params.begin(), params.end() };
}

CloudsReverbProcessor::CloudsReverbProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("CloudsReverb"), createParameterLayout())
{
    amountParam = parameters.getRawParameterValue("amount");
    inputGainParam = parameters.getRawParameterValue("input_gain");
    timeParam = parameters.getRawParameterValue("time");
    diffusionParam = parameters.getRawParameterValue("diffusion");
    lpParam = parameters.getRawParameterValue("lp");

    parameters.addParameterListener("amount", this);
    parameters.addParameterListener("input_gain", this);
    parameters.addParameterListener("time", this);
    parameters.addParameterListener("diffusion", this);
    parameters.addParameterListener("lp", this);
}

CloudsReverbProcessor::~CloudsReverbProcessor()
{
    parameters.removeParameterListener("amount", this);
    parameters.removeParameterListener("input_gain", this);
    parameters.removeParameterListener("time", this);
    parameters.removeParameterListener("diffusion", this);
    parameters.removeParameterListener("lp", this);
}

const juce::String CloudsReverbProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CloudsReverbProcessor::acceptsMidi() const { return false; }
bool CloudsReverbProcessor::producesMidi() const { return false; }
bool CloudsReverbProcessor::isMidiEffect() const { return false; }
double CloudsReverbProcessor::getTailLengthSeconds() const { return 5.0; }

int CloudsReverbProcessor::getNumPrograms()
{
    return static_cast<int>(kFactoryPresets.size());
}

int CloudsReverbProcessor::getCurrentProgram()
{
    return currentProgram;
}

void CloudsReverbProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < static_cast<int>(kFactoryPresets.size())) {
        currentProgram = index;
        const auto& preset = kFactoryPresets[static_cast<size_t>(index)];

        // Apply preset values to parameters
        if (auto* param = parameters.getParameter("amount"))
            param->setValueNotifyingHost(preset.amount);
        if (auto* param = parameters.getParameter("input_gain"))
            param->setValueNotifyingHost(preset.inputGain);
        if (auto* param = parameters.getParameter("time"))
            param->setValueNotifyingHost(preset.time);
        if (auto* param = parameters.getParameter("diffusion"))
            param->setValueNotifyingHost(preset.diffusion);
        if (auto* param = parameters.getParameter("lp"))
            param->setValueNotifyingHost(preset.lp);
    }
}

const juce::String CloudsReverbProcessor::getProgramName(int index)
{
    if (index >= 0 && index < static_cast<int>(kFactoryPresets.size()))
        return kFactoryPresets[static_cast<size_t>(index)].name;
    return {};
}

void CloudsReverbProcessor::changeProgramName(int, const juce::String&)
{
    // Factory presets are read-only
}

void CloudsReverbProcessor::prepareToPlay(double sampleRate, int)
{
    reverb.Init(static_cast<float>(sampleRate));

    // Initialize smoothed values with current parameter values
    smoothedAmount.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedAmount.setCurrentAndTargetValue(*amountParam);

    smoothedInputGain.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedInputGain.setCurrentAndTargetValue(*inputGainParam);

    smoothedTime.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedTime.setCurrentAndTargetValue(*timeParam);

    smoothedDiffusion.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedDiffusion.setCurrentAndTargetValue(*diffusionParam);

    smoothedLp.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedLp.setCurrentAndTargetValue(*lpParam);

    // Apply initial values
    reverb.SetAmount(smoothedAmount.getCurrentValue());
    reverb.SetInputGain(smoothedInputGain.getCurrentValue());
    reverb.SetTime(smoothedTime.getCurrentValue());
    reverb.SetDiffusion(smoothedDiffusion.getCurrentValue());
    reverb.SetLowpassCutoff(smoothedLp.getCurrentValue());
}

void CloudsReverbProcessor::releaseResources()
{
    reverb.Clear();
}

bool CloudsReverbProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CloudsReverbProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Update smoothed parameters if they're still ramping
    if (smoothedAmount.isSmoothing() || smoothedInputGain.isSmoothing() ||
        smoothedTime.isSmoothing() || smoothedDiffusion.isSmoothing() ||
        smoothedLp.isSmoothing()) {
        // Process in small chunks to apply smoothed parameter updates
        const int numSamples = buffer.getNumSamples();
        constexpr int kChunkSize = 32;

        for (int offset = 0; offset < numSamples; offset += kChunkSize) {
            const int samplesToProcess = std::min(kChunkSize, numSamples - offset);

            // Skip ahead in smoothing and apply current values
            smoothedAmount.skip(samplesToProcess);
            smoothedInputGain.skip(samplesToProcess);
            smoothedTime.skip(samplesToProcess);
            smoothedDiffusion.skip(samplesToProcess);
            smoothedLp.skip(samplesToProcess);

            reverb.SetAmount(smoothedAmount.getCurrentValue());
            reverb.SetInputGain(smoothedInputGain.getCurrentValue());
            reverb.SetTime(smoothedTime.getCurrentValue());
            reverb.SetDiffusion(smoothedDiffusion.getCurrentValue());
            reverb.SetLowpassCutoff(smoothedLp.getCurrentValue());

            reverb.Process(leftChannel + offset, rightChannel + offset,
                           static_cast<size_t>(samplesToProcess));
        }
    } else {
        // No smoothing needed, process entire buffer at once
        reverb.Process(leftChannel, rightChannel, static_cast<size_t>(buffer.getNumSamples()));
    }
}

bool CloudsReverbProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* CloudsReverbProcessor::createEditor()
{
    return new CloudsReverbEditor(*this);
}

void CloudsReverbProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CloudsReverbProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void CloudsReverbProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Set target values for smoothed parameters - the actual update happens in processBlock
    if (parameterID == "amount")
        smoothedAmount.setTargetValue(newValue);
    else if (parameterID == "input_gain")
        smoothedInputGain.setTargetValue(newValue);
    else if (parameterID == "time")
        smoothedTime.setTargetValue(newValue);
    else if (parameterID == "diffusion")
        smoothedDiffusion.setTargetValue(newValue);
    else if (parameterID == "lp")
        smoothedLp.setTargetValue(newValue);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CloudsReverbProcessor();
}
