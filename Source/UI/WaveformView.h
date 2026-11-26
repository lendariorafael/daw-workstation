#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>
#include "DawLookAndFeel.h"

// Mostra a forma de onda do ficheiro carregado e um cursor de reprodução.
// Clicar ou arrastar sobre a forma de onda muda a posição de reprodução.
class WaveformView : public juce::Component,
                     private juce::ChangeListener
{
public:
    explicit WaveformView(juce::AudioFormatManager& formats)
        : thumbnailCache(4),
          thumbnail(512, formats, thumbnailCache)
    {
        thumbnail.addChangeListener(this);
    }

    ~WaveformView() override
    {
        thumbnail.removeChangeListener(this);
    }

    void setFile(const juce::File& file)
    {
        thumbnail.clear();
        if (file.existsAsFile())
            thumbnail.setSource(new juce::FileInputSource(file)); // o thumbnail assume a posse da fonte
        repaint();
    }

    void setPlayhead(double seconds, double lengthSeconds)
    {
        if (seconds != playhead || lengthSeconds != length)
        {
            playhead = seconds;
            length = lengthSeconds;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        g.setColour(DawLookAndFeel::panel);
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

        if (thumbnail.getTotalLength() <= 0.0)
        {
            g.setColour(DawLookAndFeel::muted);
            g.drawText("Open a file to see its waveform", bounds, juce::Justification::centred);
            return;
        }

        g.setColour(DawLookAndFeel::accent.withAlpha(0.85f));
        thumbnail.drawChannels(g, bounds.reduced(8), 0.0, thumbnail.getTotalLength(), 1.0f);

        if (length > 0.0)
        {
            const auto x = (float) (playhead / length) * (float) bounds.getWidth();
            g.setColour(DawLookAndFeel::text);
            g.drawVerticalLine((int) x, 0.0f, (float) bounds.getHeight());
        }
    }

    void mouseDown(const juce::MouseEvent& e) override { seekTo(e); }
    void mouseDrag(const juce::MouseEvent& e) override { seekTo(e); }

    std::function<void(double)> onSeek;

private:
    void seekTo(const juce::MouseEvent& e)
    {
        if (onSeek && length > 0.0 && getWidth() > 0)
            onSeek(juce::jlimit(0.0, length, e.position.x / (double) getWidth() * length));
    }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        repaint();
    }

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    double playhead = 0.0;
    double length = 0.0;
};
