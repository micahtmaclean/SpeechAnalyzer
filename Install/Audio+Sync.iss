; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Audio+Sync"
#define MyAppVersion "0.99.0.12"
#define MyAppPublisher "SIL International, Inc."
#define MyAppURL "http://www.speechanalyzer.sil.org/"
#define MyAppExeName "AS.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{601205CF-8585-4859-ABE2-BEA614862560}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=C:\Working\SIL\MSEA\Install\Output
OutputBaseFilename=Audio+Sync
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\Working\SIL\MSEA\Output\Release\AS.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\ECInterfaces.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\mbrola.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SA_DSP.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SA_ENU.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SilEncConverters40.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SAUtils.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.mmedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.wmfsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\zGraph.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Src\Release Notes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Src\Roadmap.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\usp10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Microsoft\vcredist_x86.exe"; DestDir: "{app}\components"; Flags: ignoreversion; Check: VCRedistCheck()
Source: "C:\Working\SIL\MSEA\DistFiles\Bmp2png.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\xerces-c_3_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\notice"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\license"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\uriparser\uriparser_copying"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\iso639.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\sa3.psa"; DestDir: "{userdocs}\Speech Analyzer"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Audio+Sync Help.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Keyboard Short-Cut Keys.pdf"; DestDir: "{app}"; Flags: ignoreversion


[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

;.NET 3.5 = .NET 2.0 + .NET 3.5
;Due to the way .NET 3.5 is "just" an add-on to .NET 3.0 which is "just" an add-on to .NET 2.0, 
; the tools in the .NET 2.0 directory are still the ones to use.  
; The CLR version number is the same for all three of these frameworks (2.0.50727).
[Run]
Filename: "{app}\components\vcredist_x86.exe"; Parameters: "/q"; WorkingDir: "{app}\components"; Flags: waituntilterminated skipifdoesntexist; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; StatusMsg: "Installing Microsoft Visual C++ 2010 SP1 Redistributable"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SAUtils.dll /tlb:SAUtils.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering SpeechToolsUtils"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /tlb:yeti.mmedia.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.mmedia"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /tlb:yeti.wmfsdk.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.wmfsdk"

[UninstallRun]
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SAUtils.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden

[Dirs]
Name: "{app}\components"

[UninstallDelete]
Type: files; Name: "{app}\SAUtils.tlb"
Type: files; Name: "{app}\yeti.mmedia.tlb"
Type: files; Name: "{app}\yeti.wmfsdk.tlb"

[InstallDelete]
Type: files; Name: "{app}\SA MSEA Release Notes.txt"
Type: files; Name: "{app}\SA MSEA Roadmap.txt"

[Code]
function VCRedistCheck(): Boolean;
begin
  Result := (FileExists(ExpandConstant('{win}\WinSxS\x86_Microsoft.VC90.MFC_1fc8b3b9a1e18e3b_9.0.30729.1_x-ww_405b0943\mfc90.dll'))=false);
end;
