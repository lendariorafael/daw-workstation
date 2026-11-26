#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Note Canvas: editor de notas (equivale ao Piano Roll) do canal selecionado.
// Clicar numa célula cria uma nota de um passo; clicar numa nota existente remove-a.
class NoteCanvasPanel : public juce::Component,
                        private juce::ChangeListener
{
public:
    explicit NoteCanvasPanel(ProjectModel& m) : model(m)
    {
        model.addChangeListener(this);
    }

    ~NoteCanvasPanel() override
    {
        model.removeChangeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);

        const float rowH = rowHeight();
        const float colW = colWidth();

        // Linhas de altura: teclas pretas mais escuras
        for (int p = highestPitch; p >= lowestPitch; --p)
        {
            const int y = headerHeight + juce::roundToInt((highestPitch - p) * rowH);
            const bool black = isBlackKey(p);

            g.setColour(black ? DawLookAndFeel::panel : DawLookAndFeel::panelLight.withAlpha(0.45f));
            g.fillRect(keyWidth, y, getWidth() - keyWidth, juce::roundToInt(rowH));

            g.setColour(black ? juce::Colours::black : juce::Colour((juce::uint32) 0xffd8dde6));
            g.fillRect(0, y, keyWidth - 2, juce::roundToInt(rowH) - 1);

            if (p % 12 == 0)
            {
                g.setColour(DawLookAndFeel::background);
                g.setFont(DawLookAndFeel::uiFont(10.0f, true));
                g.drawText(noteName(p), 4, y, keyWidth - 8, juce::roundToInt(rowH), juce::Justification::centredLeft, false);
            }
        }

        // Grelha de passos
        for (int s = 0; s <= ProjectModel::numSteps; ++s)
        {
            const int x = keyWidth + juce::roundToInt(s * colW);
            g.setColour(DawLookAndFeel::muted.withAlpha((s % 4) == 0 ? 0.45f : 0.15f));
            g.drawVerticalLine(x, (float) headerHeight, (float) getHeight());
        }

        // Cabeçalho
        g.setColour(DawLookAndFeel::panel);
        g.fillRect(0, 0, getWidth(), headerHeight);
        g.setColour(DawLookAndFeel::text);
        g.setFont(DawLookAndFeel::uiFont(13.0f, true));
        g.drawText(title(), 14, 0, getWidth() - 20, headerHeight, juce::Justification::centredLeft, true);

        // Notas do canal selecionado
        if (! model.channels.empty())
        {
            const auto& ch = model.channels[(size_t) model.selectedChannel];

            for (const auto& n : ch.notes)
            {
                if (n.pitch < lowestPitch || n.pitch > highestPitch)
                    continue;

                const auto r = juce::Rectangle<float>(keyWidth + n.step * colW + 1.0f,
                                                      headerHeight + (highestPitch - n.pitch) * rowH + 1.0f,
                                                      n.length * colW - 2.0f,
                                                      rowH - 2.0f);
                g.setColour(ch.colour);
                g.fillRoundedRectangle(r, 3.0f);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (model.channels.empty())
            return;

        const auto p = e.getPosition();
        if (p.y < headerHeight || p.x < keyWidth)
            return;

        const int step = (int) ((p.x - keyWidth) / colWidth());
        const int pitch = highestPitch - (int) ((p.y - headerHeight) / rowHeight());

        if (step < 0 || step >= ProjectModel::numSteps || pitch < lowestPitch || pitch > highestPitch)
            return;

        auto& notes = model.channels[(size_t) model.selectedChannel].notes;

        for (size_t k = 0; k < notes.size(); ++k)
        {
            if (notes[k].step == step && notes[k].pitch == pitch)
            {
                notes.erase(notes.begin() + (std::ptrdiff_t) k);
                model.changed();
                return;
            }
        }

        notes.push_back(Note { step, 1, pitch });
        model.changed();
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        repaint();
    }

    float rowHeight() const
    {
        const int pitches = highestPitch - lowestPitch + 1;
        return (float) (getHeight() - headerHeight) / (float) pitches;
    }

    float colWidth() const
    {
        return (float) (getWidth() - keyWidth) / (float) ProjectModel::numSteps;
    }

    juce::String title() const
    {
        if (model.channels.empty())
            return "Note Canvas";
        return "Note Canvas  ·  " + model.channels[(size_t) model.selectedChannel].name;
    }

    static bool isBlackKey(int pitch)
    {
        const int n = pitch % 12;
        return n == 1 || n == 3 || n == 6 || n == 8 || n == 10;
    }

    static juce::String noteName(int pitch)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return juce::String(names[pitch % 12]) + juce::String(pitch / 12 - 1);
    }

    static constexpr int lowestPitch = 36;
    static constexpr int highestPitch = 83;
    static constexpr int keyWidth = 64;
    static constexpr int headerHeight = 30;

    ProjectModel& model;
};
