#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../../Model/ProjectFile.h"
#include "../DawLookAndFeel.h"

// Browser: as tuas pastas de kits e sons. Escolhe um ficheiro e atribui-o ao canal selecionado,
// ou importa um MIDI para as notas do canal.
class BrowserPanel : public juce::Component
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

        addAndMakeVisible(tree);
        addAndMakeVisible(assignButton);
        addAndMakeVisible(importButton);
        addAndMakeVisible(status);

        assignButton.onClick = [this] { assignSelected(); };
        importButton.onClick = [this] { importSelectedMidi(); };

        status.setFont(DawLookAndFeel::uiFont(12.0f));
        status.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
        status.setText("Select a sound, then assign it to the selected channel.", juce::dontSendNotification);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::panel);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        auto bottom = area.removeFromBottom(64);
        status.setBounds(bottom.removeFromBottom(22));
        bottom.removeFromBottom(6);

        auto buttons = bottom;
        assignButton.setBounds(buttons.removeFromLeft(buttons.getWidth() / 2).reduced(0, 2).withTrimmedRight(4));
        importButton.setBounds(buttons.reduced(0, 2).withTrimmedLeft(4));

        tree.setBounds(area);
    }

private:
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

    ProjectModel& model;
    juce::TimeSliceThread scanThread { "Opalo browser" };
    juce::WildcardFileFilter filter { "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mid;*.midi", "*", "Sons e MIDI" };
    juce::DirectoryContentsList contents { &filter, scanThread };
    juce::FileTreeComponent tree { contents };
    juce::TextButton assignButton { "Assign to channel" };
    juce::TextButton importButton { "Import MIDI" };
    juce::Label status;
};
