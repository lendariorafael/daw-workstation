#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../AudioEngine/AudioEngine.h"
#include "../../I18n/Strings.h"
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Mashup: dois decks com crossfader. Cada deck tem BPM (escrito à mão) e velocidade ajustável,
// para igualar o tempo do outro deck ou o tempo do projeto.
class MashupPanel : public juce::Component,
                    private juce::Timer
{
public:
    MashupPanel(ProjectModel& m, AudioEngine& e) : model(m), engine(e)
    {
        for (int i = 0; i < AudioEngine::numDecks; ++i)
            buildDeck(i);

        crossfader.setSliderStyle(juce::Slider::LinearHorizontal);
        crossfader.setRange(0.0, 1.0, 0.001);
        crossfader.setValue(0.0, juce::dontSendNotification);
        crossfader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        crossfader.onValueChange = [this] { engine.setCrossfade((float) crossfader.getValue()); };
        addAndMakeVisible(crossfader);

        crossfaderLabel.setText(I18n::t("mashup.crossfader"), juce::dontSendNotification);
        crossfaderLabel.setJustificationType(juce::Justification::centred);
        crossfaderLabel.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
        addAndMakeVisible(crossfaderLabel);

        syncButton.setButtonText(I18n::t("mashup.sync"));
        syncButton.onClick = [this] { syncDecksToProject(); };
        addAndMakeVisible(syncButton);

        matchButton.setButtonText(I18n::t("mashup.match"));
        matchButton.onClick = [this] { matchBToA(); };
        addAndMakeVisible(matchButton);

        tipLabel.setText(I18n::t("mashup.varispeed"), juce::dontSendNotification);
        tipLabel.setFont(DawLookAndFeel::uiFont(11.0f));
        tipLabel.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
        addAndMakeVisible(tipLabel);

        startTimerHz(20);
    }

    ~MashupPanel() override
    {
        stopTimer();
        for (auto& ui : decksUi)
            ui.chooser.reset();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);

        for (int i = 0; i < AudioEngine::numDecks; ++i)
        {
            const auto area = deckArea(i);
            g.setColour(DawLookAndFeel::panel);
            g.fillRoundedRectangle(area.toFloat(), 10.0f);

            g.setColour(i == 0 ? DawLookAndFeel::accent : DawLookAndFeel::text.withAlpha(0.6f));
            g.fillRect(area.getX() + 14, area.getY() + 14, 3, 18);
        }
    }

    void resized() override
    {
        for (int i = 0; i < AudioEngine::numDecks; ++i)
            layoutDeck(i, deckArea(i));

        auto bottom = getLocalBounds().removeFromBottom(120).reduced(20, 16);
        auto row = bottom.removeFromTop(24);
        crossfaderLabel.setBounds(row);
        crossfader.setBounds(bottom.removeFromTop(28).reduced(120, 0));

        bottom.removeFromTop(10);
        auto buttons = bottom.removeFromTop(34);
        syncButton.setBounds(buttons.removeFromLeft(240));
        buttons.removeFromLeft(12);
        matchButton.setBounds(buttons.removeFromLeft(200));
        tipLabel.setBounds(buttons.reduced(12, 0));
    }

private:
    struct DeckUi
    {
        std::unique_ptr<juce::TextButton> load;
        std::unique_ptr<juce::TextButton> play;
        std::unique_ptr<juce::TextButton> stop;
        std::unique_ptr<juce::Label> title;
        std::unique_ptr<juce::Label> name;
        std::unique_ptr<juce::Label> bpm;
        std::unique_ptr<juce::Label> info;
        std::unique_ptr<juce::FileChooser> chooser;
    };

    juce::Rectangle<int> deckArea(int i) const
    {
        auto area = getLocalBounds().withTrimmedBottom(130).reduced(16);
        const int half = (area.getWidth() - 16) / 2;
        return i == 0 ? area.removeFromLeft(half) : area.removeFromRight(half);
    }

    void buildDeck(int i)
    {
        auto& ui = decksUi[(size_t) i];
        ui.load = std::make_unique<juce::TextButton>(I18n::t("mashup.load"));
        ui.play = std::make_unique<juce::TextButton>(I18n::t("transport.play"));
        ui.stop = std::make_unique<juce::TextButton>(I18n::t("transport.stop"));
        ui.title = std::make_unique<juce::Label>();
        ui.name = std::make_unique<juce::Label>();
        ui.bpm = std::make_unique<juce::Label>();
        ui.info = std::make_unique<juce::Label>();

        ui.title->setText(I18n::t(i == 0 ? "mashup.deckA" : "mashup.deckB"), juce::dontSendNotification);
        ui.title->setFont(DawLookAndFeel::uiFont(14.0f, true));
        ui.name->setText(I18n::t("mashup.noFile"), juce::dontSendNotification);
        ui.name->setColour(juce::Label::textColourId, DawLookAndFeel::muted);

        ui.bpm->setText("BPM: —", juce::dontSendNotification);
        ui.bpm->setEditable(true);
        ui.bpm->setColour(juce::Label::outlineColourId, DawLookAndFeel::panelLight);
        ui.bpm->onTextChange = [this, i, bpm = ui.bpm.get()]
        {
            engine.setDeckBpm(i, bpm->getText().getDoubleValue());
        };

        ui.info->setFont(DawLookAndFeel::uiFont(11.0f));
        ui.info->setColour(juce::Label::textColourId, DawLookAndFeel::muted);

        ui.load->onClick = [this, i] { chooseFile(i); };
        ui.play->onClick = [this, i] { engine.playDeck(i); };
        ui.stop->onClick = [this, i] { engine.stopDeck(i); };

        for (auto* c : { static_cast<juce::Component*>(ui.load.get()),
                         static_cast<juce::Component*>(ui.play.get()),
                         static_cast<juce::Component*>(ui.stop.get()),
                         static_cast<juce::Component*>(ui.title.get()),
                         static_cast<juce::Component*>(ui.name.get()),
                         static_cast<juce::Component*>(ui.bpm.get()),
                         static_cast<juce::Component*>(ui.info.get()) })
            addAndMakeVisible(c);
    }

    void layoutDeck(int i, juce::Rectangle<int> area)
    {
        auto& ui = decksUi[(size_t) i];
        area = area.reduced(18, 14);

        auto row = area.removeFromTop(24);
        ui.title->setBounds(row.removeFromLeft(140));
        ui.name->setBounds(row);

        area.removeFromTop(10);
        row = area.removeFromTop(30);
        ui.load->setBounds(row.removeFromLeft(110));
        row.removeFromLeft(8);
        ui.play->setBounds(row.removeFromLeft(90));
        row.removeFromLeft(8);
        ui.stop->setBounds(row.removeFromLeft(90));

        area.removeFromTop(10);
        row = area.removeFromTop(28);
        ui.bpm->setBounds(row.removeFromLeft(160));

        area.removeFromTop(6);
        ui.info->setBounds(area.removeFromTop(20));
    }

    void chooseFile(int i)
    {
        auto& ui = decksUi[(size_t) i];
        ui.chooser = std::make_unique<juce::FileChooser>("Open audio file", juce::File{},
                                                         "*.wav;*.aif;*.aiff;*.flac;*.ogg");
        ui.chooser->launchAsync(juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles,
                                [this, i](const juce::FileChooser& fc)
                                {
                                    const auto file = fc.getResult();
                                    if (! file.existsAsFile() || ! engine.loadDeck(i, file))
                                        return;

                                    auto& d = decksUi[(size_t) i];
                                    d.name->setText(file.getFileName(), juce::dontSendNotification);
                                    d.bpm->setText(juce::String(engine.deckBpm(i), 1), juce::dontSendNotification);
                                });
    }

    void syncDecksToProject()
    {
        for (int i = 0; i < AudioEngine::numDecks; ++i)
            if (engine.deckBpm(i) > 0.0)
                engine.setDeckRatio(i, model.bpm / engine.deckBpm(i));
    }

    void matchBToA()
    {
        const double a = engine.deckBpm(0);
        const double b = engine.deckBpm(1);
        if (a > 0.0 && b > 0.0)
            engine.setDeckRatio(1, a / b);
    }

    void timerCallback() override
    {
        for (int i = 0; i < AudioEngine::numDecks; ++i)
        {
            auto& ui = decksUi[(size_t) i];
            const double pos = engine.deckPosition(i);
            const double len = engine.deckLength(i);

            ui.info->setText(formatTime(pos) + " / " + formatTime(len)
                                 + "   ·   " + juce::String(engine.deckRatio(i), 3) + "x",
                             juce::dontSendNotification);
        }
    }

    static juce::String formatTime(double seconds)
    {
        const int totalCentis = (int) juce::roundToInt(juce::jmax(0.0, seconds) * 100.0);
        return juce::String::formatted("%02d:%02d.%02d", totalCentis / 6000, (totalCentis / 100) % 60, totalCentis % 100);
    }

    ProjectModel& model;
    AudioEngine& engine;
    DeckUi decksUi[AudioEngine::numDecks];
    juce::Slider crossfader;
    juce::Label crossfaderLabel;
    juce::TextButton syncButton;
    juce::TextButton matchButton;
    juce::Label tipLabel;
};
