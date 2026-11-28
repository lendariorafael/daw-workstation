#include "MainComponent.h"

namespace
{
    constexpr int maxRecent = 8;
    constexpr int historyDelayTicks = 6;     // ~200 ms de pausa antes de registar uma alteração
    constexpr int autosaveIntervalTicks = 30 * 60;
    constexpr int topBarHeight = TopBar::height;
    constexpr int sidebarWidth = 280;
}

MainComponent::MainComponent()
{
    I18n::current() = I18n::fromCode(OpaloSettings::get().getValue("language", "pt"));

    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);
    setWantsKeyboardFocus(true);
    setSize(1440, 860);

    topBar.onMenu = [this](const juce::String& key) { showMenu(key); };
    topBar.onPlay = [this] { engine.play(); };
    topBar.onPause = [this] { engine.pause(); };
    topBar.onStop = [this] { engine.stop(); };
    topBar.onPrevious = [this] { engine.seek(0.0); };
    topBar.onNext = [this] { engine.seek(engine.getLength()); };
    topBar.onLoop = [this](bool on) { engine.setLooping(on); };
    topBar.onBpm = [this](double bpm) { model.bpm = bpm; model.changed(); };

    addAndMakeVisible(topBar);
    addAndMakeVisible(taskBar);
    addAndMakeVisible(sidebar);
    addAndMakeVisible(playlist);

    // Janelas independentes (começam escondidas).
    consoleWindow  = std::make_unique<PanelWindow>(I18n::t("view.mixer"), new ConsolePanel(model),     1280, 640);
    noteWindow     = std::make_unique<PanelWindow>("Piano Roll",           new NoteCanvasPanel(model), 980, 640);
    beatWindow     = std::make_unique<PanelWindow>("Beat Editor",          new BeatDeckPanel(model),   880, 520);
    mashupWindow   = std::make_unique<PanelWindow>(I18n::t("view.mashup"), new MashupPanel(model, engine), 1000, 560);
    vaultWindow    = std::make_unique<PanelWindow>(I18n::t("view.vault"),  new PluginVaultPanel(model), 760, 520);
    settingsWindow = std::make_unique<PanelWindow>(I18n::t("settings.title"),
                                                   new SettingsPanel(engine, [this] { refreshTexts(); }),
                                                   660, 560);

    // Barra inferior: uma aba por janela. A playlist está sempre visível.
    taskBar.addTab(I18n::t("tab.playlist"), [this] { playlist.grabKeyboardFocus(); });
    tabMixer    = taskBar.addTab(I18n::t("view.mixer"),  [this] { toggleWindow(*consoleWindow); });
    tabPiano    = taskBar.addTab("Piano Roll",            [this] { toggleWindow(*noteWindow); });
    tabBeat     = taskBar.addTab("Beat Editor",           [this] { toggleWindow(*beatWindow); });
    tabMashup   = taskBar.addTab(I18n::t("view.mashup"), [this] { toggleWindow(*mashupWindow); });
    tabVault    = taskBar.addTab(I18n::t("view.vault"),  [this] { toggleWindow(*vaultWindow); });
    tabSettings = taskBar.addTab(I18n::t("view.settings"), [this] { toggleWindow(*settingsWindow); });
    taskBar.setTabActive(0, true);

    bindWindow(*consoleWindow,  tabMixer);
    bindWindow(*noteWindow,     tabPiano);
    bindWindow(*beatWindow,     tabBeat);
    bindWindow(*mashupWindow,   tabMashup);
    bindWindow(*vaultWindow,    tabVault);
    bindWindow(*settingsWindow, tabSettings);

    loadRecentList();
    resetHistory();

    model.addChangeListener(this);
    startTimerHz(30);

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
    refreshTexts();
}

MainComponent::~MainComponent()
{
    stopTimer();
    model.removeChangeListener(this);
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

// ----- Janelas e menus -----------------------------------------------------------

void MainComponent::toggleWindow(PanelWindow& window)
{
    window.setVisible(! window.isVisible());
    if (window.isVisible())
        window.toFront(true);
}

void MainComponent::bindWindow(PanelWindow& window, int tabIndex)
{
    window.onVisibilityChanged = [this, tabIndex](bool visible)
    {
        taskBar.setTabActive(tabIndex, visible);
    };
}

void MainComponent::refreshTexts()
{
    topBar.refreshTexts();
}

void MainComponent::showMenu(const juce::String& key)
{
    juce::PopupMenu menu;

    if (key == "menu.file")
    {
        juce::PopupMenu recent;
        for (int i = 0; i < recentProjects.size(); ++i)
            recent.addItem(100 + i, juce::File(recentProjects[i]).getFileNameWithoutExtension());
        if (recentProjects.isEmpty())
            recent.addItem(999, "—", false);

        menu.addItem(1, I18n::t("file.new"));
        menu.addItem(2, I18n::t("file.open"));
        menu.addSubMenu(I18n::t("file.recent"), recent);
        menu.addSeparator();
        menu.addItem(3, I18n::t("file.save"));
        menu.addItem(4, I18n::t("file.saveAs"));
        menu.addSeparator();
        menu.addItem(5, I18n::t("file.loadAudio"));
    }
    else if (key == "menu.edit")
    {
        menu.addItem(10, I18n::t("edit.undo"));
        menu.addItem(11, I18n::t("edit.redo"));
    }
    else if (key == "menu.add")
    {
        menu.addItem(20, I18n::t("add.channel"));
    }
    else if (key == "menu.patterns")
    {
        menu.addItem(30, "Beat Editor");
        menu.addItem(31, "Piano Roll");
    }
    else if (key == "menu.view")
    {
        menu.addItem(40, I18n::t("tab.playlist"));
        menu.addItem(41, I18n::t("view.mixer"));
        menu.addItem(42, "Piano Roll");
        menu.addItem(43, "Beat Editor");
        menu.addItem(44, I18n::t("view.mashup"));
        menu.addItem(45, I18n::t("view.vault"));
    }
    else if (key == "menu.options")
    {
        menu.addItem(50, I18n::t("view.settings"));
    }
    else if (key == "menu.tools")
    {
        menu.addItem(45, I18n::t("view.vault"));
    }
    else if (key == "menu.help")
    {
        menu.addItem(60, "About Opalo");
    }

    juce::Component::SafePointer<MainComponent> safe(this);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&topBar),
                       [safe](int id)
                       {
                           if (auto* self = safe.getComponent())
                               if (id > 0)
                                   self->handleMenu(id);
                       });
}

void MainComponent::handleMenu(int id)
{
    switch (id)
    {
        case 1:  newProject(); break;
        case 2:  openProject(); break;
        case 3:  saveProject(); break;
        case 4:  saveProjectAs(); break;
        case 5:  loadAudio(); break;

        case 10: undo(); break;
        case 11: redo(); break;

        case 20:
            model.addChannel("Channel " + juce::String((int) model.channels.size() + 1),
                             ProjectModel::paletteColour((int) model.channels.size()));
            break;

        case 30: beatWindow->setVisible(true);  beatWindow->toFront(true);  break;
        case 31:
        case 42: noteWindow->setVisible(true);  noteWindow->toFront(true);  break;
        case 40: playlist.grabKeyboardFocus(); break;
        case 41: consoleWindow->setVisible(true); consoleWindow->toFront(true); break;
        case 43: beatWindow->setVisible(true);  beatWindow->toFront(true);  break;
        case 44: mashupWindow->setVisible(true); mashupWindow->toFront(true); break;
        case 45: vaultWindow->setVisible(true); vaultWindow->toFront(true); break;
        case 50: settingsWindow->setVisible(true); settingsWindow->toFront(true); break;

        case 60:
            juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Opalo",
                                                        I18n::t("about.text"));
            break;

        default:
            if (id >= 100)
                openRecent(id - 100);
            break;
    }
}

// ----- Projeto -------------------------------------------------------------------

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
        taskBar.setStatus("Opened " + file.getFileName());
    }
    else
    {
        taskBar.setStatus(error);
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
        taskBar.setStatus("Saved " + file.getFileName());
    }
    else
    {
        taskBar.setStatus(error);
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
}

void MainComponent::redo()
{
    if (changePending)
        flushHistory();

    applySnapshot(history.redo());
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
    if (c == 'z') { if (key.getModifiers().isShiftDown()) redo(); else undo(); return true; }
    if (c == 'y') { redo(); return true; }

    return false;
}

// ----- Áudio ---------------------------------------------------------------------

void MainComponent::loadAudio()
{
    audioChooser = std::make_unique<juce::FileChooser>("Open audio file",
                                                       juce::File{},
                                                       OpaloAudioFormats::audioWildcard());
    audioChooser->launchAsync(juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& fc)
                              {
                                  auto file = fc.getResult();
                                  if (! file.existsAsFile())
                                      return;

                                  taskBar.setStatus(engine.loadFile(file)
                                                        ? "Loaded " + file.getFileName()
                                                        : "Could not open " + file.getFileName());
                              });
}

void MainComponent::timerCallback()
{
    model.playheadSeconds = engine.getPosition();

    topBar.update(engine.isPlaying(), engine.isLooping(), engine.getPosition(),
                  model.bpm, engine.getCpuUsage() * 100.0);

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

// ----- Desenho e layout ----------------------------------------------------------

void MainComponent::paint(juce::Graphics& g)
{
    space.paint(g, getLocalBounds());
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    topBar.setBounds(area.removeFromTop(topBarHeight));
    taskBar.setBounds(area.removeFromBottom(TaskBar::height));
    sidebar.setBounds(area.removeFromLeft(sidebarWidth));
    playlist.setBounds(area);

    if (splash != nullptr)
        splash->setBounds(getLocalBounds());
}
