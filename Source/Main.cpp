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
        // Usar a barra nativa do Windows garante minimizar, maximizar e fechar
        // mesmo em ecrãs pequenos e com escalas de DPI diferentes.
        setUsingNativeTitleBar(true);
        setTitleBarButtonsRequired(DocumentWindow::allButtons, true);

        auto* content = new MainComponent();
        content->onTitleChanged = [this](const juce::String& title) { setName(title); };
        setContentOwned(content, true);
        setResizeLimits(640, 420, 4096, 2160);
        setResizable(true, true);
        setDropShadowEnabled(true);

        const auto displays = juce::Desktop::getInstance().getDisplays();
        const auto* display = displays.getPrimaryDisplay();
        const auto area = display != nullptr ? display->userArea
                                             : juce::Rectangle<int>(0, 0, 1280, 720);
        const int initialWidth = juce::jmax(640, juce::jmin(1280, area.getWidth() - 40));
        const int initialHeight = juce::jmax(420, juce::jmin(760, area.getHeight() - 80));
        centreWithSize(initialWidth, initialHeight);
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
