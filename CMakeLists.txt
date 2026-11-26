cmake_minimum_required(VERSION 3.22)

project(DAWWorkstation VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Versão fixa do JUCE. Confirmar que a tag existe em
# https://github.com/juce-framework/JUCE/tags antes de alterar.
set(JUCE_VERSION "8.0.4" CACHE STRING "Versão (tag) do JUCE usada pelo projeto")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${JUCE_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Nome provisório. Quando o nome comercial for escolhido, altera apenas PRODUCT_NAME.
juce_add_gui_app(DAWWorkstation
    PRODUCT_NAME "DAW Workstation"
    COMPANY_NAME "lendariorafael"
)

juce_generate_juce_header(DAWWorkstation)

target_sources(DAWWorkstation PRIVATE
    Source/Main.cpp
)

target_compile_definitions(DAWWorkstation PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(DAWWorkstation PRIVATE
    juce::juce_gui_basics
)
