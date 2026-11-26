#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Model/ProjectModel.h"
#include "../DawLookAndFeel.h"

// Uma tira do Console: nome, mute e solo, rack de efeitos (50 slots, 10 visíveis), pan e fader.
class ChannelStrip : public juce::Component,
                     public juce::ListBoxModel
{
public:
    explicit ChannelStrip(ProjectModel& m) : model(m)
    {
        name.setJustificationType(juce::Justification::centred);
        name.setFont(DawLookAndFeel::uiFont(10.0f, true));
        name.setColour(juce::Label::textColourId, DawLookAndFeel::text);

        fader.setSliderStyle(juce::Slider::LinearVertical);
        fader.setRange(0.0, 1.0, 0.001);
        fader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        fader.onValueChange = [this]
        {
            if (auto* c = channel())
            {
                c->volume = (float) fader.getValue();
                model.changed();
            }
        };

        pan.setSliderStyle(juce::Slider::LinearHorizontal);
        pan.setRange(-1.0, 1.0, 0.01);
        pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        pan.onValueChange = [this]
        {
            if (auto* c = channel())
            {
                c->pan = (float) pan.getValue();
                model.changed();
            }
        };

        mute.setButtonText("M");
        mute.setClickingTogglesState(true);
        mute.onClick = [this]
        {
            if (auto* c = channel())
            {
                c->muted = mute.getToggleState();
                model.changed();
            }
        };

        solo.setButtonText("S");
        solo.setClickingTogglesState(true);
        solo.onClick = [this]
        {
            if (auto* c = channel())
            {
                c->soloed = solo.getToggleState();
                model.changed();
            }
        };

        fxList.setModel(this);
        fxList.setRowHeight(15);
        fxList.setColour(juce::ListBox::backgroundColourId, DawLookAndFeel::background);

        addAndMakeVisible(name);
        addAndMakeVisible(mute);
        addAndMakeVisible(solo);
        addAndMakeVisible(fxList);
        addAndMakeVisible(fader);
        addAndMakeVisible(pan);
    }

    ~ChannelStrip() override
    {
        fxList.setModel(nullptr);
    }

    // Reaproveita esta tira para outro canal (pool de tiras).
    void setChannelIndex(int i)
    {
        index = i;
        refresh();
    }

    void refresh()
    {
        auto* c = channel();
        if (c == nullptr)
            return;

        name.setText(c->name, juce::dontSendNotification);
        fader.setValue(c->volume, juce::dontSendNotification);
        pan.setValue(c->pan, juce::dontSendNotification);
        mute.setToggleState(c->muted, juce::dontSendNotification);
        solo.setToggleState(c->soloed, juce::dontSendNotification);
        fxList.updateContent();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(DawLookAndFeel::panelLight);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

        if (auto* c = channel())
        {
            g.setColour(c->colour);
            g.fillRect(0, 0, getWidth(), 4);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(6);
        name.setBounds(area.removeFromTop(19));
        area.removeFromTop(4);

        auto row = area.removeFromTop(18);
        mute.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(2, 0));
        solo.setBounds(row.reduced(2, 0));

        area.removeFromTop(6);
        fxList.setBounds(area.removeFromTop(4 * 15 + 4));
        area.removeFromTop(6);

        pan.setBounds(area.removeFromBottom(22));
        area.removeFromBottom(6);
        fader.setBounds(area);
    }

    // ListBoxModel: 50 slots, o ListBox só desenha os visíveis.
    int getNumRows() override
    {
        return channel() != nullptr ? ProjectModel::maxEffectsPerChannel : 0;
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        auto* c = channel();
        if (c == nullptr || ! juce::isPositiveAndBelow(row, (int) c->effects.size()))
            return;

        const auto& slot = c->effects[(size_t) row];

        g.fillAll(selected ? DawLookAndFeel::panelLight : DawLookAndFeel::background);

        g.setColour(DawLookAndFeel::muted);
        g.setFont(DawLookAndFeel::uiFont(10.0f));
        g.drawText(juce::String::formatted("%02d", row + 1), 4, 0, 22, height, juce::Justification::centredLeft, false);

        g.setColour(slot.isEmpty() ? DawLookAndFeel::muted.withAlpha(0.5f)
                                   : (slot.bypassed ? DawLookAndFeel::muted : DawLookAndFeel::text));
        g.setFont(DawLookAndFeel::uiFont(11.0f, ! slot.isEmpty()));
        g.drawText(slot.isEmpty() ? juce::String("—") : slot.name, 28, 0, width - 32, height,
                   juce::Justification::centredLeft, true);
    }

    // Clicar num slot vazio abre a lista de plugins do scan. Num slot ocupado, permite limpar.
    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        auto* c = channel();
        if (c == nullptr)
            return;

        const auto types = model.plugins.getTypes();
        juce::PopupMenu menu;

        for (int t = 0; t < types.size(); ++t)
            menu.addItem(t + 1, types[t].name + "  ·  " + types[t].pluginFormatName);

        if (types.isEmpty())
            menu.addItem(-1, "No plugins in the Plugin Vault yet", false);

        menu.addSeparator();
        menu.addItem(1000, "Clear slot");

        const int ch = index;
        juce::Component::SafePointer<ChannelStrip> safe(this);

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&fxList),
                           [safe, ch, row, types](int result)
                           {
                               auto* self = safe.getComponent();
                               if (self == nullptr || result <= 0)
                                   return;

                               auto& slots = self->model.channels[(size_t) ch].effects;
                               if (! juce::isPositiveAndBelow(row, (int) slots.size()))
                                   return;

                               if (result == 1000)
                               {
                                   slots[(size_t) row] = EffectSlot {};
                               }
                               else if (result - 1 < types.size())
                               {
                                   const auto& d = types[result - 1];
                                   slots[(size_t) row].name = d.name;
                                   slots[(size_t) row].pluginId = d.createIdentifierString();
                                   slots[(size_t) row].bypassed = false;
                               }

                               self->model.changed();
                           });
    }

private:
    ChannelData* channel()
    {
        if (index >= 0 && index < (int) model.channels.size())
            return &model.channels[(size_t) index];
        return nullptr;
    }

    ProjectModel& model;
    int index = -1;
    juce::Label name;
    juce::TextButton mute;
    juce::TextButton solo;
    juce::ListBox fxList;
    juce::Slider fader;
    juce::Slider pan;
};

// Console: mixer com scroll horizontal. Só existem tiras suficientes para o que está no ecrã
// (pool reciclado), por isso 1000 canais não criam 1000 conjuntos de componentes.
class ConsolePanel : public juce::Component,
                     private juce::ChangeListener,
                     private juce::Timer
{
public:
    static constexpr int stripWidth = 82;
    static constexpr int poolSize = 48;

    explicit ConsolePanel(ProjectModel& m) : model(m)
    {
        model.addChangeListener(this);

        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&host, false);
        viewport.setScrollBarsShown(false, true);

        for (int k = 0; k < poolSize; ++k)
        {
            strips.push_back(std::make_unique<ChannelStrip>(model));
            host.addAndMakeVisible(*strips.back());
            strips.back()->setVisible(false);
        }

        masterLabel.setText("MASTER", juce::dontSendNotification);
        masterLabel.setJustificationType(juce::Justification::centred);
        masterLabel.setFont(DawLookAndFeel::uiFont(10.0f, true));
        masterLabel.setColour(juce::Label::textColourId, DawLookAndFeel::accent);
        addAndMakeVisible(masterLabel);

        masterFader.setSliderStyle(juce::Slider::LinearVertical);
        masterFader.setRange(0.0, 1.0, 0.001);
        masterFader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        masterFader.setValue(model.masterVolume, juce::dontSendNotification);
        masterFader.onValueChange = [this] { model.masterVolume = (float) masterFader.getValue(); };
        addAndMakeVisible(masterFader);

        startTimerHz(30);
        refresh();
    }

    ~ConsolePanel() override
    {
        model.removeChangeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto master = area.removeFromRight(94).reduced(5);
        masterLabel.setBounds(master.removeFromTop(20));
        masterFader.setBounds(master);
        viewport.setBounds(area);
        layoutStrips();
    }

    void refresh()
    {
        layoutStrips();
        repaint();
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        refresh();
    }

    void timerCallback() override
    {
        // Ao fazer scroll, recicla as tiras para os canais que entram no ecrã.
        if (viewport.getViewPositionX() / stripWidth != firstVisible)
            layoutStrips();
    }

    void layoutStrips()
    {
        const int channelCount = (int) model.channels.size();
        const int height = juce::jmax(1, viewport.getHeight());

        host.setSize(juce::jmax(1, channelCount) * stripWidth + 16, height);

        firstVisible = juce::jmax(0, viewport.getViewPositionX() / stripWidth);

        for (int k = 0; k < poolSize; ++k)
        {
            auto& strip = *strips[(size_t) k];
            const int idx = firstVisible + k;

            if (idx < channelCount)
            {
                strip.setChannelIndex(idx);
                strip.setBounds(8 + idx * stripWidth, 8, stripWidth - 10, height - 16);
                strip.setVisible(true);
            }
            else
            {
                strip.setVisible(false);
            }
        }
    }

    ProjectModel& model;
    juce::Viewport viewport;
    juce::Component host;
    std::vector<std::unique_ptr<ChannelStrip>> strips;
    int firstVisible = 0;
    juce::Label masterLabel;
    juce::Slider masterFader;
};
