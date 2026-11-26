#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "DawLookAndFeel.h"
#include "DesignTokens.h"

// Janela flutuante de um painel. Fechar só esconde a janela,
// para o estado do painel continuar vivo.
class PanelWindow : public juce::DocumentWindow
{
public:
    PanelWindow(const juce::String& title, juce::Component* content, int width, int height)
        : DocumentWindow(title, DawLookAndFeel::background, DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(false);
        setTitleBarHeight(32);
        setTitleBarButtonsRequired(DocumentWindow::allButtons, true);
        setTitleBarTextCentred(true);
        setContentOwned(content, true);
        setResizable(true, true);
        setResizeLimits(360, 240, 4096, 2160);
        centreWithSize(width, height);
        setVisible(false);
    }

    std::function<void(bool)> onVisibilityChanged;

    void closeButtonPressed() override
    {
        setVisible(false);
    }

    void visibilityChanged() override
    {
        if (onVisibilityChanged)
            onVisibilityChanged(isVisible());
    }
};
