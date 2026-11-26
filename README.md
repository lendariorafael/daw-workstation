# Opalo

Opalo is a Windows x64 digital audio workstation project built with C++20, JUCE 8 and CMake.

## Current interface

- Single main studio window. Panels open inside the workspace rather than as separate operating-system windows.
- Movable and resizable panels with a draggable title bar and resize grip.
- Default studio arrangement with Browser, Playlist, Channel Rack and Mixer visible together.
- Workspace presets: Studio Default, Arrangement Focus, Mix Focus and DJ Mashup.
- File, Edit, View, Tools and Help menus, plus a compact icon shortcut strip.
- Transport controls for Start, Play, Stop, project BPM and master output level.
- Compact mixer channel strips.
- DJ-style dual-deck Mashup panel with independent deck controls and crossfader.
- Playlist tools: Select (D), Slice (C), Pencil (P), Brush (B) and Copy (X).

The Record control is displayed but disabled until audio-input recording is implemented. VST3 scanning and plugin-slot selection are present; real-time plugin processing and full arrangement playback remain in development.

## Build from source

Requirements:
- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- Git
- CMake

Run from the repository root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable is generated at:

`build/Opalo_artefacts/Release/Opalo.exe`

## Windows installer

GitHub Actions builds the Windows executable and, on pushes to the main repository, packages it with Inno Setup 6 as:

`Opalo-Setup-Windows-x64.exe`

The installer includes the Microsoft Visual C++ x64 runtime prerequisite and creates Start Menu shortcuts. Download it from the workflow run's **Opalo-Installer-Windows-x64** artifact.

The installer definition is in `installer/Opalo.iss`.

## Project structure

- `Source/AudioEngine` — audio playback engine and DJ decks.
- `Source/Model` — shared project model, project file and undo history.
- `Source/UI` — main workspace, movable panels, themes and editing tools.
- `Source/Settings` — user preferences and device settings.
- `Documentation/ROADMAP.md` — development status.
- `.github/workflows/build.yml` — Windows build and installer pipeline.

## Licence

The repository currently declares GPLv3. Review the licence and third-party terms before distributing modified builds.
