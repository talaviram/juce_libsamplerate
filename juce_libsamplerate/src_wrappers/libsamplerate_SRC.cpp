
#include "libsamplerate_SRC.h"

namespace libsamplerate {

    /* Name of package */
#define PACKAGE "libsamplerate"
    /* Version number of package */
#define VERSION "0.1.9"

    // we assume cross-compiling for JUCE
    /* Target processor clips on negative float to int conversion. */
#define CPU_CLIPS_NEGATIVE 0

    /* Target processor clips on positive float to int conversion. */
#define CPU_CLIPS_POSITIVE 0

    /* Target processor is little endian. */
#if JUCE_LITTLE_ENDIAN
#define CPU_IS_LITTLE_ENDIAN
#else
    /* Target processor is big endian. */
#define CPU_IS_BIG_ENDIAN
#endif

    // I didn't see much cases where 'alarm' isn't available.
    /* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM

    // JUCE currently support C++11 or greater
    /* Define if you have C99's lrint function. */
#define HAVE_LRINT 1

    /* Define if you have C99's lrintf function. */
#define HAVE_LRINTF 1

    /* Define if you have signal SIGALRM. */
#define HAVE_SIGALRM 1

    /* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

    /* JUCE don't have libsndfile. */
#define HAVE_SNDFILE 0

    /* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

    /* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

    /* Set to 1 if compiling for Win32 */
#if JUCE_WINDOWS
#define OS_IS_WIN32 1
#endif

    /* The size of `int', as computed by sizeof. */
#define SIZEOF_INT __SIZEOF_INT__

    /* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG __SIZEOF_LONG__

#include "../libsamplerate/src/src_linear.c"
#include "../libsamplerate/src/src_zoh.c"
#include "../libsamplerate/src/src_sinc.c"
#include "../libsamplerate/src/samplerate.c"

int SRC::resample (const juce::AudioBuffer<float>& bufferToResample, juce::AudioBuffer<float>& outputBuffer, const double samplesInPerOutputSample, const ResamplerQuality converter_type)
{
    jassert (bufferToResample.getNumChannels() > 0 && outputBuffer.getNumChannels() >= bufferToResample.getNumChannels());
    SRC_DATA data;
    data.data_in = bufferToResample.getReadPointer (0);
    data.input_frames = bufferToResample.getNumSamples();
    data.data_out = outputBuffer.getWritePointer (0);
    data.output_frames =  outputBuffer.getNumSamples();
    data.src_ratio = 1.0 / samplesInPerOutputSample;
    return src_simple (&data, converter_type, std::min (bufferToResample.getNumChannels(), outputBuffer.getNumChannels()));
    ;
}

}
