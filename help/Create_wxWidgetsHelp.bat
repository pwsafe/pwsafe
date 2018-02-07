@echo off

:: Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
:: All rights reserved. Use of the code is allowed under the
:: Artistic License 2.0 terms, as specified in the LICENSE file
:: distributed with this code, or available from
:: http://www.opensource.org/licenses/artistic-license-2.0.php
::

:: This script creates the Help zip files required by wxWidgets for WINDOWS

:: If no language parameter given e.g. EN or DE, then all help files are created

cscript Windows_wxWidgetsHelp.vbs %1 //Nologo
