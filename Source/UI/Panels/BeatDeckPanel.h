#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Beat Deck: grelha de passos por canal (equivale ao Channel Rack).
// Só desenha as linhas visíveis, por isso aguenta os 1000 canais sem perder fluidez.
class BeatDeckPanel : public juce::Component,
                      private juce::ChangeListener
{
public:
    explicit BeatDeckPanel(ProjectModel& m) : model(m)
    {
        model.addChangeListener(this);

        addAndMakeVisible(addButton);
        addButton.onClick = [this]
        {
            const int n = (int) model.channels.size() + 1;
            model.addChannel("Channel " + juce::String(n), ProjectModel::paletteColour(n - 1));
        };
    }

    ~BeatDeckPanel() override
    {
        model.removeChangeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);

        g.setColour(DawLookAndFeel::muted);
        g.setFont(DawLookAndFeel::uiFont(11.0f, true));
        g.drawText("TRACK", 16, 0, nameWidth, headerHeight, juce::Justification::centredLeft, false);
        g.drawText("PATTERN STEPS", nameWidth + 12, 0, 200, headerHeight, juce::Justification::centredLeft, false);

        const int channels = (int) model.channels.size();
        const int first = juce::jlimit(0, channels, (scrollY / rowHeight));
        const int last = juce::jlimit(0, channels, (scrollY + getHeight()) / rowHeight + 2);

        for (int i = first; i < last; ++i)
        {
            const auto& ch = model.channels[(size_t) i];
            const auto row = rowBounds(i);

            g.setColour(i == model.selectedChannel ? DawLookAndFeel::panelLight : DawLookAndFeel::panel);
            g.fillRect(row.reduced(0, 1));

            g.setColour(ch.colour);
            g.fillRect(row.withWidth(4));

            g.setColour(DawLookAndFeel::text);
            g.setFont(DawLookAndFeel::uiFont(14.0f, true));
            g.drawText(ch.name, row.getX() + 14, row.getY(), nameWidth - 120, row.getHeight(),
                       juce::Justification::centredLeft, true);

            drawToggle(g, muteRect(i), "M", ch.muted, ch.colour);
            drawToggle(g, soloRect(i), "S", ch.soloed, ch.colour);

            for (int s = 0; s < ProjectModel::numSteps; ++s)
            {
                const auto cell = stepRect(i, s).toFloat();
                const bool on = ch.steps[(size_t) s];
                const bool beat = (s % 4) == 0;

                if (on)
                    g.setColour(ch.colour);
                else
                    g.setColour(beat ? DawLookAndFeel::panelLight.brighter(0.10f)
                                     : DawLookAndFeel::panel.brighter(0.04f));

                g.fillRoundedRectangle(cell, 4.0f);
            }
        }
    }

    void resized() override
    {
        addButton.setBounds(getWidth() - 140, 4, 128, 24);
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        scrollBy(-(int) (wheel.deltaY * rowHeight * 3.0f));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const auto p = e.getPosition();
        if (p.y < headerHeight)
            return;

        const int i = (p.y - headerHeight + scrollY) / rowHeight;
        if (i < 0 || i >= (int) model.channels.size())
            return;

        auto& ch = model.channels[(size_t) i];

        if (p.x < nameWidth)
        {
            if (muteRect(i).contains(p))       ch.muted = ! ch.muted;
            else if (soloRect(i).contains(p))  ch.soloed = ! ch.soloed;
            else                               model.selectedChannel = i;
        }
        else
        {
            for (int s = 0; s < ProjectModel::numSteps; ++s)
            {
                if (stepRect(i, s).contains(p))
                {
                    ch.steps[(size_t) s] = ! ch.steps[(size_t) s];
                    break;
                }
            }
            model.selectedChannel = i;
        }

        model.changed();
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        scrollBy(0);
        repaint();
    }

    void scrollBy(int delta)
    {
        const int contentHeight = headerHeight + (int) model.channels.size() * rowHeight;
        const int maxScroll = juce::jmax(0, contentHeight - getHeight());
        scrollY = juce::jlimit(0, maxScroll, scrollY + delta);
        repaint();
    }

    static void drawToggle(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& label,
                           bool active, juce::Colour c)
    {
        g.setColour(active ? c : DawLookAndFeel::panelLight);
        g.fillRoundedRectangle(r.toFloat(), 4.0f);
        g.setColour(active ? DawLookAndFeel::background : DawLookAndFeel::muted);
        g.setFont(DawLookAndFeel::uiFont(11.0f, true));
        g.drawText(label, r, juce::Justification::centred, false);
    }

    static constexpr int headerHeight = 32;
    static constexpr int rowHeight = 44;
    static constexpr int nameWidth = 240;
    static constexpr int stepGap = 6;

    juce::Rectangle<int> rowBounds(int i) const
    {
        return { 0, headerHeight + i * rowHeight - scrollY, getWidth(), rowHeight };
    }

    juce::Rectangle<int> muteRect(int i) const
    {
        const auto r = rowBounds(i);
        return { nameWidth - 92, r.getY() + 12, 26, 20 };
    }

    juce::Rectangle<int> soloRect(int i) const
    {
        const auto r = rowBounds(i);
        return { nameWidth - 60, r.getY() + 12, 26, 20 };
    }

    juce::Rectangle<int> stepRect(int i, int s) const
    {
        const auto r = rowBounds(i);
        const int gridX = nameWidth + 12;
        const float gridW = (float) (getWidth() - gridX - 12);
        const float cellW = (gridW - (ProjectModel::numSteps - 1) * stepGap) / (float) ProjectModel::numSteps;
        const int x = gridX + juce::roundToInt(s * (cellW + stepGap));
        return { x, r.getY() + 8, juce::roundToInt(cellW), rowHeight - 16 };
    }

    ProjectModel& model;
    int scrollY = 0;
    juce::TextButton addButton { "+ Track" };
};
