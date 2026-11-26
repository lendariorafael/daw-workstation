#include "MainComponent.h"

namespace
{
    constexpr int maxRecent = 8;
    constexpr int historyDelayTicks = 6;     // ~200 ms de pausa antes de registar uma alteração
    constexpr int autosaveIntervalTicks = 30 * 60;
}

MainComponent::MainComponent()
{
    I18n::current() = I18n::fromCode(OpaloSettings::get().getValue("language", "pt"));
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);
    setWantsKeyboardFocus(true);

    loadAudioButton.onClick = [this] { loadAudio(); };
    playButton.onClick = [this] { engine.play(); };
    stopButton.onClick = [this] { engine.stop(); };
    recordButton.setTooltip("Audio recording is not available in this build yet.");
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
             static_cast<juce::Component*>(&topLogo),
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
             static_cast<juce::Component*>(&settingsButton) })
        addAndMakeVisible(component);

    setSize(1440, 900);

    browserWindow  = std::make_unique<PanelWindow>(I18n::t("view.browser"), new BrowserPanel(model), 230, 620);
    timelineWindow = std::make_unique<PanelWindow>(I18n::t("view.playlist"), new TimelinePanel(model), 900, 330);
    consoleWindow  = std::make_unique<PanelWindow>(I18n::t("view.mixer"), new ConsolePanel(model), 900, 230);
    beatWindow     = std::make_unique<PanelWindow>(I18n::t("view.beat"), new BeatDeckPanel(model), 900, 190);
    noteWindow     = std::make_unique<PanelWindow>(I18n::t("view.notes"), new NoteCanvasPanel(model), 900, 430);
    vaultWindow    = std::make_unique<PanelWindow>(I18n::t("view.vault"), new PluginVaultPanel(model), 520, 420);
    mashupWindow   = std::make_unique<PanelWindow>(I18n::t("view.mashup"), new MashupPanel(model, engine), 920, 520);
    settingsWindow = std::make_unique<PanelWindow>(
        I18n::t("settings.title"),
        new SettingsPanel(engine, [this] { refreshTexts(); },
                          [this](int preset) { applyLayoutPreset(preset); }),
        620, 430);

    for (auto* panel : { browserWindow.get(), timelineWindow.get(), consoleWindow.get(),
                         beatWindow.get(), noteWindow.get(), vaultWindow.get(),
                         mashupWindow.get(), settingsWindow.get() })
        addAndMakeVisible(panel);

    bindPanel(browserButton, *browserWindow);
    bindPanel(timelineButton, *timelineWindow);
    bindPanel(consoleButton, *consoleWindow);
    bindPanel(beatButton, *beatWindow);
    bindPanel(noteButton, *noteWindow);
    bindPanel(vaultButton, *vaultWindow);
    bindPanel(mashupButton, *mashupWindow);
    bindPanel(settingsButton, *settingsWindow);

    browserButton.setButtonText("▤  Browser");
    timelineButton.setButtonText("▦  Playlist");
    consoleButton.setButtonText("▥  Mixer");
    beatButton.setButtonText("☷  Channel Rack");
    noteButton.setButtonText("♫  Piano Roll");
    vaultButton.setButtonText("◈  Plugins");
    mashupButton.setButtonText("◉  Mashup");
    settingsButton.setButtonText("⚙  Settings");

    browserButton.setTooltip("Show or hide the sound browser");
    timelineButton.setTooltip("Show or hide the Playlist");
    consoleButton.setTooltip("Show or hide the Mixer");
    beatButton.setTooltip("Show or hide the Channel Rack");
    noteButton.setTooltip("Show or hide the Piano Roll");
    vaultButton.setTooltip("Show or hide the VST3 Plugin Vault");
    mashupButton.setTooltip("Show or hide the DJ Mashup decks");

    refreshTexts();
    loadRecentList();
    resetHistory();
    model.addChangeListener(this);
    startTimerHz(30);

    applyLayoutPreset(0);

    splash = std::make_unique<SplashScreen>();
    addAndMakeVisible(*splash);
    splash->setBounds(getLocalBounds());
    splash->onFinished = [safe = juce::Component::SafePointer<MainComponent>(this)]
    {
        juce::MessageManager::callAsync([safe]
        {
            if (auto* self = safe.getComponent())
                self->splash.reset();
        });
    };

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
    playButton.setButtonText(I18n::t("transport.play"));
    stopButton.setButtonText(I18n::t("transport.stop"));
    browserButton.setButtonText(I18n::t("view.browser"));
    timelineButton.setButtonText(I18n::t("view.playlist"));
    consoleButton.setButtonText(I18n::t("view.mixer"));
    beatButton.setButtonText(I18n::t("view.beat"));
    noteButton.setButtonText(I18n::t("view.notes"));
    vaultButton.setButtonText(I18n::t("view.vault"));
    mashupButton.setButtonText(I18n::t("view.mashup"));
    settingsButton.setButtonText(I18n::t("view.settings"));
}

void MainComponent::bindPanel(juce::TextButton& button, PanelWindow& window)
{
    button.setClickingTogglesState(true);
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
    topLogo.setPhase(static_cast<float> (juce::Time::getMillisecondCounterHiRes() / 6000.0));
    updateControls();

    if (changePending && ++changeTicks >= historyDelayTicks)
        flushHistory();

    // Cópia de recuperação a cada minuto, se houver alterações.
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
}

// ----- Desenho e layout ----------------------------------------------------------

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(DawLookAndFeel::background);

    auto topBar = getLocalBounds().removeFromTop(64);
    g.setColour(DawLookAndFeel::panel);
    g.fillRect(topBar);
    g.setColour(DawLookAndFeel::accent.withAlpha(0.35f));
    g.fillRect(topBar.removeFromBottom(1));

    auto panelBar = getLocalBounds().withTrimmedTop(64).removeFromTop(44);
    g.setColour(DawLookAndFeel::panel.darker(0.15f));
    g.fillRect(panelBar);
}

void MainComponent::resized()
{
    auto bar = getLocalBounds().removeFromTop(64).reduced(16, 12);

    topLogo.setBounds(bar.removeFromLeft(200));
    bar.removeFromLeft(12);
    loadAudioButton.setBounds(bar.removeFromLeft(104));
    bar.removeFromLeft(8);
    playButton.setBounds(bar.removeFromLeft(72));
    bar.removeFromLeft(8);
    stopButton.setBounds(bar.removeFromLeft(72));
    bar.removeFromLeft(16);

    timeLabel.setBounds(bar.removeFromRight(190));
    bar.removeFromRight(16);
    settingsButton.setBounds(bar.removeFromRight(96));
    bar.removeFromRight(16);
    volumeSlider.setBounds(bar.removeFromRight(220));
    bar.removeFromRight(16);

    fileLabel.setBounds(bar);

    auto panelBar = getLocalBounds().withTrimmedTop(64).removeFromTop(44).reduced(16, 8);

    auto fileButtons = panelBar.removeFromRight(4 * 92 + 3 * 8);
    newButton.setBounds(fileButtons.removeFromLeft(92));
    fileButtons.removeFromLeft(8);
    openButton.setBounds(fileButtons.removeFromLeft(92));
    fileButtons.removeFromLeft(8);
    saveButton.setBounds(fileButtons.removeFromLeft(92));
    fileButtons.removeFromLeft(8);
    saveAsButton.setBounds(fileButtons);

    browserButton.setBounds(panelBar.removeFromLeft(88));
    panelBar.removeFromLeft(8);
    timelineButton.setBounds(panelBar.removeFromLeft(88));
    panelBar.removeFromLeft(8);
    consoleButton.setBounds(panelBar.removeFromLeft(88));
    panelBar.removeFromLeft(8);
    beatButton.setBounds(panelBar.removeFromLeft(96));
    panelBar.removeFromLeft(8);
    noteButton.setBounds(panelBar.removeFromLeft(104));
    panelBar.removeFromLeft(8);
    vaultButton.setBounds(panelBar.removeFromLeft(104));
    panelBar.removeFromLeft(8);
    mashupButton.setBounds(panelBar.removeFromLeft(88));

    waveform.setBounds(getLocalBounds().withTrimmedTop(108).reduced(16));

    if (splash != nullptr)
        splash->setBounds(getLocalBounds());
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
