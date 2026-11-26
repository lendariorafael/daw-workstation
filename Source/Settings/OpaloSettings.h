#pragma once

#include <juce_core/juce_core.h>

// Definições guardadas no perfil do utilizador (idioma, dispositivo de áudio, latência).
namespace OpaloSettings
{
    inline juce::PropertiesFile& get()
    {
        static juce::PropertiesFile props([]
        {
            juce::PropertiesFile::Options options;
            options.applicationName = "Opalo";
            options.filenameSuffix = ".settings";
            options.folderName = "Opalo";
            options.osxLibrarySubFolder = "Application Support";
            options.storageFormat = juce::PropertiesFile::storeAsXML;
            return options;
        }());

        return props;
    }
}
