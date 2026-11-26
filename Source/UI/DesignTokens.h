#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Tokens de design do Opalo. Todas as cores, raios e espaçamentos vêm daqui,
// para a interface ficar coerente e fácil de ajustar num único sítio.
namespace OpaloDesign
{
    // Superfícies em camadas: quanto mais clara, mais "perto" do utilizador.
    inline const juce::Colour background { (juce::uint32) 0xff151a22 };
    inline const juce::Colour surface0   { (juce::uint32) 0xff1c232d };
    inline const juce::Colour surface1   { (juce::uint32) 0xff27313e };
    inline const juce::Colour surface2   { (juce::uint32) 0xff344153 };
    inline const juce::Colour border     { (juce::uint32) 0xff435064 };

    inline const juce::Colour accent     { (juce::uint32) 0xff39d8c1 };
    inline const juce::Colour accentDim  { (juce::uint32) 0xff267f77 };

    inline const juce::Colour text       { (juce::uint32) 0xffe8ecf2 };
    inline const juce::Colour textMuted  { (juce::uint32) 0xffa8b4c4 };
    inline const juce::Colour danger     { (juce::uint32) 0xffff6b6b };

    constexpr float radiusSmall  = 4.0f;
    constexpr float radiusMedium = 6.0f;
    constexpr float radiusLarge  = 10.0f;

    constexpr int space1 = 4;
    constexpr int space2 = 8;
    constexpr int space3 = 12;
    constexpr int space4 = 16;

    constexpr float fontSmall  = 11.0f;
    constexpr float fontBody   = 13.0f;
    constexpr float fontTitle  = 15.0f;
}
