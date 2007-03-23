;
; Password Safe Installation Script
;
; Copyright 2004, David Lacy Kusters (dkusters@yahoo.com)
; Copyright 2005-2006 Rony Shapiro <ronys@users.sourceforge.net>
; This script may be redistributed and/or modified under the Artistic
; License as available at 
;
; http://www.opensource.org/licenses/artistic-license.php
;
; This script is distributed AS IS.  All warranties are explicitly
; disclaimed, including, but not limited to, the implied warranties of
; MERCHANTABILITY and FITNESS FOR A PARTICULAR PURPOSE.
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
; DESCRIPTION
;
; This script will create a self-extracting installer for the Password
; Safe program.  Password Safe was designed to be self contained.
; In general, use of this installer is not mandatory. pwsafe.exe, the
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
;    HKCU\Software\Password Safe\Password Safe.  These registry
;    values are for the use of the installer itself.  Password Safe
;    does not rely on these registry values.  If the installer is not
;    used, these registry values need not be created.
;
; 3. The installer will create an uninstaller and place an entry to
;    uninstall Password Safe in the Add or Remove Programs Wizard.
;
; As of PasswordSafe 3.05, this script allows users to choose
; between a "Regular" installation and a "Green" one, the difference
; being that the latter does not write app-specific data to the registry
; This is useful for installing to disk-on-key, and where company policy
; and/or user permissions disallow writing to the registry. Also, Green
; installation doesn't create an Uninstall.exe or entry in Add/Remove
; Software in the control panel - to unistall, just delete the install
; directory...
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
;    pwsafe.exe existing in the Release subdirectory.
;
; 2. Compile the help files for Password Safe.  The script relies on 
;    pwsafe.chm existing in the help/default subdirectory.
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
;Variables

  Var INSTALL_TYPE

;--------------------------------
; General

  ; Name and file
  Name "Password Safe ${VERSION}"

  OutFile "pwsafe-${VERSION}.exe"

  ; Default installation folder
  InstallDir "$PROGRAMFILES\Password Safe"
  
  ; Get installation folder from registry if available
  InstallDirRegKey HKCU \
                   "Software\Password Safe\Password Safe" \
                   "installdir"


;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING


;--------------------------------
; Pages

  !insertmacro MUI_PAGE_LICENSE "..\LICENSE"
  ; ask about installation type, "green" or "regular"
  Page custom GreenOrRegular
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  

;--------------------------------
; Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserve Files
  
  ; NSIS documentation states that it's a Good Idea to put the following
  ; two lines when using a custom dialog:  
  ReserveFile "pws-install.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
; The program itself

Section "Program Files" ProgramFiles
  ;Read the chosen installation type: 1 means "Green", 0 - "Regular"
  !insertmacro MUI_INSTALLOPTIONS_READ $INSTALL_TYPE "pws-install.ini" "Field 2" "State"

  ; Make the program files mandatory
  SectionIn RO

  ; Set the directory to install to
  SetOutPath "$INSTDIR"
  
  ; Get all of the files.  This list should be modified when additional
  ; files are added to the release.
  File "..\Release\pwsafe.exe"
  File "..\help\default\pwsafe.chm"
  File "..\LICENSE"
  File "..\README.TXT"
  File "..\docs\ReleaseNotes.txt"
  File "..\docs\ChangeLog.txt"
  File "..\xml\pwsafe.xsd"
  File "..\xml\pwsafe.xsl"

  ; skip over registry writes if 'Green' installation selected
  IntCmp $INSTALL_TYPE 1 GreenInstall

  ; Store installation folder
  WriteRegStr HKCU \
              "Software\Password Safe\Password Safe" \
              "installdir" \
              $INSTDIR

  ; Store the version
  WriteRegStr HKCU \
              "Software\Password Safe\Password Safe" \
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
		"Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
		"NoRepair" 1
GreenInstall:
SectionEnd

;--------------------------------
; Start with Windows
Section "Start automatically" StartUp
  CreateShortCut "$SMSTARTUP\Password Safe.lnk" \
                 "$INSTDIR\pwsafe.exe" "-s"
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

  LangString DESC_StartUp \
             ${LANG_ENGLISH} \
             "Starts Password Safe as part of Windows boot/login."

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

    !insertmacro MUI_DESCRIPTION_TEXT ${StartUp} $(DESC_StartUp)
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
  Delete "$INSTDIR\pwsafe.xsd"
  Delete "$INSTDIR\pwsafe.xsl"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.TXT"
  Delete "$INSTDIR\ReleaseNotes.txt"
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\mfc80.dll"
  Delete "$INSTDIR\msvcp80.dll"
  Delete "$INSTDIR\msvcr80.dll"
  Delete "$INSTDIR\Microsoft.VC80.CRT.manifest"
  Delete "$INSTDIR\Microsoft.VC80.MFC.manifest"

  ; remove directory if it's empty
  RMDir  "$INSTDIR"

  ; Delete the registry key for Password Safe
  DeleteRegKey HKCU "Software\Password Safe\Password Safe"

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

;-----------------------------------------
; Functions
Function .onInit

  ;Extract InstallOptions INI files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "pws-install.ini"

FunctionEnd

LangString TEXT_GC_TITLE ${LANG_ENGLISH} "Installation Type"
LangString TEXT_GC_SUBTITLE ${LANG_ENGLISH} "Choose Regular for use on a single PC, Green for portable installation. If you're not sure, 'Regular' is fine."

Function GreenOrRegular
  !insertmacro MUI_HEADER_TEXT "$(TEXT_GC_TITLE)" "$(TEXT_GC_SUBTITLE)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "pws-install.ini"
FunctionEnd
