@echo off
Rem
Rem Delete current POT file and recreate it.
Rem

setlocal
set RESTEXT=tools\bin\ResText\Release\ResText.exe
set DLL=..\bin\language\release\pwsafe_base.dll
set POT=pos\pwsafe.pot

if not exist %RESTEXT% goto no_program
if not exist %DLL% goto no_dll

if exist %POT% del %POT%

%RESTEXT% extract %DLL% %POT%
exit /b 0

:no_program
echo Can't find 'ResText.exe'. Process aborted.
exit /b /8
:no_dll
echo Can't find 'pwsafe_base.dll'. Process aborted.
exit /b 8
