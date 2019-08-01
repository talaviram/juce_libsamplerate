/*
 ==============================================================================

 This file is based on part of the JUCE library.
 Copyright (c) 2017 - ROLI Ltd.

 JUCE is an open source library subject to commercial or open-source
 licensing.

 The code included in this file is provided under the terms of the ISC license
 http://www.isc.org/downloads/software-support-policy/isc-license. Permission
 To use, copy, modify, and/or distribute this software for any purpose with or
 without fee is hereby granted provided that the above copyright notice and
 this permission notice appear in all copies.

 JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 DISCLAIMED.

 ==============================================================================
 */

namespace juce
{

    SRCAudioTransportSource::SRCAudioTransportSource()
    {
    }

    SRCAudioTransportSource::~SRCAudioTransportSource()
    {
        setSource (nullptr);
        releaseMasterResources();
    }

    void SRCAudioTransportSource::setSource (PositionableAudioSource* const newSource,
                                          int readAheadSize, TimeSliceThread* readAheadThread,
                                          double sourceSampleRateToCorrectFor, ResamplerQuality src_quality,
                                          int maxNumChannels)
    {
        if (source == newSource)
        {
            if (source == nullptr)
                return;

            setSource (nullptr, 0, nullptr); // deselect and reselect to avoid releasing resources wrongly
        }

        readAheadBufferSize = readAheadSize;
        sourceSampleRate = sourceSampleRateToCorrectFor;

        SRCAudioSource* newResamplerSource = nullptr;
        BufferingAudioSource* newBufferingSource = nullptr;
        PositionableAudioSource* newPositionableSource = nullptr;
        AudioSource* newMasterSource = nullptr;

        std::unique_ptr<SRCAudioSource> oldResamplerSource (resamplerSource);
        std::unique_ptr<BufferingAudioSource> oldBufferingSource (bufferingSource);
        AudioSource* oldMasterSource = masterSource;

        if (newSource != nullptr)
        {
            newPositionableSource = newSource;

            if (readAheadSize > 0)
            {
                // If you want to use a read-ahead buffer, you must also provide a TimeSliceThread
                // for it to use!
                jassert (readAheadThread != nullptr);

                newPositionableSource = newBufferingSource
                = new BufferingAudioSource (newPositionableSource, *readAheadThread,
                                            false, readAheadSize, maxNumChannels);
            }

            newPositionableSource->setNextReadPosition (0);

            if (sourceSampleRateToCorrectFor > 0)
                newMasterSource = newResamplerSource
                = new SRCAudioSource (newPositionableSource, false, src_quality, maxNumChannels);
            else
                newMasterSource = newPositionableSource;

            if (isPrepared)
            {
                if (newResamplerSource != nullptr && sourceSampleRate > 0 && sampleRate > 0)
                    newResamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

                newMasterSource->prepareToPlay (blockSize, sampleRate);
            }
        }

        {
            const ScopedLock sl (callbackLock);

            source = newSource;
            resamplerSource = newResamplerSource;
            bufferingSource = newBufferingSource;
            masterSource = newMasterSource;
            positionableSource = newPositionableSource;

            inputStreamEOF = false;
            playing = false;
        }

        if (oldMasterSource != nullptr)
            oldMasterSource->releaseResources();
    }

    void SRCAudioTransportSource::start()
    {
        if ((! playing) && masterSource != nullptr)
        {
            {
                const ScopedLock sl (callbackLock);
                playing = true;
                stopped = false;
                inputStreamEOF = false;
            }

            sendChangeMessage();
        }
    }

    void SRCAudioTransportSource::stop()
    {
        if (playing)
        {
            playing = false;

            int n = 500;
            while (--n >= 0 && ! stopped)
                Thread::sleep (2);

            sendChangeMessage();
        }
    }

    void SRCAudioTransportSource::setPosition (double newPosition)
    {
        if (sampleRate > 0.0)
            setNextReadPosition ((int64) (newPosition * sampleRate));
    }

    double SRCAudioTransportSource::getCurrentPosition() const
    {
        if (sampleRate > 0.0)
            return (double) getNextReadPosition() / sampleRate;

        return 0.0;
    }

    double SRCAudioTransportSource::getLengthInSeconds() const
    {
        if (sampleRate > 0.0)
            return (double) getTotalLength() / sampleRate;

        return 0.0;
    }

    void SRCAudioTransportSource::setNextReadPosition (int64 newPosition)
    {
        if (positionableSource != nullptr)
        {
            if (sampleRate > 0 && sourceSampleRate > 0)
                newPosition = (int64) ((double) newPosition * sourceSampleRate / sampleRate);

            positionableSource->setNextReadPosition (newPosition);

            if (resamplerSource != nullptr)
                resamplerSource->reset();

            inputStreamEOF = false;
        }
    }

    int64 SRCAudioTransportSource::getNextReadPosition() const
    {
        if (positionableSource != nullptr)
        {
            const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
            return (int64) ((double) positionableSource->getNextReadPosition() * ratio);
        }

        return 0;
    }

    int64 SRCAudioTransportSource::getTotalLength() const
    {
        const ScopedLock sl (callbackLock);

        if (positionableSource != nullptr)
        {
            const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
            return (int64) ((double) positionableSource->getTotalLength() * ratio);
        }

        return 0;
    }

    bool SRCAudioTransportSource::isLooping() const
    {
        const ScopedLock sl (callbackLock);
        return positionableSource != nullptr && positionableSource->isLooping();
    }

    void SRCAudioTransportSource::setGain (const float newGain) noexcept
    {
        gain = newGain;
    }

    void SRCAudioTransportSource::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
    {
        const ScopedLock sl (callbackLock);

        sampleRate = newSampleRate;
        blockSize = samplesPerBlockExpected;

        if (masterSource != nullptr)
            masterSource->prepareToPlay (samplesPerBlockExpected, sampleRate);

        if (resamplerSource != nullptr && sourceSampleRate > 0)
            resamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

        inputStreamEOF = false;
        isPrepared = true;
    }

    void SRCAudioTransportSource::releaseMasterResources()
    {
        const ScopedLock sl (callbackLock);

        if (masterSource != nullptr)
            masterSource->releaseResources();

        isPrepared = false;
    }

    void SRCAudioTransportSource::releaseResources()
    {
        releaseMasterResources();
    }

    void SRCAudioTransportSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
    {
        const ScopedLock sl (callbackLock);

        if (masterSource != nullptr && ! stopped)
        {
            masterSource->getNextAudioBlock (info);

            if (! playing)
            {
                // just stopped playing, so fade out the last block..
                for (int i = info.buffer->getNumChannels(); --i >= 0;)
                    info.buffer->applyGainRamp (i, info.startSample, jmin (256, info.numSamples), 1.0f, 0.0f);

                if (info.numSamples > 256)
                    info.buffer->clear (info.startSample + 256, info.numSamples - 256);
            }

            if (positionableSource->getNextReadPosition() > positionableSource->getTotalLength() + 1
                && ! positionableSource->isLooping())
            {
                playing = false;
                inputStreamEOF = true;
                sendChangeMessage();
            }

            stopped = ! playing;

            for (int i = info.buffer->getNumChannels(); --i >= 0;)
                info.buffer->applyGainRamp (i, info.startSample, info.numSamples, lastGain, gain);
        }
        else
        {
            info.clearActiveBufferRegion();
            stopped = true;
        }

        lastGain = gain;
    }

} // namespace juce

