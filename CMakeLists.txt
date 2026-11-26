cmake_minimum_required(VERSION 3.22)

project(Opalo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Versão fixa do JUCE. Confirmar a tag em https://github.com/juce-framework/JUCE/tags
set(JUCE_VERSION "8.0.4" CACHE STRING "Versão (tag) do JUCE usada pelo projeto")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${JUCE_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Nome do produto: Opalo.
juce_add_gui_app(Opalo
    PRODUCT_NAME "Opalo"
    COMPANY_NAME "lendariorafael"
)

juce_generate_juce_header(Opalo)

target_sources(Opalo PRIVATE
    Source/Main.cpp
    Source/AudioEngine/AudioEngine.h
    Source/UI/DawLookAndFeel.h
    Source/UI/MainComponent.h
    Source/UI/MainComponent.cpp
    Source/UI/WaveformView.h
    Source/UI/OpaloLogo.h
    Source/UI/SplashScreen.h
)

target_compile_definitions(Opalo PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(Opalo PRIVATE
    juce::juce_gui_basics
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_basics
)
