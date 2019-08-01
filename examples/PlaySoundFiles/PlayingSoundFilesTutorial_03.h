/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PlayingSoundFilesTutorial
 version:          3.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_libsamplerate
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

//==============================================================================
class MainContentComponent   : public AudioAppComponent,
                               public ChangeListener,
                               public Timer
{
public:
    MainContentComponent()
        :   state (Stopped)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (TextButton::buttonColourId, Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible (&loopingToggle);
        loopingToggle.setButtonText ("Loop");
        loopingToggle.onClick = [this] { loopButtonChanged(); };

        addAndMakeVisible (&currentPositionLabel);
        currentPositionLabel.setText ("Stopped", dontSendNotification);

        setSize (300, 200);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);

        setAudioChannels (2, 2);
        startTimer (20);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        openButton          .setBounds (10, 10,  getWidth() - 20, 20);
        playButton          .setBounds (10, 40,  getWidth() - 20, 20);
        stopButton          .setBounds (10, 70,  getWidth() - 20, 20);
        loopingToggle       .setBounds (10, 100, getWidth() - 20, 20);
        currentPositionLabel.setBounds (10, 130, getWidth() - 20, 20);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else
                changeState (Stopped);
        }
    }

    void timerCallback() override
    {
        if (transportSource.isPlaying())
        {
            RelativeTime position (transportSource.getCurrentPosition());

            auto minutes = ((int) position.inMinutes()) % 60;
            auto seconds = ((int) position.inSeconds()) % 60;
            auto millis  = ((int) position.inMilliseconds()) % 1000;

            auto positionString = String::formatted ("%02d:%02d:%03d", minutes, seconds, millis);

            currentPositionLabel.setText (positionString, dontSendNotification);
        }
        else
        {
            currentPositionLabel.setText ("Stopped", dontSendNotification);
        }
    }

    void updateLoopState (bool shouldLoop)
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping (shouldLoop);
    }

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }

    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...",
                             {},
                             "*.wav");

        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            auto* reader = formatManager.createReaderFor (file);

            if (reader != nullptr)
            {
                std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true));
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
                readerSource.reset (newSource.release());
            }
        }
    }

    void playButtonClicked()
    {
        updateLoopState (loopingToggle.getToggleState());
        changeState (Starting);
    }

    void stopButtonClicked()
    {
        changeState (Stopping);
    }

    void loopButtonChanged()
    {
        updateLoopState (loopingToggle.getToggleState());
    }

    //==========================================================================
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopingToggle;
    Label currentPositionLabel;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    SRCAudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
