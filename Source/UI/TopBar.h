#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include "DawLookAndFeel.h"
#include "DesignTokens.h"
#include "OpaloLogo.h"
#include "../I18n/Strings.h"

// Botão de texto simples, para os menus (Ficheiro, Editar, ...).
class MenuButton : public juce::Button
{
public:
    explicit MenuButton(const juce::String& text) : juce::Button(text) {}

    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        if (over || down)
        {
            g.setColour(OpaloDesign::surface2);
            g.fillRoundedRectangle(getLocalBounds().toFloat(), OpaloDesign::radiusSmall);
        }

        g.setColour(OpaloDesign::text.withAlpha(over ? 1.0f : 0.80f));
        g.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontBody));
        g.drawText(getButtonText(), getLocalBounds().reduced(OpaloDesign::space2, 0),
                   juce::Justification::centred, false);
    }
};

// Botão de transporte com glifo desenhado à mão (play, pausa, stop, anterior, seguinte, loop, gravar).
class IconButton : public juce::Button
{
public:
    enum class Kind { Play, Pause, Stop, Previous, Next, Loop, Record };

    IconButton(Kind k, const juce::String& tip) : juce::Button(tip), kind(k)
    {
        setTooltip(tip);
    }

    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        const auto area = getLocalBounds().toFloat();

        if (over && isEnabled())
        {
            g.setColour(down ? OpaloDesign::surface2.brighter(0.06f) : OpaloDesign::surface2);
            g.fillRoundedRectangle(area, OpaloDesign::radiusSmall);
        }

        juce::Colour c = OpaloDesign::text;
        if (kind == Kind::Record)                     c = OpaloDesign::danger;
        if (kind == Kind::Loop && getToggleState())   c = OpaloDesign::accent;
        if (! isEnabled())                            c = OpaloDesign::textMuted.withAlpha(0.35f);

        g.setColour(c);

        const auto r = area.reduced(area.getWidth() * 0.28f);
        const float cx = r.getCentreX();
        const float cy = r.getCentreY();
        const float s = juce::jmin(r.getWidth(), r.getHeight());

        switch (kind)
        {
            case Kind::Play:
            {
                juce::Path p;
                p.addTriangle(cx - s * 0.30f, cy - s * 0.50f, cx - s * 0.30f, cy + s * 0.50f, cx + s * 0.50f, cy);
                g.fillPath(p);
                break;
            }
            case Kind::Pause:
                g.fillRoundedRectangle(cx - s * 0.46f, cy - s * 0.46f, s * 0.30f, s * 0.92f, 1.5f);
                g.fillRoundedRectangle(cx + s * 0.16f, cy - s * 0.46f, s * 0.30f, s * 0.92f, 1.5f);
                break;
            case Kind::Stop:
                g.fillRoundedRectangle(cx - s * 0.42f, cy - s * 0.42f, s * 0.84f, s * 0.84f, 2.0f);
                break;
            case Kind::Previous:
            {
                g.fillRoundedRectangle(cx - s * 0.50f, cy - s * 0.45f, s * 0.14f, s * 0.90f, 1.0f);
                juce::Path p;
                p.addTriangle(cx + s * 0.40f, cy - s * 0.45f, cx + s * 0.40f, cy + s * 0.45f, cx - s * 0.20f, cy);
                g.fillPath(p);
                break;
            }
            case Kind::Next:
            {
                g.fillRoundedRectangle(cx + s * 0.36f, cy - s * 0.45f, s * 0.14f, s * 0.90f, 1.0f);
                juce::Path p;
                p.addTriangle(cx - s * 0.40f, cy - s * 0.45f, cx - s * 0.40f, cy + s * 0.45f, cx + s * 0.20f, cy);
                g.fillPath(p);
                break;
            }
            case Kind::Loop:
                g.drawRoundedRectangle(cx - s * 0.46f, cy - s * 0.32f, s * 0.92f, s * 0.64f, s * 0.22f, 1.8f);
                break;
            case Kind::Record:
                g.fillEllipse(cx - s * 0.36f, cy - s * 0.36f, s * 0.72f, s * 0.72f);
                break;
        }
    }

private:
    Kind kind;
};

// Barra superior: logótipo, menus, transporte, BPM, contador de tempo e uso de CPU.
class TopBar : public juce::Component
{
public:
    std::function<void(const juce::String&)> onMenu;
    std::function<void()> onPlay, onPause, onStop, onPrevious, onNext;
    std::function<void(bool)> onLoop;
    std::function<void(double)> onBpm;

    static constexpr int height = 92;

    TopBar()
    {
        addAndMakeVisible(logo);

        for (const char* key : menuKeys())
        {
            auto b = std::make_unique<MenuButton>(I18n::t(key));
            const juce::String k(key);
            b->onClick = [this, k] { if (onMenu) onMenu(k); };
            addAndMakeVisible(*b);
            menus.push_back(std::move(b));
        }

        recordButton = std::make_unique<IconButton>(IconButton::Kind::Record, I18n::t("tab.record"));
        recordButton->setEnabled(false);
        previousButton = std::make_unique<IconButton>(IconButton::Kind::Previous, "");
        playButton = std::make_unique<IconButton>(IconButton::Kind::Play, I18n::t("transport.play"));
        pauseButton = std::make_unique<IconButton>(IconButton::Kind::Pause, "Pause");
        stopButton = std::make_unique<IconButton>(IconButton::Kind::Stop, I18n::t("transport.stop"));
        nextButton = std::make_unique<IconButton>(IconButton::Kind::Next, "");
        loopButton = std::make_unique<IconButton>(IconButton::Kind::Loop, "Loop");
        loopButton->setClickingTogglesState(true);

        previousButton->onClick = [this] { if (onPrevious) onPrevious(); };
        playButton->onClick     = [this] { if (onPlay) onPlay(); };
        pauseButton->onClick    = [this] { if (onPause) onPause(); };
        stopButton->onClick     = [this] { if (onStop) onStop(); };
        nextButton->onClick     = [this] { if (onNext) onNext(); };
        loopButton->onClick     = [this] { if (onLoop) onLoop(loopButton->getToggleState()); };

        for (auto* b : { recordButton.get(), previousButton.get(), playButton.get(), pauseButton.get(),
                         stopButton.get(), nextButton.get(), loopButton.get() })
            addAndMakeVisible(*b);

        bpmLabel.setFont(DawLookAndFeel::uiFont(18.0f, true));
        bpmLabel.setJustificationType(juce::Justification::centred);
        bpmLabel.setEditable(true);
        bpmLabel.setColour(juce::Label::backgroundColourId, OpaloDesign::surface1);
        bpmLabel.setColour(juce::Label::outlineColourId, OpaloDesign::border);
        bpmLabel.onTextChange = [this]
        {
            const double v = juce::jlimit(20.0, 300.0, bpmLabel.getText().getDoubleValue());
            if (onBpm) onBpm(v);
        };
        addAndMakeVisible(bpmLabel);

        timeLabel.setFont(DawLookAndFeel::uiFont(26.0f, true));
        timeLabel.setJustificationType(juce::Justification::centredLeft);
        timeLabel.setColour(juce::Label::textColourId, OpaloDesign::text);
        addAndMakeVisible(timeLabel);

        cpuLabel.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontSmall));
        cpuLabel.setJustificationType(juce::Justification::centredRight);
        cpuLabel.setColour(juce::Label::textColourId, OpaloDesign::textMuted);
        addAndMakeVisible(cpuLabel);

        update(false, false, 0.0, 120.0, 0.0);
    }

    // Atualiza os nomes dos menus quando o idioma muda.
    void refreshTexts()
    {
        const auto keys = menuKeys();
        for (size_t i = 0; i < menus.size() && i < keys.size(); ++i)
            menus[i]->setButtonText(I18n::t(keys[i]));
        resized();
    }

    void update(bool playing, bool looping, double seconds, double bpm, double cpuPercent)
    {
        playButton->setToggleState(playing, juce::dontSendNotification);
        loopButton->setToggleState(looping, juce::dontSendNotification);

        timeLabel.setText(formatTime(seconds), juce::dontSendNotification);

        if (std::abs(bpmLabel.getText().getDoubleValue() - bpm) > 0.005)
            bpmLabel.setText(juce::String(bpm, 2) + " BPM", juce::dontSendNotification);

        cpuLabel.setText("CPU " + juce::String((int) std::round(cpuPercent)) + "%", juce::dontSendNotification);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(OpaloDesign::surface0);
        g.setColour(OpaloDesign::border);
        g.fillRect(0, height - 1, getWidth(), 1);
    }

    void resized() override
    {
        auto top = getLocalBounds().removeFromTop(38).reduced(OpaloDesign::space3, 4);
        logo.setBounds(top.removeFromLeft(150));
        top.removeFromLeft(OpaloDesign::space3);

        for (auto& m : menus)
        {
            const int w = juce::jmax(56, (int) DawLookAndFeel::textWidth(DawLookAndFeel::uiFont(OpaloDesign::fontBody), m->getButtonText()) + 24);
            m->setBounds(top.removeFromLeft(w));
        }

        auto row = getLocalBounds().withTrimmedTop(40).reduced(OpaloDesign::space3, 0);

        cpuLabel.setBounds(row.removeFromRight(120));
        row.removeFromRight(OpaloDesign::space3);

        const int btn = 36;
        recordButton->setBounds(row.removeFromLeft(btn));
        previousButton->setBounds(row.removeFromLeft(btn));
        playButton->setBounds(row.removeFromLeft(btn));
        pauseButton->setBounds(row.removeFromLeft(btn));
        stopButton->setBounds(row.removeFromLeft(btn));
        nextButton->setBounds(row.removeFromLeft(btn));
        loopButton->setBounds(row.removeFromLeft(btn));
        row.removeFromLeft(OpaloDesign::space3);

        bpmLabel.setBounds(row.removeFromLeft(170).reduced(0, 10));
        row.removeFromLeft(OpaloDesign::space3);

        timeLabel.setBounds(row.removeFromLeft(190));
    }

private:
    static std::array<const char*, 8> menuKeys()
    {
        return { "menu.file", "menu.edit", "menu.add", "menu.patterns",
                 "menu.view", "menu.options", "menu.tools", "menu.help" };
    }

    static juce::String formatTime(double seconds)
    {
        const int totalCentis = (int) juce::roundToInt(juce::jmax(0.0, seconds) * 100.0);
        const int minutes = totalCentis / 6000;
        const int secs = (totalCentis / 100) % 60;
        const int centis = totalCentis % 100;
        return juce::String::formatted("%d:%02d:%02d", minutes, secs, centis);
    }

    OpaloLogo logo;
    std::vector<std::unique_ptr<MenuButton>> menus;
    std::unique_ptr<IconButton> recordButton, previousButton, playButton, pauseButton, stopButton, nextButton, loopButton;
    juce::Label bpmLabel;
    juce::Label timeLabel;
    juce::Label cpuLabel;
};
