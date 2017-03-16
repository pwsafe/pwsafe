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

:: No platform means build both x86 and x64
if $%1$ == $$ goto :buildx86
if $%1 == $x86 goto :buildx86
if $%1 == $x64 goto :buildx64

::
:: Help
::
:help
  echo. 
  echo Syntax: BuildWixInstall [platform]
  echo   platform = [x86 ^| x64]
  echo Build all platforms: BuildWixInstall
  echo Build only x86: BuildWixInstall x86
  echo Build only x54: BuildWixInstall x64
  echo. 
  goto :exit
  
::
:: Build the MSI installer
::
:buildx86

echo.
echo ***Building pwsafe.msi for version %version_mfc%
echo.

candle -dPWSAFE_VERSION=%version_mfc% -dPlatform=x86 install\windows\pwsafe.wxs
light -ext WixUIExtension -cultures:en-us pwsafe.wixobj -out pwsafe.msi
if $%1 == $x86 goto :exit

:buildx64

echo.
echo ***Building pwsafe64.msi for version %version_mfc%
echo.

candle -dPWSAFE_VERSION=%version_mfc% -dPlatform=x64 -arch x64 install\windows\pwsafe.wxs
light -ext WixUIExtension -cultures:en-us pwsafe.wixobj -out pwsafe64.msi
goto :exit

:exit
endlocal
