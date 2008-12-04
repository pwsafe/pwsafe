rem  Called via: pre_build.cmd

rem This section does "Update Revision Number in Resources"
rem Requires environment variables ProjectDir & TortoiseSVNDir
rem  set in UserVariables.vsprops

rem Other processing can be added as required.

if not exist "%TortoiseSVNDir%\bin\SubWCRev.exe" goto :Error1
"%TortoiseSVNDir%\bin\SubWCRev.exe" .\ version.in version.h
goto :Done
:Error1
if not exist "%ProjectDir%\version.h" goto :Error2
echo "Using static version.h"
goto :Done
:Error2
echo "Can't find TortoiseSVN's SubWCRev.exe"
echo "Please install it or create version.h from version.in manually"
:Done
echo .
rem  Called via: pre_build.cmd

rem This section does "Update Revision Number in Resources"
rem Requires environment variables ProjectDir & TortoiseSVNDir
rem  set in UserVariables.vsprops

rem Other processing can be added as required.

if not exist "%TortoiseSVNDir%\bin\SubWCRev.exe" goto :Error1
"%TortoiseSVNDir%\bin\SubWCRev.exe" .\ version.in version.h
goto :Done
:Error1
if not exist "%ProjectDir%\version.h" goto :Error2
echo "Using static version.h"
goto :Done
:Error2
echo "Can't find TortoiseSVN's SubWCRev.exe"
echo "Please install it or create version.h from version.in manually"
:Done
echo .
