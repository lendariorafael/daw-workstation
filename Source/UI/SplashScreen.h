#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "DawLookAndFeel.h"
#include "OpaloLogo.h"

// Ecrã de arranque: a gema ganha brilho, o nome aparece e depois o ecrã termina.
class SplashScreen : public juce::Component,
                     private juce::Timer
{
public:
    SplashScreen()
    {
        addAndMakeVisible(logo);
        startTimerHz(60);
    }

    ~SplashScreen() override
    {
        stopTimer();
    }

    std::function<void()> onFinished;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(DawLookAndFeel::background);
    }

    void resized() override
    {
        logo.setBounds(getLocalBounds().withSizeKeepingCentre(540, 200));
    }

private:
    void timerCallback() override
    {
        elapsed += 1.0f / 60.0f;

        logo.setPhase(elapsed * 0.35f);
        logo.setWordmarkAlpha(juce::jlimit(0.0f, 1.0f, (elapsed - 0.5f) / 0.7f));

        if (elapsed >= totalSeconds)
        {
            stopTimer();
            if (onFinished)
                onFinished();
        }
    }

    static constexpr float totalSeconds = 2.8f;
    float elapsed = 0.0f;
    OpaloLogo logo;
};
