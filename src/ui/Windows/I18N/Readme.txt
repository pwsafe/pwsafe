Language Translations
=====================

Pwsafe uses language translation DLLs to implement I18N. Each
DLL provides a given translation. A language DLL is constructed
from a .po file. The "pos" subdirectory contains the current
set of .po files. Note that not every .po file is processed into
a language DLL. Only those files defined in the Translations.txt
file are turned into language DLLs.

Translations.txt
================
The Translations.txt file defines the language translations 
that are included in the pwsafe installer. A language translation
is defined by a non-comment record with the following format:

  FileName   LCID   LL_CC LL

Where

  FileName - the name of the file containing the translation source.
    A typical file name is something like pwsafe_zh.po. The file must be in 
    the pos directory. thus, it's path must be pos/FileName.
  LCID - the language ID(aka a locale). See https://msdn.microsoft.com/en-us/library/cc233982.aspx
    Expressed as hex digits of the form 0xnnnn. For example, 0x0804 is ZH_CN (or zh-CN), 
    which is Chinese Simplified, Republic of China.
  LL_CC - language tag. For example, ZH_CN.
  LL - 2 character language code. For example, ZH. This really isn't the full language code, but
    it is how the output file is named. In the case of ZH, the output file is pwsafZH.dll.

A comment is any line whose first non-blank character is a # character.
    
How to Add a New Translation
============================
To add a new translation:

  - Create a .po file and place it in the pos directory.
  - Add a new record to the Translations.txt file.

How to Create Language DLLs
===========================
Language DLLs can be produced for both x86 and x64 versions of pwsafe.
To produce all of them:

  - Open a command prompt
  - CD to the src\ui\windows\I18N directory
  - Run these VB scripts
    cscript Create_DLLs.vbs x86
    cscript Create_DLLs.vbs x64
  - To get help on how to use the scripts
    cscript Create_DLLs.vbs

Requirements and Dependencies
=============================
The Create_DLLs.vbs script has several dependencies.

  - ResText.exe (src\Tools\Windows\I18N\ResText\ResText-14.sln)
  - ResPWSL.exe (src\Tools\Windows\I18N\ResPWSL\ResPWSL-14.sln)
  - pwsafe_base.dll (the language project within pwsafe-14.sln)
  
There are 32 and 64-bit versions of all of these. Make sure they 
are built before running Create_DLLs.vbs.

To build the ResText and ResPWSL utilities:

  1. Open the ResText-14.sln file in Visual Studio (e.g. VS 2015 CE)
  2. Select the Release x86 configuration
  3. Select the Build/Build Solution menu item
  4. Select the Release x64 configuration
  5. Select the Build/Build Solution menu item
  6. Close the solution
  7. Open the ResPWSL-14.sln file
  8. Repeat steps 2-6
  
The pwsafe_base.dll is built from the pwsafe-14.sln. It is the
output of the languageDLL project.

  1. Load the pswafe-14.sln file into Visual Studio (e.g. VS 2015 CE).
  2. Select the Release x86 configuration
  3. Right click on the languageDLL project
  4. Select Build
  5. Select the Release x64 configuration
  6. Right click on the languageDLL project
  7. Select Build
  8. Close the solution
  
The languageDLL is part of the Release build configuration, so if
you build the whole solution (Build/Build Solution) you will
get pwsafe_base.dll for the selected platform (Win32/x64).

Testing Language DLLs
=====================

The language DLLs produced by the Create_DLLs.vbs script are
put into the following directories.

  build\bin\pwsafe\I18N (32-bit)
  build\bin\pwsafe\I18N64 (64-bit)

To test the DLLs without going through an installer, simply copy
the DLLs into the directory where pwsafe.exe is located. For
the x86 build use one of the following. Be sure to copy 32-bit
language DLLs (even though either will work).

  build\bin\pwsafe\Debug
  build\bin\pwsafe\Release

For the x64 build use one of these. Be sure to copy 64-bit DLLs
(event though either will work).

  build\bin\pwsafe\Debug64
  build\bin\pwsafe\Release64

To test the languages, run pwsafe and select each of the languages 
to be tested.
