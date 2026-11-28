#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"
#include "../DesignTokens.h"
#include "BrowserPanel.h"

// Barra lateral esquerda: categorias da biblioteca no topo e o Browser de ficheiros em baixo.
// Cada categoria aponta para uma pasta própria em Documentos/Opalo/Library.
class SidebarPanel : public juce::Component,
                     public juce::ListBoxModel
{
public:
    explicit SidebarPanel(ProjectModel& m) : browser(m)
    {
        list.setModel(this);
        list.setRowHeight(28);
        list.setColour(juce::ListBox::backgroundColourId, OpaloDesign::surface0);
        list.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(list);

        header.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontSmall, true));
        header.setColour(juce::Label::textColourId, OpaloDesign::textMuted);
        header.setText("LIBRARY", juce::dontSendNotification);
        addAndMakeVisible(header);

        addAndMakeVisible(browser);

        list.selectRow(0);
    }

    static const juce::StringArray& categories()
    {
        static const juce::StringArray names {
            "Current Project", "Recent Files", "Plugin Database", "Plugin Presets", "Channel Presets",
            "Mixer Presets", "Scores", "Backup", "Clipboard Files", "Demo Projects", "Envelopes",
            "Impulses", "Packs", "My Projects", "Recorded", "Rendered", "Sliced Audio", "Soundfonts", "Templates"
        };
        return names;
    }

    // A pasta de cada categoria. "Current Project" usa a pasta de kits.
    static juce::File folderFor(const juce::String& category)
    {
        if (category == "Current Project")
            return BrowserPanel::kitsFolder();

        auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                          .getChildFile("Opalo")
                          .getChildFile("Library")
                          .getChildFile(category);
        folder.createDirectory();
        return folder;
    }

    void resized() override
    {
        auto area = getLocalBounds();
        header.setBounds(area.removeFromTop(28).reduced(OpaloDesign::space3, 0));
        list.setBounds(area.removeFromTop(juce::jmin(area.getHeight() / 2, 19 * 28)));
        browser.setBounds(area.reduced(OpaloDesign::space1, OpaloDesign::space1));
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(OpaloDesign::surface0);
        g.setColour(OpaloDesign::border);
        g.fillRect(getWidth() - 1, 0, 1, getHeight());
    }

    int getNumRows() override { return categories().size(); }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (! juce::isPositiveAndBelow(row, categories().size()))
            return;

        if (selected)
        {
            g.setColour(OpaloDesign::surface2);
            g.fillRoundedRectangle(juce::Rectangle<float>(4.0f, 1.0f, (float) width - 8.0f, (float) height - 2.0f),
                                   OpaloDesign::radiusSmall);
        }

        const auto colours = palette();
        g.setColour(colours[(size_t) (row % (int) colours.size())]);
        g.fillEllipse(14.0f, height * 0.5f - 4.0f, 8.0f, 8.0f);

        g.setColour(selected ? OpaloDesign::text : OpaloDesign::text.withAlpha(0.80f));
        g.setFont(DawLookAndFeel::uiFont(OpaloDesign::fontBody, selected));
        g.drawText(categories()[row], 30, 0, width - 36, height, juce::Justification::centredLeft, true);
    }

    void selectedRowsChanged(int lastRowSelected) override
    {
        if (juce::isPositiveAndBelow(lastRowSelected, categories().size()))
            browser.setRoot(folderFor(categories()[lastRowSelected]));
    }

private:
    static std::array<juce::Colour, 6> palette()
    {
        return { OpaloDesign::accent,
                 juce::Colour((juce::uint32) 0xff7aa2ff),
                 juce::Colour((juce::uint32) 0xffc792ea),
                 juce::Colour((juce::uint32) 0xffffb86b),
                 juce::Colour((juce::uint32) 0xffc3e88d),
                 juce::Colour((juce::uint32) 0xff89ddff) };
    }

    juce::Label header;
    juce::ListBox list;
    BrowserPanel browser;
};
