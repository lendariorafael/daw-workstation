#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

struct Note
{
    int step = 0;
    int length = 1;
    int pitch = 60;
    float velocity = 0.8f;
    bool slide = false;
    bool palmMute = false;
};

struct Clip
{
    int channel = 0;
    int startBar = 0;
    int lengthBars = 1;
};

struct PatternData
{
    juce::String name;
    juce::Colour colour;
    int lengthBars = 1;
};

struct EffectSlot
{
    juce::String name;
    juce::String pluginId;
    bool bypassed = false;
    bool isEmpty() const { return pluginId.isEmpty(); }
};

struct ChannelData
{
    static constexpr int kMaxEffects = 50;

    juce::String name;
    juce::Colour colour;
    juce::String samplePath;
    bool muted = false;
    bool soloed = false;
    bool mono = false;
    float volume = 0.8f;
    float pan = 0.0f;
    std::array<bool, 16> steps {};
    std::vector<Note> notes;
    std::vector<EffectSlot> effects = std::vector<EffectSlot>(kMaxEffects);
};

class ProjectModel : public juce::ChangeBroadcaster
{
public:
    static constexpr int maxChannels = 250;
    static constexpr int initialChannels = 8;
    static constexpr int numSteps = 16;
    static constexpr int maxEffectsPerChannel = ChannelData::kMaxEffects;

    ProjectModel() { resetToDefault(); }

    void resetToDefault()
    {
        channels.clear();
        clips.clear();
        patterns.clear();
        selectedChannel = 0;
        selectedPattern = 0;
        bpm = 120.0;
        masterVolume = 0.8f;

        for (int i = 0; i < initialChannels; ++i)
            addChannel("Track " + juce::String(i + 1), paletteColour(i));

        patterns.push_back({ "Pattern 1", paletteColour(0), 1 });
        sendChangeMessage();
    }

    std::vector<ChannelData> channels;
    std::vector<Clip> clips;
    std::vector<PatternData> patterns;
    int selectedChannel = 0;
    int selectedPattern = 0;
    double bpm = 120.0;
    float masterVolume = 0.8f;
    double playheadSeconds = 0.0;
    juce::KnownPluginList plugins;

    double secondsPerBar() const { return 4.0 * 60.0 / bpm; }

    bool addChannel(const juce::String& name, juce::Colour colour)
    {
        if ((int) channels.size() >= maxChannels)
            return false;
        ChannelData c;
        c.name = name;
        c.colour = colour;
        channels.push_back(c);
        sendChangeMessage();
        return true;
    }

    void changed() { sendChangeMessage(); }

    static juce::Colour paletteColour(int index)
    {
        static const juce::uint32 palette[] = { 0xff00d1b2, 0xff7aa2ff, 0xffffb86b, 0xffc792ea,
                                                0xfff07178, 0xff89ddff, 0xffc3e88d, 0xffffcb6b };
        return juce::Colour(palette[(size_t) (index % 8)]);
    }
};
