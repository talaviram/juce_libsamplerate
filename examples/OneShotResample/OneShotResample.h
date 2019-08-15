/*
 ==============================================================================
 Copyright (c) 2019, Tal Aviram
 All rights reserved.

 This code is released under 2-clause BSD license. Please see the
 file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING
 ==============================================================================
 */

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             OneShotResample
 version:          1.0.0
 vendor:           JUCE
 website:          https://github.com/talaviram/juce_libsamplerate
 description:      Example of one shot resample API.
                   Generate a sine wave into buffer in one sample rate and then resample it
                   to output samplerate.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_libsamplerate,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

//==============================================================================
class MainContentComponent   : public AudioAppComponent
{
public:
    MainContentComponent()
    {
        addAndMakeVisible (levelSlider);
        levelSlider.setRange (0.0, 0.125);
        levelSlider.setValue ((double) currentLevel, dontSendNotification);
        levelSlider.onValueChange = [this] { currentLevel = (float) levelSlider.getValue(); };

        generatedSine.setSize (1, 88200);
        for (auto sample = 0; sample < generatedSine.getNumSamples(); ++sample)
        {
            auto currentSample = (float) std::sin (currentAngle);
            updateAngleDelta();
            currentAngle += angleDelta;
            generatedSine.setSample (0, sample, currentSample);
        }
        setSize (200, 200);
        setAudioChannels (0, 2); // no inputs, two outputs
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        levelSlider    .setBounds (10, 40, getWidth() - 20, 20);
    }

    inline void updateAngleDelta()
    {
        auto cyclesPerSample = sineFrequency / generatedSampleRate;
        angleDelta = cyclesPerSample * 2.0 * MathConstants<double>::pi;
    }

    void prepareToPlay (int samplesPerBlock, double sampleRate) override
    {
        readPos = 0;
        currentSampleRate = sampleRate;
        const auto ratio =  generatedSampleRate / sampleRate;
        resampledBuffer.setSize (1, ratio * generatedSine.getNumSamples());
        libsamplerate::SRC::resample (generatedSine, resampledBuffer, ratio, libsamplerate::SRC::ResamplerQuality::SRC_SINC_BEST_QUALITY);
    }

    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        writePos = 0;
        while (writePos < bufferToFill.numSamples)
        {
            readPos %= resampledBuffer.getNumSamples();
            int samplesToCopy = jmin (resampledBuffer.getNumSamples() - readPos, bufferToFill.numSamples - writePos);
            bufferToFill.buffer->copyFrom (0, writePos, resampledBuffer, 0, readPos, samplesToCopy);
            writePos += samplesToCopy;
            readPos += samplesToCopy;
        }
        bufferToFill.buffer->copyFrom (1, 0, *bufferToFill.buffer, 0, 0, bufferToFill.numSamples);
        // no smoothing to keep code simple.
        bufferToFill.buffer->applyGain (currentLevel);
    }

private:
    Slider levelSlider;
    AudioBuffer<float> generatedSine, resampledBuffer;
    int readPos = 0;
    int writePos = 0;
    double generatedSampleRate = 11025.0;
    double currentSampleRate = 0.0, currentAngle = 0.0, angleDelta = 0.0;

    double sineFrequency = 440.0;
    float currentLevel = 0.1f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
