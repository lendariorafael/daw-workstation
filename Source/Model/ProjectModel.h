#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

// Modelo central do projeto. Todas as janelas leem e escrevem aqui:
// nunca existem duas cópias do mesmo estado.

struct Note
{
    int step = 0;
    int length = 1;
    int pitch = 60;
};

struct Clip
{
    int channel = 0;
    int startBar = 0;
    int lengthBars = 1;
};

// Um slot de efeito guarda apenas a identidade do plugin escolhido no Plugin Vault.
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
    juce::String samplePath;   // ficheiro de som atribuído a partir do Browser
    bool muted = false;
    bool soloed = false;
    float volume = 0.8f;
    float pan = 0.0f;
    std::array<bool, 16> steps {};
    std::vector<Note> notes;
    std::vector<EffectSlot> effects = std::vector<EffectSlot>(kMaxEffects);
};

class ProjectModel : public juce::ChangeBroadcaster
{
public:
    static constexpr int maxChannels = 1000;
    static constexpr int initialChannels = 20;
    static constexpr int numSteps = 16;
    static constexpr int maxEffectsPerChannel = ChannelData::kMaxEffects;

    ProjectModel()
    {
        resetToDefault();
    }

    // Projeto novo: 20 canais com um padrão de exemplo.
    void resetToDefault()
    {
        channels.clear();
        clips.clear();
        selectedChannel = 0;
        bpm = 120.0;
        masterVolume = 0.8f;

        addChannel("Kick",  paletteColour(0));
        addChannel("Snare", paletteColour(1));
        addChannel("Hat",   paletteColour(2));
        addChannel("Bass",  paletteColour(3));

        for (int i = 4; i < initialChannels; ++i)
            addChannel("Channel " + juce::String(i + 1), paletteColour(i));

        for (int s = 0; s < numSteps; s += 4) channels[0].steps[(size_t) s] = true;
        channels[1].steps[4] = true;
        channels[1].steps[12] = true;
        for (int s = 0; s < numSteps; s += 2) channels[2].steps[(size_t) s] = true;

        sendChangeMessage();
    }

    std::vector<ChannelData> channels;
    std::vector<Clip> clips;
    int selectedChannel = 0;
    double bpm = 120.0;
    float masterVolume = 0.8f;
    double playheadSeconds = 0.0; // atualizado pelo motor, sem notificar

    // Lista de plugins encontrados pelo scan. Partilhada pelo Plugin Vault e pelo Console.
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
