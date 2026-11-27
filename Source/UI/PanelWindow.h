#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>
#include "DawLookAndFeel.h"
#include "DesignTokens.h"

// Painel flutuante DENTRO da janela principal: pode ser movido e redimensionado
// sem abrir uma segunda janela do sistema operativo.
class PanelWindow : public juce::Component
{
public:
    PanelWindow(const juce::String& title, juce::Component* child, int width, int height)
        : titleText(title), content(child)
    {
        setSize(width, height);
        addAndMakeVisible(*content);
        setVisible(false);
        setWantsKeyboardFocus(true);
    }

    juce::Component* getContentComponent() const { return content.get(); }

    std::function<void(bool)> onVisibilityChanged;
    std::function<void()> onUserBoundsChanged;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(OpaloDesign::surface0);
        g.fillRoundedRectangle(bounds.toFloat(), 7.0f);
        g.setColour(OpaloDesign::border);
        g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 7.0f, 1.0f);

        auto titleBar = bounds.removeFromTop(titleHeight);
        g.setColour(OpaloDesign::surface2);
        g.fillRoundedRectangle(titleBar.toFloat(), 7.0f);
        g.fillRect(titleBar.withTrimmedBottom(7));
        g.setColour(OpaloDesign::accent.withAlpha(0.8f));
        g.fillRect(titleBar.getX(), titleBar.getBottom() - 2, titleBar.getWidth(), 2);

        g.setColour(OpaloDesign::text);
        g.setFont(DawLookAndFeel::uiFont(12.0f, true));
        g.drawText(titleText, titleBar.reduced(12, 0).withTrimmedRight(30),
                   juce::Justification::centredLeft, true);

        const auto close = closeBounds();
        g.setColour(OpaloDesign::danger.withAlpha(0.9f));
        g.fillEllipse(close.toFloat().reduced(5.0f));
        g.setColour(OpaloDesign::text.withAlpha(0.85f));
        g.drawLine((float) close.getX() + 7, (float) close.getY() + 7,
                   (float) close.getRight() - 7, (float) close.getBottom() - 7, 1.4f);
        g.drawLine((float) close.getRight() - 7, (float) close.getY() + 7,
                   (float) close.getX() + 7, (float) close.getBottom() - 7, 1.4f);

        g.setColour(OpaloDesign::textMuted);
        for (int i = 0; i < 3; ++i)
            g.fillEllipse((float) getWidth() - 13.0f + i * 3.0f,
                          (float) getHeight() - 8.0f, 1.5f, 1.5f);
    }

    void resized() override
    {
        content->setBounds(getLocalBounds().reduced(1).withTrimmedTop(titleHeight).withTrimmedRight(resizeGrip).withTrimmedBottom(resizeGrip));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        toFront(false);
        const auto p = e.getPosition();
        if (closeBounds().contains(p))
        {
            setVisible(false);
            return;
        }

        resizeLeft = p.x <= resizeGrip;
        resizeRight = p.x >= getWidth() - resizeGrip;
        resizeTop = p.y <= resizeGrip;
        resizeBottom = p.y >= getHeight() - resizeGrip;
        resizing = resizeLeft || resizeRight || resizeTop || resizeBottom;
        dragging = !resizing && p.y < titleHeight;
        dragOrigin = e.getScreenPosition();
        originalBounds = getBounds();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto delta = e.getScreenPosition() - dragOrigin;
        if (auto* parent = getParentComponent())
        {
            const auto parentBounds = parent->getLocalBounds();
            if (resizing)
            {
                constexpr int minWidth = 180;
                constexpr int minHeight = 150;
                int left = originalBounds.getX();
                int top = originalBounds.getY();
                int right = originalBounds.getRight();
                int bottom = originalBounds.getBottom();

                if (resizeLeft)
                    left = juce::jlimit(parentBounds.getX(), right - minWidth,
                                        originalBounds.getX() + delta.x);
                if (resizeRight)
                    right = juce::jlimit(left + minWidth, parentBounds.getRight(),
                                         originalBounds.getRight() + delta.x);
                if (resizeTop)
                    top = juce::jlimit(108, bottom - minHeight,
                                       originalBounds.getY() + delta.y);
                if (resizeBottom)
                    bottom = juce::jlimit(top + minHeight, parentBounds.getBottom(),
                                          originalBounds.getBottom() + delta.y);

                setBounds(left, top, right - left, bottom - top);
            }
            else if (dragging)
            {
                const int newX = juce::jlimit(0, juce::jmax(0, parentBounds.getWidth() - getWidth()),
                                              originalBounds.getX() + delta.x);
                const int newY = juce::jlimit(108, juce::jmax(108, parentBounds.getBottom() - getHeight()),
                                              originalBounds.getY() + delta.y);
                setTopLeftPosition(newX, newY);
            }
        }

        if (dragging || resizing)
            if (onUserBoundsChanged) onUserBoundsChanged();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        dragging = resizing = false;
        resizeLeft = resizeRight = resizeTop = resizeBottom = false;
    }

    void visibilityChanged() override
    {
        if (onVisibilityChanged)
            onVisibilityChanged(isVisible());
    }

private:
    juce::Rectangle<int> closeBounds() const
    {
        return { getWidth() - 28, 2, 24, titleHeight - 4 };
    }

    static constexpr int titleHeight = 30;
    static constexpr int resizeGrip = 16;
    juce::String titleText;
    std::unique_ptr<juce::Component> content;
    juce::Point<int> dragOrigin;
    juce::Rectangle<int> originalBounds;
    bool dragging = false;
    bool resizing = false;
    bool resizeLeft = false;
    bool resizeRight = false;
    bool resizeTop = false;
    bool resizeBottom = false;
};
