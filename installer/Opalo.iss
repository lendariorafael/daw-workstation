; Opalo Windows x64 installer (Inno Setup 6)
#define MyAppName "Opalo"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Opalo Audio"
#define MyAppExeName "Opalo.exe"

[Setup]
AppId={{D5E2A88B-59CF-4F2A-A2B4-0B9B5B77F601}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Opalo
DefaultGroupName=Opalo
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=Opalo-Setup-Windows-x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupLogging=yes

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "..\build\Opalo_artefacts\Release\Opalo.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Opalo"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\Opalo"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch Opalo"; Flags: postinstall nowait skipifsilent
