#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&lookAndFeel);

    openButton.onClick = [this] { openFile(); };
    playButton.onClick = [this] { engine.play(); };
    stopButton.onClick = [this] { engine.stop(); };

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 22);
    volumeSlider.setRange(0.0, 1.0, 0.001);
    volumeSlider.setValue(0.8, juce::dontSendNotification);
    engine.setGain(0.8f);
    volumeSlider.onValueChange = [this] { engine.setGain((float) volumeSlider.getValue()); };

    fileLabel.setText("No file loaded", juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, DawLookAndFeel::muted);

    timeLabel.setText("00:00.00 / 00:00.00", juce::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centredRight);

    for (auto* c : { static_cast<juce::Component*>(&openButton),
                     static_cast<juce::Component*>(&playButton),
                     static_cast<juce::Component*>(&stopButton),
                     static_cast<juce::Component*>(&volumeSlider),
                     static_cast<juce::Component*>(&fileLabel),
                     static_cast<juce::Component*>(&timeLabel) })
        addAndMakeVisible(c);

    setSize(1200, 680);
    startTimerHz(30);
    updateControls();
}

MainComponent::~MainComponent()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(DawLookAndFeel::background);

    // Faixa de acento sob a barra de transporte.
    auto bar = getLocalBounds().removeFromTop(64);
    g.setColour(DawLookAndFeel::panel);
    g.fillRect(bar);
    g.setColour(DawLookAndFeel::accent.withAlpha(0.35f));
    g.fillRect(bar.removeFromBottom(1));
}

void MainComponent::resized()
{
    auto bar = getLocalBounds().removeFromTop(64).reduced(16, 12);

    openButton.setBounds(bar.removeFromLeft(96));
    bar.removeFromLeft(8);
    playButton.setBounds(bar.removeFromLeft(72));
    bar.removeFromLeft(8);
    stopButton.setBounds(bar.removeFromLeft(72));
    bar.removeFromLeft(16);

    timeLabel.setBounds(bar.removeFromRight(190));
    bar.removeFromRight(16);
    volumeSlider.setBounds(bar.removeFromRight(240));
    bar.removeFromRight(16);

    fileLabel.setBounds(bar);
}

void MainComponent::timerCallback()
{
    timeLabel.setText(formatTime(engine.getPosition()) + " / " + formatTime(engine.getLength()),
                      juce::dontSendNotification);
    updateControls();
}

void MainComponent::updateControls()
{
    const bool hasFile = engine.hasFile();
    const bool playing = engine.isPlaying();

    playButton.setEnabled(hasFile && ! playing);
    stopButton.setEnabled(hasFile);
    playButton.setToggleState(playing, juce::dontSendNotification);
}

void MainComponent::openFile()
{
    chooser = std::make_unique<juce::FileChooser>("Open audio file",
                                                  juce::File{},
                                                  "*.wav;*.aif;*.aiff;*.flac;*.ogg");

    chooser->launchAsync(juce::FileBrowserComponent::openMode
                             | juce::FileBrowserComponent::canSelectFiles,
                         [this](const juce::FileChooser& fc)
                         {
                             auto file = fc.getResult();
                             if (! file.existsAsFile())
                                 return;

                             if (engine.loadFile(file))
                                 fileLabel.setText(file.getFileName(), juce::dontSendNotification);
                             else
                                 fileLabel.setText("Could not open: " + file.getFileName(),
                                                   juce::dontSendNotification);

                             updateControls();
                         });
}

juce::String MainComponent::formatTime(double seconds)
{
    if (seconds < 0.0)
        seconds = 0.0;

    const int totalCentis = (int) juce::roundToInt(seconds * 100.0);
    const int minutes = totalCentis / 6000;
    const int secs = (totalCentis / 100) % 60;
    const int centis = totalCentis % 100;

    return juce::String::formatted("%02d:%02d.%02d", minutes, secs, centis);
}
