#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>
#include "DawLookAndFeel.h"
#include "DesignTokens.h"

// Janela interna do workspace. Os painéis são filhos da janela principal:
// podem ser movidos, redimensionados, minimizados, maximizados e restaurados
// sem criar janelas nativas fora da área de trabalho da OPALO.
class PanelWindow : public juce::Component
{
public:
    static constexpr int titleBarHeight = 30;

    PanelWindow(const juce::String& titleText, juce::Component* panelContent, int width, int height)
        : title(titleText), content(panelContent)
    {
        setOpaque(false);
        title.setFont(DawLookAndFeel::uiFont(12.5f, true));
        title.setColour(juce::Label::textColourId, OpaloDesign::text);
        title.setInterceptsMouseClicks(false, false);

        for (auto* button : { &minimiseButton, &maximiseButton, &closeButton })
            addAndMakeVisible(*button);

        addAndMakeVisible(title);
        if (content != nullptr)
            addAndMakeVisible(*content);

        minimiseButton.setButtonText("—");
        maximiseButton.setButtonText("□");
        closeButton.setButtonText("×");
        minimiseButton.setTooltip("Minimizar painel");
        maximiseButton.setTooltip("Maximizar / restaurar painel");
        closeButton.setTooltip("Fechar painel");

        minimiseButton.onClick = [this] { setVisible(false); };
        maximiseButton.onClick = [this] { toggleMaximise(); };
        closeButton.onClick = [this] { setVisible(false); };

        setSize(width, height);
        setVisible(false);
    }

    std::function<void(bool)> onVisibilityChanged;

    void setVisible(bool shouldBeVisible)
    {
        juce::Component::setVisible(shouldBeVisible);
        if (shouldBeVisible)
            toFront(true);
        if (onVisibilityChanged)
            onVisibilityChanged(shouldBeVisible);
    }

    void toFront(bool shouldGrabFocus)
    {
        juce::Component::toFront(shouldGrabFocus);
    }

    void setWorkspaceBounds(juce::Rectangle<int> bounds)
    {
        if (! userMoved && ! maximised)
            setBounds(bounds);
        else if (maximised && getParentComponent() != nullptr)
            setBounds(getParentComponent()->getLocalBounds().reduced(6));
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(OpaloDesign::surface0.withAlpha(0.98f));
        g.fillRoundedRectangle(bounds, 7.0f);
        g.setColour(OpaloDesign::border.brighter(0.12f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 7.0f, 1.0f);

        g.setColour(OpaloDesign::surface1);
        g.fillRoundedRectangle(1.0f, 1.0f, (float) getWidth() - 2.0f,
                               (float) titleBarHeight + 2.0f, 6.0f);
        g.setColour(OpaloDesign::accent.withAlpha(0.48f));
        g.fillRect(1, titleBarHeight, getWidth() - 2, 1);

        // Pequeno puxador de redimensionamento no canto inferior direito.
        g.setColour(OpaloDesign::textMuted.withAlpha(0.65f));
        for (int i = 0; i < 3; ++i)
            g.drawLine((float) getWidth() - 5.0f - i * 4.0f, (float) getHeight() - 2.0f,
                       (float) getWidth() - 2.0f, (float) getHeight() - 5.0f - i * 4.0f, 1.0f);
    }

    void resized() override
    {
        auto header = getLocalBounds().removeFromTop(titleBarHeight).reduced(5, 3);
        closeButton.setBounds(header.removeFromRight(24));
        header.removeFromRight(3);
        maximiseButton.setBounds(header.removeFromRight(24));
        header.removeFromRight(3);
        minimiseButton.setBounds(header.removeFromRight(24));
        header.removeFromRight(5);
        title.setBounds(header.removeFromLeft(juce::jmax(60, header.getWidth() - 6)));

        if (content != nullptr)
            content->setBounds(1, titleBarHeight + 1, getWidth() - 2,
                               juce::jmax(1, getHeight() - titleBarHeight - 2));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        dragStartBounds = getBounds();
        resizing = e.x >= getWidth() - 16 && e.y >= getHeight() - 16;
        dragging = e.y < titleBarHeight && e.x < getWidth() - 82;
        if (dragging || resizing)
            toFront(true);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (getParentComponent() == nullptr)
            return;

        const int dx = e.getDistanceFromDragStartX();
        const int dy = e.getDistanceFromDragStartY();
        auto next = dragStartBounds;

        if (resizing)
        {
            next.setWidth(juce::jmax(240, dragStartBounds.getWidth() + dx));
            next.setHeight(juce::jmax(150, dragStartBounds.getHeight() + dy));
        }
        else if (dragging && ! maximised)
        {
            next.translate(dx, dy);
        }
        else
        {
            return;
        }

        auto parentBounds = getParentComponent()->getLocalBounds();
        next.setWidth(juce::jmin(next.getWidth(), parentBounds.getWidth()));
        next.setHeight(juce::jmin(next.getHeight(), parentBounds.getHeight()));
        next.setX(juce::jlimit(0, juce::jmax(0, parentBounds.getWidth() - next.getWidth()), next.getX()));
        next.setY(juce::jlimit(0, juce::jmax(0, parentBounds.getHeight() - next.getHeight()), next.getY()));
        setBounds(next);
        userMoved = true;
    }

private:
    void toggleMaximise()
    {
        if (getParentComponent() == nullptr)
            return;

        if (! maximised)
        {
            restoreBounds = getBounds();
            maximised = true;
            setBounds(getParentComponent()->getLocalBounds().reduced(6));
        }
        else
        {
            maximised = false;
            setBounds(restoreBounds);
        }
        toFront(true);
    }

    juce::Label title;
    std::unique_ptr<juce::Component> content;
    juce::TextButton minimiseButton, maximiseButton, closeButton;
    juce::Rectangle<int> dragStartBounds, restoreBounds;
    bool dragging = false, resizing = false, maximised = false, userMoved = false;
};
