/*
  ==============================================================================

    LowpassHighpassFilter.cpp
    Created: 1 May 2024 3:08:53pm
    Author:  sofiy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LowpassHighpassFilter.h"

//==============================================================================
LowpassHighpassFilter::LowpassHighpassFilter()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

LowpassHighpassFilter::~LowpassHighpassFilter()
{
}

void LowpassHighpassFilter::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("LowpassHighpassFilter", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void LowpassHighpassFilter::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

//===== SETTERS ================================================================
void LowpassHighpassFilter::setHighPass(bool highPass)
{
    this->highPass = highPass;
}

void LowpassHighpassFilter::setCutoffFrequency(float cutoffFrequency)
{
    this->cutoffFrequency = cutoffFrequency;
}

void LowpassHighpassFilter::setSamplingRate(float samplingRate)
{
    this->samplingRate = samplingRate;
}

//===== THIS IS WHERE THE WORK IS DONE ========================================
void LowpassHighpassFilter::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    constexpr auto PI = 3.14159265359f;

    // resize the allpass buffers vector to the number of channels
    // and zero the new ones
    dnBuffer.resize(buffer.getNumChannels(), 0.f);

    // if we perform highpass filtering, 
    // we need to invert the output of the allpass 
    // (multiply it by -1)
    const auto sign = highPass ? -1.f : 1.f;

    // helper variable
    const auto tan = std::tan(PI * cutoffFrequency / samplingRate);
    // allpass coefficient is constant during the
    // processing of a block of samples
    const auto a1 = (tan - 1.f) / (tan + 1.f);

    // actual processing; applies to each channel separately
    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
        // to access the samples in the channel as a C-style array
        auto channelSamples = buffer.getWritePointer(channel);

        // for each sample in the channel
        for (auto i = 0; i < buffer.getNumSamples(); ++i) {
            // just reading the input sample
            const auto inputSample = channelSamples[i];

            // allpass filtering
            // differential equation at order 1:
            // s[n]  = a1*e[n] + e[n-1] - a1*s[n-1]
            const auto allpassFilteredSample = a1 * inputSample + dnBuffer[channel];
            // at the next iteration this will be e[n-1] -a1*s[n-1]
            dnBuffer[channel] = inputSample - a1 * allpassFilteredSample;

            // here the final filtering occurs
            // we scale by 0.5 to stay in the [-1, 1] range
            const auto filterOutput =
                0.5f * (inputSample + sign * allpassFilteredSample);

            // assign to the output 
            // write the modified sample on the channel (= output operation)
            channelSamples[i] = filterOutput;
        }
    }
}
