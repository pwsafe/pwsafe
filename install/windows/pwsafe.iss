;$Id: pwsafe.iss,v 1.1 2006/06/11 17:58:40 laurent Exp $
;
; Usage
; -----
; 
; This ".iss" file is used by InnoSetup to produce a setup program for Password
; safe.
; 
; When a new release is made, this file should be updated to change the version
; number in the first three lines of the "[Setup]" section.
;
;
; Silent/unattended setup
; -----------------------
;
; The resulting setup program may be used for silent install. 
; 
; 1) Prepare an answer file by running the setup program with the appropriate options:
;    pwsafe-%VERSION%.exe /SAVEINF="pwsafe.iss.inf"
;
; 2) Then, use the answer file to make subsequent silent setups :
;    pwsafe-%VERSION%.exe /SILENT /LOADINF="pwsafe.iss.inf"
;
; See the "Setup command line parameters" in the InnoSetup Help File for details.
;
;
; Microsoft Redistributable MFC dlls
; ----------------------------------
; 
; This setup program will copy Microsoft MFC dlls in the program folder. See these
; URLs about this behaviour:
;  - http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/vcoriRedistributingMFCATLOLEDBTemplatesApplications.asp
;  - http://support.microsoft.com/kb/326922/en-us
;  - http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt_c_run.2d.time_libraries.asp
; 

[Setup]
AppVersion=3.01
AppVerName=Password Safe 3.01
OutputBaseFilename=pwsafe-3.01
DefaultDirName={pf}\Password Safe
AppName=Password Safe
SourceDir=.
OutputDir=.
DefaultGroupName=Password Safe
UninstallDisplayIcon={app}\pwsafe.exe
AppPublisher=Password Safe
AppPublisherURL=http://passwordsafe.sourceforge.net/
AppSupportURL=https://sourceforge.net/support/getsupport.php?group_id=41019
AllowNoIcons=yes
Compression=lzma
SolidCompression=yes
LicenseFile=LICENSE
ChangesAssociations=yes
PrivilegesRequired=poweruser

[Files]
Source: "pwsafe.exe";              DestDir: "{app}"
Source: "pwsafe.chm";              DestDir: "{app}"
Source: "README.MD";              DestDir: "{app}"; Flags: isreadme
Source: "LICENSE";                 DestDir: "{app}"
Source: "pwsafe.chm";              DestDir: "{app}"
Source: "ReleaseNotes.md";        DestDir: "{app}"
Source: "ChangeLog.txt";           DestDir: "{app}"
Source: "mfc71.dll";               DestDir: "{app}"
Source: "msvcp71.dll";             DestDir: "{app}"
Source: "msvcr71.dll";             DestDir: "{app}"

[Icons]
Name: "{group}\Password Safe";      Filename: "{app}\pwsafe.exe"
Name: "{group}\Password Safe Help"; Filename: "{app}\pwsafe.chm"


[Registry]
;
; Delete user's registry parameters upon uninstall.
; (These keys and subkeys will be created by pwsafe.exe on first execution)
;
Root: HKCU; Subkey: "Software\Counterpane Systems\Password Safe"; Flags: dontcreatekey uninsdeletekey

;
; Add registry key to allow running password safe by using the "Start > Run > pwsafe" 
; command.
;
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\pwsafe.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\pwsafe.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\pwsafe.exe"; Flags: uninsdeletekey

;
; File association
;
Root: HKCR; Subkey: ".psafe3"; ValueType: string; ValueName: ""; ValueData: "PasswordSafe"; Flags: uninsdeletevalue noerror
Root: HKCR; Subkey: "PasswordSafe"; ValueType: string; ValueName: ""; ValueData: "My Program File"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "PasswordSafe\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\pwsafe.exe,0"; Flags: noerror
Root: HKCR; Subkey: "PasswordSafe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\pwsafe.exe"" ""%1"""; Flags: noerror
