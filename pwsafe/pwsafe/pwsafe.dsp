# Microsoft Developer Studio Project File - Name="pwsafe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=pwsafe - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pwsafe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pwsafe.mak" CFG="pwsafe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pwsafe - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "pwsafe - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pwsafe - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /MD /W4 /GX /Zi /O2 /I "C:\local\msvc\HTMLHelpWorkshop\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# SUBTRACT CPP /nologo
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 htmlhelp.lib corelib.lib WS2_32.lib rpcrt4.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /libpath:".\corelib\Release" /libpath:"C:\local\msvc\HTMLHelpWorkshop\lib"
# SUBTRACT LINK32 /verbose /map /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "pwsafe - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "C:\local\msvc\HTMLHelpWorkshop\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40d /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 htmlhelp.lib corelib.lib WS2_32.lib Rpcrt4.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:".\corelib\Debug" /libpath:"C:\local\msvc\HTMLHelpWorkshop\lib"
# SUBTRACT LINK32 /verbose

!ENDIF 

# Begin Target

# Name "pwsafe - Win32 Release"
# Name "pwsafe - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ClearQuestionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfirmDeleteDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptKeyEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\DboxMain.cpp
# End Source File
# Begin Source File

SOURCE=.\DboxOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\DboxPassword.cpp

!IF  "$(CFG)" == "pwsafe - Win32 Release"

!ELSEIF  "$(CFG)" == "pwsafe - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DboxView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportText.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\KeySend.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\model.cpp
# End Source File
# Begin Source File

SOURCE=.\MyTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPasswordPolicy.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsSecurity.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsUsername.cpp
# End Source File
# Begin Source File

SOURCE=.\PasskeyChangeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PasskeyEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\PasskeySetup.cpp
# End Source File
# Begin Source File

SOURCE=.\PwFont.cpp
# End Source File
# Begin Source File

SOURCE=.\QueryAddName.cpp
# End Source File
# Begin Source File

SOURCE=.\QuerySetDef.cpp
# End Source File
# Begin Source File

SOURCE=.\RemindSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SysColStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemTray.cpp
# End Source File
# Begin Source File

SOURCE=.\ThisMfcApp.cpp
# End Source File
# Begin Source File

SOURCE=.\TryAgainDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\winview.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddDlg.h
# End Source File
# Begin Source File

SOURCE=.\BlowFish.h
# End Source File
# Begin Source File

SOURCE=.\ClearQuestionDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfirmDeleteDlg.h
# End Source File
# Begin Source File

SOURCE=.\CryptKeyEntry.h
# End Source File
# Begin Source File

SOURCE=.\DboxMain.h
# End Source File
# Begin Source File

SOURCE=.\EditDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportText.h
# End Source File
# Begin Source File

SOURCE=.\FileDialogExt.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\ImportDlg.h
# End Source File
# Begin Source File

SOURCE=.\ItemData.h
# End Source File
# Begin Source File

SOURCE=.\jprdebug.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\MyString.h
# End Source File
# Begin Source File

SOURCE=.\MyTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDisplay.h
# End Source File
# Begin Source File

SOURCE=.\OptionsMisc.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPasswordPolicy.h
# End Source File
# Begin Source File

SOURCE=.\OptionsSecurity.h
# End Source File
# Begin Source File

SOURCE=.\OptionsUsername.h
# End Source File
# Begin Source File

SOURCE=.\PasskeyChangeDlg.h
# End Source File
# Begin Source File

SOURCE=.\PasskeyEntry.h
# End Source File
# Begin Source File

SOURCE=.\PasskeySetup.h
# End Source File
# Begin Source File

SOURCE=.\PasswordDb.h
# End Source File
# Begin Source File

SOURCE=.\PasswordSafe.h
# End Source File
# Begin Source File

SOURCE=.\PWCharPool.h
# End Source File
# Begin Source File

SOURCE=.\PwFont.h
# End Source File
# Begin Source File

SOURCE=.\PwsBackend.h
# End Source File
# Begin Source File

SOURCE=.\QueryAddName.h
# End Source File
# Begin Source File

SOURCE=.\QuerySetDef.h
# End Source File
# Begin Source File

SOURCE=.\RemindSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sha1.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SysColStatic.h
# End Source File
# Begin Source File

SOURCE=.\SystemTray.h
# End Source File
# Begin Source File

SOURCE=.\ThisMfcApp.h
# End Source File
# Begin Source File

SOURCE=.\TryAgainDlg.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\winview.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=".\cpane-bw.ico"
# End Source File
# Begin Source File

SOURCE=.\cpane.bmp
# End Source File
# Begin Source File

SOURCE=.\cpane.ico
# End Source File
# Begin Source File

SOURCE=.\cpane_s.bmp
# End Source File
# Begin Source File

SOURCE=.\cpanetxt.bmp
# End Source File
# Begin Source File

SOURCE=.\ibubble.ico
# End Source File
# Begin Source File

SOURCE=.\leaf.bmp
# End Source File
# Begin Source File

SOURCE=.\node.bmp
# End Source File
# Begin Source File

SOURCE=.\PasswordSafe.rc
# End Source File
# Begin Source File

SOURCE=.\psafetxt.bmp
# End Source File
# Begin Source File

SOURCE=".\toolbar1-greyedout.bmp"
# End Source File
# Begin Source File

SOURCE=.\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=".\toolbar2-greyedout.bmp"
# End Source File
# Begin Source File

SOURCE=.\toolbar2.bmp
# End Source File
# Begin Source File

SOURCE=".\toolbar3-greyedout.bmp"
# End Source File
# Begin Source File

SOURCE=.\toolbar3.bmp
# End Source File
# Begin Source File

SOURCE=.\tray.ico
# End Source File
# Begin Source File

SOURCE=.\w95mbx01.ico
# End Source File
# Begin Source File

SOURCE=.\w95mbx02.ico
# End Source File
# Begin Source File

SOURCE=.\w95mbx03.ico
# End Source File
# Begin Source File

SOURCE=.\w95mbx04.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\PasswordSafe.exe.manifest
# End Source File
# End Target
# End Project
