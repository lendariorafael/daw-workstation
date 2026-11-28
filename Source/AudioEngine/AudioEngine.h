#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include "../Settings/OpaloSettings.h"
#include "../Audio/AudioFormats.h"

// Motor de áudio: saída do sistema, reprodução do ficheiro principal e dois decks para mashup.
// Tudo passa por um MixerAudioSource, por isso há uma só cadeia de áudio.
class AudioEngine
{
public:
    static constexpr int numDecks = 2;

    AudioEngine()
    {
        OpaloAudioFormats::registerAll(formatManager);

        auto saved = OpaloSettings::get().getXmlValue("audioDevice");
        const auto error = deviceManager.initialise(0, 2, saved.get(), true);
        if (error.isNotEmpty())
            juce::Logger::writeToLog("Audio device error: " + error);

        mixer.addInputSource(&transport, false);
        for (auto& d : decks)
            mixer.addInputSource(&d.resampler, false);

        player.setSource(&mixer);
        deviceManager.addAudioCallback(&player);
    }

    ~AudioEngine()
    {
        deviceManager.removeAudioCallback(&player);
        player.setSource(nullptr);
        mixer.removeAllInputs();
        for (auto& d : decks)
            d.close();
        transport.setSource(nullptr);

        auto state = deviceManager.createStateXml();
        if (state != nullptr)
        {
            OpaloSettings::get().setValue("audioDevice", state.get());
            OpaloSettings::get().save();
        }
    }

    juce::AudioDeviceManager& devices() { return deviceManager; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    // ----- Ficheiro principal -----

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
        currentSource->setLooping(looping);
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

    // Pausa mantém a posição; stop volta ao início.
    void pause() { transport.stop(); }

    void setLooping(bool shouldLoop)
    {
        looping = shouldLoop;
        if (currentSource != nullptr)
            currentSource->setLooping(shouldLoop);
    }

    bool isLooping() const { return looping; }

    // Carga do processador, em 0..1, segundo o driver de áudio.
    double getCpuUsage() { return deviceManager.getCpuUsage(); }

    void stop()
    {
        transport.stop();
        transport.setPosition(0.0);
    }

    void setGain(float gain)        { transport.setGain(gain); }
    void seek(double seconds)       { transport.setPosition(juce::jlimit(0.0, getLength(), seconds)); }

    // ----- Decks de mashup -----

    bool loadDeck(int i, const juce::File& file)
    {
        if (! juce::isPositiveAndBelow(i, numDecks))
            return false;

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr)
            return false;

        auto& d = decks[(size_t) i];
        const double sampleRate = reader->sampleRate;

        d.transport.stop();
        d.transport.setSource(nullptr);
        d.reader = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
        d.transport.setSource(d.reader.get(), 0, nullptr, sampleRate);
        d.transport.setPosition(0.0);
        return true;
    }

    void playDeck(int i)
    {
        if (juce::isPositiveAndBelow(i, numDecks) && decks[(size_t) i].reader != nullptr)
            decks[(size_t) i].transport.start();
    }

    void stopDeck(int i)
    {
        if (juce::isPositiveAndBelow(i, numDecks))
        {
            decks[(size_t) i].transport.stop();
            decks[(size_t) i].transport.setPosition(0.0);
        }
    }

    bool deckPlaying(int i) const    { return juce::isPositiveAndBelow(i, numDecks) && decks[(size_t) i].transport.isPlaying(); }
    double deckPosition(int i) const { return juce::isPositiveAndBelow(i, numDecks) ? decks[(size_t) i].transport.getCurrentPosition() : 0.0; }
    double deckLength(int i) const   { return juce::isPositiveAndBelow(i, numDecks) ? decks[(size_t) i].transport.getLengthInSeconds() : 0.0; }
    double deckBpm(int i) const      { return juce::isPositiveAndBelow(i, numDecks) ? decks[(size_t) i].bpm : 0.0; }

    void setDeckBpm(int i, double bpm)
    {
        if (juce::isPositiveAndBelow(i, numDecks))
            decks[(size_t) i].bpm = juce::jmax(0.0, bpm);
    }

    // Varispeed: a razão ajusta a velocidade (e o tom). 1.0 = velocidade original.
    void setDeckRatio(int i, double ratio)
    {
        if (juce::isPositiveAndBelow(i, numDecks))
        {
            auto& d = decks[(size_t) i];
            d.ratio = juce::jlimit(0.5, 2.0, ratio);
            d.resampler.setResamplingRatio(d.ratio);
        }
    }

    double deckRatio(int i) const { return juce::isPositiveAndBelow(i, numDecks) ? decks[(size_t) i].ratio : 1.0; }

    // Crossfader com potência constante: 0 = só A, 1 = só B.
    void setCrossfade(float x)
    {
        const double angle = juce::jlimit(0.0, 1.0, (double) x) * juce::MathConstants<double>::halfPi;
        decks[0].transport.setGain((float) std::cos(angle));
        decks[1].transport.setGain((float) std::sin(angle));
    }

private:
    struct Deck
    {
        juce::AudioTransportSource transport;
        juce::ResamplingAudioSource resampler { &transport, false, 2 };
        std::unique_ptr<juce::AudioFormatReaderSource> reader;
        double bpm = 0.0;
        double ratio = 1.0;

        void close()
        {
            transport.stop();
            transport.setSource(nullptr);
            reader.reset();
        }
    };

    juce::AudioFormatManager formatManager;
    juce::AudioDeviceManager deviceManager;
    juce::AudioTransportSource transport;
    std::unique_ptr<juce::AudioFormatReaderSource> currentSource;
    bool looping = false;
    Deck decks[numDecks];
    juce::MixerAudioSource mixer;
    juce::AudioSourcePlayer player;
};
