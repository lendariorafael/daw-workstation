#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../../Model/ProjectFile.h"
#include "../../Audio/AudioFormats.h"
#include "../../Analysis/KeyDetector.h"
#include "../DawLookAndFeel.h"
#include "../../I18n/Strings.h"

// Browser: as tuas pastas de kits e sons. Escolhe um ficheiro e atribui-o ao canal selecionado,
// importa um MIDI para as notas do canal, ou deteta o tom de um áudio.
class BrowserPanel : public juce::Component,
                     private juce::FileBrowserListener
{
public:
    // Pasta padrão dos kits: Documentos/Opalo/Kits. Criada na primeira utilização.
    static juce::File kitsFolder()
    {
        auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                          .getChildFile("Opalo")
                          .getChildFile("Kits");
        folder.createDirectory();
        return folder;
    }

    explicit BrowserPanel(ProjectModel& m) : model(m)
    {
        scanThread.startThread();
        contents.setDirectory(kitsFolder(), true, true);

        tree.addListener(this);

        for (auto* l : { &infoName, &infoAuthor, &infoDate, &infoSize })
        {
            l->setFont(DawLookAndFeel::uiFont(OpaloDesign::fontSmall));
            l->setColour(juce::Label::textColourId, DawLookAndFeel::muted);
            addAndMakeVisible(*l);
        }
        infoName.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontBody, true));
        infoName.setColour(juce::Label::textColourId, DawLookAndFeel::text);
        infoName.setText(I18n::t("browser.info.none"), juce::dontSendNotification);

        addAndMakeVisible(tree);
        addAndMakeVisible(assignButton);
        addAndMakeVisible(importButton);
        addAndMakeVisible(keyButton);
        addAndMakeVisible(status);

        assignButton.onClick = [this] { assignSelected(); };
        importButton.onClick = [this] { importSelectedMidi(); };
        keyButton.onClick    = [this] { detectKey(); };

        status.setFont(DawLookAndFeel::uiFont(12.0f));
        status.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
        status.setText("Select a sound, then assign it to the selected channel.", juce::dontSendNotification);
    }

    ~BrowserPanel() override
    {
        tree.removeListener(this);
        keyPool.removeAllJobs(true, 5000);
    }

    // Muda a pasta mostrada (usado pelas categorias da barra lateral).
    void setRoot(const juce::File& folder)
    {
        folder.createDirectory();
        contents.setDirectory(folder, true, true);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);

        auto bottom = area.removeFromBottom(64);
        status.setBounds(bottom.removeFromBottom(22));
        bottom.removeFromBottom(6);
        auto buttons = bottom.reduced(0, 2);
        const int third = (buttons.getWidth() - 8) / 3;
        assignButton.setBounds(buttons.removeFromLeft(third));
        buttons.removeFromLeft(4);
        importButton.setBounds(buttons.removeFromLeft(third));
        buttons.removeFromLeft(4);
        keyButton.setBounds(buttons);

        area.removeFromBottom(8);
        auto info = area.removeFromBottom(88);
        infoName.setBounds(info.removeFromTop(20));
        infoAuthor.setBounds(info.removeFromTop(18));
        infoDate.setBounds(info.removeFromTop(18));
        infoSize.setBounds(info.removeFromTop(18));

        tree.setBounds(area);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::panel);
    }

    // FileBrowserListener
    void selectionChanged() override { updateInfo(); }
    void fileClicked(const juce::File&, const juce::MouseEvent&) override {}
    void fileDoubleClicked(const juce::File&) override {}
    void browserRootChanged(const juce::File&) override {}

private:
    void updateInfo()
    {
        const auto file = tree.getSelectedFile();
        if (! file.exists())
        {
            infoName.setText(I18n::t("browser.info.none"), juce::dontSendNotification);
            infoAuthor.setText({}, juce::dontSendNotification);
            infoDate.setText({}, juce::dontSendNotification);
            infoSize.setText({}, juce::dontSendNotification);
            return;
        }

        infoName.setText(file.getFileName(), juce::dontSendNotification);
        infoAuthor.setText(I18n::t("browser.info.author") + ": —", juce::dontSendNotification);
        infoDate.setText(I18n::t("browser.info.date") + ": "
                             + file.getLastModificationTime().toString(true, false), juce::dontSendNotification);
        infoSize.setText(I18n::t("browser.info.size") + ": "
                             + juce::File::descriptionOfSizeInBytes(file.getSize()), juce::dontSendNotification);
    }

    void assignSelected()
    {
        const auto file = tree.getSelectedFile();
        if (! file.existsAsFile())
        {
            status.setText("Select a file first.", juce::dontSendNotification);
            return;
        }

        if (file.hasFileExtension("mid;midi"))
        {
            importSelectedMidi();
            return;
        }

        if (model.channels.empty())
            return;

        auto& ch = model.channels[(size_t) model.selectedChannel];
        ch.samplePath = file.getFullPathName();
        ch.name = file.getFileNameWithoutExtension();
        model.changed();

        status.setText("Assigned to channel " + juce::String(model.selectedChannel + 1), juce::dontSendNotification);
    }

    void importSelectedMidi()
    {
        const auto file = tree.getSelectedFile();
        if (! file.existsAsFile() || ! file.hasFileExtension("mid;midi"))
        {
            status.setText("Select a .mid file first.", juce::dontSendNotification);
            return;
        }

        if (model.channels.empty())
            return;

        int imported = 0, outside = 0;
        juce::String error;
        auto& ch = model.channels[(size_t) model.selectedChannel];
        OpaloFile::importMidi(file, ch, imported, outside, error);

        if (error.isNotEmpty())
        {
            status.setText(error, juce::dontSendNotification);
            return;
        }

        model.changed();
        status.setText(juce::String(imported) + " notes imported, " + juce::String(outside)
                           + " outside the first 16 steps.", juce::dontSendNotification);
    }

    void detectKey()
    {
        const auto file = tree.getSelectedFile();
        if (! file.existsAsFile() || file.hasFileExtension("mid;midi"))
        {
            status.setText("Select an audio file first.", juce::dontSendNotification);
            return;
        }

        status.setText("Detecting key of " + file.getFileName() + "…", juce::dontSendNotification);

        keyPool.addJob(new KeyJob(file,
                                  [safe = juce::Component::SafePointer<BrowserPanel>(this)](juce::File f, KeyResult r)
                                  {
                                      if (auto* self = safe.getComponent())
                                          self->showKey(f, r);
                                  }),
                       true);
    }

    void showKey(const juce::File& file, const KeyResult& r)
    {
        if (! r.valid)
        {
            status.setText("Could not detect the key of " + file.getFileName(), juce::dontSendNotification);
            return;
        }

        status.setText(file.getFileName() + "  ·  " + r.name + "  ·  confidence "
                           + juce::String(r.confidence, 2), juce::dontSendNotification);
    }

    ProjectModel& model;
    juce::TimeSliceThread scanThread { "Opalo browser" };
    juce::WildcardFileFilter filter { OpaloAudioFormats::audioWildcard() + ";*.mid;*.midi", "*", "Sons e MIDI" };
    juce::DirectoryContentsList contents { &filter, scanThread };
    juce::FileTreeComponent tree { contents };
    juce::TextButton assignButton { "Assign to channel" };
    juce::TextButton importButton { "Import MIDI" };
    juce::TextButton keyButton    { "Detect key" };
    juce::Label status;
    juce::Label infoName, infoAuthor, infoDate, infoSize;
    juce::ThreadPool keyPool { 1 };
};
