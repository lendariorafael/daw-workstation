#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DesignTokens.h"

// Fundo espacial do Opalo: nebulosas, estrelas e um horizonte azulado.
// É desenhado uma vez por tamanho de janela e depois reutilizado (sem custo por frame).
class SpaceBackground
{
public:
    void paint(juce::Graphics& g, juce::Rectangle<int> area)
    {
        if (area.isEmpty())
            return;

        if (cache.isNull() || cache.getWidth() != area.getWidth() || cache.getHeight() != area.getHeight())
            render(area.getWidth(), area.getHeight());

        g.drawImageAt(cache, area.getX(), area.getY());
    }

private:
    void render(int w, int h)
    {
        cache = juce::Image(juce::Image::ARGB, w, h, true);
        juce::Graphics g(cache);

        g.fillAll(juce::Colour((juce::uint32) 0xff05060b));

        const auto fw = (float) w;
        const auto fh = (float) h;

        // Nebulosas: manchas radiais roxas e azuis.
        blob(g, fw * 0.18f, fh * 0.25f, fw * 0.45f, 0xff3a1d6e, 0.55f);
        blob(g, fw * 0.82f, fh * 0.30f, fw * 0.40f, 0xff1d3a7a, 0.45f);
        blob(g, fw * 0.55f, fh * 0.80f, fw * 0.60f, 0xff2a1450, 0.40f);

        // Horizonte de um planeta, em baixo.
        juce::ColourGradient horizon(juce::Colour((juce::uint32) 0x00000000), 0.0f, fh * 0.78f,
                                     juce::Colour((juce::uint32) 0xff2b4f9a), 0.0f, fh, false);
        horizon.addColour(0.6, juce::Colour((juce::uint32) 0x552b4f9a));
        g.setGradientFill(horizon);
        g.fillRect(0.0f, fh * 0.78f, fw, fh * 0.22f);

        // Estrelas com semente fixa, para o fundo ser sempre igual.
        juce::Random rng(7);
        for (int i = 0; i < 420; ++i)
        {
            const float x = rng.nextFloat() * fw;
            const float y = rng.nextFloat() * fh * 0.85f;
            const float r = 0.4f + rng.nextFloat() * 1.3f;
            const float a = 0.25f + rng.nextFloat() * 0.75f;
            g.setColour(juce::Colours::white.withAlpha(a));
            g.fillEllipse(x, y, r, r);
        }
    }

    static void blob(juce::Graphics& g, float cx, float cy, float radius, juce::uint32 argb, float strength)
    {
        juce::ColourGradient grad(juce::Colour(argb).withAlpha(strength), cx, cy,
                                  juce::Colour(argb).withAlpha(0.0f), cx + radius, cy, true);
        g.setGradientFill(grad);
        g.fillRect(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    juce::Image cache;
};
