; $Id$
;
; Password Safe Installation Script
;
;
; COPYRIGHT NOTICE
;
; Copyright 2004, David Lacy Kusters (dkusters@yahoo.com)
; This script may be redistributed and/or modified under the Artistic
; License as available at 
;
; http://www.opensource.org/licenses/artistic-license.php
;
; This script is distributed AS IS.  All warranties are explicitly
; disclaimed, including, but not limited to, the implied warranties of
; MERCHANTABILITY and FITNESS FOR A PARTICULAR PURPOSE.
; 
;
; SYNOPSIS
;
; This script will create a self-extracting installer for the Password
; safe program.  It is intended to be used with the Nullsoft 
; Installation System, available at http://nsis.sf.net.   After use, 
; pwsafe-X.XX.exe will be placed in the current directory (where X.XX
; is the version number of Password Safe).  The generated file can be 
; placed on a publicly available location.
;
;
; DESCRIPTION
;
; This script will create a self-extracting installer for the Password
; Safe program.  Password Safe is intentionally designed to be self
; contained.  Use of this installer is not mandatory.  pwsafe.exe, the
; executable for Password Safe, can be placed in any location and run
; without any registration of DLLs, creation of directories, or entry
; of registry values.  When Password Safe intializes, any necessary
; setup will be performed by pwsafe.exe.  So, what is the purpose of
; this installer?
;
; This installer puts a familiar face on the installation process.
; Most Windows users are used to running a program to install 
; software, not copying a file or unzipping an archive.  Also, this
; installer performs some minor tasks that are common to many 
; Windows installers:
; 
; 1. The installer will allow the user to place icons on the desktop
;    or in the Start Menu, for easy access.  
;
; 2. The installer places two registry values in 
;    HKCU\Software\Counterpane Systems\Password Safe.  These registry
;    values are for the use of the installer itself.  Password Safe
;    does not rely on these registry values.  If the installer is not
;    used, these registry values need not be created.
;
; 3. The installer will create an uninstaller and place an entry to
;    uninstall Password Safe in the Add or Remove Programs Wizard.
;
; If you wish to avoid use of this installer, it is easy to do so.  
; Merely place pwsafe.exe in any location.  It is executable by users
; with fairly limited permissions. Since it is self-contained, so
; no actions other than running pwsafe.exe are required.  To reiterate, 
; use of this installer is completely optional.
; 
; -- Change for version 2.16 of PasswordSafe and later:
; Since the move to Visual Studio 2003 build environment
; requires new versions of DLLs, this script will now
; install the new files if required.
; The files are also available from the project's homepage,
; so the installer still isn't strictly necessary, even for users
; who don't have the latest & greatest DLLs.
;
; USE
;
; To use this script, the following requirements must be satisfied:
;
; 1. Install NSIS (available at http://nsis.sf.net).  At least version
;    2.0 should be used.  This script is compatible with version 2.0
;    of NSIS.
;
; 2. Make sure that makensis.exe is on your path.  This is only to make
;    easier step 3 of the creation process detailed below.  This script
;    does not recursively call makensis.exe, so this step is merely for
;    convenience sake.
;
; After the above requirements are fulfilled, the following steps 
; should be followed each time you want to create a release:
;
; 1. Compile Password Safe in release mode.  The script relies on 
;    pwsafe.exe existing in the release directory.
;
; 2. Compile the help files for Password Safe.  The script relies on 
;    pwsafe.chm existing in the current directory.
;
; 3. At the command line (or in a build script such as the .dsp file,
;    makefile, or other scripted build process), execute the following:
;
;        makensis.exe /DVERSION=X.XX pwsafe.nsi
;
;    where X.XX is the version number of the current build of Password
;    Safe.
;
; The output from the above process should be pwsafe-X.XX.exe.  This is
; the installer.  It can be placed, by itself, on a publicly available
; location.

;-----------------------------------------
; Set verbosity appropriate for a Makefile
!verbose 2

;--------------------------------
; Include Modern UI

  !include "MUI.nsh"

;--------------------------------
; Include Library macros (for DLL work)

  !include "Library.nsh"

;--------------------------------
; Version Info
;
; Hopefully, this file will be compiled via the following command line
; command:
;
; makensis.exe /DVERSION=X.XX pwsafe.nsi
;
; where X.XX is the version number of Password Safe.

  !ifndef VERSION
    !error "VERSION undefined. Usage: makensis.exe /DVERSION=X.XX pwsafe.nsi"
  !endif


;--------------------------------
; General

  ; Name and file
  Name "Password Safe ${VERSION}"

  OutFile "pwsafe-${VERSION}.exe"

  ; Default installation folder
  InstallDir "$PROGRAMFILES\Password Safe"
  
  ; Get installation folder from registry if available
  InstallDirRegKey HKCU \
                   "Software\Counterpane Systems\Password Safe" \
                   "installdir"


;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING


;--------------------------------
; Pages

  !insertmacro MUI_PAGE_LICENSE "LICENSE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  

;--------------------------------
; Languages
 
  !insertmacro MUI_LANGUAGE "English"


;--------------------------------
; The program itself

Section "Program Files" ProgramFiles

  ; Make the program files mandatory
  SectionIn RO

  ; Set the directory to install to
  SetOutPath "$INSTDIR"
  
  ; Get all of the files.  This list should be modified when additional
  ; files are added to the release.
  File "Release\pwsafe.exe"
  File "pwsafe.chm"
  File "LICENSE"
  File "README.TXT"
  File "ReleaseNotes.txt"
  File "ChangeLog.txt"

  Var /GLOBAL MFC71_INSTALLED

  IfFileExists "$SYSDIR\mfc71.dll" 0 hasnt_mfc71
   StrCpy $MFC71_INSTALLED 1
 hasnt_mfc71:

   !insertmacro InstallLib REGDLL $MFC71_INSTALLED NOREBOOT_PROTECTED ..\redist\mfc71.dll $SYSDIR\mfc71.dll $SYSDIR

  
  ; Store installation folder
  WriteRegStr HKCU \
              "Software\Counterpane Systems\Password Safe" \
              "installdir" \
              $INSTDIR

  ; Store the version
  WriteRegStr HKCU \
              "Software\Counterpane Systems\Password Safe" \
              "installversion" \
              "${VERSION}"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Add the uninstaller to the Add/Remove Programs window.  If the 
  ; current user doesn't have permission to write to HKLM, then the
  ; uninstaller will not appear in the Add or Remove Programs window.
  WriteRegStr HKLM \
	      "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
	      "DisplayName" "Password Safe"
  WriteRegStr HKLM \
	      "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
	       "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM \
		"Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
		"NoModify" 1
  WriteRegDWORD HKLM \
		"Software\Microsoft\Windows\CurrentVersion\Uninstall\PasswordSafe" \
		"NoRepair" 1
SectionEnd


;--------------------------------
; Start menu
Section "Show in start menu" StartMenu

  ; Create the Password Safe menu under the programs part of the start
  ; menu
  CreateDirectory "$SMPROGRAMS\Password Safe"

  ; Create shortcuts
  CreateShortCut "$SMPROGRAMS\Password Safe\Password Safe.lnk" \
                 "$INSTDIR\pwsafe.exe"

  CreateShortCut "$SMPROGRAMS\Password Safe\Password Safe Help.lnk" \
                 "$INSTDIR\pwsafe.chm"

SectionEnd


;--------------------------------
; Desktop shortcut
Section "Install desktop shortcut" DesktopShortcut

  ; Create desktop icon
  CreateShortCut "$DESKTOP\Password Safe.lnk" "$INSTDIR\pwsafe.exe"

SectionEnd


;--------------------------------
; Descriptions

  ; Language strings
  LangString DESC_ProgramFiles \
             ${LANG_ENGLISH} \
             "Installs the basic files necessary to run Password Safe."

  LangString DESC_StartMenu \
             ${LANG_ENGLISH} \
             "Creates an entry in the start menu for Password Safe."

  LangString DESC_DesktopShortcut \
             ${LANG_ENGLISH} \
             "Places a shortcut to Password Safe on your desktop."
  
  ; Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT \
                 ${ProgramFiles} \
                 $(DESC_ProgramFiles)

    !insertmacro MUI_DESCRIPTION_TEXT ${StartMenu} $(DESC_StartMenu)

    !insertmacro MUI_DESCRIPTION_TEXT \
                 ${DesktopShortcut} \
                 $(DESC_DesktopShortcut)

  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Delete all installed files in the directory
  Delete "$INSTDIR\pwsafe.exe"
  Delete "$INSTDIR\pwsafe.chm"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.TXT"
  Delete "$INSTDIR\ReleaseNotes.txt"
  Delete "$INSTDIR\ChangeLog.txt"

  ; remove directory if it's empty
  RMDir  "$INSTDIR"

  ; Delete the registry key for Password Safe
  DeleteRegKey HKCU "Software\Counterpane Systems\Password Safe"

  ; Delete the registry key for the Add or Remove Programs window.  If
  ; the current user doesn't have permission to delete registry keys
  ; from HKLM, then the entry in the Add or Remove Programs window will
  ; remain.  The next time a user tries to click on the uninstaller,
  ; they will be prompted to remove the entry.
  DeleteRegKey HKLM \
     "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe"
  ; Delete shortcuts, if created
  RMDir /r "$SMPROGRAMS\Password Safe"
  Delete "$DESKTOP\Password Safe.lnk"



SectionEnd
;
; $Log$
; Revision 1.6.2.1  2006/01/27 06:21:44  ronys
; V2.16 release
;
; Revision 1.6  2005/02/25 10:38:39  ronys
; [1123373] Uninstall will only remove installed files, and will delete the installation directory if and only if it's empty.
;
; Revision 1.5  2004/06/11 03:57:30  ronys
; Moved older changes from ReleaseNotes to ChangeLog.
;
; Revision 1.4  2004/06/09 19:28:12  ronys
; Merged dlacykusters fixes into post-2.03
;
; Revision 1.3  2004/06/08 03:35:21  ronys
; - add unstaller to control panel
; - cleanup shortcuts on uninstall
;
; Revision 1.2  2004/06/06 05:32:39  ronys
; release 2.03
;
