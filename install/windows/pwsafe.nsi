;======================================================================================================
;
; Password Safe Installation Script
;
; Copyright 2004, David Lacy Kusters (dkusters@yahoo.com)
; Copyright 2005-2018 Rony Shapiro <ronys@pwsafe.org>
; 2009 extended by Karel Van der Gucht for multiple language use
; This script may be redistributed and/or modified under the Artistic
; License 2.0 terms as available at 
; http://www.opensource.org/licenses/artistic-license-2.0.php
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
; of registry values.  When Password Safe initializes, any necessary
; setup will be performed by pwsafe.exe.  So, what is the purpose of
; this installer?
;
; This installer puts a familiar face on the installation process.
; Most Windows users are used to running a program to install 
; software, not copying a file or unzipping an archive.  Also, this
; installer performs some minor tasks that are common to many 
; Windows installers:
; 
; 1. The installer allows the user to place icons on the desktop
;    or in the Start Menu, for easy access.  
;
; 2. The installer optionally places four registry values in 
;    HKCU\Software\Password Safe\Password Safe.  These registry
;    values are for the use of the installer itself.  Password Safe
;    does not rely on these registry values.  If the installer is not
;    used, or if a "Green" installation is selected (see below), these
;    registry values are not created. 
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
; Software in the control panel - to uninstall, just delete the install
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
; 2. Install the nsProcess NSIS plug in. This can be found at
;    http://nsis.sourceforge.net/NsProcess_plugin
;    Choose version 1.6 or later. 
;
; 3. Make sure that makensis.exe is on your path.  This is only to make
;    easier step 3 of the creation process detailed below.  This script
;    does not recursively call makensis.exe, so this step is merely for
;    convenience sake.
;
; After the above requirements are fulfilled, the following steps 
; should be followed each time you want to create a release:
;
; 1. Compile Password Safe in Release and Release64 mode.  The script 
;    relies on pwsafe.exe existing in the Release and Release64 
;    subdirectory.
;
; 2. Compile the help files for Password Safe.  The script relies on 
;    the English pwsafe.chm existing in the help/default subdirectory,
;    and other translations in their respective help/* subdirectories.
;
; 3. At the command line (or in a build script such as the .dsp file,
;    makefile, or other scripted build process), execute the following:
;
;        makensis.exe /DVERSION=X.XX /DARCH=x86 pwsafe.nsi
;        makensis.exe /DVERSION=X.XX /DARCH=x64 pwsafe.nsi
;
;    where X.XX is the version number of the current build of Password
;    Safe.
;
; The output from the above process should be:
;
;    pwsafe-X.XX.exe (the 32-bit version) 
;    pwsafe64-X.XX.exe (the 64-bit version) 
;
; These are the installers.  They can be placed on a publicly 
; available location.
; 
; The script is setup for several languages, and ready for others.
; Just remove the comments ";-L-" where appropriate.
; Additional languages can be easily added, the following "pieces"
; have to be provided:
; - a .\I18N\pwsafe_xx.lng file with the installation texts in the language
; - several MUI_LANGUAGE
; - several "File" for the language specific DLL
; - "Delete ...DLL" for each language (at install time)
; - 'Delete "$INSTDIR\pwsafeXX.dll"'  for each language (at uninstall time)
; - 'CreateShortCut "Password Safe Help XX.lnk" for each language (at install time)
; - "Push" in the "Language selection dialog"

;-----------------------------------------
; Set verbosity appropriate for a Makefile
!verbose 2

;--------------------------------
; Include Modern UI
  !include "MUI2.nsh"
  !include "InstallOptions.nsh"
 
;--------------------------------
; process detection support
; (requires nsProcess plugin, from
;  http://nsis.sourceforge.net/NsProcess_plugin)
!include "nsProcess.nsh"

;--------------------------------
; Host OS version detection
!include "WinVer.nsh"


;--------------------------------
; Version Info
;
; Hopefully, this file will be compiled via the following command line
; command:
;
; makensis.exe /DVERSION=X.XX /DARCH=xNN pwsafe.nsi
;
; where X.XX is the version number of Password Safe and
; xNN is x86 or x64.

  !ifndef VERSION
    !error "VERSION undefined. Usage: makensis.exe /DVERSION=X.XX /DARCH=[x86|x64] pwsafe.nsi"
  !endif

;--------------------------------
; Installer architecture x86 or x64

!ifndef ARCH
  !error "ARCH undefined. Usage: makensis.exe /DVERSION=X.XX /DARCH=[x86|x64] pwsafe.nsi"
!endif  

;--------------------------------
;Variables

  Var INSTALL_TYPE
  
  ;Request application privileges for Windows Vista and later.
  RequestExecutionLevel admin

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_LICENSE "..\..\LICENSE" ;$(myLicenseData)
  ; ask about installation type, "green" or "regular"
  Page custom GreenOrRegular
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; General

  ; Default installation folder based on chosen architecture
  !if ${ARCH} == "x86"
    OutFile "pwsafe-${VERSION}.exe"
    InstallDir "$PROGRAMFILES\Password Safe"
    ; Name and file
    Name "Password Safe ${VERSION} (32-bit)"
    BrandingText "PasswordSafe ${VERSION} (32-bit) Installer"
    !define LANG_DLL "..\..\build\bin\pwsafe\I18N"
    !define BIN_DIR "..\..\build\bin\pwsafe\release"
    !define TARGET_ARCH "(32-bit)"
    !echo "Building x86 installer"
  !else if ${ARCH} == "x64" 
    OutFile "pwsafe64-${VERSION}.exe"
    InstallDir "$PROGRAMFILES64\Password Safe"
    ; Name and file
    Name "Password Safe ${VERSION} (64-bit)"
    BrandingText "PasswordSafe ${VERSION} (64-bit) Installer"
    !define LANG_DLL "..\..\build\bin\pwsafe\I18N64"
    !define BIN_DIR "..\..\build\bin\pwsafe\release64"
    !define TARGET_ARCH "(64-bit)"
    !echo "Building x64 installer"
  !else
    !error "ARCH must be either x86 or x64"
  !endif
  
  ; Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Password Safe\Password Safe" "installdir"

;--------------------------------
; Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "German"
  !include ".\I18N\pwsafe_de.lng"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !include ".\I18N\pwsafe_zh.lng"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !include ".\I18N\pwsafe_zh_tw.lng"
  !insertmacro MUI_LANGUAGE "Spanish"
  !include ".\I18N\pwsafe_es.lng"
  !insertmacro MUI_LANGUAGE "Swedish"
  !include ".\I18N\pwsafe_sv.lng"
  !insertmacro MUI_LANGUAGE "Dutch"
  !include ".\I18N\pwsafe_nl.lng"
  !insertmacro MUI_LANGUAGE "French"
  !include ".\I18N\pwsafe_fr.lng"
  !insertmacro MUI_LANGUAGE "Russian"
  !include ".\I18N\pwsafe_ru.lng"
  !insertmacro MUI_LANGUAGE "Polish"
  !include ".\I18N\pwsafe_pl.lng"
  !insertmacro MUI_LANGUAGE "Italian"
  !include ".\I18N\pwsafe_it.lng"
  !insertmacro MUI_LANGUAGE "Danish"
  !include ".\I18N\pwsafe_dk.lng"
  !insertmacro MUI_LANGUAGE "Korean"
  !include ".\I18N\pwsafe_kr.lng"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !include ".\I18N\pwsafe_pt-br.lng"
  !insertmacro MUI_LANGUAGE "Czech"
  !include ".\I18N\pwsafe_cz.lng"
  !insertmacro MUI_LANGUAGE "Turkish"
  !include ".\I18N\pwsafe_tr.lng"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !include ".\I18N\pwsafe_hu.lng"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !include ".\I18N\pwsafe_sl.lng"

; English texts here
; Note that if we add a string, it needs to be added in all the
; .\I18N\pwsafe_xx.lng files

;Reserve Files
LangString RESERVE_TITLE ${LANG_ENGLISH} "Choose Installation Type"
LangString RESERVE_FIELD1 ${LANG_ENGLISH} "Regular (uses Registry, suitable for home or single user PC)"
LangString RESERVE_FIELD2 ${LANG_ENGLISH} "Green (for Disk-on-Key; does not use host Registry)"

; The program itself
LangString PROGRAM_FILES ${LANG_ENGLISH} "Program Files"

; Start with Windows
LangString START_AUTO ${LANG_ENGLISH} "Start automatically"

; Start menu
LangString START_SHOW ${LANG_ENGLISH} "Show in start menu"

; Desktop shortcut
LangString START_SHORTCUT ${LANG_ENGLISH} "Install desktop shortcut"

; Uninstall shortcut
LangString UNINSTALL_SHORTCUT ${LANG_ENGLISH} "Uninstall shortcut"

; Descriptions
LangString DESC_ProgramFiles ${LANG_ENGLISH} "Installs the basic files necessary to run Password Safe."
LangString DESC_StartUp ${LANG_ENGLISH} "Starts Password Safe as part of Windows boot/login."
LangString DESC_StartMenu ${LANG_ENGLISH} "Creates an entry in the start menu for Password Safe."
LangString DESC_DesktopShortcut ${LANG_ENGLISH} "Places a shortcut to Password Safe on your desktop."
LangString DESC_UninstallMenu ${LANG_ENGLISH} "Places a shortcut in the start menu to Uninstall Password Safe."
LangString DESC_LangSupport ${LANG_ENGLISH} "Please select the language(s) that PasswordSafe will use."

; "LangString" (for "Function GreenOrRegular") are setup here because they cannot be defined in the function body
LangString TEXT_GC_TITLE ${LANG_ENGLISH} "Installation Type"
LangString TEXT_GC_SUBTITLE ${LANG_ENGLISH} "Choose Regular for use on a single PC, Green for portable installation. If you're not sure, 'Regular' is fine."

; several messages on install, check, ...
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LangString RUNNING_INSTALL ${LANG_ENGLISH} "The installer is already running."
LangString RUNNING_APPLICATION ${LANG_ENGLISH} "Please exit all running instances of PasswordSafe before installing a new version"
LangString LANG_INSTALL ${LANG_ENGLISH} "Installation Language"
LangString LANG_SELECT ${LANG_ENGLISH} "Please select the language for the installation"
LangString LANGUAGE_SUPPORT ${LANG_ENGLISH} "Language Support"
LangString ENGLISH_SUPPORT ${LANG_ENGLISH} "English"
LangString CHINESE_CN_SUPPORT ${LANG_ENGLISH} "Chinese (Simplified)"
LangString CHINESE_TW_SUPPORT ${LANG_ENGLISH} "Chinese (Traditional)"
LangString GERMAN_SUPPORT ${LANG_ENGLISH} "German"
LangString SPANISH_SUPPORT ${LANG_ENGLISH} "Spanish"
LangString SWEDISH_SUPPORT ${LANG_ENGLISH} "Swedish"
LangString DUTCH_SUPPORT ${LANG_ENGLISH} "Dutch"
LangString FRENCH_SUPPORT ${LANG_ENGLISH} "French"
LangString RUSSIAN_SUPPORT ${LANG_ENGLISH} "Russian"
LangString POLISH_SUPPORT ${LANG_ENGLISH} "Polish"
LangString ITALIAN_SUPPORT ${LANG_ENGLISH} "Italian"
LangString DANISH_SUPPORT ${LANG_ENGLISH} "Danish"
LangString KOREAN_SUPPORT ${LANG_ENGLISH} "Korean"
LangString PORTUGUESEBR_SUPPORT ${LANG_ENGLISH} "Portuguese (Brazil)"
LangString CZECH_SUPPORT ${LANG_ENGLISH} "Czech"
LangString TURKISH_SUPPORT ${LANG_ENGLISH} "Turkish"
LangString HUNGARIAN_SUPPORT ${LANG_ENGLISH} "Hungarian"
LangString SLOVENIAN_SUPPORT ${LANG_ENGLISH} "Slovenian"

LangString LANG_PROGRAM ${LANG_ENGLISH} "Program Language"
LangString SORRY_NO_95 ${LANG_ENGLISH} "Sorry, Windows 95 is no longer supported. Try PasswordSafe 2.16"
LangString SORRY_NO_98 ${LANG_ENGLISH} "Sorry, Windows 98 is no longer supported. Try PasswordSafe 2.16"
LangString SORRY_NO_ME ${LANG_ENGLISH} "Sorry, Windows ME is no longer supported. Try PasswordSafe 2.16"
LangString SORRY_NO_2K ${LANG_ENGLISH} "Sorry, Windows 2000 is no longer supported. Try PasswordSafe 3.18"

LangString Icon_description_Uninstall ${LANG_ENGLISH} "Password Safe Uninstall"
LangString Icon_description_Help ${LANG_ENGLISH} "Password Safe Help"

;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Reserve Files

  ; NSIS documentation states that it's a Good Idea to put the following
  ; line when using a custom dialog:  
  ReserveFile "pws-install.ini"

;-----------------------------------------------------------------
; The program itself

Section "$(PROGRAM_FILES)" ProgramFiles
  ;Read the chosen installation type: 1 means "Green", 0 - "Regular"
  !insertmacro INSTALLOPTIONS_READ $INSTALL_TYPE "pws-install.ini" "Field 2" "State"

  ; Make the program files mandatory
  SectionIn RO

  ; Set the directory to install to
  SetOutPath "$INSTDIR"
  
  ; Get all of the files.  This list should be modified when additional
  ; files are added to the install.
  File "${BIN_DIR}\pwsafe.exe"
  File "${BIN_DIR}\pws_at.dll"
  File "${BIN_DIR}\pws_osk.dll"
  File "..\..\help\default\pwsafe.chm"
  File "..\..\LICENSE"
  File "..\..\README.md"
  File "..\..\docs\ReleaseNotes.txt"
  File "..\..\docs\ReleaseNotes.html"
  File "..\..\docs\ChangeLog.txt"
  File "..\..\xml\pwsafe.xsd"
  File "..\..\xml\pwsafe.xsl"
  File "..\..\xml\pwsafe_filter.xsd"
  File "..\..\xml\KPV1_to_PWS.xslt"
  File "..\..\xml\KPV2_to_PWS.xslt"

  ; skip over registry writes if 'Green' installation selected
  IntCmp $INSTALL_TYPE 1 GreenInstall

  ; Store installation folder
  WriteRegStr HKCU "Software\Password Safe\Password Safe" "installdir" $INSTDIR

  ; Store the version
  WriteRegStr HKCU "Software\Password Safe\Password Safe" "installversion" "${VERSION}"
  
  ; and the language
  WriteRegStr HKCU "Software\Password Safe\Password Safe" "Language" "$LANGUAGE"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Add the uninstaller to the Add/Remove Programs window.  If the 
  ; current user doesn't have permission to write to HKLM, then the
  ; uninstaller will not appear in the Add or Remove Programs window.
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
        "DisplayName" "Password Safe ${TARGET_ARCH}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
        "DisplayIcon" "$INSTDIR\pwsafe.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
        "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
        "Publisher" "Rony Shapiro"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
         "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
    "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
    "NoRepair" 1
GreenInstall:
SectionEnd

;--------------------------------
; Section per-supported language
SectionGroup /e "$(LANGUAGE_SUPPORT)" LanguageSupport
Section  "$(ENGLISH_SUPPORT)" EnglishSection
  SectionIn RO ; mandatory
  SetOutPath "$INSTDIR"  
  File "..\..\help\default\pwsafe.chm"
SectionEnd
Section /o "$(CHINESE_CN_SUPPORT)" ChineseSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeZH.dll"
  File /nonfatal "..\..\help\pwsafeZH\pwsafeZH.chm"
SectionEnd
Section /o "$(CHINESE_TW_SUPPORT)" ChineseTWSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeZH_TW.dll"
  File /nonfatal "..\..\help\pwsafeZH\pwsafeZH_TW.chm"
SectionEnd
Section /o "$(GERMAN_SUPPORT)" GermanSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeDE.dll"
  File /nonfatal "..\..\help\pwsafeDE\pwsafeDE.chm"
SectionEnd
Section /o "$(SPANISH_SUPPORT)" SpanishSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeES.dll"
  File /nonfatal "..\..\help\pwsafeES\pwsafeES.chm"
SectionEnd
Section /o "$(SWEDISH_SUPPORT)" SwedishSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeSV.dll"
  File /nonfatal "..\..\help\pwsafeSV\pwsafeSV.chm"
SectionEnd
Section /o "$(DUTCH_SUPPORT)" DutchSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeNL.dll"
  File /nonfatal "..\..\help\pwsafeNL\pwsafeNL.chm"
SectionEnd
Section /o "$(FRENCH_SUPPORT)" FrenchSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeFR.dll"
  File /nonfatal "..\..\help\pwsafeFR\pwsafeFR.chm"
SectionEnd
Section /o "$(RUSSIAN_SUPPORT)" RussianSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeRU.dll"
  File /nonfatal "..\..\help\pwsafeRU\pwsafeRU.chm"
SectionEnd
Section /o "$(POLISH_SUPPORT)" PolishSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafePL.dll"
  File /nonfatal "..\..\help\pwsafePL\pwsafePL.chm"
SectionEnd
Section /o "$(ITALIAN_SUPPORT)" ItalianSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeIT.dll"
  File /nonfatal "..\..\help\pwsafeIT\pwsafeIT.chm"
SectionEnd
Section /o "$(DANISH_SUPPORT)" DanishSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeDA.dll"
  File /nonfatal "..\..\help\pwsafeDA\pwsafeDA.chm"
SectionEnd
Section /o "$(KOREAN_SUPPORT)" KoreanSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeKO.dll"
  File /nonfatal "..\..\help\pwsafeKO\pwsafeKO.chm"
SectionEnd
Section /o "$(CZECH_SUPPORT)" CzechSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeCZ.dll"
  File /nonfatal "..\..\help\pwsafeCZ\pwsafeCZ.chm"
SectionEnd
Section /o "$(TURKISH_SUPPORT)" TurkishSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeTR.dll"
  File /nonfatal "..\..\help\pwsafeTR\pwsafeTR.chm"
SectionEnd
Section /o "$(HUNGARIAN_SUPPORT)" HungarianSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeHU.dll"
  File /nonfatal "..\..\help\pwsafeHU\pwsafeHU.chm"
SectionEnd
Section /o "$(SLOVENIAN_SUPPORT)" SlovenianSection
  SetOutPath "$INSTDIR"  
  File /nonfatal "${LANG_DLL}\pwsafeSL.dll"
  File /nonfatal "..\..\help\pwsafeSL\pwsafeSL.chm"
SectionEnd
SectionGroupEnd

;--------------------------------
; Start with Windows
; May be deselected by .onInit
Section "$(START_AUTO)" StartUp
  CreateShortCut "$SMSTARTUP\Password Safe.lnk" "$INSTDIR\pwsafe.exe" "-s"
SectionEnd

;--------------------------------
; Start menu

Section "$(START_SHOW)" StartMenu

  ; Create the Password Safe menu under the programs part of the start menu
  CreateDirectory "$SMPROGRAMS\Password Safe"

  ; Create shortcuts
  CreateShortCut "$SMPROGRAMS\Password Safe\Password Safe.lnk" "$INSTDIR\pwsafe.exe"
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(ENGLISH_SUPPORT)).lnk" "$INSTDIR\pwsafe.chm"

  ; Shortcuts for help in other languages
  SectionGetFlags ${GermanSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(GERMAN_SUPPORT)).lnk" "$INSTDIR\pwsafeDE.chm"
  SectionGetFlags ${ChineseSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(CHINESE_CN_SUPPORT)).lnk" "$INSTDIR\pwsafeZH.chm"
;  SectionGetFlags ${ChineseTWSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(CHINESE_TW_SUPPORT)).lnk" "$INSTDIR\pwsafeZH_TW.chm"
  SectionGetFlags ${SpanishSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(SPANISH_SUPPORT)).lnk" "$INSTDIR\pwsafeES.chm"
;  SectionGetFlags ${SwedishSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(SWEDISH_SUPPORT)).lnk" "$INSTDIR\pwsafeSV.chm"
;  SectionGetFlags ${DutchSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(DUTCH_SUPPORT)).lnk" "$INSTDIR\pwsafeNL.chm"
  SectionGetFlags ${FrenchSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(FRENCH_SUPPORT)).lnk" "$INSTDIR\pwsafeFR.chm"
  SectionGetFlags ${RussianSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(RUSSIAN_SUPPORT)).lnk" "$INSTDIR\pwsafeRU.chm"
  SectionGetFlags ${PolishSection} $0
  IntCmp $0 ${SF_SELECTED} 0 +2 +2
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(POLISH_SUPPORT)).lnk" "$INSTDIR\pwsafePL.chm"
;  SectionGetFlags ${ItalianSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(ITALIAN_SUPPORT)).lnk" "$INSTDIR\pwsafeIT.chm"
;  SectionGetFlags ${DanishSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(DANISH_SUPPORT)).lnk" "$INSTDIR\pwsafeDA.chm"
;  SectionGetFlags ${KoreanSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(KOREAN_SUPPORT)).lnk" "$INSTDIR\pwsafeKO.chm"
;  SectionGetFlags ${CzechSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(CZECH_SUPPORT)).lnk" "$INSTDIR\pwsafeCZ.chm"
;  SectionGetFlags ${TurkishSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(TURKISH_SUPPORT)).lnk" "$INSTDIR\pwsafeTR.chm"
;  SectionGetFlags ${HungarianSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(HUNGARIAN_SUPPORT)).lnk" "$INSTDIR\pwsafeHU.chm"
;  SectionGetFlags ${SlovenianSection} $0
;  IntCmp $0 ${SF_SELECTED} 0 +2 +2
;  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Help) ($(SLOVENIAN_SUPPORT)).lnk" "$INSTDIR\pwsafeSL.chm"
SectionEnd

;--------------------------------
; PasswordSafe Uninstall

Section "$(UNINSTALL_SHORTCUT)" UninstallMenu

  ; Create the Password Safe menu under the programs part of the start menu
  ; should be already available (START_SHOW)
  CreateDirectory "$SMPROGRAMS\Password Safe"

  ; Create Uninstall icon
  CreateShortCut "$SMPROGRAMS\Password Safe\$(Icon_description_Uninstall).lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
; Desktop shortcut

Section "$(START_SHORTCUT)" DesktopShortcut

  ; Create desktop icon
  CreateShortCut "$DESKTOP\Password Safe.lnk" "$INSTDIR\pwsafe.exe"

SectionEnd

;--------------------------------
; Descriptions
  
  ; Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${ProgramFiles} $(DESC_ProgramFiles)
    !insertmacro MUI_DESCRIPTION_TEXT ${StartUp} $(DESC_StartUp)
    !insertmacro MUI_DESCRIPTION_TEXT ${StartMenu} $(DESC_StartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${UninstallMenu} $(DESC_UninstallMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${DesktopShortcut} $(DESC_DesktopShortcut)
    !insertmacro MUI_DESCRIPTION_TEXT ${LanguageSupport} $(DESC_LangSupport)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"

; Now protect against running instance of pwsafe
	${nsProcess::FindProcess} "pwsafe.exe" $R0
	StrCmp $R0 0 0 +3
	MessageBox MB_OK $(RUNNING_APPLICATION)
  Abort
	${nsProcess::Unload}
  ; Always delete uninstaller first
  Delete "$INSTDIR\Uninstall.exe"
  ; Delete all installed files in the directory
  Delete "$INSTDIR\pwsafe.exe"
  Delete "$INSTDIR\pws_at.dll"
  Delete "$INSTDIR\pws_osk.dll"
  Delete "$INSTDIR\pwsafe.chm"
  Delete "$INSTDIR\pwsafe.chw"
  Delete "$INSTDIR\pwsafe.xsd"
  Delete "$INSTDIR\pwsafe.xsl"
  Delete "$INSTDIR\pwsafe_filter.xsd"
  Delete "$INSTDIR\KPV1_to_PWS.xslt"
  Delete "$INSTDIR\KPV2_to_PWS.xslt"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\ReleaseNotes.txt"
  Delete "$INSTDIR\ReleaseNotes.html"
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\pwsafeCZ.chm"
  Delete "$INSTDIR\pwsafeCZ.chw"
  Delete "$INSTDIR\pwsafeCZ.dll"
  Delete "$INSTDIR\pwsafeDA.chm"
  Delete "$INSTDIR\pwsafeDA.chw"
  Delete "$INSTDIR\pwsafeDA.dll"
  Delete "$INSTDIR\pwsafeDE.chm"
  Delete "$INSTDIR\pwsafeDE.chw"
  Delete "$INSTDIR\pwsafeDE.dll"
  Delete "$INSTDIR\pwsafeES.chm"
  Delete "$INSTDIR\pwsafeES.chw"
  Delete "$INSTDIR\pwsafeES.dll"
  Delete "$INSTDIR\pwsafeFR.chm"
  Delete "$INSTDIR\pwsafeFR.chw"
  Delete "$INSTDIR\pwsafeFR.dll"
  Delete "$INSTDIR\pwsafeHU.chm"
  Delete "$INSTDIR\pwsafeHU.chw"
  Delete "$INSTDIR\pwsafeHU.dll"
  Delete "$INSTDIR\pwsafeIT.chm"
  Delete "$INSTDIR\pwsafeIT.chw"
  Delete "$INSTDIR\pwsafeIT.dll"
  Delete "$INSTDIR\pwsafeKO.chm"
  Delete "$INSTDIR\pwsafeKO.chw"
  Delete "$INSTDIR\pwsafeKO.dll"
  Delete "$INSTDIR\pwsafeNL.chm"
  Delete "$INSTDIR\pwsafeNL.chw"
  Delete "$INSTDIR\pwsafeNL.dll"
  Delete "$INSTDIR\pwsafePL.chm"
  Delete "$INSTDIR\pwsafePL.chw"
  Delete "$INSTDIR\pwsafePL.dll"
  Delete "$INSTDIR\pwsafeRU.chm"
  Delete "$INSTDIR\pwsafeRU.chw"
  Delete "$INSTDIR\pwsafeRU.dll"
  Delete "$INSTDIR\pwsafeSL.chm"
  Delete "$INSTDIR\pwsafeSL.chw"
  Delete "$INSTDIR\pwsafeSL.dll"
  Delete "$INSTDIR\pwsafeSV.chm"
  Delete "$INSTDIR\pwsafeSV.chw"
  Delete "$INSTDIR\pwsafeSV.dll"
  Delete "$INSTDIR\pwsafeTR.chm"
  Delete "$INSTDIR\pwsafeTR.chw"
  Delete "$INSTDIR\pwsafeTR.dll"
  Delete "$INSTDIR\pwsafeZH.chm"
  Delete "$INSTDIR\pwsafeZH.chw"
  Delete "$INSTDIR\pwsafeZH.dll"

  ; remove directory if it's empty
  RMDir  "$INSTDIR"

  ; Delete the registry key for Password Safe
  DeleteRegKey HKCU "Software\Password Safe\Password Safe"

  ; Delete the registry key for the Add or Remove Programs window.  If
  ; the current user doesn't have permission to delete registry keys
  ; from HKLM, then the entry in the Add or Remove Programs window will
  ; remain.  The next time a user tries to click on the uninstaller,
  ; they will be prompted to remove the entry.
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe"
  ; Delete shortcuts, if created
  Delete "$SMPROGRAMS\Password Safe\Password Safe.lnk"
  Delete "$SMPROGRAMS\Password Safe\$(Icon_description_Help).lnk"
  RMDir /r "$SMPROGRAMS\Password Safe"
  Delete "$DESKTOP\Password Safe.lnk"
  Delete "$SMSTARTUP\Password Safe.lnk"

SectionEnd

;-----------------------------------------
; Functions
Function .onInit

 ;Extract InstallOptions INI files
 !insertmacro INSTALLOPTIONS_EXTRACT "pws-install.ini"

 ${If} ${IsWin95}
  MessageBox MB_OK|MB_ICONSTOP $(SORRY_NO_95)
  Quit
 ${EndIf}
 ${If} ${IsWin98}
  MessageBox MB_OK|MB_ICONSTOP $(SORRY_NO_98)
  Quit
 ${EndIf}
 ${If} ${IsWinME}
  MessageBox MB_OK|MB_ICONSTOP $(SORRY_NO_ME)
  Quit
 ${EndIf}
 ${If} ${IsWin2000}
  MessageBox MB_OK|MB_ICONSTOP $(SORRY_NO_2K)
  Quit
 ${EndIf}

; Following tests should really be done
; after .onInit, since language is initialized
; then. However, there's no easy way to do this,
; so these error messages will only be in English. Sorry. 
; Protect against multiple instances
; of the installer

 System::Call 'kernel32::CreateMutexA(i 0, i 0, t "pwsInstallMutex") i .r1 ?e'
 Pop $R0 
 StrCmp $R0 0 +3
   MessageBox MB_OK|MB_ICONEXCLAMATION $(RUNNING_INSTALL)
   Abort

; Now protect against running instance of pwsafe
	${nsProcess::FindProcess} "pwsafe.exe" $R0
	StrCmp $R0 0 0 +3
	MessageBox MB_OK $(RUNNING_APPLICATION)
  Abort
	${nsProcess::Unload}

  ;Language selection dialog
  ; (1) Installation language

 
  Push ""
  Push ${LANG_ENGLISH}
  Push English
  Push ${LANG_GERMAN}
  Push Deutsch
  Push ${LANG_SIMPCHINESE}
  Push "Chinese (Simplified)"
  Push ${LANG_TRADCHINESE}
  Push "Chinese (Traditional)"
  Push ${LANG_SPANISH}
;  Push Español - encoding problem - displays as gibberish
  Push Espanol
  Push ${LANG_SWEDISH}
  Push Svenska
  Push ${LANG_DUTCH}
  Push Dutch
  Push ${LANG_FRENCH}
;  Push Français - encoding problem - displays as gibberish
  Push Francais
  Push ${LANG_RUSSIAN}
  Push Russian
  Push ${LANG_POLISH}
  Push Polski
  Push ${LANG_ITALIAN}
  Push Italiano
  Push ${LANG_DANISH}
  Push Dansk
  Push ${LANG_KOREAN}
  Push Korean
  Push ${LANG_PORTUGUESEBR}
;  Push "Português (Brasil)" - encoding problem - displays as gibberish
  Push "Portuguese (Brasil)"
  Push ${LANG_CZECH}
;  Push "Čeština" - encoding problem - displays as gibberish
  Push "Czech"
  Push ${LANG_TURKISH}
;  Push "Türkçe" - encoding problem - displays as gibberish
  Push "Turkish"
  Push ${LANG_HUNGARIAN}
  Push "Magyar"
  Push ${LANG_SLOVENIAN}
  Push "Slovenian"
  Push A ; A means auto count languages
         ; for the auto count to work the first empty push (Push "") must remain
  LangDLL::LangDialog $(LANG_INSTALL) $(LANG_SELECT)

  Pop $LANGUAGE
  StrCmp $LANGUAGE "cancel" 0 +2
  Abort
  ; autoselect language for selected installer language
  StrCmp $LANGUAGE  ${LANG_GERMAN} 0 +2
  SectionSetFlags ${GermanSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +2
  SectionSetFlags ${ChineseSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_TRADCHINESE} 0 +2
  SectionSetFlags ${ChineseTWSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_SPANISH} 0 +2
  SectionSetFlags ${SpanishSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_SWEDISH} 0 +2
  SectionSetFlags ${SwedishSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_DUTCH} 0 +2
  SectionSetFlags ${DutchSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_FRENCH} 0 +2
  SectionSetFlags ${FrenchSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
  SectionSetFlags ${RussianSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_POLISH} 0 +2
  SectionSetFlags ${PolishSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_ITALIAN} 0 +2
  SectionSetFlags ${ItalianSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_DANISH} 0 +2
  SectionSetFlags ${DanishSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_KOREAN} 0 +2
  SectionSetFlags ${KoreanSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_CZECH} 0 +2
  SectionSetFlags ${CzechSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_TURKISH} 0 +2
  SectionSetFlags ${TurkishSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_HUNGARIAN} 0 +2
  SectionSetFlags ${HungarianSection} ${SF_SELECTED}
  StrCmp $LANGUAGE ${LANG_SLOVENIAN} 0 +2
  SectionSetFlags ${SlovenianSection} ${SF_SELECTED}
  ;
  ; Check if this is an upgrade or not. If so, default "startup" to
  ; disabled, so as not to create unwanted shortcut
  IfFileExists "$INSTDIR\pwsafe.exe" 0 NewInstall
    SectionSetFlags ${StartUp} 0
NewInstall:

  
FunctionEnd

Function GreenOrRegular
  !insertmacro MUI_HEADER_TEXT "$(TEXT_GC_TITLE)" "$(TEXT_GC_SUBTITLE)"
  ; english is in "pws-install.ini" by default, so no writing necessary
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Settings" "Title" $(RESERVE_TITLE)
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Field 1" "Text" $(RESERVE_FIELD1)
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Field 2" "Text" $(RESERVE_FIELD2)
  !insertmacro INSTALLOPTIONS_DISPLAY "pws-install.ini"
FunctionEnd
