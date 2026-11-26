#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../AudioEngine/AudioEngine.h"
#include "DawLookAndFeel.h"
#include "WaveformView.h"

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void openFile();
    void updateControls();
    static juce::String formatTime(double seconds);

    // Ordem importa: o look and feel e o motor existem antes dos componentes que os usam.
    DawLookAndFeel lookAndFeel;
    AudioEngine engine;

    juce::TextButton openButton  { "Open" };
    juce::TextButton playButton  { "Play" };
    juce::TextButton stopButton  { "Stop" };
    juce::Slider volumeSlider;
    juce::Label fileLabel;
    juce::Label timeLabel;
    WaveformView waveform { engine.getFormatManager() };

    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
