rem  Called via the Project Build Events: Pre-Build Event

rem This section does "Update Revision Number in Resources"
rem Requires environment variables ProjectDir & TortoiseSVNDir
rem  set in UserVariables.vsprops

rem Other processing can be added as required.
rem VdG some echoing of the process being done (helps if it stucks)

: goto weiter
if not exist "%TortoiseSVNDir%\bin\SubWCRev.exe" goto Error1
echo executing: "%TortoiseSVNDir%\bin\SubWCRev.exe" ..\..\.. version.in version.h
"%TortoiseSVNDir%\bin\SubWCRev.exe" ..\..\.. version.in version.h
goto Done
: Error1
if not exist "%ProjectDir%\version.h" goto Error2
: weiter
echo "%ProjectDir%\version.h" does not exist.
echo "Using static version.h"
goto Done
: Error2
echo "Can't find TortoiseSVN's SubWCRev.exe"
echo "Please install it or create version.h from version.in manually"
: Done
echo .
