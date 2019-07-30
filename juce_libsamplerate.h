/*
 ==============================================================================
 Copyright (c) 2019, Tal Aviram
 All rights reserved.

 This code is released under 2-clause BSD license. Please see the
 file at : https://github.com/talaviram/juce_libsamplerate/blob/master/COPYING
 ==============================================================================
 */

/*******************************************************************************
 * This file declares a JUCE module for the shared library code implemented in
 * this directory. The block below is read by the Projucer to automatically
 * generate project code that uses the module. For details, see the
 * JUCE Module Format.txt file in the JUCE directory.

   BEGIN_JUCE_MODULE_DECLARATION
    ID:                 juce_libsamplerate
    vendor:             Tal Aviram
    version:            0.1.0
    name:               Secret-Rabbit-Code (libsamplerate)
    description:        Secret-Rabbit-Code (libsamplerate by Erik de Castro Lopo) wrapped for JUCE
    minimumCppStandard: 11
    dependencies:       juce_audio_basics
    searchpaths:        ./libsamplerate/src
   END_JUCE_MODULE_DECLARATION
*******************************************************************************/

#pragma once

#define JUCE_LIBSAMPLERATE_H_INCLUDED

#include <juce_audio_basics/juce_audio_basics.h>
#include "src_wrappers/libsamplerate_SRC.h"
#include "src_wrappers/SRCAudioSource.h"
