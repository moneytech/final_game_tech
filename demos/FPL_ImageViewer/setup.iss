; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "FPL_ImageViewer"
#define MyAppVersion "1.0"
#define MyAppPublisher "Torsten Spaete"
#define MyAppURL "https://www.libfpl.org"
#define MyAppExeName "FPL_ImageViewer.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{6C9E5A9E-86C4-4777-B5D3-D06D833AB4D2}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
DisableWelcomePage=no
OutputDir=..\setup\{#MyAppName}
OutputBaseFilename={#MyAppName}-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\bin\FPL_ImageViewer\Windows-x64-Release\FPL_ImageViewer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{sys}\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion external
Source: "{sys}\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion external

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

