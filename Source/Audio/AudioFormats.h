#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

// Formatos de áudio que o Opalo consegue ler.
// WAV, AIFF, FLAC e OGG vêm do JUCE; MP3 entra quando JUCE_USE_MP3AUDIOFORMAT estiver ativo.
// AAC/M4A, Opus e WMA exigem um descodificador externo (FFmpeg), previsto no roadmap.
namespace OpaloAudioFormats
{
    inline void registerAll(juce::AudioFormatManager& manager)
    {
        manager.registerBasicFormats();

#if JUCE_USE_MP3AUDIOFORMAT
        manager.registerFormat(new juce::MP3AudioFormat(), false);
#endif
    }

    // Padrão de extensões para as janelas de abrir ficheiro.
    inline juce::String audioWildcard()
    {
        juce::String s = "*.wav;*.aif;*.aiff;*.flac;*.ogg";

#if JUCE_USE_MP3AUDIOFORMAT
        s += ";*.mp3";
#endif

        return s;
    }
}
