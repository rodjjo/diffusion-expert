[Setup]
AppName=Diffusion Expert
AppVersion=1.0
DefaultDirName={userappdata}\Diffusion Expert
DefaultGroupName=Diffusion Expert
UninstallDisplayIcon={app}\uninstall.ico
Compression=lzma2
SolidCompression=yes
OutputBaseFilename=Setup
OutputDir=.\build\bin
PrivilegesRequiredOverridesAllowed=commandline dialog

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; The python files should be installed first as we replace python configs after
Source: "temp\python3.10\*.*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs
Source: "build\bin\diffusion-exp.exe"; DestDir: "{app}\bin"; Flags: ignoreversion 
Source: "build\bin\diffusion-exp-win.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "build\mingw64\bin\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "build\mingw64\opt\bin\libgcc_s_seh-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "build\mingw64\opt\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "data\images\app.ico"; DestDir: "{app}\bin"
Source: "data\images\uninstall.ico"; DestDir: "{app}\bin"
Source: "python_stuff\*.*"; DestDir: "{app}\python_stuff"; Flags: ignoreversion recursesubdirs
Source: "python_configs\*.*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs


[Icons]
Name: "{group}\Diffusion Expert"; Filename: "{app}\bin\diffusion-exp-win.exe"; IconFilename: {app}\bin\app.ico ; WorkingDir: "{app}\bin"
Name: "{userdesktop}\Diffusion Expert"; Filename: "{app}\bin\diffusion-exp-win.exe"; IconFilename: {app}\bin\app.ico; Tasks: desktopicon ; WorkingDir: "{app}\bin"

