PlaySoundFiles
--------------

This is basically 'Playing Sound Files' tutorial.
With 2 minor changes:
- adds dependency of the `juce_libsamplerate` module
- changed the use of [`juce::AudioTransportSource`](https://docs.juce.com/master/classAudioTransportSource.html) to `SRCAudioTransportSource` to utilize Secret-Rabbit-Code resampling.

Requirements:
- Projucer to make the PIP file into a project
- copy/symlink/change your User Modules to include the `juce_libsamplerate` module.

https://docs.juce.com/master/tutorial_playing_sound_files.html

