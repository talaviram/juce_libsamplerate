/*
 ==============================================================================
 Copyright (c) 2019, Tal Aviram
 All rights reserved.

 This code is released under 2-clause BSD license. Please see the
 file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING
 ==============================================================================
 */

#include "SRCAudioSource.h"

namespace juce
{
SRCAudioSource::SRCAudioSource (AudioSource* const inputSource,
                                const bool deleteInputWhenDeleted,
                                const libsamplerate::SRC::ResamplerQuality quality,
                                const int channels)
: input (inputSource, deleteInputWhenDeleted),
  conversionType (quality),
  numChannels (channels)
{
    jassert (input != nullptr);
    resamplers_.malloc (numChannels);
    for (auto channel = 0; channel < numChannels; channel++)
    {
        resamplers_[channel] = libsamplerate::src_new (quality, 1, &src_error);
    }
}

SRCAudioSource::~SRCAudioSource()
{
    for (auto channel = 0; channel < numChannels; channel++)
    {
        resamplers_[channel] = libsamplerate::src_delete (resamplers_[channel]);
        jassert (resamplers_[channel] == nullptr);
    }
}

void SRCAudioSource::setResamplingRatio (const double samplesInPerOutputSample, bool shouldSmooth)
{
    jassert (samplesInPerOutputSample > 0);
    const SpinLock::ScopedLockType sl (ratioLock);
    ratio = samplesInPerOutputSample;
    if (!shouldSmooth)
    {
        for (auto channel = 0; channel < numChannels; channel++)
        {
            libsamplerate::src_set_ratio (resamplers_[channel], jmax (0.0, jmax (0.0, 1.0 / ratio)));
        }
    }
}

void SRCAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const SpinLock::ScopedLockType sl (ratioLock);
    auto scaledBlockSize = roundToInt (samplesPerBlockExpected * ratio);
    input->prepareToPlay (scaledBlockSize, sampleRate * ratio);
    buffer.setSize (numChannels, scaledBlockSize + 32);
    srcBuffers.calloc (numChannels);
    destBuffers.calloc (numChannels);
    data_.clear();
    for (auto channel = 0; channel < numChannels; channel++)
    {
        data_.add (new libsamplerate::SRC_DATA);
        auto result = libsamplerate::src_set_ratio (resamplers_[channel], jmax (0.0, 1.0 / ratio));
        result = libsamplerate::src_reset (resamplers_[channel]);
    }
}

void SRCAudioSource::releaseResources()
{
    input->releaseResources();
}

void SRCAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (callbackLock);

    double localRatio;

    {
        const SpinLock::ScopedLockType ratioSl (ratioLock);
        localRatio = ratio;
    }

    if (lastRatio != localRatio)
    {
        lastRatio = localRatio;
    }

    const int sampsNeeded = roundToInt (info.numSamples) * localRatio; //+ 3;

    int bufferSize = buffer.getNumSamples();

    if (bufferSize < sampsNeeded + 8)
    {
        bufferPos %= bufferSize;
        bufferSize = sampsNeeded + 32;
        buffer.setSize (buffer.getNumChannels(), bufferSize, true, true);
    }

    const int channelsToProcess = jmin (numChannels, info.buffer->getNumChannels());

    int samplesGenerated = 0;
    int samplesConsumed = 0;
    while (info.numSamples > samplesGenerated)
    {
        int endOfBufferPos = bufferPos + sampsInBuffer;
        // prepare data struct for process
        bufferPos %= bufferSize;

        endOfBufferPos %= bufferSize;
        int numToDo = bufferSize - endOfBufferPos;
        AudioSourceChannelInfo readInfo (&buffer, endOfBufferPos, numToDo);

        sampsInBuffer += numToDo;
        input->getNextAudioBlock (readInfo);
        for (int channel = 0; channel < numChannels; ++channel)
        {
            destBuffers[channel] = info.buffer->getWritePointer (channel, info.startSample + samplesGenerated);
            srcBuffers[channel] = buffer.getReadPointer (jmin(channel, channelsToProcess - 1), endOfBufferPos);

            auto* data = data_[channel];
            data->data_in = srcBuffers[channel];
            data->data_out = destBuffers[channel];
            data->input_frames = numToDo;
            data->output_frames = info.numSamples - samplesGenerated;
            data->src_ratio = 1.0 / lastRatio;
            data->end_of_input = 0; //  Equal to 0 if more input data is available and 1 otherwise.
            auto result = libsamplerate::src_process (resamplers_[channel], data);
            jassert (result == 0);
            // this should be the same for all resamplers
            jassert (data->input_frames_used == data_[jmax(channel - 1, 0)]->input_frames_used);
            jassert (data->output_frames_gen == data_[jmax(channel - 1, 0)]->output_frames_gen);
            jassert (data->end_of_input == 0);
        }
        sampsInBuffer -= data_[0]->input_frames_used;
        samplesConsumed += data_[0]->input_frames_used;
        samplesGenerated += data_[0]->output_frames_gen;
        jassert (sampsInBuffer >= 0);
        jassert (samplesGenerated > 0);
    }
    jassert (sampsInBuffer >= 0);
}

} // namespace juce
