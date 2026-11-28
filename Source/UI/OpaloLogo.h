#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include "DawLookAndFeel.h"

// Marca Opalo: gema de opala (cabochon) com brilho iridescente e o nome em letras espaçadas.
class OpaloLogo : public juce::Component
{
public:
    // phase: valor livre; a parte fracionária escolhe a cor do brilho.
    void setPhase(float newPhase)
    {
        phase = newPhase - std::floor(newPhase);
        repaint();
    }

    void setWordmarkAlpha(float alpha)
    {
        wordmarkAlpha = alpha;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        const float h = area.getHeight();

        auto gemArea = area.removeFromLeft(h).reduced(h * 0.06f);

        // Corpo da gema com gradiente iridescente.
        juce::Path gem;
        gem.addEllipse(gemArea);

        auto c1 = juce::Colour::fromHSV(phase, 0.55f, 1.0f, 1.0f);
        auto c2 = juce::Colour::fromHSV(std::fmod(phase + 0.33f, 1.0f), 0.50f, 1.0f, 1.0f);
        auto c3 = juce::Colour::fromHSV(std::fmod(phase + 0.66f, 1.0f), 0.60f, 0.95f, 1.0f);

        juce::ColourGradient grad(c1, gemArea.getX(), gemArea.getY(),
                                  c3, gemArea.getRight(), gemArea.getBottom(), false);
        grad.addColour(0.5, c2);
        g.setGradientFill(grad);
        g.fillPath(gem);

        // Reflexo e contorno suave.
        g.setColour(juce::Colours::white.withAlpha(0.55f));
        g.fillEllipse(gemArea.getX() + gemArea.getWidth() * 0.20f,
                      gemArea.getY() + gemArea.getHeight() * 0.14f,
                      gemArea.getWidth() * 0.24f,
                      gemArea.getHeight() * 0.16f);

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.drawEllipse(gemArea, 1.0f);

        // Nome com espaçamento entre letras.
        if (wordmarkAlpha > 0.0f)
        {
            juce::Font font(juce::FontOptions().withName("Georgia")
                                               .withHeight(h * 0.55f)
                                               .withStyle("Bold"));
            g.setFont(font);
            g.setColour(DawLookAndFeel::text.withAlpha(wordmarkAlpha));

            const juce::String word = "Opalo";
            const float spacing = h * 0.06f;
            float x = area.getX() + h * 0.12f;

            for (int i = 0; i < word.length(); ++i)
            {
                auto letter = word.substring(i, i + 1);
                const float w = DawLookAndFeel::textWidth(font, letter);
                g.drawText(letter,
                           juce::Rectangle<float>(x, area.getY(), w + 2.0f, area.getHeight()),
                           juce::Justification::centredLeft,
                           false);
                x += w + spacing;
            }
        }
    }

private:
    float phase = 0.0f;
    float wordmarkAlpha = 1.0f;
};
