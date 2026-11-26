#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"
#include "../../I18n/Strings.h"

// Playlist (Timeline): faixas por canal, clips de 1 compasso, grelha por compasso e cursor.
// Clicar numa célula cria ou remove um clip. Passar o rato destaca a célula.
class TimelinePanel : public juce::Component,
                      private juce::ChangeListener,
                      private juce::Timer
{
public:
    explicit TimelinePanel(ProjectModel& m) : model(m)
    {
        model.addChangeListener(this);
        setWantsKeyboardFocus(true);
        startTimerHz(30);
    }

    enum class Tool { Select, Cut, Pencil, Brush, Copy };

    bool keyPressed(const juce::KeyPress& key) override
    {
        const auto c = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
        if (c == 'c') { setTool(Tool::Cut); return true; }
        if (c == 'd') { setTool(Tool::Select); return true; }
        if (c == 'p') { setTool(Tool::Pencil); return true; }
        if (c == 'b') { setTool(Tool::Brush); return true; }
        if (c == 'x') { setTool(Tool::Copy); return true; }
        return false;
    }

    void setTool(Tool t)
    {
        tool = t;
        repaint();
    }

    ~TimelinePanel() override
    {
        stopTimer();
        model.removeChangeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);
        const float barW = barWidth();

        // Cabeçalho (régua) com gradiente suave
        juce::ColourGradient rulerGrad(DawLookAndFeel::panelLight, 0.0f, 0.0f,
                                       DawLookAndFeel::panel, 0.0f, (float) rulerHeight, false);
        g.setGradientFill(rulerGrad);
        g.fillRect(0, 0, getWidth(), rulerHeight);

        g.setFont(DawLookAndFeel::uiFont(11.0f));
        for (int b = 0; b < totalBars; ++b)
        {
            const int x = nameWidth + juce::roundToInt(b * barW);
            const bool strong = (b % 4) == 0;

            // Divisórias verticais tracejadas no estilo de uma grelha de arranjo profissional.
            g.setColour(DawLookAndFeel::muted.withAlpha(strong ? 0.52f : 0.27f));
            const float dashTop = (float) rulerHeight;
            const float dashBottom = (float) getHeight();
            for (float y = dashTop; y < dashBottom; y += 7.0f)
                g.drawLine((float) x, y, (float) x, juce::jmin(y + 3.0f, dashBottom), strong ? 1.25f : 0.8f);

            g.setColour(strong ? DawLookAndFeel::text : DawLookAndFeel::muted);
            g.drawText(juce::String(b + 1), x + 5, 0, 40, rulerHeight, juce::Justification::centredLeft, false);
        }

        // Faixas visíveis (zebra + destaque da seleção e do hover)
        const int channels = (int) model.channels.size();
        const int first = juce::jlimit(0, channels, scrollY / rowHeight);
        const int last = juce::jlimit(0, channels, (scrollY + getHeight()) / rowHeight + 2);

        for (int i = first; i < last; ++i)
        {
            const auto& ch = model.channels[(size_t) i];
            const int y = rowY(i);

            juce::Colour rowColour = (i % 2 == 0) ? DawLookAndFeel::panel : DawLookAndFeel::background.brighter(0.03f);
            if (i == model.selectedChannel)
                rowColour = DawLookAndFeel::panelLight;
            g.setColour(rowColour);
            g.fillRect(0, y, getWidth(), rowHeight);

            if (i == hoverRow)
            {
                g.setColour(DawLookAndFeel::text.withAlpha(0.04f));
                g.fillRect(nameWidth, y, getWidth() - nameWidth, rowHeight);
            }

            // Barra de cor à esquerda e nome do canal
            g.setColour(ch.colour);
            g.fillRoundedRectangle(juce::Rectangle<float>(4.0f, (float) (y + 6), 4.0f, (float) (rowHeight - 12)), 2.0f);

            g.setColour(DawLookAndFeel::text);
            g.setFont(DawLookAndFeel::uiFont(13.0f, true));
            g.drawText(ch.name, 16, y, nameWidth - 24, rowHeight, juce::Justification::centredLeft, true);
        }

        // Clips com contorno e nome
        for (const auto& c : model.clips)
        {
            if (c.channel < first || c.channel >= last)
                continue;

            const auto& ch = model.channels[(size_t) c.channel];
            auto r = juce::Rectangle<float>(nameWidth + c.startBar * barW + 3.0f,
                                            (float) (rowY(c.channel) + 6),
                                            c.lengthBars * barW - 6.0f,
                                            (float) (rowHeight - 12));

            g.setColour(ch.colour.withAlpha(0.22f));
            g.fillRoundedRectangle(r, 6.0f);
            g.setColour(ch.colour.withAlpha(0.9f));
            g.drawRoundedRectangle(r, 6.0f, 1.0f);

            g.setColour(ch.colour);
            g.setFont(DawLookAndFeel::uiFont(10.0f, true));
            g.drawText(ch.name, r.toNearestInt().reduced(8, 0), juce::Justification::centredLeft, true);
        }

        // Botão da ferramenta (Selecionar / Cortar)
        const auto tb = toolButtonBounds();
        static const char* toolLabels[] = { "SEL", "SLICE", "PENCIL", "BRUSH", "COPY" };
        const int segmentWidth = tb.getWidth() / 5;
        for (int i = 0; i < 5; ++i)
        {
            auto segment = tb.withX(tb.getX() + i * segmentWidth).withWidth(
                i == 4 ? tb.getWidth() - i * segmentWidth : segmentWidth);
            const bool active = i == static_cast<int>(tool);
            g.setColour(active ? DawLookAndFeel::accent : DawLookAndFeel::panelLight);
            g.fillRoundedRectangle(segment.toFloat().reduced(1.0f), 4.0f);
            g.setColour(active ? DawLookAndFeel::background : DawLookAndFeel::text);
            g.setFont(DawLookAndFeel::uiFont(9.5f, true));
            g.drawText(toolLabels[i], segment, juce::Justification::centred, false);
        }

        // Cursor de reprodução
        const float px = nameWidth + (float) (model.playheadSeconds / model.secondsPerBar()) * barW;
        if (px >= nameWidth && px <= (float) getWidth())
        {
            g.setColour(DawLookAndFeel::accent);
            g.drawVerticalLine(juce::roundToInt(px), 0.0f, (float) getHeight());
        }
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        const auto p = e.getPosition();
        const int row = p.y < rulerHeight ? -1 : (p.y - rulerHeight + scrollY) / rowHeight;
        if (row != hoverRow)
        {
            hoverRow = row;
            repaint();
        }
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        hoverRow = -1;
        repaint();
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        scrollBy(-(int) (wheel.deltaY * rowHeight * 3.0f));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        grabKeyboardFocus();
        const auto p = e.getPosition();

        // Botão da ferramenta no canto superior direito da régua.
        if (toolButtonBounds().contains(p))
        {
            const auto bounds = toolButtonBounds();
            const int selected = juce::jlimit(0, 4, (p.x - bounds.getX()) * 5 / juce::jmax(1, bounds.getWidth()));
            setTool(static_cast<Tool>(selected));
            return;
        }

        if (p.y < rulerHeight)
            return;

        const int row = (p.y - rulerHeight + scrollY) / rowHeight;
        if (row < 0 || row >= (int) model.channels.size())
            return;

        if (e.mods.isPopupMenu())
        {
            model.selectedChannel = row;
            juce::PopupMenu menu;
            menu.addSectionHeader(model.channels[(size_t) row].name);
            juce::PopupMenu colours;
            static const juce::uint32 palette[] = { 0xff00d1b2, 0xff7aa2ff, 0xffffb86b, 0xffc792ea,
                                                    0xfff07178, 0xff89ddff, 0xffc3e88d, 0xffffcb6b,
                                                    0xffe6a8d7, 0xff9aa5b1, 0xffd6d0c4, 0xff8bc5a8 };
            for (int i = 0; i < 12; ++i)
                colours.addItem(i + 1, "Colour " + juce::String(i + 1));
            menu.addSubMenu("Track colour", colours);
            menu.addItem(100, "Open in note editor");
            juce::Component::SafePointer<TimelinePanel> safe(this);
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                [safe, row](int result)
                {
                    auto* self = safe.getComponent();
                    if (self == nullptr) return;
                    if (result >= 1 && result <= 12)
                    {
                        self->model.channels[(size_t) row].colour = juce::Colour(palette[result - 1]);
                        self->model.changed();
                    }
                    else if (result == 100)
                    {
                        self->model.selectedChannel = row;
                        self->model.changed();
                    }
                });
            return;
        }

        model.selectedChannel = row;

        if (p.x < nameWidth)
        {
            model.changed();
            return;
        }

        if (tool == Tool::Cut)
        {
            cutAt(row, p.x);
            return;
        }

        const int bar = juce::jlimit(0, totalBars - 1, (int) ((p.x - nameWidth) / barWidth()));
        if (tool == Tool::Select)
            return;

        if (tool == Tool::Copy)
        {
            for (const auto& clip : model.clips)
                if (clip.channel == row && bar >= clip.startBar && bar < clip.startBar + clip.lengthBars)
                {
                    const int destination = clip.startBar + clip.lengthBars;
                    if (destination < totalBars)
                    {
                        model.clips.push_back(Clip { row, destination, clip.lengthBars });
                        model.changed();
                    }
                    return;
                }
            return;
        }

        addClipAt(row, bar);
        lastPaintRow = row;
        lastPaintBar = bar;
    }

private:
    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto position = e.getPosition();
        if (tool != Tool::Brush || position.y < rulerHeight || position.x < nameWidth)
            return;

        const int row = (position.y - rulerHeight + scrollY) / rowHeight;
        if (row < 0 || row >= (int) model.channels.size())
            return;
        const int bar = juce::jlimit(0, totalBars - 1, (int) ((position.x - nameWidth) / barWidth()));
        if (row != lastPaintRow || bar != lastPaintBar)
        {
            addClipAt(row, bar);
            lastPaintRow = row;
            lastPaintBar = bar;
        }
    }

    void addClipAt(int row, int bar)
    {
        for (size_t k = 0; k < model.clips.size(); ++k)
            if (model.clips[k].channel == row && model.clips[k].startBar == bar)
                return;

        model.clips.push_back(Clip { row, bar, 1 });
        model.changed();
    }

    // Corta o clip sob o cursor no compasso mais próximo. Só corta dentro de um clip.
    void cutAt(int row, int x)
    {
        const float exact = (x - nameWidth) / barWidth();
        const int boundary = juce::roundToInt(exact);

        for (size_t k = 0; k < model.clips.size(); ++k)
        {
            auto& c = model.clips[k];
            if (c.channel != row || boundary <= c.startBar || boundary >= c.startBar + c.lengthBars)
                continue;

            Clip right = c;
            right.startBar = boundary;
            right.lengthBars = c.startBar + c.lengthBars - boundary;

            c.lengthBars = boundary - c.startBar;
            model.clips.push_back(right);
            model.changed();
            return;
        }
    }

    juce::Rectangle<int> toolButtonBounds() const
    {
        const int width = juce::jmin(280, juce::jmax(120, getWidth() - 16));
        return { getWidth() - width - 8, 4, width, rulerHeight - 8 };
    }

    // Ícone de lâmina, desenhado de propósito para ser diferente das tesouras de outros programas:
    // uma lâmina diagonal com um cabo circular e uma linha de corte pontilhada.
    static void drawBladeIcon(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour colour)
    {
        g.setColour(colour);

        juce::Path blade;
        blade.startNewSubPath(r.getX() + r.getWidth() * 0.18f, r.getBottom() - r.getHeight() * 0.18f);
        blade.lineTo(r.getRight() - r.getWidth() * 0.12f, r.getY() + r.getHeight() * 0.22f);
        blade.lineTo(r.getRight() - r.getWidth() * 0.02f, r.getY() + r.getHeight() * 0.36f);
        blade.lineTo(r.getX() + r.getWidth() * 0.30f, r.getBottom() - r.getHeight() * 0.04f);
        blade.closeSubPath();
        g.fillPath(blade);

        g.drawEllipse(r.getX(), r.getBottom() - r.getHeight() * 0.42f, r.getWidth() * 0.34f, r.getHeight() * 0.34f, 1.6f);

        g.setColour(colour.withAlpha(0.5f));
        for (int d = 0; d < 3; ++d)
            g.fillEllipse(r.getX() + r.getWidth() * (0.46f + 0.12f * d), r.getY() + r.getHeight() * 0.62f, 2.0f, 2.0f);
    }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        repaint();
    }

    void timerCallback() override
    {
        repaint();
    }

    void scrollBy(int delta)
    {
        const int contentHeight = (int) model.channels.size() * rowHeight;
        const int maxScroll = juce::jmax(0, contentHeight - (getHeight() - rulerHeight));
        scrollY = juce::jlimit(0, maxScroll, scrollY + delta);
        repaint();
    }

    int rowY(int i) const
    {
        return rulerHeight + i * rowHeight - scrollY;
    }

    float barWidth() const
    {
        return (float) (getWidth() - nameWidth) / (float) totalBars;
    }

    static constexpr int totalBars = 32;
    static constexpr int nameWidth = 160;
    static constexpr int rulerHeight = 30;
    static constexpr int rowHeight = 46;

    ProjectModel& model;
    int scrollY = 0;
    int hoverRow = -1;
    Tool tool = Tool::Select;
    int lastPaintRow = -1;
    int lastPaintBar = -1;
};
