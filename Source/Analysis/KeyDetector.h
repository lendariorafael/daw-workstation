#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include <functional>
#include <vector>
#include "../Audio/AudioFormats.h"

// Deteção de tom: cromagrama (energia por nota, ignorando a oitava) + perfis de
// Krumhansl-Schmuckler. Compara o perfil do material com os 24 perfis (12 maiores, 12 menores)
// e devolve o de maior correlação de Pearson.
struct KeyResult
{
    juce::String name;
    double confidence = 0.0;   // correlação de Pearson, de -1 a 1
    bool valid = false;
};

class KeyDetector
{
public:
    static constexpr int fftOrder = 13;              // 8192 amostras por janela
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr double maxSeconds = 120.0;      // analisa até 2 minutos, para ser rápido

    static KeyResult analyse(juce::AudioFormatManager& formats, const juce::File& file)
    {
        KeyResult result;

        std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
        if (reader == nullptr || reader->lengthInSamples <= 0)
            return result;

        const double sr = reader->sampleRate;
        const int channels = (int) juce::jmax((unsigned int) 1, reader->numChannels);
        const auto total = (juce::int64) juce::jmin((double) reader->lengthInSamples, sr * maxSeconds);

        // Lê e converte para mono.
        std::vector<float> mono((size_t) total, 0.0f);
        juce::AudioBuffer<float> chunk(channels, 16384);
        juce::int64 pos = 0;

        while (pos < total)
        {
            const int n = (int) juce::jmin((juce::int64) chunk.getNumSamples(), total - pos);
            if (! reader->read(&chunk, 0, n, pos, true, true))
                break;

            for (int i = 0; i < n; ++i)
            {
                float sum = 0.0f;
                for (int c = 0; c < channels; ++c)
                    sum += chunk.getReadPointer(c)[i];
                mono[(size_t) (pos + i)] = sum / (float) channels;
            }
            pos += n;
        }

        // Cromagrama com janelas de Hann sobrepostas em 50%.
        juce::dsp::FFT fft(fftOrder);
        std::vector<float> window((size_t) fftSize);
        for (int i = 0; i < fftSize; ++i)
            window[(size_t) i] = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * (float) i / (float) (fftSize - 1));

        std::vector<float> spectrum((size_t) (2 * fftSize), 0.0f);
        std::array<double, 12> chroma {};
        const int hop = fftSize / 2;

        for (juce::int64 start = 0; start + fftSize <= total; start += hop)
        {
            std::fill(spectrum.begin(), spectrum.end(), 0.0f);
            for (int i = 0; i < fftSize; ++i)
                spectrum[(size_t) i] = mono[(size_t) (start + i)] * window[(size_t) i];

            fft.performFrequencyOnlyForwardTransform(spectrum.data());

            for (int k = 1; k < fftSize / 2; ++k)
            {
                const double f = k * sr / fftSize;
                if (f < 65.0 || f > 2000.0)
                    continue;

                const double midi = 69.0 + 12.0 * std::log2(f / 440.0);
                const int pitchClass = (int) (((std::lround(midi) % 12) + 12) % 12);
                chroma[(size_t) pitchClass] += spectrum[(size_t) k];
            }
        }

        double energy = 0.0;
        for (double c : chroma) energy += c;
        if (energy <= 0.0)
            return result;

        for (auto& c : chroma) c /= energy;

        // Krumhansl-Kessler: perfis de tonalidade maior e menor.
        static constexpr double majorProfile[12] = { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
        static constexpr double minorProfile[12] = { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };
        static const char* names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

        double best = -2.0;
        int bestTonic = 0;
        bool bestMajor = true;

        for (int tonic = 0; tonic < 12; ++tonic)
        {
            for (int mode = 0; mode < 2; ++mode)
            {
                const double* profile = mode == 0 ? majorProfile : minorProfile;

                std::array<double, 12> rotated {};
                for (int i = 0; i < 12; ++i)
                    rotated[(size_t) i] = profile[(i - tonic + 12) % 12];

                const double r = pearson(chroma.data(), rotated.data());
                if (r > best)
                {
                    best = r;
                    bestTonic = tonic;
                    bestMajor = (mode == 0);
                }
            }
        }

        result.name = juce::String(names[bestTonic]) + (bestMajor ? " major" : " minor");
        result.confidence = best;
        result.valid = true;
        return result;
    }

private:
    static double pearson(const double* a, const double* b)
    {
        double meanA = 0.0, meanB = 0.0;
        for (int i = 0; i < 12; ++i) { meanA += a[i]; meanB += b[i]; }
        meanA /= 12.0;
        meanB /= 12.0;

        double num = 0.0, da = 0.0, db = 0.0;
        for (int i = 0; i < 12; ++i)
        {
            const double x = a[i] - meanA;
            const double y = b[i] - meanB;
            num += x * y;
            da += x * x;
            db += y * y;
        }

        const double denom = std::sqrt(da * db);
        return denom > 0.0 ? num / denom : 0.0;
    }
};

// Trabalho de fundo: a análise de tom não bloqueia a interface.
class KeyJob : public juce::ThreadPoolJob
{
public:
    using Callback = std::function<void(juce::File, KeyResult)>;

    KeyJob(juce::File f, Callback cb)
        : juce::ThreadPoolJob("Opalo key detection"), file(std::move(f)), callback(std::move(cb))
    {
    }

    juce::ThreadPoolJob::JobStatus runJob() override
    {
        juce::AudioFormatManager formats;
        OpaloAudioFormats::registerAll(formats);

        const auto result = KeyDetector::analyse(formats, file);

        auto cb = callback;
        auto f = file;
        juce::MessageManager::callAsync([cb, f, result] { cb(f, result); });

        return jobHasFinished;
    }

private:
    juce::File file;
    Callback callback;
};
