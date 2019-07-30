/*
 ==============================================================================
 Copyright (c) 2019, Tal Aviram
 All rights reserved.

 This code is released under 2-clause BSD license. Please see the
 file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING
 ==============================================================================
 */

/**
Frontend for libsamplerate needed API to be used with JUCE
 @see ResamplingAudioSource

 @tags{Audio}
 */

#pragma once

namespace libsamplerate
{
#include "samplerate.h"

class SRC
{
public:
    enum ResamplerQuality
    {
        SRC_SINC_BEST_QUALITY        = libsamplerate::SRC_SINC_BEST_QUALITY,
        SRC_SINC_MEDIUM_QUALITY      = libsamplerate::SRC_SINC_MEDIUM_QUALITY,
        SRC_SINC_FASTEST             = libsamplerate::SRC_SINC_FASTEST,
        SRC_ZERO_ORDER_HOLD          = libsamplerate::SRC_ZERO_ORDER_HOLD,
        SRC_LINEAR                   = libsamplerate::SRC_LINEAR
    };

    //==============================================================================
    /** Resamples an audio buffer.
     Important Note: This callback is not designed to work on small chunks of a larger piece of audio. If you attempt to use it this way you are doing it wrong and will not get the results you want.
      @see SRCAudioSource
     */
    static int resample (const juce::AudioBuffer<float>& bufferToResample, juce::AudioBuffer<float>& outputBuffer, double samplesInPerOutputSample, ResamplerQuality converter_type);
};
}
