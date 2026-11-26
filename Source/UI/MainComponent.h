#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../AudioEngine/AudioEngine.h"
#include "../Model/ProjectModel.h"
#include "../Model/ProjectHistory.h"
#include "DawLookAndFeel.h"
#include "PanelWindow.h"
#include "WaveformView.h"
#include "Panels/BeatDeckPanel.h"
#include "Panels/BrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/NoteCanvasPanel.h"
#include "Panels/PluginVaultPanel.h"
#include "Panels/TimelinePanel.h"
#include "Panels/MashupPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/AutomationPanel.h"
#include "../I18n/Strings.h"
#include "../Settings/OpaloSettings.h"


class OpaloIconButton : public juce::TextButton
{
public:
    enum class Icon { Browser, Playlist, Mixer, Patterns, Notes, Plugins, Decks, Automation, Settings };
    explicit OpaloIconButton(Icon i) : icon(i) { setButtonText({}); setMouseCursor(juce::MouseCursor::PointingHandCursor); }
    void paint(juce::Graphics& g) override
    {
        auto r=getLocalBounds().toFloat().reduced(2.0f);
        const auto bg=getToggleState()?DawLookAndFeel::accentDim:(isMouseOver()?DawLookAndFeel::panelLight:DawLookAndFeel::panel);
        g.setColour(bg); g.fillRoundedRectangle(r,5.0f);
        g.setColour(getToggleState()?DawLookAndFeel::accent:DawLookAndFeel::text.withAlpha(0.88f));
        const auto c=r.getCentre(); const float x=c.x,y=c.y;
        const float sw=1.7f;
        auto line=[&](float x1,float y1,float x2,float y2){g.drawLine(x1,y1,x2,y2,sw);};
        switch(icon)
        {
            case Icon::Browser:
                g.drawRoundedRectangle(x-7,y-7,14,14,2,sw); line(x-4,y-3,x+4,y-3); line(x-4,y,x+3,y); line(x-4,y+3,x+1,y+3); break;
            case Icon::Playlist:
                g.drawRoundedRectangle(x-8,y-7,16,14,2,sw); line(x-5,y-3,x+5,y-3); line(x-5,y,x+5,y); line(x-5,y+3,x+2,y+3); break;
            case Icon::Mixer:
                for(int k=-1;k<=1;++k){line(x+k*5,y-7,x+k*5,y+7);g.fillEllipse(x+k*5-2,y+(k==0?-1:(k<0?2:-4))-2,4,4);} break;
            case Icon::Patterns:
                for(int row=0;row<3;++row) for(int col=0;col<3;++col){g.drawRoundedRectangle(x-7+col*5.5f,y-6+row*5.5f,4,4,1,sw);} break;
            case Icon::Notes:
                line(x-2,y-6,x-2,y+3); line(x-2,y-6,x+5,y-8); line(x+5,y-8,x+5,y+1); g.drawEllipse(x-7,y+1,6,4,sw); g.drawEllipse(x,y-1,6,4,sw); break;
            case Icon::Plugins:
                {juce::Path p; p.addPolygon({x,y-8,x+7,y,x,y+8,x-7,y},4); g.strokePath(p,juce::PathStrokeType(sw)); line(x-4,y,x+4,y);} break;
            case Icon::Decks:
                g.drawEllipse(x-7,y-7,14,14,sw); g.drawEllipse(x-3,y-3,6,6,sw); g.fillEllipse(x-1,y-1,2,2); break;
            case Icon::Automation:
                {juce::Path p;p.startNewSubPath(x-8,y+5);p.lineTo(x-3,y+1);p.lineTo(x+1,y+3);p.lineTo(x+7,y-6);g.strokePath(p,juce::PathStrokeType(sw));} g.fillEllipse(x-5,y-1,4,4);g.fillEllipse(x-1,y+1,4,4);g.fillEllipse(x+5,y-8,4,4); break;
            case Icon::Settings:
                g.drawEllipse(x-6,y-6,12,12,sw); g.drawEllipse(x-2,y-2,4,4,sw);
                for(int k=0;k<8;++k){const float a=k*juce::MathConstants<float>::pi/4.0f;line(x+7*std::cos(a),y+7*std::sin(a),x+9*std::cos(a),y+9*std::sin(a));} break;
        }
    }
private:
    Icon icon;
};

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

    OpaloIconButton browserButton   { OpaloIconButton::Icon::Browser } "Browser" };
    OpaloIconButton timelineButton  { OpaloIconButton::Icon::Playlist } "Playlist" };
    OpaloIconButton consoleButton   { OpaloIconButton::Icon::Mixer } "Mixer" };
    OpaloIconButton beatButton      { "Patterns" };
    OpaloIconButton noteButton      { "Note Canvas" };
    OpaloIconButton vaultButton     { "Plugin Vault" };
    OpaloIconButton mashupButton    { "Mashup" };
    OpaloIconButton settingsButton  { OpaloIconButton::Icon::Settings } "Settings" };
    OpaloIconButton automationButton { OpaloIconButton::Icon::Automation } "Automation" };

    juce::TextButton newButton    { "New" };
    juce::TextButton openButton   { "Open" };
    juce::TextButton saveButton   { "Save" };
    juce::TextButton saveAsButton { "Save as" };

    std::unique_ptr<juce::FileChooser> audioChooser;
    std::unique_ptr<juce::FileChooser> projectChooser;
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
    std::unique_ptr<PanelWindow> automationWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
