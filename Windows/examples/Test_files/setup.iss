[Setup]
AppName=XenonUI
AppVersion=0.9.0
AppPublisher=Sarowar
DefaultDirName={autopf}\My Application
DefaultGroupName=My Application
AllowCancelDuringInstall=no 
OutputBaseFilename=MyApp_Setup_1.0
Compression=lzma2/ultra
SolidCompression=yes
WizardImageFile=logo.bmp
SetupIconFile=logo.ico
LicenseFile=../../LICENSE/LICENSE.txt

[Files]
Source: "build/xenui_test.exe"; DestDir: "{app}"
Source: "build/SDL3.dll"; DestDir: "{app}"
Source: "build/SDL3_image.dll"; DestDir: "{app}"
Source: "build/SDL3_ttf.dll"; DestDir: "{app}"
Source: "build/libXenUI.dll"; DestDir: "{app}"
Source: "build/libgcc_s_seh-1.dll"; DestDir: "{app}"
Source: "build/libstdc++-6.dll"; DestDir: "{app}"
Source: "build/libwinpthread-1.dll"; DestDir: "{app}"

Source: "Images\*";            DestDir: "{app}\Images"


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Icons]
Name: "{group}\My Application"; Filename: "{app}\xenui_test.exe"
Name: "{group}\Uninstall My Application"; Filename: "{uninstallexe}"
Name: "{autodesktop}\My Application"; Filename: "{app}\xenui_test.exe"; Tasks: desktopicon