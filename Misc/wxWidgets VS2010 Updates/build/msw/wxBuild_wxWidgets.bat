@echo off
::**************************************************************************
:: File:           wxBuild_wxWidgets.bat
:: Version:        1.12
:: Name:           RJP Computing - modified for 64-bit VS compilation
:: Date:           09/03/2009
:: Description:    Build wxWidgets with the MinGW/Visual C++.
::                 
::                 	v1.01 - Added Compiler setup for VC7.1 and VC8.0.
::                 	v1.02 - Added INCLUDE variable to VC7.1 and VC8.0 setups.
:: 					v1.03 - Added FLAGS. Use to set extra command line options.
:: 					v1.04 - Added flags for specific wxWidgets build options.
:: 					v1.05 - Added mono static library creation. (Only needed for
::					        building wxWidgets)
::					        Added a better clean method.
::					        Added an app name parameter.
:: 					v1.06 - Fixed that the monolithic static libraries were not
::							built using the static run-time library.
:: 					v1.07 - Added MinGW Gcc 4.x.x compiler.
:: 					v1.08 - Added a move command to help automate building with
::							multiple compilers from the same family.
::							(ie. vc and gcc).
::					v1.09 - Removed 'CXXFLAGS=/Zc:wchar_t-' from VC8.0 setup.
::					v1.10 - Added USE_GDIPLUS=1 to FLAGS for wxGraphicsContext.
::					v1.11 - Added support for VC 9.0
::          V1.12 - Added support for VC 10.0 and x64 builds
::**************************************************************************
SETLOCAL
set WXBUILD_VERSION=1.12
set WXBUILD_APPNAME=wxBuild_wxWidgets
:: MinGW Gcc install location. This must match your systems configuration.
set GCCDIR=C:\MinGW
set GCC4DIR=C:\MinGW4
set CPU=X86

if (%1) == () goto ERROR
:: -- Check if user wants help --
if (%1) == (/?)  goto SHOW_USAGE
if (%1) == (-?)  goto SHOW_USAGE
if (%1) == HELP  goto SHOW_USAGE
if (%1) == help  goto SHOW_USAGE
if (%2) == ()    goto ERROR

:: -- Check which compiler was selected. --
if %1 == VCTK     goto SETUP_VC71_TOOLKIT_BUILD_ENVIRONMENT
if %1 == vctk     goto SETUP_VC71_TOOLKIT_BUILD_ENVIRONMENT
if %1 == VC71     goto SETUP_VC71_BUILD_ENVIRONMENT
if %1 == vc71     goto SETUP_VC71_BUILD_ENVIRONMENT
if %1 == VC80     goto SETUP_VC80_BUILD_ENVIRONMENT
if %1 == vc80     goto SETUP_VC80_BUILD_ENVIRONMENT
if %1 == VC80_64  goto SETUP_VC80_64_BUILD_ENVIRONMENT
if %1 == vc80_64  goto SETUP_VC80_64_BUILD_ENVIRONMENT
if %1 == VC90     goto SETUP_VC90_BUILD_ENVIRONMENT
if %1 == vc90     goto SETUP_VC90_BUILD_ENVIRONMENT
if %1 == VC90_64  goto SETUP_VC90_64_BUILD_ENVIRONMENT
if %1 == vc90_64  goto SETUP_VC90_64_BUILD_ENVIRONMENT
if %1 == VC100    goto SETUP_VC100_BUILD_ENVIRONMENT
if %1 == vc100    goto SETUP_VC100_BUILD_ENVIRONMENT
if %1 == VC100_64 goto SETUP_VC100_64_BUILD_ENVIRONMENT
if %1 == vc100_64 goto SETUP_VC100_64_BUILD_ENVIRONMENT
if %1 == MINGW    goto SETUP_GCC_BUILD_ENVIRONMENT
if %1 == mingw    goto SETUP_GCC_BUILD_ENVIRONMENT
if %1 == MINGW4   goto SETUP_GCC4_BUILD_ENVIRONMENT
if %1 == mingw4   goto SETUP_GCC4_BUILD_ENVIRONMENT
goto COMPILER_ERROR

:SETUP_VC71_TOOLKIT_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS
if %2 == clean goto CLEAN_VS
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: -- Add Visual C++ directories to the systems PATH --
echo Setting environment for Visual C++ 7.1 Toolkit...
set MSVC=C:\Program Files\Microsoft Visual C++ Toolkit 2003
set MSSDK=C:\Program Files\Microsoft Platform SDK
set DOTNETSDK=C:\Program Files\Microsoft Visual Studio .NET 2003\vc7

set PATH=%MSVC%\bin;%MSSDK%\bin;%MSSDK%\bin\win64;%DOTNETSDK%\bin;%PATH%
set INCLUDE=%MSVC%\include;%MSSDK%\include;%DOTNETSDK%\include;%WXWIN%\include;%INCLUDE%
set LIB=%MSVC%\lib;%MSSDK%\lib;%DOTNETSDK%\lib;%LIB%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC71_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS
if %2 == clean goto CLEAN_VS
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the full VC 2003 .net includes.
echo Setting environment for Visual C++ 7.1...
echo.
call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC80_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS
if %2 == clean goto CLEAN_VS
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the full VC 2005 includes.
echo Setting environment for Visual C++ 8.0...
echo.
call "%VS80COMNTOOLS%vsvars32.bat"
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC80_64_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS64
if %2 == clean goto CLEAN_VS64
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the VC 2005 64-bit includes.
echo Setting environment for Visual C++ 8.0 64-bit...
echo.
set CPU=AMD64
set CMD32="%VS80COMNTOOLS%vcvarsall.bat"
set CMD64=%CMD32:\Common7\Tools\=\VC\%
call %CMD64% amd64
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC90_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS
if %2 == clean goto CLEAN_VS
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the VC 2008 includes.
echo Setting environment for Visual C++ 9.0...
echo.
call "%VS90COMNTOOLS%vsvars32.bat"
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC90_64_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS64
if %2 == clean goto CLEAN_VS64
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the VC 2009 64-bit includes.
echo Setting environment for Visual C++ 9.0 64-bit...
echo.
set CPU=AMD64
set CMD32="%VS90COMNTOOLS%vcvarsall.bat"
set CMD64=%CMD32:\Common7\Tools\=\VC\%
call %CMD64% amd64
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC100_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS
if %2 == clean goto CLEAN_VS
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the VC 2010 includes.
echo Setting environment for Visual C++ 10.0...
echo.
call "%VS100COMNTOOLS%vsvars32.bat"
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_VC100_64_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_VS64
if %2 == clean goto CLEAN_VS64
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

:: Add the VC 2010 64-bit includes.
echo Setting environment for Visual C++ 10.0 64-bit...
echo.
set CPU=AMD64
set CMD32="%VS100COMNTOOLS%vcvarsall.bat"
set CMD64=%CMD32:\Common7\Tools\=\VC\%
call %CMD64% amd64
set INCLUDE=%WXWIN%\include;%INCLUDE%
:: -- Setup the make executable and the actual makefile name --
set MAKE=nmake
set MAKEFILE=makefile.vc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=1
goto START

:SETUP_GCC_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_MINGW
if %2 == clean goto CLEAN_MINGW
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

echo Assuming that MinGW has been installed to:
echo   %GCCDIR%
echo.
:: -- Add MinGW directory to the systems PATH --
echo Setting environment for MinGW Gcc...
if "%OS%" == "Windows_NT" set PATH=%GCCDIR%\BIN;%PATH%
if "%OS%" == "" set PATH="%GCCDIR%\BIN";"%PATH%"
echo.
:: -- Setup the make executable and the actual makefile name --
set MAKE=mingw32-make.exe
set MAKEFILE=makefile.gcc
set FLAGS=USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=0 -j %NUMBER_OF_PROCESSORS%
goto START

:SETUP_GCC4_BUILD_ENVIRONMENT
:: If cleaning or moving - no need to set up environment
if %2 == CLEAN goto CLEAN_MINGW
if %2 == clean goto CLEAN_MINGW
if %2 == MOVE  goto MOVE
if %2 == move  goto MOVE

echo Assuming that MinGW has been installed to:
echo   %GCC4DIR%
echo.
:: -- Add MinGW directory to the systems PATH --
echo Setting environment for MinGW Gcc...
if "%OS%" == "Windows_NT" set PATH=%GCC4DIR%\BIN;%PATH%
if "%OS%" == "" set PATH="%GCC4DIR%\BIN";"%PATH%"
echo.
:: -- Setup the make executable and the actual makefile name --
set MAKE=mingw32-make.exe
set MAKEFILE=makefile.gcc
set FLAGS=CXXFLAGS=-Wno-attributes USE_ODBC=1 USE_OPENGL=1 USE_QA=1 USE_GDIPLUS=0 -j %NUMBER_OF_PROCESSORS%
goto START

:START
echo %WXBUILD_APPNAME% v%WXBUILD_VERSION%
echo.

if %2 == LIB   goto LIB_BUILD
if %2 == lib   goto LIB_BUILD
if %2 == DLL   goto DLL_BUILD
if %2 == dll   goto DLL_BUILD
if %2 == ALL   goto ALL_BUILD
if %2 == all   goto ALL_BUILD
if %2 == NULL  goto SECIFIC_BUILD
if %2 == null  goto SECIFIC_BUILD
goto WRONGPARAM

:SECIFIC_BUILD
echo Specific mode...
echo.
IF (%3) == () goto ERROR
goto %3

:ALL_BUILD
echo Compiling all versions.
echo.
goto LIB_BUILD

:CLEAN_VS
:: Clean Visual C++ 32-bit directories.
echo Cleaning 32-bit directories...
echo.
if exist ..\..\lib\vc_lib del /Q ..\..\lib\vc_lib\*.*
if exist ..\..\lib\vc_dll del /Q ..\..\lib\vc_dll\*.*

if exist vc_msw del /Q vc_msw\*.*
if exist vc_mswd del /Q vc_mswd\*.*
if exist vc_mswdll del /Q vc_mswdll\*.*
if exist vc_mswddll del /Q vc_mswddll\*.*
if exist vc_mswu del /Q vc_mswu\*.*
if exist vc_mswud del /Q vc_mswud\*.*
if exist vc_mswudll del /Q vc_mswudll\*.*
if exist vc_mswuddll del /Q vc_mswuddll\*.*
echo Done.
echo.
goto END

:CLEAN_VS64
:: Clean Visual C++ 64-bit directories.
echo Cleaning 64-bit directories...
echo.
if exist ..\..\lib\vc_amd64_lib del /Q ..\..\lib\vc_amd64_lib\*.*
if exist ..\..\lib\vc_amd64_dll del /Q ..\..\lib\vc_amd64_dll\*.*

if exist vc_msw_amd64 del /Q vc_msw_amd64\*.*
if exist vc_mswd_amd64 del /Q vc_mswd_amd64\*.*
if exist vc_mswdll_amd64 del /Q vc_mswdll_amd64\*.*
if exist vc_mswddll_amd64 del /Q vc_mswddll_amd64\*.*
if exist vc_mswu_amd64 del /Q vc_mswu_amd64\*.*
if exist vc_mswud_amd64 del /Q vc_mswud_amd64\*.*
if exist vc_mswudll_amd64 del /Q vc_mswudll_amd64\*.*
if exist vc_mswuddll_amd64 del /Q vc_mswuddll_amd64\*.*
echo Done.
echo.
goto END

:CLEAN_MINGW
:: Clean MinGW directories.
echo Cleaning GCC directories...
echo.
if exist ..\..\lib\gcc_lib del /Q ..\..\lib\gcc_lib\*.*
if exist ..\..\lib\gcc_dll del /Q ..\..\lib\gcc_dll\*.*

if exist gcc_msw del /Q gcc_msw\*.*
if exist gcc_mswdll del /Q gcc_mswdll\*.*
if exist gcc_mswd del /Q gcc_mswd\*.*
if exist gcc_mswddll del /Q gcc_mswddll\*.*
if exist gcc_mswu del /Q gcc_mswu\*.*
if exist gcc_mswudll del /Q gcc_mswudll\*.*
if exist gcc_mswud del /Q gcc_mswud\*.*
if exist gcc_mswuddll del /Q gcc_mswuddll\*.*
::%MAKE% -f %MAKEFILE% clean
echo Done.
echo.
goto END

:MOVE
echo Moving binary files...
echo.
if %1 == VCTK     goto MOVE_VCTK
if %1 == vctk     goto MOVE_VCTK
if %1 == VC71     goto MOVE_VCTK
if %1 == vc71     goto MOVE_VCTK
if %1 == VC80     goto MOVE_VC80
if %1 == vc80     goto MOVE_VC80
if %1 == VC80_64  goto MOVE_VC80_64
if %1 == vc80_64  goto MOVE_VC80_64
if %1 == VC90     goto MOVE_VC90
if %1 == vc90     goto MOVE_VC90
if %1 == VC90_64  goto MOVE_VC90_64
if %1 == vc90_64  goto MOVE_VC90_64
if %1 == VC100    goto MOVE_VC100
if %1 == vc100    goto MOVE_VC100
if %1 == VC100_64 goto MOVE_VC100_64
if %1 == vc100_64 goto MOVE_VC100_64
if %1 == MINGW    goto MOVE_MINGW
if %1 == mingw    goto MOVE_MINGW
if %1 == MINGW4   goto MOVE_MINGW4
if %1 == mingw4   goto MOVE_MINGW4
goto MOVE_ERROR

:MOVE_VCTK
:: Move Visual C++ 7.1 directories.
if exist ..\..\lib\vc_lib move /Y ..\..\lib\vc_lib ..\..\lib\vc7_lib
if exist ..\..\lib\vc_dll move /Y ..\..\lib\vc_dll ..\..\lib\vc7_dll
echo.
goto END

:MOVE_VC80
:: Move Visual C++ 8.0 directories.
if exist ..\..\lib\vc_lib move /Y ..\..\lib\vc_lib ..\..\lib\vc8_lib
if exist ..\..\lib\vc_dll move /Y ..\..\lib\vc_dll ..\..\lib\vc8_dll
echo.
goto END

:MOVE_VC80_64
:: Move Visual C++ 8.0 64-bit directories.
if not exist ..\..\lib64 mkdir ..\..\lib64
if exist ..\..\lib\vc_amd64_lib move /Y ..\..\lib\vc_amd64_lib ..\..\lib64\vc8_lib
if exist ..\..\lib\vc_amd64_dll move /Y ..\..\lib\vc_amd64_dll ..\..\lib64\vc8_dll
echo.
goto END

:MOVE_VC90
:: Move Visual C++ 9.0 directories.
if exist ..\..\lib\vc_lib move /Y ..\..\lib\vc_lib ..\..\lib\vc9_lib
if exist ..\..\lib\vc_dll move /Y ..\..\lib\vc_dll ..\..\lib\vc9_dll
echo.
goto END

:MOVE_VC90_64
:: Move Visual C++ 190 64-bit directories.
if not exist ..\..\lib64 mkdir ..\..\lib64
if exist ..\..\lib\vc_amd64_lib move /Y ..\..\lib\vc_amd64_lib ..\..\lib64\vc9_lib
if exist ..\..\lib\vc_amd64_dll move /Y ..\..\lib\vc_amd64_dll ..\..\lib64\vc9_dll
echo.
goto END

:MOVE_VC100
:: Move Visual C++ 10.0 directories.
if exist ..\..\lib\vc_lib move /Y ..\..\lib\vc_lib ..\..\lib\vc10_lib
if exist ..\..\lib\vc_dll move /Y ..\..\lib\vc_dll ..\..\lib\vc10_dll
echo.
goto END

:MOVE_VC100_64
:: Move Visual C++ 10.0 64-bit directories.
if not exist ..\..\lib64 mkdir ..\..\lib64
if exist ..\..\lib\vc_amd64_lib move /Y ..\..\lib\vc_amd64_lib ..\..\lib64\vc10_lib
if exist ..\..\lib\vc_amd64_dll move /Y ..\..\lib\vc_amd64_dll ..\..\lib64\vc10_dll
echo.
goto END

:MOVE_MINGW
:: Move MinGW 3.x.x directories.
if exist ..\..\lib\gcc_lib move /Y ..\..\lib\gcc_lib ..\..\lib\gcc3_lib
if exist ..\..\lib\gcc_dll move /Y ..\..\lib\gcc_dll ..\..\lib\gcc3_dll
echo.
goto END

:MOVE_MINGW4
:: Move MinGW 4.x.x directories.
if exist ..\..\lib\gcc_lib move /Y ..\..\lib\gcc_lib ..\..\lib\gcc4_lib
if exist ..\..\lib\gcc_dll move /Y ..\..\lib\gcc_dll ..\..\lib\gcc4_dll
echo.
goto END

:LIB_BUILD
echo Building libs's...
echo.
goto LIB_DEBUG

:LIB_DEBUG
echo Compiling lib debug...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE% BUILD=debug SHARED=0 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_RELEASE

:LIB_RELEASE
echo Compiling lib release...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE% BUILD=release SHARED=0 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_BUILD_UNICODE

:LIB_BUILD_UNICODE
echo Building Unicode Lib's...
echo.
goto LIB_DEBUG_UNICODE

:LIB_DEBUG_UNICODE
echo Compiling lib debug Unicode...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug UNICODE=1 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_RELEASE_UNICODE

:LIB_RELEASE_UNICODE
echo Compiling lib release Unicode...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release UNICODE=1 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_BUILD_MONO

::_____________________________________________________________________________
:LIB_BUILD_MONO
echo Building Monolithic lib's...
echo.
goto LIB_DEBUG_MONO

:LIB_DEBUG_MONO
echo Compiling lib debug monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug MONOLITHIC=1 SHARED=0 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_RELEASE_MONO

:LIB_RELEASE_MONO
echo Compiling lib release monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release MONOLITHIC=1 SHARED=0 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_BUILD_MONO_UNICODE

:LIB_BUILD_MONO_UNICODE
echo Building Monolithic Unicode lib's...
echo.
goto LIB_DEBUG_MONO_UNICODE

:LIB_DEBUG_MONO_UNICODE
echo Compiling lib debug Unicode monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug MONOLITHIC=1 SHARED=0 UNICODE=1 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto LIB_RELEASE_MONO_UNICODE

:LIB_RELEASE_MONO_UNICODE
echo Compiling lib release Unicode monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release MONOLITHIC=1 SHARED=0 UNICODE=1 OFFICIAL_BUILD=1 RUNTIME_LIBS=static TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for build all
if %2 == all goto DLL_BUILD
if %2 == ALL goto DLL_BUILD
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto END

:DLL_BUILD
echo Building Dll's...
echo.
goto DLL_DEBUG

:DLL_DEBUG
echo Compiling dll debug...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug SHARED=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_RELEASE

:DLL_RELEASE
echo Compiling dll release...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release SHARED=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_BUILD_UNICODE

:DLL_BUILD_UNICODE
echo Building Unicode Dll's...
echo.
goto DLL_DEBUG_UNICODE

:DLL_DEBUG_UNICODE
echo Compiling dll debug Unicode...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug SHARED=1 UNICODE=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_RELEASE_UNICODE

:DLL_RELEASE_UNICODE
echo Compiling dll release Unicode...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release SHARED=1 UNICODE=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_BUILD_MONO

:DLL_BUILD_MONO
echo Building Monolithic Dll's...
echo.
goto DLL_DEBUG_MONO

:DLL_DEBUG_MONO
echo Compiling dll debug monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug MONOLITHIC=1 SHARED=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_RELEASE_MONO

:DLL_RELEASE_MONO
echo Compiling dll release monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release MONOLITHIC=1 SHARED=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_BUILD_MONO_UNICODE

:DLL_BUILD_MONO_UNICODE
echo Building Monolithic Unicode Dll's...
echo.
goto DLL_DEBUG_MONO_UNICODE

:DLL_DEBUG_MONO_UNICODE
echo Compiling dll debug Unicode monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=debug MONOLITHIC=1 SHARED=1 UNICODE=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto DLL_RELEASE_MONO_UNICODE

:DLL_RELEASE_MONO_UNICODE
echo Compiling dll release Unicode monolithic...
:: Calling the compilers  make
%MAKE% -f %MAKEFILE%  BUILD=release MONOLITHIC=1 SHARED=1 UNICODE=1 OFFICIAL_BUILD=1 TARGET_CPU=%CPU% %FLAGS%

echo.
:: Check for specific mode.
if %2 == null goto END
if %2 == NULL goto END
goto END

:ERROR
echo.
echo ERROR OCCURED!
echo Not enough command line parameters.
goto SHOW_USAGE

:WRONGPARAM
echo.
echo ERROR OCCURED!
echo The command line parameters was %1. This is not an available option.
goto SHOW_USAGE

:COMPILER_ERROR
echo.
echo ERROR OCCURED!
echo Unsupported compiler. %1 is not an available compiler option.
goto SHOW_USAGE

:MOVE_ERROR
echo.
echo ERROR OCCURED!
echo Unsupported compiler users as a parameter to move. (%1)
goto SHOW_USAGE

:SHOW_USAGE
echo.
echo %WXBUILD_APPNAME% v%WXBUILD_VERSION%
echo     Build wxWidgets with the MinGW/Visual C++ Tool chains.
echo.
echo Usage: "wxBuild_wxWidgets.bat <Compiler{MINGW|VCTK|VC71|VC80|VC90}> <BuildTarget{LIB|DLL|ALL|CLEAN|MOVE|NULL}> [Specific Option (See Below)]"
goto SHOW_OPTIONS

:SHOW_OPTIONS
echo.
echo      Compiler Options:
echo           MINGW    = MinGW Gcc v3.x.x compiler
echo           MINGW4   = MinGW Gcc v4.x.x compiler
echo           VCTK     = Visual C++ 7.1 Toolkit
echo           VC71     = Visual C++ 7.1
echo           VC80     = Visual C++ 8.0
echo           VC80_64  = Visual C++ 8.0 64-bit
echo           VC90     = Visual C++ 9.0
echo           VC90_64  = Visual C++ 9.0 64-bit
echo           VC100    = Visual C++ 10.0
echo           VC100_64 = Visual C++ 10.0 64-bit
echo.
echo      BuildTarget Options:
echo           LIB   = Builds all the static library targets.
echo           DLL   = Builds all the dynamic library targets.
echo           ALL   = Builds all the targets (Recommended).
echo           CLEAN = Cleans the solution.
echo           MOVE  = Moves the binary files to a compiler specific directory.
echo                   This helps to allow for automated builds using compilers
echo                   from the same family. (ex. VC 7.1 and VC8.0))
echo           NULL  = Used so that you can specify a specific target. (See below)
echo.
echo      Specific Options(Used with NULL): 
echo           LIB_DEBUG, LIB_RELEASE, LIB_DEBUG_UNICODE, LIB_RELEASE_UNICODE,
echo            LIB_DEBUG_MONO, LIB_RELEASE_MONO, LIB_DEBUG_MONO_UNICODE, 
echo            LIB_RELEASE_MONO_UNICODE,
echo.
echo           DLL_DEBUG, DLL_RELEASE, DLL_DEBUG_UNICODE, DLL_RELEASE_UNICODE,
echo            DLL_DEBUG_MONO, DLL_RELEASE_MONO, DLL_DEBUG_MONO_UNICODE,
echo            DLL_RELEASE_MONO_UNICODE
echo.
echo      Examples:
echo           wxBuild_default.bat MINGW ALL
echo             Builds all targets with MinGW Gcc Compiler.
echo.
echo           wxBuild_default.bat VCTK LIB
echo             Builds just the static libraries with Visual C++ 7.1 Toolkit.
echo.
echo           wxBuild_default.bat VCTK NULL LIB_RELEASE
echo             Builds only the release static library with Visual C++ 7.1 Toolkit
goto END

:END
set WXBUILD_VERSION=
set WXBUILD_APPNAME=
set GCCDIR=
set GCC4DIR=
set MAKE=
set MAKEFILE=
set FLAGS=
set CPU=
ENDLOCAL
