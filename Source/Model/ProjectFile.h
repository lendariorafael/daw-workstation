#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ProjectModel.h"

namespace OpaloFile
{
    constexpr int formatVersion = 3;
    inline juce::String extensionName() { return ".opal"; }

    inline juce::String stepsToString(const std::array<bool, 16>& steps)
    {
        juce::String s;
        for (bool b : steps) s += (b ? "1" : "0");
        return s;
    }

    inline std::unique_ptr<juce::XmlElement> toXml(const ProjectModel& m)
    {
        auto root = std::make_unique<juce::XmlElement>("OpaloProject");
        root->setAttribute("version", formatVersion);
        root->setAttribute("bpm", m.bpm);
        root->setAttribute("masterVolume", (double) m.masterVolume);
        root->setAttribute("selectedChannel", m.selectedChannel);
        root->setAttribute("selectedPattern", m.selectedPattern);

        auto* masterFx = root->createNewChildElement("MasterEffects");
        for (int i = 0; i < (int) m.masterEffects.size(); ++i)
        {
            const auto& slot = m.masterEffects[(size_t) i];
            if (slot.isEmpty()) continue;
            auto* se = masterFx->createNewChildElement("Slot");
            se->setAttribute("index", i); se->setAttribute("name", slot.name);
            se->setAttribute("pluginId", slot.pluginId); se->setAttribute("bypassed", slot.bypassed ? 1 : 0);
        }

        auto* automation = root->createNewChildElement("Automation");
        for (const auto& lane : m.automationLanes)
        {
            auto* le = automation->createNewChildElement("Lane");
            le->setAttribute("name", lane.name); le->setAttribute("target", lane.target);
            le->setAttribute("colour", lane.colour.toString());
            for (const auto& point : lane.points)
            { auto* pe = le->createNewChildElement("Point"); pe->setAttribute("time", point.time); pe->setAttribute("value", (double) point.value); }
        }

        auto* patterns = root->createNewChildElement("Patterns");
        for (const auto& p : m.patterns)
        {
            auto* e = patterns->createNewChildElement("Pattern");
            e->setAttribute("name", p.name);
            e->setAttribute("colour", p.colour.toString());
            e->setAttribute("lengthBars", p.lengthBars);
        }

        auto* channels = root->createNewChildElement("Channels");
        for (const auto& ch : m.channels)
        {
            auto* e = channels->createNewChildElement("Channel");
            e->setAttribute("name", ch.name);
            e->setAttribute("colour", ch.colour.toString());
            e->setAttribute("sample", ch.samplePath);
            e->setAttribute("muted", ch.muted ? 1 : 0);
            e->setAttribute("soloed", ch.soloed ? 1 : 0);
            e->setAttribute("mono", ch.mono ? 1 : 0);
            e->setAttribute("outputBus", 0);
            e->setAttribute("volume", (double) ch.volume);
            e->setAttribute("pan", (double) ch.pan);
            e->setAttribute("steps", stepsToString(ch.steps));

            auto* notes = e->createNewChildElement("Notes");
            for (const auto& n : ch.notes)
            {
                auto* ne = notes->createNewChildElement("Note");
                ne->setAttribute("step", n.step);
                ne->setAttribute("length", n.length);
                ne->setAttribute("pitch", n.pitch);
                ne->setAttribute("velocity", (double) n.velocity);
                ne->setAttribute("slide", n.slide ? 1 : 0);
                ne->setAttribute("palmMute", n.palmMute ? 1 : 0);
            }

            auto* fx = e->createNewChildElement("Effects");
            for (int i = 0; i < (int) ch.effects.size(); ++i)
            {
                const auto& slot = ch.effects[(size_t) i];
                if (slot.isEmpty()) continue;
                auto* se = fx->createNewChildElement("Slot");
                se->setAttribute("index", i);
                se->setAttribute("name", slot.name);
                se->setAttribute("pluginId", slot.pluginId);
                se->setAttribute("bypassed", slot.bypassed ? 1 : 0);
            }
        }

        auto* clips = root->createNewChildElement("Clips");
        for (const auto& c : m.clips)
        {
            auto* ce = clips->createNewChildElement("Clip");
            ce->setAttribute("channel", c.channel);
            ce->setAttribute("startBar", c.startBar);
            ce->setAttribute("lengthBars", c.lengthBars);
        }
        return root;
    }

    inline bool fromXml(ProjectModel& m, const juce::XmlElement& root, juce::String& error)
    {
        if (root.getTagName() != "OpaloProject")
        {
            error = "This is not a valid Opalo project.";
            return false;
        }
        if (root.getIntAttribute("version", 0) > formatVersion)
        {
            error = "This project was saved with a newer version of Opalo.";
            return false;
        }

        m.channels.clear();
        m.clips.clear();
        m.patterns.clear();
        m.bpm = juce::jlimit(20.0, 300.0, root.getDoubleAttribute("bpm", 120.0));
        m.masterVolume = (float) juce::jlimit(0.0, 1.0, root.getDoubleAttribute("masterVolume", 0.8));
        m.masterEffects.assign(ChannelData::kMaxEffects, EffectSlot{});
        m.automationLanes.clear();
        if (auto* fx = root.getChildByName("MasterEffects"))
            forEachXmlChildElementWithTagName(*fx, se, "Slot")
            { const int i = juce::jlimit(0, ChannelData::kMaxEffects - 1, se->getIntAttribute("index")); auto& slot=m.masterEffects[(size_t)i]; slot.name=se->getStringAttribute("name"); slot.pluginId=se->getStringAttribute("pluginId"); slot.bypassed=se->getIntAttribute("bypassed")!=0; }
        if (auto* automation = root.getChildByName("Automation"))
            forEachXmlChildElementWithTagName(*automation, le, "Lane")
            { AutomationLane lane; lane.name=le->getStringAttribute("name", "Automation"); lane.target=le->getStringAttribute("target"); lane.colour=juce::Colour::fromString(le->getStringAttribute("colour", "ff62cdbb")); forEachXmlChildElementWithTagName(*le, pe, "Point") { lane.points.push_back({pe->getDoubleAttribute("time"), (float)juce::jlimit(0.0,1.0,pe->getDoubleAttribute("value"))}); } m.automationLanes.push_back(std::move(lane)); }

        if (auto* patterns = root.getChildByName("Patterns"))
            forEachXmlChildElementWithTagName(*patterns, e, "Pattern")
                m.patterns.push_back({ e->getStringAttribute("name", "Pattern"),
                                       juce::Colour::fromString(e->getStringAttribute("colour", "ff00d1b2")),
                                       juce::jlimit(1, 256, e->getIntAttribute("lengthBars", 1)) });

        if (auto* channels = root.getChildByName("Channels"))
        {
            forEachXmlChildElementWithTagName(*channels, e, "Channel")
            {
                if ((int) m.channels.size() >= ProjectModel::maxChannels) break;
                ChannelData ch;
                ch.name = e->getStringAttribute("name", "Track");
                ch.colour = juce::Colour::fromString(e->getStringAttribute("colour", "ff00d1b2"));
                ch.samplePath = e->getStringAttribute("sample");
                ch.muted = e->getIntAttribute("muted") != 0;
                ch.soloed = e->getIntAttribute("soloed") != 0;
                ch.mono = e->getIntAttribute("mono") != 0;
                ch.outputBus = 0; // All current tracks feed the master bus.
                ch.volume = (float) juce::jlimit(0.0, 1.0, e->getDoubleAttribute("volume", 0.8));
                ch.pan = (float) juce::jlimit(-1.0, 1.0, e->getDoubleAttribute("pan", 0.0));
                const auto steps = e->getStringAttribute("steps");
                for (int s = 0; s < ProjectModel::numSteps; ++s)
                    ch.steps[(size_t) s] = s < steps.length() && steps[s] == '1';

                if (auto* notes = e->getChildByName("Notes"))
                    forEachXmlChildElementWithTagName(*notes, ne, "Note")
                    {
                        Note n;
                        n.step = juce::jlimit(0, ProjectModel::numSteps - 1, ne->getIntAttribute("step"));
                        n.length = juce::jlimit(1, ProjectModel::numSteps, ne->getIntAttribute("length", 1));
                        n.pitch = juce::jlimit(0, 127, ne->getIntAttribute("pitch", 60));
                        n.velocity = (float) juce::jlimit(0.0, 1.0, ne->getDoubleAttribute("velocity", 0.8));
                        n.slide = ne->getIntAttribute("slide") != 0;
                        n.palmMute = ne->getIntAttribute("palmMute") != 0;
                        ch.notes.push_back(n);
                    }

                if (auto* fx = e->getChildByName("Effects"))
                    forEachXmlChildElementWithTagName(*fx, se, "Slot")
                    {
                        const int idx = juce::jlimit(0, ChannelData::kMaxEffects - 1, se->getIntAttribute("index"));
                        auto& slot = ch.effects[(size_t) idx];
                        slot.name = se->getStringAttribute("name");
                        slot.pluginId = se->getStringAttribute("pluginId");
                        slot.bypassed = se->getIntAttribute("bypassed") != 0;
                    }
                m.channels.push_back(ch);
            }
        }

        if (auto* clips = root.getChildByName("Clips"))
            forEachXmlChildElementWithTagName(*clips, ce, "Clip")
            {
                Clip c;
                c.channel = ce->getIntAttribute("channel");
                c.startBar = juce::jmax(0, ce->getIntAttribute("startBar"));
                c.lengthBars = juce::jmax(1, ce->getIntAttribute("lengthBars", 1));
                if (c.channel >= 0 && c.channel < (int) m.channels.size()) m.clips.push_back(c);
            }

        if (m.channels.empty())
            m.resetToDefault();
        if (m.patterns.empty())
            m.patterns.push_back({ "Pattern 1", ProjectModel::paletteColour(0), 1 });
        m.selectedChannel = juce::jlimit(0, (int) m.channels.size() - 1, root.getIntAttribute("selectedChannel"));
        m.selectedPattern = juce::jlimit(0, (int) m.patterns.size() - 1, root.getIntAttribute("selectedPattern"));
        return true;
    }

    inline bool saveProject(const ProjectModel& m, const juce::File& file, juce::String& error)
    {
        auto root = toXml(m);
        const auto temp = file.getSiblingFile(file.getFileName() + ".tmp");
        if (! root->writeTo(temp))
        {
            error = "Could not write the project file.";
            return false;
        }
        if (! temp.moveFileTo(file))
        {
            error = "Could not replace the project file.";
            return false;
        }
        return true;
    }

    inline bool loadProject(ProjectModel& m, const juce::File& file, juce::String& error)
    {
        auto root = juce::XmlDocument::parse(file);
        if (root == nullptr)
        {
            error = "This is not a valid Opalo project.";
            return false;
        }
        return fromXml(m, *root, error);
    }

    inline void importMidi(const juce::File& file, ChannelData& ch, int& imported, int& outside, juce::String& error)
    {
        imported = 0;
        outside = 0;
        juce::FileInputStream in(file);
        juce::MidiFile midi;
        if (! in.openedOk() || ! midi.readFrom(in))
        {
            error = "Could not read the MIDI file.";
            return;
        }
        const int ticksPerQuarter = midi.getTimeFormat();
        if (ticksPerQuarter <= 0)
        {
            error = "Unsupported MIDI time format.";
            return;
        }
        const double ticksPerStep = ticksPerQuarter / 4.0;
        for (int t = 0; t < midi.getNumTracks(); ++t)
        {
            const auto* seq = midi.getTrack(t);
            if (seq == nullptr) continue;
            for (int e = 0; e < seq->getNumEvents(); ++e)
            {
                auto* ev = seq->getEventPointer(e);
                if (! ev->message.isNoteOn()) continue;
                const int step = juce::roundToInt(ev->message.getTimeStamp() / ticksPerStep);
                if (step < 0 || step >= ProjectModel::numSteps) { ++outside; continue; }
                Note n;
                n.step = step;
                n.length = 1;
                n.pitch = ev->message.getNoteNumber();
                ch.notes.push_back(n);
                ++imported;
            }
        }
    }
}
