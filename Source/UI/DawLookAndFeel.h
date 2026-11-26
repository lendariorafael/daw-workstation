#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DesignTokens.h"

// Look and feel do Opalo: botões, faders, caixas de seleção e menus com estados
// de hover, pressionado, ativo e desativado, todos com as mesmas cores e raios.
class DawLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Nomes curtos usados pelos painéis (mantidos para não quebrar o código existente).
    static inline const juce::Colour background = OpaloDesign::background;
    static inline const juce::Colour panel      = OpaloDesign::surface0;
    static inline const juce::Colour panelLight = OpaloDesign::surface1;
    static inline const juce::Colour accent     = OpaloDesign::accent;
    static inline const juce::Colour text       = OpaloDesign::text;
    static inline const juce::Colour muted      = OpaloDesign::textMuted;

    static juce::Font uiFont(float height, bool bold = false)
    {
        return juce::Font(juce::FontOptions().withHeight(height).withStyle(bold ? "Bold" : "Regular"));
    }

    DawLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, OpaloDesign::background);
        setColour(juce::DocumentWindow::backgroundColourId, OpaloDesign::surface0);
        setColour(juce::DocumentWindow::textColourId, OpaloDesign::text);

        setColour(juce::Label::textColourId, OpaloDesign::text);
        setColour(juce::Label::outlineColourId, OpaloDesign::border);

        setColour(juce::Slider::thumbColourId, OpaloDesign::accent);
        setColour(juce::Slider::trackColourId, OpaloDesign::accentDim);
        setColour(juce::Slider::backgroundColourId, OpaloDesign::surface2);
        setColour(juce::Slider::textBoxTextColourId, OpaloDesign::text);
        setColour(juce::Slider::textBoxBackgroundColourId, OpaloDesign::surface1);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxHighlightColourId, OpaloDesign::accentDim);

        setColour(juce::ComboBox::backgroundColourId, OpaloDesign::surface1);
        setColour(juce::ComboBox::textColourId, OpaloDesign::text);
        setColour(juce::ComboBox::outlineColourId, OpaloDesign::border);
        setColour(juce::ComboBox::arrowColourId, OpaloDesign::textMuted);

        setColour(juce::PopupMenu::backgroundColourId, OpaloDesign::surface0);
        setColour(juce::PopupMenu::textColourId, OpaloDesign::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, OpaloDesign::accentDim);
        setColour(juce::PopupMenu::highlightedTextColourId, OpaloDesign::background);
        setColour(juce::PopupMenu::headerTextColourId, OpaloDesign::textMuted);

        setColour(juce::TooltipWindow::backgroundColourId, OpaloDesign::surface2);
        setColour(juce::TooltipWindow::textColourId, OpaloDesign::text);
        setColour(juce::TooltipWindow::outlineColourId, OpaloDesign::border);

        setColour(juce::ListBox::backgroundColourId, OpaloDesign::surface0);
        setColour(juce::ListBox::outlineColourId, OpaloDesign::border);

        setColour(juce::ScrollBar::thumbColourId, OpaloDesign::surface2);
        setColour(juce::ScrollBar::trackColourId, OpaloDesign::surface0);

        setColour(juce::FileBrowserComponent::currentPathBoxBackgroundColourId, OpaloDesign::surface1);
        setColour(juce::FileBrowserComponent::filenameBoxBackgroundColourId, OpaloDesign::surface1);
    }

    // Botões: normal, hover, pressionado, ativo (toggle) e desativado.
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&,
                              bool isMouseOver, bool isButtonDown) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(0.5f);
        const bool on = b.getToggleState();

        juce::Colour fill = on ? OpaloDesign::accentDim : OpaloDesign::surface1;
        if (isButtonDown)       fill = fill.darker(0.18f);
        else if (isMouseOver)   fill = fill.brighter(0.10f);
        if (! b.isEnabled())    fill = fill.withAlpha(0.40f);

        g.setColour(fill);
        g.fillRoundedRectangle(r, OpaloDesign::radiusSmall);

        g.setColour(on ? OpaloDesign::accent : OpaloDesign::border);
        g.drawRoundedRectangle(r, OpaloDesign::radiusSmall, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        const bool on = b.getToggleState();
        juce::Colour c = on ? OpaloDesign::background : OpaloDesign::text;
        if (! b.isEnabled())
            c = c.withAlpha(0.45f);

        g.setColour(c);
        g.setFont(uiFont(OpaloDesign::fontSmall + 1.0f, true));
        g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(OpaloDesign::space2, 0),
                         juce::Justification::centred, 1);
    }

    // Faders: trilho com preenchimento em acento, polegar em forma de barra.
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        if (style == juce::Slider::LinearHorizontal)
        {
            const float cy = (float) y + height * 0.5f;
            auto track = juce::Rectangle<float>((float) x, cy - 2.0f, (float) width, 4.0f);

            g.setColour(OpaloDesign::surface2);
            g.fillRoundedRectangle(track, 2.0f);

            g.setColour(OpaloDesign::accent);
            g.fillRoundedRectangle(track.withRight(sliderPos), 2.0f);

            g.setColour(s.isMouseOverOrDragging() ? OpaloDesign::accent : OpaloDesign::text);
            g.fillEllipse(sliderPos - 5.0f, cy - 5.0f, 10.0f, 10.0f);
            return;
        }

        if (style == juce::Slider::LinearVertical)
        {
            const float cx = (float) x + width * 0.5f;
            auto track = juce::Rectangle<float>(cx - 2.0f, (float) y, 4.0f, (float) height);

            g.setColour(OpaloDesign::surface2);
            g.fillRoundedRectangle(track, 2.0f);

            g.setColour(OpaloDesign::accent);
            g.fillRoundedRectangle(track.withTop(sliderPos), 2.0f);

            g.setColour(s.isMouseOverOrDragging() ? OpaloDesign::accent : OpaloDesign::text);
            g.fillRoundedRectangle(cx - 12.0f, sliderPos - 3.0f, 24.0f, 6.0f, 2.0f);
            return;
        }

        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, s);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float>(0.0f, 0.0f, (float) width, (float) height).reduced(0.5f);

        g.setColour(isButtonDown ? OpaloDesign::surface2 : OpaloDesign::surface1);
        g.fillRoundedRectangle(r, OpaloDesign::radiusSmall);

        g.setColour(box.hasKeyboardFocus(true) ? OpaloDesign::accent : OpaloDesign::border);
        g.drawRoundedRectangle(r, OpaloDesign::radiusSmall, 1.0f);

        // Seta em forma de chevron, desenhada à mão para ficar nítida em qualquer escala.
        const float ax = width - 14.0f;
        const float ay = height * 0.5f;
        juce::Path chevron;
        chevron.startNewSubPath(ax - 4.0f, ay - 2.0f);
        chevron.lineTo(ax, ay + 2.0f);
        chevron.lineTo(ax + 4.0f, ay - 2.0f);
        g.setColour(OpaloDesign::textMuted);
        g.strokePath(chevron, juce::PathStrokeType(1.5f));
    }

    // Barra de título do Opalo: fundo em camada, título centrado e uma linha de acento por baixo.
    void drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& g,
                                    int width, int height, int titleSpaceX, int titleSpaceWidth,
                                    const juce::Image*, bool) override
    {
        g.fillAll(OpaloDesign::surface0);

        g.setColour(OpaloDesign::accent.withAlpha(0.30f));
        g.fillRect(0, height - 1, width, 1);

        g.setColour(OpaloDesign::text);
        g.setFont(uiFont(OpaloDesign::fontBody, true));
        g.drawText(window.getName(), titleSpaceX, 0, titleSpaceWidth, height,
                   juce::Justification::centred, true);
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        auto r = juce::Rectangle<float>(0.0f, 0.0f, (float) width, (float) height).reduced(0.5f);
        g.setColour(OpaloDesign::surface2);
        g.fillRoundedRectangle(r, OpaloDesign::radiusSmall);
        g.setColour(OpaloDesign::border);
        g.drawRoundedRectangle(r, OpaloDesign::radiusSmall, 1.0f);
        g.setColour(OpaloDesign::text);
        g.setFont(uiFont(OpaloDesign::fontSmall));
        g.drawFittedText(text, juce::Rectangle<int>(0, 0, width, height).reduced(OpaloDesign::space2, 4),
                         juce::Justification::centredLeft, 3);
    }
};
