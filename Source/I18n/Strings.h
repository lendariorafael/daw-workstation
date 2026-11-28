#pragma once

#include <juce_core/juce_core.h>

// Traduções da interface: Português, English e Français.
// Para acrescentar um texto, adiciona uma linha à tabela e usa I18n::t("chave").
namespace I18n
{
    enum class Language { Portuguese = 0, English = 1, French = 2 };

    inline Language& current()
    {
        static Language language = Language::Portuguese;
        return language;
    }

    inline juce::String code(Language l)
    {
        switch (l)
        {
            case Language::English: return "en";
            case Language::French:  return "fr";
            default:                return "pt";
        }
    }

    inline Language fromCode(const juce::String& c)
    {
        if (c == "en") return Language::English;
        if (c == "fr") return Language::French;
        return Language::Portuguese;
    }

    struct Entry
    {
        const char* key;
        const char* pt;
        const char* en;
        const char* fr;
    };

    inline juce::String t(const char* key)
    {
        static const Entry table[] =
        {
            { "file.new",        "Novo",                    "New",               "Nouveau" },
            { "file.open",       "Abrir",                   "Open",              "Ouvrir" },
            { "file.save",       "Guardar",                 "Save",              "Enregistrer" },
            { "file.saveAs",     "Guardar como",            "Save as",           "Enregistrer sous" },
            { "audio.load",      "Carregar áudio",          "Load audio",        "Charger audio" },
            { "transport.play",  "Tocar",                   "Play",              "Lecture" },
            { "transport.stop",  "Parar",                   "Stop",              "Arrêt" },
            { "view.browser",    "Biblioteca",              "Browser",           "Navigateur" },
            { "view.playlist",   "Playlist",                "Playlist",          "Playlist" },
            { "view.mixer",      "Mesa",                    "Mixer",             "Mixeur" },
            { "view.beat",       "Grelha de batida",        "Beat Deck",         "Grille de rythme" },
            { "view.notes",      "Editor de notas",         "Note Canvas",       "Éditeur de notes" },
            { "view.vault",      "Cofre de plugins",        "Plugin Vault",      "Coffre de plugins" },
            { "view.mashup",     "Mashup",                  "Mashup",            "Mashup" },
            { "view.settings",   "Definições",              "Settings",          "Réglages" },
            { "settings.title",  "Definições",              "Settings",          "Réglages" },
            { "settings.language", "Idioma",                "Language",          "Langue" },
            { "settings.latency",  "Tamanho do buffer",     "Buffer size",       "Taille du tampon" },
            { "settings.latencyInfo", "Latência de saída estimada", "Estimated output latency", "Latence de sortie estimée" },
            { "mashup.load",     "Carregar",                "Load",              "Charger" },
            { "mashup.sync",     "Sincronizar com o projeto", "Sync to project", "Synchroniser au projet" },
            { "mashup.match",    "Igualar B a A",           "Match B to A",      "Caler B sur A" },
            { "mashup.crossfader", "Crossfader",            "Crossfader",        "Crossfader" },
            { "mashup.deckA",    "Deck A",                  "Deck A",            "Platine A" },
            { "mashup.deckB",    "Deck B",                  "Deck B",            "Platine B" },
            { "mashup.noFile",   "Sem ficheiro",            "No file",           "Aucun fichier" },
            { "mashup.varispeed","Varispeed: mudar a velocidade também muda o tom.",
                                  "Varispeed: changing speed also changes pitch.",
                                  "Varispeed : changer la vitesse change aussi la hauteur." },
            { "tool.select",     "Selecionar (D)",          "Select (D)",        "Sélection (D)" },
            { "tool.cut",        "Cortar (C)",              "Cut (C)",           "Couper (C)" },
            { "status.ready",    "Pronto",                  "Ready",             "Prêt" },
            { "menu.file",     "Ficheiro",         "File",       "Fichier" },
            { "menu.edit",     "Editar",           "Edit",       "Édition" },
            { "menu.add",      "Adicionar",        "Add",        "Ajouter" },
            { "menu.patterns", "Padrões",          "Patterns",   "Motifs" },
            { "menu.view",     "Ver",              "View",       "Affichage" },
            { "menu.options",  "Opções",           "Options",    "Options" },
            { "menu.tools",    "Ferramentas",      "Tools",      "Outils" },
            { "menu.help",     "Ajuda",            "Help",       "Aide" },
            { "file.recent",   "Abrir recentes",   "Open recent","Ouvrir récents" },
            { "file.loadAudio","Carregar áudio…",  "Load audio…","Charger audio…" },
            { "edit.undo",     "Desfazer",         "Undo",       "Annuler" },
            { "edit.redo",     "Refazer",          "Redo",       "Rétablir" },
            { "add.channel",   "Novo canal",       "New channel","Nouveau canal" },
            { "tab.playlist",  "Playlist",         "Playlist",   "Playlist" },
            { "tab.record",    "Gravação indisponível ainda", "Recording not available yet", "Enregistrement pas encore disponible" },
            { "browser.info.author", "Autor",      "Author",     "Auteur" },
            { "browser.info.date",   "Data",       "Date",       "Date" },
            { "browser.info.size",   "Tamanho",    "Size",       "Taille" },
            { "browser.info.none",   "Nenhum ficheiro selecionado.", "No file selected.", "Aucun fichier sélectionné." },
            { "about.text",    "Opalo 0.1.0 — estúdio de produção musical.", "Opalo 0.1.0 — music production studio.", "Opalo 0.1.0 — studio de production musicale." },
        };

        const juce::String wanted(key);

        for (const auto& e : table)
        {
            if (wanted == e.key)
            {
                switch (current())
                {
                    case Language::English:  return e.en;
                    case Language::French:   return e.fr;
                    default:                 return e.pt;
                }
            }
        }

        return wanted;
    }
}
