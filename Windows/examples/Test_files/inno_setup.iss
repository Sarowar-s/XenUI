
[Setup]
AppId={{A4B83E12-CE21-4E8C-AF19-10C8F92B1234}
AppName=XenonUI Example
AppVersion=0.9.0
AppPublisher=Sarowar
DefaultDirName={autopf}\XenonUI Example
DefaultGroupName=XenonUI Example
OutputBaseFilename=XenonUI_Example_Setup_0.9.0
Compression=lzma2/ultra
SolidCompression=yes
WizardImageFile=logo.bmp
SetupIconFile=logo.ico
LicenseFile=../../LICENSE/LICENSE.txt

[Files]
Source: "build-app\xenui_test.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\SDL3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\SDL3_image.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\SDL3_ttf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\libXenUI.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build-app\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "Images\*"; DestDir: "{app}\Images"; Flags: ignoreversion recursesubdirs createallsubdirs

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{group}\XenonUI Example"; Filename: "{app}\xenui_test.exe"
Name: "{group}\Uninstall XenonUI Example"; Filename: "{uninstallexe}"
Name: "{autodesktop}\XenonUI Example"; Filename: "{app}\xenui_test.exe"; Tasks: desktopicon
