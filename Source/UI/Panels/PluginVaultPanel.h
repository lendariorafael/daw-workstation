#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Analisa as pastas de plugins num thread de fundo, sem bloquear a interface.
class ScanThread : public juce::Thread
{
public:
    ScanThread(juce::KnownPluginList& l, juce::AudioPluginFormat& f)
        : juce::Thread("Opalo plugin scan"),
          list(l),
          format(f),
          paths(f.getDefaultLocationsToSearch())
    {
    }

    void run() override
    {
        // O ficheiro "dead man's pedal" deteta plugins que travam o scan.
        const auto deadMan = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("opalo-scan-deadman.txt");

        juce::PluginDirectoryScanner scanner(list, format, paths, true, deadMan, true);

        juce::String currentFile;
        while (! threadShouldExit() && scanner.scanNextFile(true, currentFile))
        {
        }
    }

private:
    juce::KnownPluginList& list;
    juce::AudioPluginFormat& format;
    juce::FileSearchPath paths;
};

// Plugin Vault: gestor de plugins VST3. Faz o scan das pastas padrão e guarda a lista.
class PluginVaultPanel : public juce::Component,
                         public juce::ListBoxModel,
                         private juce::Timer
{
public:
    explicit PluginVaultPanel(ProjectModel& m) : knownList(m.plugins)
    {
        loadFromDisk();

        addAndMakeVisible(scanButton);
        scanButton.onClick = [this] { startScan(); };

        statusLabel.setFont(DawLookAndFeel::uiFont(12.0f));
        statusLabel.setColour(juce::Label::textColourId, DawLookAndFeel::muted);
        addAndMakeVisible(statusLabel);

        list.setModel(this);
        list.setRowHeight(46);
        list.setColour(juce::ListBox::backgroundColourId, DawLookAndFeel::panel);
        addAndMakeVisible(list);

        updateStatus();
    }

    ~PluginVaultPanel() override
    {
        stopTimer();
        scanThread.stopThread(5000);
        saveToDisk();
        list.setModel(nullptr);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);
    }

    void resized() override
    {
        auto top = getLocalBounds().removeFromTop(44).reduced(12, 8);
        scanButton.setBounds(top.removeFromRight(160));
        top.removeFromRight(12);
        statusLabel.setBounds(top);

        list.setBounds(getLocalBounds().withTrimmedTop(44).reduced(12, 0));
    }

    int getNumRows() override
    {
        return knownList.getNumTypes();
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        auto types = knownList.getTypes();
        if (! juce::isPositiveAndBelow(row, types.size()))
            return;

        const auto& d = types.getReference(row);

        g.fillAll(selected ? DawLookAndFeel::panelLight : DawLookAndFeel::panel);
        g.setColour(DawLookAndFeel::accent);
        g.fillRect(0, 0, 4, height);

        g.setColour(DawLookAndFeel::text);
        g.setFont(DawLookAndFeel::uiFont(14.0f, true));
        g.drawText(d.name, 16, 4, width - 24, height / 2, juce::Justification::centredLeft, true);

        g.setColour(DawLookAndFeel::muted);
        g.setFont(DawLookAndFeel::uiFont(11.0f));
        g.drawText(d.manufacturerName + "  ·  " + d.pluginFormatName + "  ·  " + d.category,
                   16, height / 2, width - 24, height / 2 - 4, juce::Justification::centredLeft, true);
    }

private:
    void startScan()
    {
        if (scanThread.isThreadRunning())
            return;

        statusLabel.setText("Scanning VST3 folders…", juce::dontSendNotification);
        scanThread.startThread();
        startTimerHz(4);
    }

    void timerCallback() override
    {
        if (! scanThread.isThreadRunning())
        {
            stopTimer();
            saveToDisk();
            list.updateContent();
            updateStatus();
        }
    }

    void updateStatus()
    {
        statusLabel.setText(juce::String(knownList.getNumTypes()) + " plugins in the vault",
                            juce::dontSendNotification);
    }

    static juce::File dataFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("Opalo")
                   .getChildFile("plugins.xml");
    }

    void saveToDisk()
    {
        auto file = dataFile();
        file.getParentDirectory().createDirectory();

        if (auto xml = knownList.createXml())
            xml->writeTo(file);
    }

    void loadFromDisk()
    {
        if (auto xml = juce::XmlDocument::parse(dataFile()))
            knownList.recreateFromXml(*xml);
    }

    juce::KnownPluginList& knownList;
    juce::VST3PluginFormat vst3;
    ScanThread scanThread { knownList, vst3 };

    juce::TextButton scanButton { "Scan VST3" };
    juce::Label statusLabel;
    juce::ListBox list;
};
