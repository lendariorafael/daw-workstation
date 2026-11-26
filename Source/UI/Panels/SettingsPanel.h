#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../../AudioEngine/AudioEngine.h"
#include "../../I18n/Strings.h"
#include "../../Settings/OpaloSettings.h"
#include "../DawLookAndFeel.h"

// Definições: idioma, dispositivo de áudio e tamanho do buffer (latência).
class SettingsPanel : public juce::Component,
                      private juce::Timer
{
public:
    SettingsPanel(AudioEngine& e, std::function<void()> onLanguageChanged,
                  std::function<void(int)> onLayoutChanged)
        : engine(e),
          onLanguage(std::move(onLanguageChanged)),
          onLayout(std::move(onLayoutChanged)),
          deviceSelector(e.devices(), 0, 2, 0, 2, false, false, true, false)
    {
        languageLabel.setText(I18n::t("settings.language"), juce::dontSendNotification);
        bufferLabel.setText(I18n::t("settings.latency"), juce::dontSendNotification);
        latencyLabel.setFont(DawLookAndFeel::uiFont(13.0f, true));
        latencyLabel.setColour(juce::Label::textColourId, DawLookAndFeel::accent);

        languageBox.addItem("Português", 1);
        languageBox.addItem("English", 2);
        languageBox.addItem("Français", 3);
        languageBox.setSelectedId((int) I18n::current() + 1, juce::dontSendNotification);
        languageBox.onChange = [this]
        {
            const auto lang = (I18n::Language) juce::jlimit(0, 2, languageBox.getSelectedId() - 1);
            I18n::current() = lang;
            OpaloSettings::get().setValue("language", I18n::code(lang));
            OpaloSettings::get().save();
            onLanguage();
        };

        layoutLabel.setText("Workspace layout", juce::dontSendNotification);
        layoutBox.addItem("Studio Default", 1);
        layoutBox.addItem("Arrangement Focus", 2);
        layoutBox.addItem("Mix Focus", 3);
        layoutBox.addItem("DJ Mashup", 4);
        layoutBox.setSelectedId((int) OpaloSettings::get().getIntValue("layoutPreset", 0) + 1,
                                juce::dontSendNotification);
        layoutBox.onChange = [this]
        {
            const int preset = layoutBox.getSelectedId() - 1;
            OpaloSettings::get().setValue("layoutPreset", preset);
            OpaloSettings::get().save();
            if (onLayout) onLayout(preset);
        };

        for (int i = 0; i < 5; ++i)
            bufferBox.addItem(juce::String(bufferSizes[i]), i + 1);
        bufferBox.onChange = [this] { applyBufferSize(); };

        for (auto* c : { static_cast<juce::Component*>(&languageLabel),
                         static_cast<juce::Component*>(&languageBox),
                         static_cast<juce::Component*>(&bufferLabel),
                         static_cast<juce::Component*>(&layoutLabel),
                         static_cast<juce::Component*>(&layoutBox),
                         static_cast<juce::Component*>(&bufferBox),
                         static_cast<juce::Component*>(&latencyLabel),
                         static_cast<juce::Component*>(&deviceSelector) })
            addAndMakeVisible(c);

        syncBufferBox();
        startTimerHz(2);
    }

    ~SettingsPanel() override
    {
        stopTimer();
        saveDeviceState();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::panel);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(16);

        auto row = area.removeFromTop(28);
        languageLabel.setBounds(row.removeFromLeft(160));
        languageBox.setBounds(row.removeFromLeft(220));

        area.removeFromTop(10);
        row = area.removeFromTop(28);
        bufferLabel.setBounds(row.removeFromLeft(160));
        bufferBox.setBounds(row.removeFromLeft(120));
        row.removeFromLeft(16);
        latencyLabel.setBounds(row);

        area.removeFromTop(12);
        row = area.removeFromTop(28);
        layoutLabel.setBounds(row.removeFromLeft(160));
        layoutBox.setBounds(row.removeFromLeft(220));

        area.removeFromTop(16);
        deviceSelector.setBounds(area);
    }

private:
    void timerCallback() override
    {
        refreshLatency();
    }

    void syncBufferBox()
    {
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        engine.devices().getAudioDeviceSetup(setup);

        for (int i = 0; i < 5; ++i)
            if (bufferSizes[i] == setup.bufferSize)
                bufferBox.setSelectedId(i + 1, juce::dontSendNotification);

        refreshLatency();
    }

    void applyBufferSize()
    {
        const int index = juce::jlimit(0, 4, bufferBox.getSelectedId() - 1);

        juce::AudioDeviceManager::AudioDeviceSetup setup;
        engine.devices().getAudioDeviceSetup(setup);
        setup.bufferSize = bufferSizes[index];

        // Um buffer mais pequeno reduz a latência, mas exige mais do processador.
        engine.devices().setAudioDeviceSetup(setup, true);
        saveDeviceState();
        refreshLatency();
    }

    void refreshLatency()
    {
        auto* device = engine.devices().getCurrentAudioDevice();
        if (device == nullptr)
        {
            latencyLabel.setText("—", juce::dontSendNotification);
            return;
        }

        const double sampleRate = device->getCurrentSampleRate();
        if (sampleRate <= 0.0)
            return;

        const double samples = device->getCurrentBufferSizeSamples() + device->getOutputLatencyInSamples();
        const double ms = samples * 1000.0 / sampleRate;

        latencyLabel.setText(I18n::t("settings.latencyInfo") + ": " + juce::String(ms, 1) + " ms",
                             juce::dontSendNotification);
    }

    void saveDeviceState()
    {
        auto state = engine.devices().createStateXml();
        if (state != nullptr)
        {
            OpaloSettings::get().setValue("audioDevice", state.get());
            OpaloSettings::get().save();
        }
    }

    static constexpr int bufferSizes[5] = { 64, 128, 256, 512, 1024 };

    AudioEngine& engine;
    std::function<void()> onLanguage;
    std::function<void(int)> onLayout;

    juce::Label languageLabel;
    juce::ComboBox languageBox;
    juce::Label bufferLabel;
    juce::Label layoutLabel;
    juce::ComboBox layoutBox;
    juce::ComboBox bufferBox;
    juce::Label latencyLabel;
    juce::AudioDeviceSelectorComponent deviceSelector;
};
