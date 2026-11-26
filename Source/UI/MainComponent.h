#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../AudioEngine/AudioEngine.h"
#include "../Model/ProjectModel.h"
#include "../Model/ProjectHistory.h"
#include "DawLookAndFeel.h"
#include "OpaloLogo.h"
#include "PanelWindow.h"
#include "SplashScreen.h"
#include "WaveformView.h"
#include "Panels/BeatDeckPanel.h"
#include "Panels/BrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/NoteCanvasPanel.h"
#include "Panels/PluginVaultPanel.h"
#include "Panels/TimelinePanel.h"
#include "Panels/MashupPanel.h"
#include "Panels/SettingsPanel.h"
#include "../I18n/Strings.h"
#include "../Settings/OpaloSettings.h"

class MainComponent : public juce::Component,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    // Atualiza os textos da interface para o idioma atual.
    void refreshTexts();

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    // Chamado sempre que o título da janela deve mudar.
    std::function<void(const juce::String&)> onTitleChanged;

private:
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;

    void loadAudio();
    void newProject();
    void openProject();
    void showOpenMenu();
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
    void updateControls();
    void bindPanel(juce::TextButton& button, PanelWindow& window);
    void showMenu(const juce::String& menuName);
    void applyLayoutPreset(int preset);
    static juce::File projectsFolder();
    static juce::File appDataFolder();
    static juce::File autosaveFile();
    static juce::File recentListFile();
    static juce::String formatTime(double seconds);

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

    OpaloLogo topLogo;
    juce::TextButton loadAudioButton { "Load audio" };
    juce::TextButton startButton { "Start" };
    juce::TextButton playButton  { "Play" };
    juce::TextButton stopButton  { "Stop" };
    juce::TextButton recordButton { "REC" };
    juce::Label bpmLabel;
    juce::TextButton fileMenuButton { "File" };
    juce::TextButton editMenuButton { "Edit" };
    juce::TextButton viewMenuButton { "View" };
    juce::TextButton toolsMenuButton { "Tools" };
    juce::TextButton helpMenuButton { "Help" };
    juce::Slider volumeSlider;
    juce::Label fileLabel;
    juce::Label timeLabel;
    WaveformView waveform { engine.getFormatManager() };

    juce::TextButton browserButton   { "Browser" };
    juce::TextButton timelineButton  { "Playlist" };
    juce::TextButton consoleButton   { "Mixer" };
    juce::TextButton beatButton      { "Patterns" };
    juce::TextButton noteButton      { "Note Canvas" };
    juce::TextButton vaultButton     { "Plugin Vault" };
    juce::TextButton mashupButton    { "Mashup" };
    juce::TextButton settingsButton  { "Settings" };

    juce::TextButton newButton    { "New" };
    juce::TextButton openButton   { "Open" };
    juce::TextButton saveButton   { "Save" };
    juce::TextButton saveAsButton { "Save as" };

    std::unique_ptr<juce::FileChooser> audioChooser;
    std::unique_ptr<juce::FileChooser> projectChooser;
    std::unique_ptr<SplashScreen> splash;
    bool layoutInitialized = false;
    bool recordNoticeShown = false;

    // Janelas por último: são destruídas antes do modelo.
    std::unique_ptr<PanelWindow> browserWindow;
    std::unique_ptr<PanelWindow> timelineWindow;
    std::unique_ptr<PanelWindow> consoleWindow;
    std::unique_ptr<PanelWindow> beatWindow;
    std::unique_ptr<PanelWindow> noteWindow;
    std::unique_ptr<PanelWindow> vaultWindow;
    std::unique_ptr<PanelWindow> mashupWindow;
    std::unique_ptr<PanelWindow> settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
