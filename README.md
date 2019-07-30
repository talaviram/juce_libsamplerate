juce_libsamplerate
------------------

```
WIP Notice: I haven't tested it enough, currently it is a WIP.

```

__THERE WILL/MIGHT BE FORCE PUSHES TO THIS REPO!!!__



JUCE module to provide [libsamplerate](http://www.mega-nerd.com/SRC/) (aka Secret-Rabbit-Code).


Why?
----
Currently,
JUCE has ~2~ 1.5 ways for doing sample-rate-conversion.

- [juce::ResamplingAudioSource](https://docs.juce.com/master/classResamplingAudioSource.html)
- [juce::dsp::Oversampling](https://docs.juce.com/master/classdsp_1_1Oversampling.html#details)

The first one is pretty bad. see [here](http://src.infinitewave.ca/).
The second one was designed for different purposes (and use fixed ratios) which makes it less usable for plain-SRC.
