@echo off
::
:: Create an MSI installer using the Wix Toolset
::
:: Syntax
::   BuildWixInstall [platform]
::
::   where
::     platform = [x86 | x64]
::

:: validate platform
if $%1 == $-h goto :help
if $%1 == $--help goto :help

:: Add Wix toolset to path. The Wix location may vary depending on 
:: the version installed.
setlocal
set PATH="%ProgramFiles(x86)%\WiX Toolset v3.10\bin";%PATH%

:: Read version.mfc to get the version number
:: Set environment variables with major, minor, rev values
FOR /F "eol=# tokens=1,2,3* " %%i IN (version.mfc) DO set PWS_%%i=%%k
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
  echo Syntax: BuildWixMSI [platform]
  echo   platform = [x86 ^| x64]
  echo Build all platforms: BuildWixInstall
  echo Build only x86: BuildWixMSI x86
  echo Build only x54: BuildWixMSI x64
  echo. 
  goto :exit
  
::
:: Build the MSI installer
::
:buildx86

echo.
echo Building pwsafe.msi for version %version_mfc%
echo.

candle -dPWSAFE_VERSION=%version_mfc% -dpwsafe_platform=%2 install\windows\pwsafe-template-x86.wxs
light -ext WixUIExtension -cultures:en-us pwsafe-template-x86.wixobj -out pwsafe.msi
if $%1 == $x86 goto :exit

:buildx64

echo.
echo Building pwsafe64.msi for version %version_mfc%
echo.

candle -dPWSAFE_VERSION=%version_mfc% -dpwsafe_platform=%2 -arch x64 install\windows\pwsafe-template-x64.wxs
light -ext WixUIExtension -cultures:en-us pwsafe-template-x64.wixobj -out pwsafe64.msi
goto :exit

:exit
endlocal
