@echo off
::
:: Create an EXE installer using NSIS
::
:: Syntax
::   BuildNSISInstall [platform]
::
::   where
::     platform = [x86 | x64]
::

:: validate platform
if $%1 == $-h goto :help
if $%1 == $--help goto :help

:: Add NSIS tool (makensis) to path. The location may vary depending on 
:: the version installed.
setlocal
set PATH="%ProgramFiles(x86)%\NSIS";%PATH%

:: Read version.mfc to get the version number
:: Set environment variables with major, minor, rev values
FOR /F "eol=# tokens=1,2,3* " %%i IN (..\..\version.mfc) DO set PWS_%%i=%%k
set "version_mfc=%PWS_VER_MAJOR%.%PWS_VER_MINOR%.%PWS_VER_REV%"

:: No platform means build all
if $%1$ == $$ goto :buildx86
if $%1 == $x86 goto :buildx86
if $%1 == $x64 goto :buildx64

::
:: Help
::
:help
  echo. 
  echo Syntax: BuildNSISInstaller [platform]
  echo   platform = [x86 ^| x64]
  echo Build all platforms: BuildNSISInstall
  echo Build only x86: BuildNSISInstall x86
  echo Build only x64: BuildNSISInstall x64
  echo. 
  goto :exit
  
::
:: Build the MSI installer
::
:buildx86

echo.
echo Building pwsafe%version_mfc%.exe
echo.

makensis /DVERSION=%version_mfc% /DARCH=x86 pwsafe.nsi
if $%1 == $x86 goto :exit

:buildx64

echo.
echo Building pwsafe64-%version_mfc%.exe
echo.

makensis /DVERSION=%version_mfc% /DARCH=x64 pwsafe.nsi
goto :exit

:exit
endlocal
