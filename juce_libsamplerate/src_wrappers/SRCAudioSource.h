/*
 ==============================================================================
    Copyright (c) 2019, Tal Aviram
    All rights reserved.

    This code is released under 2-clause BSD license. Please see the
    file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING
 ==============================================================================
 */

//==============================================================================

/**
 A type of AudioSource that takes an input source and changes its sample rate.
 It is based on juce::ResampleAudioSource but uses libsamplerate for better sounding SRC.

 @see ResamplingAudioSource

 @tags{Audio}
 */

#pragma once

namespace juce
{

class SRCAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a SRCAudioSource for a given input source.

     @param inputSource              the input source to read from
     @param deleteInputWhenDeleted   if true, the input source will be deleted when
     this object is deleted
     @param numChannels              the number of channels to process
     */
    SRCAudioSource (AudioSource* inputSource,
                    bool deleteInputWhenDeleted,
                    libsamplerate::SRC::ResamplerQuality quality = libsamplerate::SRC::SRC_SINC_MEDIUM_QUALITY,
                    int numChannels = 2);

    /** Destructor. */
    ~SRCAudioSource() override;

    /** Changes the resampling ratio.

     (This value can be changed at any time, even while the source is running).

     @param samplesInPerOutputSample     if set to 1.0, the input is passed through; higher
     values will speed it up; lower values will slow it
     down. The ratio must be greater than 0
     @param shouldSmooth - will try to smoothly transition between the conversion ratio of the last call and the conversion ratio of the current call. 
     */
    void setResamplingRatio (double samplesInPerOutputSample, bool shouldSmooth = true);

    /** Returns the current resampling ratio.

     This is the value that was set by setResamplingRatio().
     */
    double getResamplingRatio() const noexcept                  { return ratio; }

    /** Resets resampler state **/
    void reset();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo&) override;

private:

    //==============================================================================
    juce::OptionalScopedPointer<juce::AudioSource> input;
    double ratio = 1.0, lastRatio = 1.0;
    libsamplerate::SRC::ResamplerQuality conversionType; // SRC quality
    juce::AudioBuffer<float> buffer;
    int bufferPos = 0, sampsInBuffer = 0;

    HeapBlock<libsamplerate::SRC_STATE*> resamplers_;
    OwnedArray<libsamplerate::SRC_DATA> data_;
    juce::SpinLock ratioLock;
    juce::CriticalSection callbackLock;

    int src_error;
    int src_result;
    const int numChannels;
    HeapBlock<float*> destBuffers;
    HeapBlock<const float*> srcBuffers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SRCAudioSource)
};

} // namespace juce
