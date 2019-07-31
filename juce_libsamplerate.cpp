/*
 ==============================================================================
 Copyright (c) 2019, Tal Aviram
 All rights reserved.

 This code is released under 2-clause BSD license. Please see the
 file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING

 juce_libsamplerate IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 DISCLAIMED.

 ==============================================================================
 */

#ifdef JUCE_LIBSAMPLERATE_H_INCLUDED
/* When you add this cpp file to your project, you mustn't include it in a file where you've
 already included any other headers - just put it inside a file on its own, possibly with your config
 flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
 header files that the compiler may be using.
 */
#error "Incorrect use of JUCE LIBSAMPLERATE cpp file"
#endif

#include "juce_libsamplerate.h"

//==============================================================================
#include "src_wrappers/libsamplerate_SRC.cpp"
#include "src_wrappers/SRCAudioSource.cpp"
#include "src_wrappers/SRCAudioTransportSource.cpp"
