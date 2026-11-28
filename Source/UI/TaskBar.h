#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DawLookAndFeel.h"
#include "DesignTokens.h"

// Separador de uma janela aberta na barra inferior.
class TabButton : public juce::Button
{
public:
    explicit TabButton(const juce::String& title) : juce::Button(title) {}

    void paintButton(juce::Graphics& g, bool over, bool) override
    {
        const auto area = getLocalBounds().toFloat().reduced(2.0f, 4.0f);
        const bool active = getToggleState();

        g.setColour(active ? OpaloDesign::surface2 : (over ? OpaloDesign::surface1 : juce::Colours::transparentBlack));
        g.fillRoundedRectangle(area, OpaloDesign::radiusSmall);

        if (active)
        {
            g.setColour(OpaloDesign::accent);
            g.fillRect(area.getX() + 10.0f, area.getBottom() - 2.0f, area.getWidth() - 20.0f, 2.0f);
        }

        g.setColour(active ? OpaloDesign::text : OpaloDesign::textMuted);
        g.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontBody, active));
        g.drawText(getButtonText(), getLocalBounds().reduced(OpaloDesign::space3, 0),
                   juce::Justification::centredLeft, true);
    }
};

// Barra inferior: um separador por janela (minimizada ou aberta), e estado da aplicação à direita.
class TaskBar : public juce::Component
{
public:
    static constexpr int height = 38;

    int addTab(const juce::String& title, std::function<void()> onClick)
    {
        auto b = std::make_unique<TabButton>(title);
        b->setClickingTogglesState(false);
        b->onClick = std::move(onClick);
        addAndMakeVisible(*b);
        tabs.push_back(std::move(b));
        resized();
        return (int) tabs.size() - 1;
    }

    void setTabActive(int index, bool active)
    {
        if (juce::isPositiveAndBelow(index, (int) tabs.size()))
            tabs[(size_t) index]->setToggleState(active, juce::dontSendNotification);
    }

    void setStatus(const juce::String& text)
    {
        status.setText(text, juce::dontSendNotification);
    }

    TaskBar()
    {
        status.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontSmall));
        status.setColour(juce::Label::textColourId, OpaloDesign::textMuted);
        status.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(status);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(OpaloDesign::surface0);
        g.setColour(OpaloDesign::border);
        g.fillRect(0, 0, getWidth(), 1);

        // Ponto verde de atividade.
        g.setColour(OpaloDesign::accent);
        g.fillEllipse(getWidth() - 150.0f, getHeight() * 0.5f - 4.0f, 8.0f, 8.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(OpaloDesign::space2, 0);
        status.setBounds(area.removeFromRight(juce::jmin(190, area.getWidth() / 4)).withTrimmedRight(18));

        const int count = (int) tabs.size();
        const int tabWidth = count > 0 ? juce::jlimit(76, 128, area.getWidth() / count) : 0;
        for (auto& t : tabs)
            t->setBounds(area.removeFromLeft(tabWidth));
    }

private:
    std::vector<std::unique_ptr<TabButton>> tabs;
    juce::Label status;
};
