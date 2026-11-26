#include "MainComponent.h"

namespace
{
    constexpr int maxRecent = 8;
    constexpr int historyDelayTicks = 6;     // ~200 ms de pausa antes de registar uma alteração
    constexpr int autosaveIntervalTicks = 30 * 60 * 5;
}

MainComponent::MainComponent()
{
    I18n::current() = I18n::fromCode(OpaloSettings::get().getValue("language", "pt"));
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);
    setWantsKeyboardFocus(true);

    loadAudioButton.onClick = [this] { loadAudio(); };
    startButton.onClick = [this] { engine.seek(0.0); engine.play(); };
    playButton.onClick = [this] { engine.play(); };
    stopButton.onClick = [this] { engine.stop(); };
    recordButton.setTooltip(I18n::t("status.recordUnavailable"));
    recordButton.setEnabled(false);
    recordButton.onClick = [this]
    {
        fileLabel.setText("Recording input will be added with the audio capture engine.",
                          juce::dontSendNotification);
    };

    bpmLabel.setText(juce::String(model.bpm, 1) + " BPM", juce::dontSendNotification);
    bpmLabel.setEditable(true, true, false);
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setTooltip("Double-click to edit the project tempo.");
    bpmLabel.onTextChange = [this]
    {
        const double bpm = bpmLabel.getText().upToFirstOccurrenceOf(" ", false, false).getDoubleValue();
        if (bpm >= 20.0 && bpm <= 300.0)
        {
            model.bpm = bpm;
            model.changed();
        }
        bpmLabel.setText(juce::String(model.bpm, 1) + " BPM", juce::dontSendNotification);
    };

    fileMenuButton.onClick = [this] { showMenu("File"); };
    editMenuButton.onClick = [this] { showMenu("Edit"); };
    viewMenuButton.onClick = [this] { showMenu("View"); };
    toolsMenuButton.onClick = [this] { showMenu("Tools"); };
    helpMenuButton.onClick = [this] { showMenu("Help"); };

    newButton.onClick = [this] { newProject(); };
    openButton.onClick = [this] { showOpenMenu(); };
    saveButton.onClick = [this] { saveProject(); };
    saveAsButton.onClick = [this] { saveProjectAs(); };

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 20);
    volumeSlider.setRange(0.0, 1.0, 0.001);
    volumeSlider.setValue(0.8, juce::dontSendNotification);
    engine.setGain(0.8f);
    volumeSlider.onValueChange = [this] { engine.setGain((float) volumeSlider.getValue()); };

    fileLabel.setText(I18n::t("status.ready"), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
    timeLabel.setText("00:00.00 / 00:00.00", juce::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centredRight);
    waveform.onSeek = [this](double seconds) { engine.seek(seconds); };

    for (auto* component : {
             static_cast<juce::Component*>(&fileMenuButton),
             static_cast<juce::Component*>(&editMenuButton),
             static_cast<juce::Component*>(&viewMenuButton),
             static_cast<juce::Component*>(&toolsMenuButton),
             static_cast<juce::Component*>(&helpMenuButton),
             static_cast<juce::Component*>(&newButton),
             static_cast<juce::Component*>(&openButton),
             static_cast<juce::Component*>(&saveButton),
             static_cast<juce::Component*>(&saveAsButton),
             static_cast<juce::Component*>(&loadAudioButton),
             static_cast<juce::Component*>(&startButton),
             static_cast<juce::Component*>(&playButton),
             static_cast<juce::Component*>(&stopButton),
             static_cast<juce::Component*>(&recordButton),
             static_cast<juce::Component*>(&bpmLabel),
             static_cast<juce::Component*>(&volumeSlider),
             static_cast<juce::Component*>(&fileLabel),
             static_cast<juce::Component*>(&timeLabel),
             static_cast<juce::Component*>(&waveform),
             static_cast<juce::Component*>(&browserButton),
             static_cast<juce::Component*>(&timelineButton),
             static_cast<juce::Component*>(&consoleButton),
             static_cast<juce::Component*>(&beatButton),
             static_cast<juce::Component*>(&noteButton),
             static_cast<juce::Component*>(&vaultButton),
             static_cast<juce::Component*>(&mashupButton),
             static_cast<juce::Component*>(&settingsButton),
             static_cast<juce::Component*>(&automationButton) })
        addAndMakeVisible(component);

    browserWindow  = std::make_unique<PanelWindow>(I18n::t("view.browser"), new BrowserPanel(model), 230, 620);
    timelineWindow = std::make_unique<PanelWindow>(I18n::t("view.playlist"), new TimelinePanel(model), 900, 330);
    consoleWindow  = std::make_unique<PanelWindow>(I18n::t("view.mixer"), new ConsolePanel(model), 900, 230);
    beatWindow     = std::make_unique<PanelWindow>(I18n::t("view.beat"), new BeatDeckPanel(model), 900, 190);
    noteWindow     = std::make_unique<PanelWindow>(I18n::t("view.notes"), new NoteCanvasPanel(model), 900, 430);
    vaultWindow    = std::make_unique<PanelWindow>(I18n::t("view.vault"), new PluginVaultPanel(model), 520, 420);
    mashupWindow   = std::make_unique<PanelWindow>(I18n::t("view.mashup"), new MashupPanel(model, engine), 920, 520);
    automationWindow = std::make_unique<PanelWindow>("Automation", new AutomationPanel(model), 760, 360);
    settingsWindow = std::make_unique<PanelWindow>(
        I18n::t("settings.title"),
        new SettingsPanel(engine, [this] { refreshTexts(); },
                          [this](int preset) { applyLayoutPreset(preset); }),
        620, 430);

    for (auto* panel : { browserWindow.get(), timelineWindow.get(), consoleWindow.get(),
                         beatWindow.get(), noteWindow.get(), vaultWindow.get(),
                         mashupWindow.get(), settingsWindow.get(), automationWindow.get() })
        addAndMakeVisible(panel);

    setSize(1440, 900);

    bindPanel(browserButton, *browserWindow);
    bindPanel(timelineButton, *timelineWindow);
    bindPanel(consoleButton, *consoleWindow);
    bindPanel(beatButton, *beatWindow);
    bindPanel(noteButton, *noteWindow);
    bindPanel(vaultButton, *vaultWindow);
    bindPanel(mashupButton, *mashupWindow);
    bindPanel(settingsButton, *settingsWindow);
    bindPanel(automationButton, *automationWindow);

    browserButton.setButtonText("⌕");
    timelineButton.setButtonText("▦");
    consoleButton.setButtonText("▥");
    beatButton.setButtonText("▧");
    noteButton.setButtonText("♫");
    vaultButton.setButtonText("◇");
    mashupButton.setButtonText("◉");
    settingsButton.setButtonText("⚙");
    automationButton.setButtonText("⌁");
    automationButton.setTooltip("Automation editor");

    browserButton.setTooltip("Browser — show or hide sound library");
    timelineButton.setTooltip("Playlist — show or hide arrangement");
    consoleButton.setTooltip("Mixer — show or hide channel mixer");
    beatButton.setTooltip("Patterns — show or hide pattern editor");
    noteButton.setTooltip("Note editor — show or hide");
    vaultButton.setTooltip("Plugin library — show or hide");
    mashupButton.setTooltip("DJ decks — show or hide");

    refreshTexts();
    loadRecentList();
    resetHistory();
    model.addChangeListener(this);
    startTimerHz(30);

    applyLayoutPreset((int) OpaloSettings::get().getIntValue("layoutPreset", 0));


    updateTitle();
    updateControls();
}

MainComponent::~MainComponent()
{
    stopTimer();
    model.removeChangeListener(this);
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::refreshTexts()
{
    newButton.setButtonText(I18n::t("file.new"));
    openButton.setButtonText(I18n::t("file.open"));
    saveButton.setButtonText(I18n::t("file.save"));
    saveAsButton.setButtonText(I18n::t("file.saveAs"));
    loadAudioButton.setButtonText(I18n::t("audio.load"));
    startButton.setButtonText("|◀");
    startButton.setTooltip("Start playback from the beginning");
    playButton.setButtonText("▶");
    stopButton.setButtonText("■");
    recordButton.setButtonText("● REC");

    browserButton.setButtonText("⌕");
    timelineButton.setButtonText("▦");
    consoleButton.setButtonText("▥");
    beatButton.setButtonText("▧");
    noteButton.setButtonText("♫");
    vaultButton.setButtonText("◇");
    mashupButton.setButtonText("◉");
    settingsButton.setButtonText("⚙");
}

void MainComponent::bindPanel(juce::TextButton& button, PanelWindow& window)
{
    button.setClickingTogglesState(true);
    button.setToggleState(window.isVisible(), juce::dontSendNotification);
    button.onClick = [&button, &window]
    {
        window.setVisible(button.getToggleState());
        if (button.getToggleState())
            window.toFront(false);
    };

    window.onVisibilityChanged = [&button](bool visible)
    {
        button.setToggleState(visible, juce::dontSendNotification);
    };
}

void MainComponent::showMenu(const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuName == "File")
    {
        menu.addItem(1, "New project");
        menu.addItem(2, "Open project");
        menu.addItem(3, "Save");
        menu.addItem(4, "Save as...");
        menu.addSeparator();
        menu.addItem(5, "Import audio...");
        menu.addItem(6, "Quit");
    }
    else if (menuName == "Edit")
    {
        menu.addItem(1, "Undo");
        menu.addItem(2, "Redo");
        menu.addSeparator();
        menu.addItem(3, "Cut tool  (C)");
        menu.addItem(4, "Select tool  (D)");
    }
    else if (menuName == "View")
    {
        menu.addItem(1, "Browser", true, browserWindow->isVisible());
        menu.addItem(2, "Playlist", true, timelineWindow->isVisible());
        menu.addItem(3, "Patterns", true, beatWindow->isVisible());
        menu.addItem(4, "Mixer", true, consoleWindow->isVisible());
        menu.addItem(5, "Note Editor", true, noteWindow->isVisible());
        menu.addItem(6, "Plugin Vault", true, vaultWindow->isVisible());
        menu.addItem(7, "Mashup decks", true, mashupWindow->isVisible());
        menu.addSeparator();
        menu.addItem(8, "Settings");
        menu.addItem(9, "Automation");
    }
    else if (menuName == "Tools")
    {
        menu.addItem(1, "DJ Mashup");
        menu.addItem(2, "Plugin Vault");
        menu.addItem(3, "Note Editor");
        menu.addItem(4, "Load audio");
        menu.addItem(5, "Automation editor");
    }
    else
    {
        menu.addItem(1, "Keyboard shortcuts");
        menu.addItem(2, "About Opalo");
    }

    juce::Component::SafePointer<MainComponent> safe(this);
    auto* target = menuName == "File" ? static_cast<juce::Component*>(&fileMenuButton)
                : menuName == "Edit" ? static_cast<juce::Component*>(&editMenuButton)
                : menuName == "View" ? static_cast<juce::Component*>(&viewMenuButton)
                : menuName == "Tools" ? static_cast<juce::Component*>(&toolsMenuButton)
                                      : static_cast<juce::Component*>(&helpMenuButton);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(target),
        [safe, menuName](int result)
        {
            auto* self = safe.getComponent();
            if (self == nullptr || result == 0) return;

            if (menuName == "File")
            {
                if (result == 1) self->newProject();
                else if (result == 2) self->openProject();
                else if (result == 3) self->saveProject();
                else if (result == 4) self->saveProjectAs();
                else if (result == 5) self->loadAudio();
                else if (result == 6) juce::JUCEApplication::getInstance()->systemRequestedQuit();
            }
            else if (menuName == "Edit")
            {
                if (result == 1) self->undo();
                else if (result == 2) self->redo();
                else if (result == 3)
                {
                    if (auto* timeline = dynamic_cast<TimelinePanel*>(self->timelineWindow->getContentComponent()))
                        timeline->setTool(TimelinePanel::Tool::Cut);
                    self->timelineWindow->setVisible(true);
                    self->timelineWindow->toFront(false);
                }
                else if (result == 4)
                {
                    if (auto* timeline = dynamic_cast<TimelinePanel*>(self->timelineWindow->getContentComponent()))
                        timeline->setTool(TimelinePanel::Tool::Select);
                    self->timelineWindow->setVisible(true);
                    self->timelineWindow->toFront(false);
                }
            }
            else if (menuName == "View")
            {
                PanelWindow* panels[] = { self->browserWindow.get(), self->timelineWindow.get(),
                    self->beatWindow.get(), self->consoleWindow.get(), self->noteWindow.get(),
                    self->vaultWindow.get(), self->mashupWindow.get(), self->settingsWindow.get() };
                if (result >= 1 && result <= 8)
                {
                    auto* panel = panels[result - 1];
                    panel->setVisible(!panel->isVisible());
                    if (panel->isVisible()) panel->toFront(false);
                }
            }
            else if (menuName == "Tools")
            {
                if (result == 1) { self->mashupWindow->setVisible(true); self->mashupWindow->toFront(false); }
                else if (result == 2) { self->vaultWindow->setVisible(true); self->vaultWindow->toFront(false); }
                else if (result == 3) { self->noteWindow->setVisible(true); self->noteWindow->toFront(false); }
                else if (result == 4) self->loadAudio();
                else if (result == 5) { self->automationWindow->setVisible(true); self->automationWindow->toFront(false); }
            }
            else
            {
                if (result == 1)
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Opalo shortcuts",
                        "Ctrl+N  New project\nCtrl+O  Open project\nCtrl+S  Save\nCtrl+Z  Undo\nCtrl+Y  Redo\nC  Slice\nD  Select\nP  Pencil\nB  Brush\nX  Copy");
                else
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "About Opalo",
                        "Opalo Digital Audio Workstation\nVersion 0.1.0\nA creative audio workspace.");
            }
        });
}

void MainComponent::applyLayoutPreset(int preset)
{
    preset = juce::jlimit(0, 3, preset);
    const auto area = getLocalBounds().withTrimmedTop(132).withTrimmedBottom(40).reduced(10);
    const int gap = 8;
    const int browserW = juce::jlimit(190, 280, area.getWidth() / 6);
    const int workX = area.getX() + browserW + gap;
    const int workW = juce::jmax(320, area.getRight() - workX);
    const int workH = area.getHeight();

    // Bounds de abertura para painéis opcionais, sem os colocar por cima da barra superior.
    noteWindow->setBounds(workX + 36, area.getY() + 34, juce::jmin(workW - 72, 980), juce::jmin(workH - 68, 560));
    vaultWindow->setBounds(workX + 70, area.getY() + 50, juce::jmin(workW - 140, 620), juce::jmin(workH - 100, 500));
    mashupWindow->setBounds(workX + 24, area.getY() + 24, workW - 48, juce::jmax(300, workH - 48));
    automationWindow->setBounds(area.getX() + 80, area.getY() + 70, juce::jmin(760, area.getWidth() - 160), 360);\n    settingsWindow->setBounds(area.getX() + (area.getWidth() - 620) / 2,
                              area.getY() + (area.getHeight() - 430) / 2,
                              juce::jmin(620, area.getWidth()), juce::jmin(430, area.getHeight()));

    for (auto* panel : { browserWindow.get(), timelineWindow.get(), consoleWindow.get(),
                         beatWindow.get(), noteWindow.get(), vaultWindow.get(),
                         mashupWindow.get(), settingsWindow.get(), automationWindow.get() })
        panel->setVisible(false);

    if (preset == 0) // Opalo Studio — arrangement above, mixer below
    {
        // Layout principal inspirado na referência: Playlist ampla e Mixer fixo na base.
        // Browser e Patterns ficam disponíveis por ícone, sem ocupar espaço no arranque.
        const int playlistHeight = juce::jlimit(280, juce::jmax(280, workH - 190), workH * 57 / 100);
        const int mixerY = area.getY() + playlistHeight + gap;
        const int mixerHeight = juce::jmax(190, area.getBottom() - mixerY);
        const int dockBrowserW = juce::jlimit(190, 250, area.getWidth() / 5);
        const int dockGap = 8;
        const int editorX = area.getX() + dockBrowserW + dockGap;
        const int editorW = juce::jmax(320, area.getRight() - editorX);
        browserWindow->setBounds(area.getX(), area.getY(), dockBrowserW, workH);
        timelineWindow->setBounds(editorX, area.getY(), editorW, playlistHeight);
        consoleWindow->setBounds(editorX, mixerY, editorW, mixerHeight);
        beatWindow->setBounds(editorX + 24, area.getY() + 28,
                              juce::jmin(editorW - 48, 980), juce::jmin(workH - 56, 260));
        noteWindow->setBounds(editorX + 18, area.getY() + 18,
                              juce::jmin(editorW - 36, 1100), juce::jmin(workH - 36, 560));
        browserWindow->setVisible(true);
        timelineWindow->setVisible(true);
        beatWindow->setVisible(false);
        noteWindow->setVisible(false);
        consoleWindow->setVisible(true);
    }
    else if (preset == 1) // Arrangement Focus
    {
        browserWindow->setBounds(area.getX(), area.getY(), browserW, workH);
        timelineWindow->setBounds(workX, area.getY(), workW, workH * 58 / 100);
        noteWindow->setBounds(workX, area.getY() + workH * 58 / 100 + gap, workW, workH * 42 / 100 - gap);
        browserWindow->setVisible(true);
        timelineWindow->setVisible(true);
        noteWindow->setVisible(true);
    }
    else if (preset == 2) // Mix Focus
    {
        timelineWindow->setBounds(area.getX(), area.getY(), area.getWidth(), workH * 34 / 100);
        consoleWindow->setBounds(area.getX(), area.getY() + workH * 34 / 100 + gap,
                                 area.getWidth(), workH * 66 / 100 - gap);
        timelineWindow->setVisible(true);
        consoleWindow->setVisible(true);
    }
    else // DJ Mashup
    {
        browserWindow->setBounds(area.getX(), area.getY(), browserW, workH);
        mashupWindow->setBounds(workX, area.getY(), workW, workH * 72 / 100);
        consoleWindow->setBounds(workX, area.getY() + workH * 72 / 100 + gap,
                                 workW, workH * 28 / 100 - gap);
        browserWindow->setVisible(true);
        mashupWindow->setVisible(true);
        consoleWindow->setVisible(true);
    }

    layoutInitialized = true;
    for (auto* panel : { browserWindow.get(), timelineWindow.get(), beatWindow.get(),
                         consoleWindow.get(), noteWindow.get(), vaultWindow.get(),
                         mashupWindow.get(), settingsWindow.get(), automationWindow.get() })
        if (panel->isVisible()) panel->toFront(false);

    repaint();
}

// ----- Pastas e ficheiros de apoio -----------------------------------------------

juce::File MainComponent::appDataFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("Opalo");
    folder.createDirectory();
    return folder;
}

juce::File MainComponent::projectsFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("Opalo")
                      .getChildFile("Projects");
    folder.createDirectory();
    return folder;
}

juce::File MainComponent::autosaveFile()
{
    return appDataFolder().getChildFile("autosave" + OpaloFile::extensionName());
}

juce::File MainComponent::recentListFile()
{
    return appDataFolder().getChildFile("recent.txt");
}

void MainComponent::loadRecentList()
{
    recentProjects.clear();

    juce::StringArray lines;
    lines.addLines(recentListFile().loadFileAsString());

    for (const auto& line : lines)
        if (recentProjects.size() < maxRecent && juce::File(line).existsAsFile())
            recentProjects.add(line);
}

void MainComponent::rememberRecent(const juce::File& file)
{
    const auto path = file.getFullPathName();
    recentProjects.removeString(path);
    recentProjects.insert(0, path);

    while (recentProjects.size() > maxRecent)
        recentProjects.remove(recentProjects.size() - 1);

    recentListFile().replaceWithText(recentProjects.joinIntoString("\n"));
}

// ----- Projeto -------------------------------------------------------------------

bool MainComponent::canDiscardChanges()
{
    if (! dirty)
        return true;

    return juce::NativeMessageBox::showOkCancelBox(juce::MessageBoxIconType::WarningIcon,
                                                   "Opalo",
                                                   "There are unsaved changes. Discard them?",
                                                   this, nullptr);
}

void MainComponent::newProject()
{
    if (! canDiscardChanges())
        return;

    loading = true;
    model.resetToDefault();
    loading = false;

    currentFile = juce::File{};
    dirty = false;
    resetHistory();
    fileLabel.setText("New project", juce::dontSendNotification);
    updateTitle();
}

void MainComponent::openProject()
{
    if (! canDiscardChanges())
        return;

    projectChooser = std::make_unique<juce::FileChooser>("Open Opalo project",
                                                         projectsFolder(),
                                                         "*" + OpaloFile::extensionName());
    projectChooser->launchAsync(juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles,
                                [this](const juce::FileChooser& fc)
                                {
                                    const auto file = fc.getResult();
                                    if (file.existsAsFile())
                                        loadProjectFrom(file);
                                });
}

void MainComponent::showOpenMenu()
{
    juce::PopupMenu menu;

    for (int i = 0; i < recentProjects.size(); ++i)
        menu.addItem(i + 1, juce::File(recentProjects[i]).getFileNameWithoutExtension());

    if (recentProjects.isEmpty())
        menu.addItem(-1, "No recent projects", false);

    menu.addSeparator();
    menu.addItem(1000, "Browse…");

    juce::Component::SafePointer<MainComponent> safe(this);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&openButton),
                       [safe](int result)
                       {
                           auto* self = safe.getComponent();
                           if (self == nullptr)
                               return;

                           if (result == 1000)
                               self->openProject();
                           else if (result >= 1)
                               self->openRecent(result - 1);
                       });
}

void MainComponent::openRecent(int index)
{
    if (! juce::isPositiveAndBelow(index, recentProjects.size()))
        return;

    if (! canDiscardChanges())
        return;

    loadProjectFrom(juce::File(recentProjects[index]));
}

void MainComponent::loadProjectFrom(const juce::File& file)
{
    juce::String error;
    loading = true;
    const bool ok = OpaloFile::loadProject(model, file, error);
    model.changed();
    loading = false;

    if (ok)
    {
        currentFile = file;
        dirty = false;
        rememberRecent(file);
        resetHistory();
        fileLabel.setText("Opened " + file.getFileName(), juce::dontSendNotification);
    }
    else
    {
        fileLabel.setText(error, juce::dontSendNotification);
    }

    updateTitle();
}

void MainComponent::saveProject()
{
    if (currentFile == juce::File{})
    {
        saveProjectAs();
        return;
    }

    writeProject(currentFile);
}

void MainComponent::saveProjectAs()
{
    projectChooser = std::make_unique<juce::FileChooser>("Save Opalo project",
                                                         projectsFolder().getChildFile("Untitled" + OpaloFile::extensionName()),
                                                         "*" + OpaloFile::extensionName());
    projectChooser->launchAsync(juce::FileBrowserComponent::saveMode
                                    | juce::FileBrowserComponent::warnAboutOverwriting,
                                [this](const juce::FileChooser& fc)
                                {
                                    auto file = fc.getResult();
                                    if (file == juce::File{})
                                        return;

                                    if (file.getFileExtension() != OpaloFile::extensionName())
                                        file = file.withFileExtension(OpaloFile::extensionName());

                                    writeProject(file);
                                });
}

void MainComponent::writeProject(const juce::File& file)
{
    if (changePending)
        flushHistory();

    juce::String error;
    if (OpaloFile::saveProject(model, file, error))
    {
        currentFile = file;
        dirty = false;
        rememberRecent(file);
        fileLabel.setText("Saved " + file.getFileName(), juce::dontSendNotification);
    }
    else
    {
        fileLabel.setText(error, juce::dontSendNotification);
    }

    updateTitle();
}

void MainComponent::updateTitle()
{
    const auto name = currentFile == juce::File{} ? juce::String("Untitled")
                                                  : currentFile.getFileNameWithoutExtension();
    const juce::String title = "Opalo  —  " + name + (dirty ? "  •" : "");

    if (onTitleChanged)
        onTitleChanged(title);
}

// ----- Desfazer e refazer --------------------------------------------------------

juce::String MainComponent::snapshot() const
{
    return OpaloFile::toXml(model)->toString();
}

void MainComponent::resetHistory()
{
    history.reset(snapshot());
    changePending = false;
}

void MainComponent::flushHistory()
{
    changePending = false;
    history.record(snapshot());
}

void MainComponent::applySnapshot(const juce::String& state)
{
    if (state.isEmpty())
        return;

    auto xml = juce::XmlDocument::parse(state);
    if (xml == nullptr)
        return;

    juce::String error;
    loading = true;
    OpaloFile::fromXml(model, *xml, error);
    model.changed();
    loading = false;

    dirty = true;
    updateTitle();
}

void MainComponent::undo()
{
    if (changePending)
        flushHistory();

    applySnapshot(history.undo());
    fileLabel.setText("Undo", juce::dontSendNotification);
}

void MainComponent::redo()
{
    if (changePending)
        flushHistory();

    applySnapshot(history.redo());
    fileLabel.setText("Redo", juce::dontSendNotification);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (loading)
        return;

    // Agrupa alterações seguidas (por exemplo, arrastar um fader) num só passo de desfazer.
    changePending = true;
    changeTicks = 0;

    if (! dirty)
    {
        dirty = true;
        updateTitle();
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (! key.getModifiers().isCtrlDown())
        return false;

    const auto c = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());

    if (c == 's')
    {
        if (key.getModifiers().isShiftDown()) saveProjectAs();
        else                                  saveProject();
        return true;
    }

    if (c == 'o') { openProject(); return true; }
    if (c == 'n') { newProject();  return true; }

    if (c == 'z')
    {
        if (key.getModifiers().isShiftDown()) redo();
        else                                  undo();
        return true;
    }

    if (c == 'y') { redo(); return true; }

    return false;
}

// ----- Áudio ---------------------------------------------------------------------

void MainComponent::loadAudio()
{
    audioChooser = std::make_unique<juce::FileChooser>("Open audio file",
                                                       juce::File{},
                                                       "*.wav;*.aif;*.aiff;*.flac;*.ogg");
    audioChooser->launchAsync(juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& fc)
                              {
                                  auto file = fc.getResult();
                                  if (! file.existsAsFile())
                                      return;

                                  if (engine.loadFile(file))
                                  {
                                      fileLabel.setText(file.getFileName(), juce::dontSendNotification);
                                      waveform.setFile(file);
                                  }
                                  else
                                  {
                                      fileLabel.setText("Could not open: " + file.getFileName(),
                                                        juce::dontSendNotification);
                                      waveform.setFile(juce::File{});
                                  }

                                  updateControls();
                              });
}

void MainComponent::timerCallback()
{
    model.playheadSeconds = engine.getPosition();

    timeLabel.setText(formatTime(engine.getPosition()) + " / " + formatTime(engine.getLength()),
                      juce::dontSendNotification);
    waveform.setPlayhead(engine.getPosition(), engine.getLength());
    updateControls();

    if (changePending && ++changeTicks >= historyDelayTicks)
        flushHistory();

    // Cópia de recuperação a cada cinco minutos, se houver alterações.
    if (++autosaveTicks >= autosaveIntervalTicks)
    {
        autosaveTicks = 0;
        if (dirty)
        {
            juce::String error;
            OpaloFile::saveProject(model, autosaveFile(), error);
        }
    }
}

void MainComponent::updateControls()
{
    const bool hasFile = engine.hasFile();
    const bool playing = engine.isPlaying();

    playButton.setEnabled(hasFile && ! playing);
    stopButton.setEnabled(hasFile);
    playButton.setToggleState(playing, juce::dontSendNotification);
    bpmLabel.setText(juce::String(model.bpm, 1) + " BPM", juce::dontSendNotification);
}

// ----- Desenho e layout ----------------------------------------------------------

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(OpaloDesign::background);
    auto menuBar = getLocalBounds().removeFromTop(34);
    g.setColour(OpaloDesign::surface0);
    g.fillRect(menuBar);
    auto transport = getLocalBounds().withTrimmedTop(34).removeFromTop(52);
    g.setColour(OpaloDesign::surface1);
    g.fillRect(transport);
    auto shortcuts = getLocalBounds().withTrimmedTop(86).removeFromTop(46);
    g.setColour(OpaloDesign::surface0);
    g.fillRect(shortcuts);
    auto waveformFooter = getLocalBounds().removeFromBottom(40);
    g.setColour(OpaloDesign::surface1);
    g.fillRect(waveformFooter);
    g.setColour(OpaloDesign::accent.withAlpha(0.48f));
    g.fillRect(0, 132, getWidth(), 1);
}

void MainComponent::resized()
{
    auto menu = getLocalBounds().removeFromTop(34).reduced(10, 2);
    fileMenuButton.setBounds(menu.removeFromLeft(58));
    editMenuButton.setBounds(menu.removeFromLeft(58));
    viewMenuButton.setBounds(menu.removeFromLeft(58));
    toolsMenuButton.setBounds(menu.removeFromLeft(62));
    helpMenuButton.setBounds(menu.removeFromLeft(58));

    auto transport = getLocalBounds().withTrimmedTop(34).removeFromTop(52).reduced(10, 7);
    newButton.setBounds(transport.removeFromLeft(58));
    transport.removeFromLeft(5);
    openButton.setBounds(transport.removeFromLeft(58));
    transport.removeFromLeft(5);
    saveButton.setBounds(transport.removeFromLeft(58));
    transport.removeFromLeft(5);
    saveAsButton.setBounds(transport.removeFromLeft(70));
    transport.removeFromLeft(10);
    loadAudioButton.setBounds(transport.removeFromLeft(94));
    transport.removeFromLeft(8);
    startButton.setBounds(transport.removeFromLeft(42));
    transport.removeFromLeft(4);
    playButton.setBounds(transport.removeFromLeft(38));
    transport.removeFromLeft(4);
    stopButton.setBounds(transport.removeFromLeft(38));
    transport.removeFromLeft(4);
    recordButton.setBounds(transport.removeFromLeft(58));
    transport.removeFromLeft(10);
    bpmLabel.setBounds(transport.removeFromLeft(86));
    transport.removeFromLeft(10);
    timeLabel.setBounds(transport.removeFromRight(90));
    transport.removeFromRight(10);
    volumeSlider.setBounds(transport.removeFromRight(100));
    transport.removeFromRight(12);
    fileLabel.setBounds(transport);

    auto shortcuts = getLocalBounds().withTrimmedTop(86).removeFromTop(46).reduced(10, 6);
    browserButton.setBounds(shortcuts.removeFromLeft(38));
    timelineButton.setBounds(shortcuts.removeFromLeft(38));
    beatButton.setBounds(shortcuts.removeFromLeft(38));
    consoleButton.setBounds(shortcuts.removeFromLeft(38));
    noteButton.setBounds(shortcuts.removeFromLeft(38));
    vaultButton.setBounds(shortcuts.removeFromLeft(38));
    mashupButton.setBounds(shortcuts.removeFromLeft(38));
    automationButton.setBounds(shortcuts.removeFromLeft(38));
    settingsButton.setBounds(shortcuts.removeFromRight(38));
    for (auto* button : { &browserButton, &timelineButton, &beatButton, &consoleButton,
                          &noteButton, &vaultButton, &mashupButton, &automationButton, &settingsButton })
    {
        button->setColour(juce::TextButton::buttonColourId, OpaloDesign::surface1);
        button->setColour(juce::TextButton::buttonOnColourId, OpaloDesign::accentDim);
        button->setColour(juce::TextButton::textColourOffId, OpaloDesign::text);
        button->setColour(juce::TextButton::textColourOnId, OpaloDesign::background);
    }

    waveform.setBounds(getLocalBounds().withTrimmedTop(getHeight() - 40).reduced(10, 4));

    if (! layoutInitialized)
        applyLayoutPreset(0);
    else
    {
        const auto workspace = getLocalBounds().withTrimmedTop(132).withTrimmedBottom(40).reduced(4);
        for (auto* panel : { browserWindow.get(), timelineWindow.get(), consoleWindow.get(),
                             beatWindow.get(), noteWindow.get(), vaultWindow.get(),
                             mashupWindow.get(), settingsWindow.get() })
        {
            auto bounds = panel->getBounds();
            bounds.setWidth(juce::jlimit(280, workspace.getWidth(), bounds.getWidth()));
            bounds.setHeight(juce::jlimit(150, workspace.getHeight(), bounds.getHeight()));
            bounds.setX(juce::jlimit(workspace.getX(), workspace.getRight() - bounds.getWidth(), bounds.getX()));
            bounds.setY(juce::jlimit(workspace.getY(), workspace.getBottom() - bounds.getHeight(), bounds.getY()));
            panel->setBounds(bounds);
        }
    }

}

juce::String MainComponent::formatTime(double seconds)
{
    if (seconds < 0.0)
        seconds = 0.0;

    const int totalCentis = (int) juce::roundToInt(seconds * 100.0);
    const int minutes = totalCentis / 6000;
    const int secs = (totalCentis / 100) % 60;
    const int centis = totalCentis % 100;

    return juce::String::formatted("%02d:%02d.%02d", minutes, secs, centis);
}
