// Opalo: ponto de entrada.

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/DawLookAndFeel.h"
#include "UI/MainComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name)
        : DocumentWindow(name, DawLookAndFeel::background, DocumentWindow::allButtons)
    {
        // Barra de título própria do Opalo, com os botões de janela no canto superior esquerdo.
        setUsingNativeTitleBar(false);
        setTitleBarHeight(40);
        setTitleBarButtonsRequired(DocumentWindow::allButtons, true);
        setTitleBarTextCentred(true);

        auto* content = new MainComponent();
        content->onTitleChanged = [this](const juce::String& title) { setName(title); };
        setContentOwned(content, true);
        setResizeLimits(960, 720, 4096, 2160);
        setResizable(true, true);
        setDropShadowEnabled(true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class DAWApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "Opalo"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(DAWApplication)
