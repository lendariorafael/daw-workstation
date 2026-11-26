#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

// Motor de áudio do Milestone 001: saída de áudio padrão e reprodução de um ficheiro.
// Toda a comunicação com a thread de áudio passa pelo AudioTransportSource,
// que é seguro para chamadas a partir da thread de interface.
class AudioEngine
{
public:
    AudioEngine()
    {
        formatManager.registerBasicFormats(); // WAV, AIFF, FLAC, OGG

        auto error = deviceManager.initialiseWithDefaultDevices(0, 2);
        if (error.isNotEmpty())
            juce::Logger::writeToLog("Audio device error: " + error);

        player.setSource(&transport);
        deviceManager.addAudioCallback(&player);
    }

    ~AudioEngine()
    {
        transport.setSource(nullptr);
        deviceManager.removeAudioCallback(&player);
        player.setSource(nullptr);
    }

    bool loadFile(const juce::File& file)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr)
            return false;

        const double sampleRate = reader->sampleRate;
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);

        transport.stop();
        transport.setSource(nullptr);
        currentSource = std::move(newSource);
        transport.setSource(currentSource.get(), 0, nullptr, sampleRate);
        transport.setPosition(0.0);
        return true;
    }

    bool hasFile() const            { return currentSource != nullptr; }
    bool isPlaying() const          { return transport.isPlaying(); }
    double getPosition() const      { return transport.getCurrentPosition(); }
    double getLength() const        { return transport.getLengthInSeconds(); }

    void play()
    {
        if (hasFile())
            transport.start();
    }

    void stop()
    {
        transport.stop();
        transport.setPosition(0.0);
    }

    void setGain(float gain)        { transport.setGain(gain); }

    void seek(double seconds)       { transport.setPosition(juce::jlimit(0.0, getLength(), seconds)); }

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioDeviceManager deviceManager;
    juce::AudioTransportSource transport;
    juce::AudioSourcePlayer player;
    std::unique_ptr<juce::AudioFormatReaderSource> currentSource;
};
