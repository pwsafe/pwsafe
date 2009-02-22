;======================================================================================================
;
; Password Safe Installation Script
;
; Copyright 2004, David Lacy Kusters (dkusters@yahoo.com)
; Copyright 2005-2007 Rony Shapiro <ronys@users.sourceforge.net>
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
; 
; the script is setup for 2 languages now (English + German)
; It's prepared for using Swedish and Spanish. Remove the comments ";-L-"
; supplementary languages can be easily brought in
; following "peaces" have to be provided
; - all language specific "Langstring", done in file "pwsafe.lng"
; - several MUI_LANGUAGE
; - several "File" for the language specific DLL
; - "Delete ...DLL" for each language (at install time)
; - 'Delete "$INSTDIR\pwsafeDE_DE.dll"'  for each language (at uninstall time)
; - "Push" in the "Language selection dialog"

;-----------------------------------------
; Set verbosity appropriate for a Makefile
!verbose 2

;--------------------------------
; Include Modern UI
  !include "MUI2.nsh"
  !include "InstallOptions.nsh"
 
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
  Var HOST_OS

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_LICENSE "..\LICENSE" ;$(myLicenseData)
  ; ask about installation type, "green" or "regular"
  Page custom GreenOrRegular
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; General

  ; Name and file
  Name "Password Safe ${VERSION}"

  OutFile "pwsafe-${VERSION}.exe"

  ; Default installation folder
  InstallDir "$PROGRAMFILES\Password Safe"
  
  ; Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Password Safe\Password Safe" "installdir"

;--------------------------------
; Languages
; to enable a language : remove the ";" in front, to disable: put a ";" in front

!define LANGUAGE_GERMAN
!define LANGUAGE_CHINESE
!define LANGUAGE_SPANISH
!define LANGUAGE_SWEDISH
;!define LANGUAGE_DUTCH
;!define LANGUAGE_FRENCH
;!define LANGUAGE_RUSSIAN
!define LANGUAGE_POLISH
;!define LANGUAGE_ITALIAN

  !insertmacro MUI_LANGUAGE "English"
!ifdef LANGUAGE_GERMAN
  !insertmacro MUI_LANGUAGE "German"
!endif
!ifdef LANGUAGE_CHINESE
  !insertmacro MUI_LANGUAGE "SimpChinese"
!endif
!ifdef LANGUAGE_SPANISH
  !insertmacro MUI_LANGUAGE "Spanish"
!endif
!ifdef LANGUAGE_SWEDISH
  !insertmacro MUI_LANGUAGE "Swedish"
!endif
!ifdef LANGUAGE_DUTCH
  !insertmacro MUI_LANGUAGE "Dutch"
!endif
!ifdef LANGUAGE_FRENCH
  !insertmacro MUI_LANGUAGE "French"
!endif
!ifdef LANGUAGE_RUSSIAN
  !insertmacro MUI_LANGUAGE "Russian"
!endif
!ifdef LANGUAGE_POLISH
  !insertmacro MUI_LANGUAGE "Polish"
!endif
!ifdef LANGUAGE_ITALIAN
  !insertmacro MUI_LANGUAGE "Italian"
!endif

  !include "pwsafe.lng"

;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Reserve Files

  ; NSIS documentation states that it's a Good Idea to put the following
  ; two lines when using a custom dialog:  
  ReserveFile "pws-install.ini"
  ;!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS  : VdG for MUI-2 :  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS is no longer supported as InstallOptions

;--------------------------------
; The program itself

Section "$(PROGRAM_FILES)" ProgramFiles
  ;Read the chosen installation type: 1 means "Green", 0 - "Regular"
  !insertmacro INSTALLOPTIONS_READ $INSTALL_TYPE "pws-install.ini" "Field 2" "State"

  ; Make the program files mandatory
  SectionIn RO

  ; Set the directory to install to
  SetOutPath "$INSTDIR"
  
  ; Get all of the files.  This list should be modified when additional
  ; files are added to the release.
  File "..\src\bin\releasem\pwsafe.exe"
;"no Win98"  File /oname=p98.exe "..\src\bin\nu-releasem\pwsafe.exe" 
  File "..\help\default\pwsafe.chm"
  File "..\LICENSE"
  File "..\README.TXT"
  File "..\docs\ReleaseNotes.txt"
  File "..\docs\ChangeLog.txt"
  File "..\xml\pwsafe.xsd"
  File "..\xml\pwsafe.xsl"
  File "..\xml\pwsafe_filter.xsd"
  
!ifdef LANGUAGE_CHINESE
  File /nonfatal "..\src\bin\release\pwsafeZH_CN.dll"
  File /nonfatal "..\help\pwsafeDE\pwsafeZH_CN.chm"
!endif
!ifdef LANGUAGE_GERMAN
  File /nonfatal "..\src\bin\release\pwsafeDE_DE.dll"
  File /nonfatal "..\help\pwsafeDE\pwsafeDE_DE.chm"
!endif
!ifdef LANGUAGE_SPANISH
  File /nonfatal "..\src\bin\release\pwsafeES_ES.dll"
  File /nonfatal "..\help\pwsafeES\pwsafeES_ES.chm"
!endif
!ifdef LANGUAGE_SWEDISH
  File /nonfatal "..\src\bin\release\pwsafeSV_SE.dll"
  File /nonfatal "..\help\pwsafeSV\pwsafeSV_SE.chm"
!endif
!ifdef LANGUAGE_DUTCH
  File /nonfatal "..\src\bin\release\pwsafeNL_NL.dll"
  File /nonfatal "..\help\pwsafeNL\pwsafeNL_NL.chm"
!endif
!ifdef LANGUAGE_FRENCH
  File /nonfatal "..\src\bin\release\pwsafeFR_FR.dll"
  File /nonfatal "..\help\pwsafeFR\pwsafeFR_FR.chm"
!endif
!ifdef LANGUAGE_RUSSIAN
  File /nonfatal "..\src\bin\release\pwsafeRU_RU.dll"
  File /nonfatal "..\help\pwsafeRU\pwsafeRU_RU.chm"
!endif
!ifdef LANGUAGE_POLISH
  File /nonfatal "..\src\bin\release\pwsafePL_PL.dll"
  File /nonfatal "..\help\pwsafePL\pwsafePL_PL.chm"
!endif
!ifdef LANGUAGE_ITALIAN
  File /nonfatal "..\src\bin\release\pwsafeIT_IT.dll"
  File /nonfatal "..\help\pwsafePL\pwsafeIT_IT.chm"
!endif

  Goto dont_install_Win98
  ; If installing under Windows98, delete pwsafe.exe, rename
  ; p98.exe pwsafe.exe
  ; Otherwise, delete p98.exe
  StrCmp $HOST_OS '98' is_98 isnt_98
  is_98:
    Delete $INSTDIR\pwsafe.exe
    Rename $INSTDIR\p98.exe $INSTDIR\pwsafe.exe
    Goto lbl_cont
  isnt_98:
    Delete $INSTDIR\p98.exe
  lbl_cont:
dont_install_Win98:

!ifdef LANGUAGE_CHINESE
  IntCmp $LANGUAGE 2052 languageChinese
!endif
!ifdef LANGUAGE_SPANISH
  IntCmp $LANGUAGE 1034 languageSpanish
!endif
!ifdef LANGUAGE_GERMAN
  IntCmp $LANGUAGE 1031 languageGerman
!endif
!ifdef LANGUAGE_SWEDISH
  IntCmp $LANGUAGE 1053 languageSwedish
!endif
!ifdef LANGUAGE_DUTCH
  IntCmp $LANGUAGE 1043 languageDutch
!endif
!ifdef LANGUAGE_FRENCH
  IntCmp $LANGUAGE 1036 languageFrench
!endif
!ifdef LANGUAGE_RUSSIAN
  IntCmp $LANGUAGE 1049 languageRussian
!endif
!ifdef LANGUAGE_POLISH
  IntCmp $LANGUAGE 1045 languagePolish
!endif
!ifdef LANGUAGE_ITALIAN
  IntCmp $LANGUAGE 1040 languageItalian
!endif
  ; if language = english or "other" : remove all languageXX_XX.DLL
  ; else : English or no specific language
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_GERMAN
languageGerman:
!ifdef LANGUAGE_GERMAN
;    Delete $INSTDIR\pwsafeDE_DE.dll
;    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_CHINESE
languageCHINESE:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
;    Delete $INSTDIR\pwsafeZH_CN.dll
;    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_SPANISH
languageSpanish:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
;    Delete $INSTDIR\pwsafeES_ES.dll
;    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_SWEDISH
languageSwedish:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
;    Delete $INSTDIR\pwsafeSV_SE.dll
;    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_DUTCH
languageDutch:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
!endif
!ifdef LANGUAGE_DUTCH
;    Delete $INSTDIR\pwsafeNL_NL.dll
;    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_FRENCH
languageFrench:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
;    Delete $INSTDIR\pwsafeFR_FR.dll
;    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_RUSSIAN
languageRussian:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
;    Delete $INSTDIR\pwsafeRU_RU.dll
;    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_POLISH
languagePolish:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
;    Delete $INSTDIR\pwsafePL_PL.dll
;    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
    Delete $INSTDIR\pwsafeIT_IT.dll
    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
!ifdef LANGUAGE_ITALIAN
languageItalian:
!ifdef LANGUAGE_GERMAN
    Delete $INSTDIR\pwsafeDE_DE.dll
    Delete $INSTDIR\pwsafeDE_DE.chm
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
    Delete $INSTDIR\pwsafeES_ES.dll
    Delete $INSTDIR\pwsafeES_ES.chm
!endif
!ifdef LANGUAGE_SWEDISH
    Delete $INSTDIR\pwsafeSV_SE.dll
    Delete $INSTDIR\pwsafeSV_SE.chm
!endif
!ifdef LANGUAGE_DUTCH
    Delete $INSTDIR\pwsafeNL_NL.dll
    Delete $INSTDIR\pwsafeNL_NL.chm
!endif
!ifdef LANGUAGE_FRENCH
    Delete $INSTDIR\pwsafeFR_FR.dll
    Delete $INSTDIR\pwsafeFR_FR.chm
!endif
!ifdef LANGUAGE_RUSSIAN
    Delete $INSTDIR\pwsafeRU_RU.dll
    Delete $INSTDIR\pwsafeRU_RU.chm
!endif
!ifdef LANGUAGE_POLISH
    Delete $INSTDIR\pwsafePL_PL.dll
    Delete $INSTDIR\pwsafePL_PL.chm
!endif
!ifdef LANGUAGE_ITALIAN
;    Delete $INSTDIR\pwsafeIT_IT.dll
;    Delete $INSTDIR\pwsafeIT_IT.chm
!endif
    goto languageDone
!endif
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
languageDone:

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
        "DisplayName" "Password Safe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
         "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
    "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Password Safe" \
    "NoRepair" 1
GreenInstall:
SectionEnd

;--------------------------------
; Start with Windows
Section "$(START_AUTO)" StartUp
  CreateShortCut "$SMSTARTUP\Password Safe.lnk" "$INSTDIR\pwsafe.exe" "-s"
SectionEnd

;--------------------------------
; Start menu

Section "$(START_SHOW)" StartMenu

  ; Create the Password Safe menu under the programs part of the start
  ; menu
  CreateDirectory "$SMPROGRAMS\Password Safe"

  ; Create shortcuts
  CreateShortCut "$SMPROGRAMS\Password Safe\Password Safe.lnk" "$INSTDIR\pwsafe.exe"

  CreateShortCut "$SMPROGRAMS\Password Safe\Password Safe Help.lnk" "$INSTDIR\pwsafe.chm"

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
    !insertmacro MUI_DESCRIPTION_TEXT ${DesktopShortcut} $(DESC_DesktopShortcut)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Delete all installed files in the directory
  Delete "$INSTDIR\pwsafe.exe"
  Delete "$INSTDIR\pwsafe.chm"
  Delete "$INSTDIR\pwsafe.xsd"
  Delete "$INSTDIR\pwsafe.xsl"
  Delete "$INSTDIR\pwsafe_filter.xsd"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.TXT"
  Delete "$INSTDIR\ReleaseNotes.txt"
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\mfc80.dll"
  Delete "$INSTDIR\msvcp80.dll"
  Delete "$INSTDIR\msvcr80.dll"
  Delete "$INSTDIR\Microsoft.VC80.CRT.manifest"
  Delete "$INSTDIR\Microsoft.VC80.MFC.manifest"
!ifdef LANGUAGE_GERMAN
  Delete "$INSTDIR\pwsafeDE_DE.dll"
  Delete "$INSTDIR\pwsafeDE_DE.chm"
!endif
!ifdef LANGUAGE_CHINESE
    Delete $INSTDIR\pwsafeZH_CN.dll
    Delete $INSTDIR\pwsafeZH_CN.chm
!endif
!ifdef LANGUAGE_SPANISH
  Delete "$INSTDIR\pwsafeES_ES.dll"
  Delete "$INSTDIR\pwsafeES_ES.chm"
!endif
!ifdef LANGUAGE_SWEDISH
  Delete "$INSTDIR\pwsafeSV_SE.dll"
  Delete "$INSTDIR\pwsafeSV_SE.chm"
!endif
!ifdef LANGUAGE_DUTCH
  Delete "$INSTDIR\pwsafeNL_NL.dll"
  Delete "$INSTDIR\pwsafeNL_NL.chm"
!endif
!ifdef LANGUAGE_FRENCH
  Delete "$INSTDIR\pwsafeFR_FR.dll"
  Delete "$INSTDIR\pwsafeFR_FR.chm"
!endif
!ifdef LANGUAGE_RUSSIAN
  Delete "$INSTDIR\pwsafeRU_RU.dll"
  Delete "$INSTDIR\pwsafeRU_RU.chm"
!endif
!ifdef LANGUAGE_POLISH
  Delete "$INSTDIR\pwsafePL_PL.dll"
  Delete "$INSTDIR\pwsafePL_PL.chm"
!endif
!ifdef LANGUAGE_ITALIAN
  Delete "$INSTDIR\pwsafeIT_IT.dll"
  Delete "$INSTDIR\pwsafeIT_IT.chm"
!endif

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
  RMDir /r "$SMPROGRAMS\Password Safe"
  Delete "$DESKTOP\Password Safe.lnk"

SectionEnd

;-----------------------------------------
; Functions
Function .onInit

 ;Extract InstallOptions INI files
 !insertmacro INSTALLOPTIONS_EXTRACT "pws-install.ini"
 Call GetWindowsVersion
 Pop $R0
; Strcpy $R0 '98' ; ONLY for test
 StrCmp $R0 '95' is_win95
 StrCmp $R0 '98' is_win98
 StrCmp $R0 'ME' is_winME
 StrCpy $HOST_OS $R0
 
  ;Language selection dialog

!ifdef LANGUAGE_GERMAN
  goto extraLanguage
!endif
!ifdef LANGUAGE_CHINESE
  goto extraLanguage
!endif
!ifdef LANGUAGE_SPANISH
  goto extraLanguage
!endif
!ifdef LANGUAGE_SWEDISH
  goto extraLanguage
!endif
!ifdef LANGUAGE_DUTCH
  goto extraLanguage
!endif
!ifdef LANGUAGE_FRENCH
  goto extraLanguage
!endif
!ifdef LANGUAGE_RUSSIAN
  goto extraLanguage
!endif
!ifdef LANGUAGE_POLISH
  goto extraLanguage
!endif
!ifdef LANGUAGE_ITALIAN
  goto extraLanguage
!endif
  goto NOextraLanguage
 
extraLanguage:  
  Push ""
  Push ${LANG_ENGLISH}
  Push English
!ifdef LANGUAGE_GERMAN
  Push ${LANG_GERMAN}
  Push German
!endif
!ifdef LANGUAGE_CHINESE
  Push ${LANG_SIMPCHINESE}
  Push Chinese
!endif
!ifdef LANGUAGE_SPANISH
  Push ${LANG_SPANISH}
  Push Spanish
!endif
!ifdef LANGUAGE_SWEDISH
  Push ${LANG_SWEDISH}
  Push Swedish
!endif
!ifdef LANGUAGE_DUTCH
  Push ${LANG_DUTCH}
  Push Dutch
!endif
!ifdef LANGUAGE_FRENCH
  Push ${LANG_FRENCH}
  Push French
!endif
!ifdef LANGUAGE_RUSSIAN
  Push ${LANG_RUSSIAN}
  Push Russian
!endif
!ifdef LANGUAGE_POLISH
  Push ${LANG_POLISH}
  Push Polish
!endif
!ifdef LANGUAGE_ITALIAN
  Push ${LANG_ITALIAN}
  Push Italian
!endif
  Push A ; A means auto count languages
         ; for the auto count to work the first empty push (Push "") must remain
  LangDLL::LangDialog "Installation Language" "Please select the language for the installation"

  Pop $LANGUAGE
  StrCmp $LANGUAGE "cancel" 0 +2
    Abort
 Return
is_win95:
  MessageBox MB_OK|MB_ICONSTOP "Sorry, Windows 95 is no longer supported. Try PasswordSafe 2.16"
  Quit
is_win98:
  MessageBox MB_OK|MB_ICONSTOP "Sorry, Windows 98 is no longer supported. Try PasswordSafe 2.16"
  Quit
is_winME:
  MessageBox MB_OK|MB_ICONSTOP "Sorry, Windows ME is no longer supported. Try PasswordSafe 2.16"
  Quit
NOextraLanguage:
FunctionEnd

Function GreenOrRegular
  !insertmacro MUI_HEADER_TEXT "$(TEXT_GC_TITLE)" "$(TEXT_GC_SUBTITLE)"
  ; english is in "pws-install.ini" by default, so no writing necesarry
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Settings" "Title" $(PSWINI_TITLE)
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Field 1" "Text" $(PSWINI_TEXT1)
  !insertmacro INSTALLOPTIONS_WRITE "pws-install.ini" "Field 2" "Text" $(PSWINI_TEXT2)
  !insertmacro INSTALLOPTIONS_DISPLAY "pws-install.ini"
FunctionEnd
  
 ;--------------------------------
 ;
 ; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
 ; Updated by Joost Verburg
 ;
 ; Returns on top of stack
 ;
 ; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003, Vista)
 ; or
 ; '' (Unknown Windows Version)
 ;
 ; Usage:
 ;   Call GetWindowsVersion
 ;   Pop $R0
 ;   ; at this point $R0 is "NT 4.0" or whatnot
 
 Function GetWindowsVersion
 
   Push $R0
   Push $R1
 
   ClearErrors
 
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

   IfErrors 0 lbl_winnt
   
   ; we are not NT
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done
 
   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done
 
   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done
 
   lbl_winnt:
 
   StrCpy $R1 $R0 1
 
   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003
   StrCmp $R1 '6.0' lbl_winnt_vista lbl_error
 
   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
   Goto lbl_done
 
   lbl_winnt_2000:
     Strcpy $R0 '2000'
   Goto lbl_done
 
   lbl_winnt_XP:
     Strcpy $R0 'XP'
   Goto lbl_done
 
   lbl_winnt_2003:
     Strcpy $R0 '2003'
   Goto lbl_done
 
   lbl_winnt_vista:
     Strcpy $R0 'Vista'
   Goto lbl_done
 
   lbl_error:
     Strcpy $R0 ''
   lbl_done:
 
   Pop $R1
   Exch $R0
 
 FunctionEnd
