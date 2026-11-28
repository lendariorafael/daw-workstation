#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../AudioEngine/AudioEngine.h"
#include "../Audio/AudioFormats.h"
#include "../I18n/Strings.h"
#include "../Model/ProjectFile.h"
#include "../Model/ProjectHistory.h"
#include "../Model/ProjectModel.h"
#include "../Settings/OpaloSettings.h"
#include "DawLookAndFeel.h"
#include "PanelWindow.h"
#include "SpaceBackground.h"
#include "SplashScreen.h"
#include "TaskBar.h"
#include "TopBar.h"
#include "Panels/BeatDeckPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/MashupPanel.h"
#include "Panels/NoteCanvasPanel.h"
#include "Panels/PluginVaultPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/SidebarPanel.h"
#include "Panels/TimelinePanel.h"

// Janela principal do Opalo: barra superior, barra lateral (biblioteca), playlist no centro,
// e janelas independentes para o Mixer, o Piano Roll e o Beat Editor.
class MainComponent : public juce::Component,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    // Chamado sempre que o título da janela deve mudar.
    std::function<void(const juce::String&)> onTitleChanged;

private:
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;

    void showMenu(const juce::String& key);
    void handleMenu(int id);
    void toggleWindow(PanelWindow& window);
    void bindWindow(PanelWindow& window, int tabIndex);
    void refreshTexts();

    void loadAudio();
    void newProject();
    void openProject();
    void openRecent(int index);
    void saveProject();
    void saveProjectAs();
    void writeProject(const juce::File& file);
    void loadProjectFrom(const juce::File& file);
    bool canDiscardChanges();

    void undo();
    void redo();
    void flushHistory();
    void resetHistory();
    void applySnapshot(const juce::String& snapshot);
    juce::String snapshot() const;

    void loadRecentList();
    void rememberRecent(const juce::File& file);
    void updateTitle();

    static juce::File projectsFolder();
    static juce::File appDataFolder();
    static juce::File autosaveFile();
    static juce::File recentListFile();

    // Ordem importa: o tema, o motor e o modelo existem antes de quem os usa.
    DawLookAndFeel lookAndFeel;
    AudioEngine engine;
    ProjectModel model;

    juce::File currentFile;
    juce::StringArray recentProjects;
    ProjectHistory history;
    bool dirty = false;
    bool loading = false;
    bool changePending = false;
    int changeTicks = 0;
    int autosaveTicks = 0;

    SpaceBackground space;
    TopBar topBar;
    TaskBar taskBar;
    SidebarPanel sidebar { model };
    TimelinePanel playlist { model };

    int tabMixer = -1, tabPiano = -1, tabBeat = -1, tabMashup = -1, tabVault = -1, tabSettings = -1;

    std::unique_ptr<juce::FileChooser> audioChooser;
    std::unique_ptr<juce::FileChooser> projectChooser;
    std::unique_ptr<SplashScreen> splash;

    // Janelas independentes, por último: são destruídas antes do modelo.
    std::unique_ptr<PanelWindow> consoleWindow;
    std::unique_ptr<PanelWindow> noteWindow;
    std::unique_ptr<PanelWindow> beatWindow;
    std::unique_ptr<PanelWindow> mashupWindow;
    std::unique_ptr<PanelWindow> vaultWindow;
    std::unique_ptr<PanelWindow> settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
