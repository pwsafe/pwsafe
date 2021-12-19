This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying README.md file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.58.0 Release Dec 2021
====================================

Bugs fixed in 3.58.0
--------------------
* [BR1550](https://sourceforge.net/p/passwordsafe/bugs/1550/) Autotype delay now applies to key codes (e.g., &bsol;{Down}) as well as "regular" text.
* [BR1549](https://sourceforge.net/p/passwordsafe/bugs/1549/) '-g' command line option now handles UNC path correctly
* [GH800](https://github.com/pwsafe/pwsafe/issues/800) PasswordSafe now prevents data that it places on the clipboard from appearing in the Windows Clipboard History.

Changes in 3.58.0
-----------------
* [FR894](https://sourceforge.net/p/passwordsafe/feature-requests/894/) Add support for GoTrust Idem Key 2FA

New features in 3.58.0
----------------------
* [GH785] (https://github.com/pwsafe/pwsafe/issues/785) Added "Open Another" menu item in the system tray popup menu, allowing easy switching to a new PasswordSafe database.


PasswordSafe 3.57 Release October 2021
======================================
* [SF1547](https://sourceforge.net/p/passwordsafe/bugs/1547/) No longer crashes when incremental backup 999 reached.
* [SF896](https://sourceforge.net/p/passwordsafe/feature-requests/896/) Added \e to ESC mapping in autotype.
* [SF1544](https://sourceforge.net/p/passwordsafe/bugs/1544/) Fixed regression in German installer texts.
* [SF1539](https://sourceforge.net/p/passwordsafe/bugs/1539/) Workaround touchscreen issue with Manage->Options tabs.


PasswordSafe non-Windows Release 1.14
=====================================
* [SF1543](https://sourceforge.net/p/passwordsafe/bugs/1543/) Fixed crash when importing XML with empty password in password history


Password Safe 3.56 Release July 2021
====================================

New features in 3.56
-----------------------

* [FR894](https://sourceforge.net/p/passwordsafe/feature-requests/894/) Support OnlyKey 2FA

Bugs fixed in 3.56
------------------
* [SF1542](https://sourceforge.net/p/passwordsafe/bugs/1542) UNC paths now accepted as backup directory
* [SF1530](https://sourceforge.net/p/passwordsafe/bugs/1530) Maximized window is now restored as such after unlock
* [GH737](https://github.com/pwsafe/pwsafe/issues/737) Fixed crash in 64 bit version when selecting View -> Filters -> New/edit Filter -> Field DCA (or Shift-DCA) followed by Citeria selection
* [GH573](https://github.com/pwsafe/pwsafe/issues/573),[SF1488](https://sourceforge.net/p/passwordsafe/bugs/1488/) "Remind" Windows when PasswordSafe should stay topmost
* Fixed Notes line parsing in Run command

Changes in 3.56
------------------

* Entire password can be selected in password subset display by specifying '*'
* Arabic support added

PasswordSafe non-Windows Release 1.13
=====================================

Bugs fixed in 1.13
------------------

* [GH718](https://github.com/pwsafe/pwsafe/issues/718) Clear YubiSK when Yubi protection is removed.
* [SF1536](https://sourceforge.net/p/passwordsafe/bugs/1536/) pwsafe-cli now accepts master passphrases with spaces.

Password Safe 3.55 Release January 2021
=======================================

New features in 3.55
--------------------

* Replace the tabs in Manage->Options dialog with a more modern interface.
* [SF892](https://sourceforge.net/p/passwordsafe/feature-requests/892/) Added tooltip to copy password button in Add/Edit and Password Policy windows.
* [SF525](https://sourceforge.net/p/passwordsafe/support-requests/525/) Removed limitation on encrypting/decrypting files larger than 2GB.

Bugs fixed in 3.54.01
---------------------

* [GH696](https://github.com/pwsafe/pwsafe/issues/696),[SF1526](https://sourceforge.net/p/passwordsafe/bugs/1526) Regression: Works again under Windows 7 and 8.
* [SF1519](https://sourceforge.net/p/passwordsafe/bugs/1519) Fixed false change detection due to different line endings.

Bugs fixed in 3.54
------------------

* [SF1524](https://sourceforge.net/p/passwordsafe/bugs/1524) Added missing "Show Combination" checkbox to Export Dialogs.
* [SF1523](https://sourceforge.net/p/passwordsafe/bugs/1523) PasswordSafe now displays correctly on high DPI monitors.
* [SF1521](https://sourceforge.net/p/passwordsafe/bugs/1521) In Add/Edit entry's Policy tab, allow lengths to be set when easy vision is selected.
* [SF1519](https://sourceforge.net/p/passwordsafe/bugs/1519) Fixed false change detection when an entry's notes field has > 1 line
* [SF1517](https://sourceforge.net/p/passwordsafe/bugs/1517) Maximized PasswordSafe window is now restored as such.

Password Safe 3.53 Release September 2020
=========================================

Bugs Fixed in 3.53
------------------

* [SF1518](https://sourceforge.net/p/passwordsafe/bugs/1518) Fixed rare crash after dragging item outside of PasswordSafe.
* [SF1504](https://sourceforge.net/p/passwordsafe/bugs/1504/) Fixed regression: lock file is now deleted upon exit.
* [SF1030](https://sourceforge.net/p/passwordsafe/bugs/1030) Incremental search in list view no longer messes up selection.

Changes in 3.53
---------------

* This is the first release built with Visual Studio 2019 Community Edition and CMake.

PasswordSafe non-Windows release 1.11
=====================================

* Fix an issue with the width of spinbox controls in several versions of GTK3 which causes
  spinbox controls to be displayed incorrectly - too wide or too narrow.
  In addition to some heuristics based on the distribution type and version, we allow
  the user control of the width of the spinbox via the PWS_FIX_GTK_SPINBOX environment variable as follows:
  * 0 - This is the same as not setting the environment variable, i.e., let PasswordSafe try to determine the correct width
  * 1 - This lets wx set the width to wxDefaultSize, which may be way too wide for some versions of GTK
  * 2..10 - This sets the width to display this many characters in the text entry field of the spinner.

Password Safe 3.52 Release April 2020
=====================================

Bugs Fixed in 3.52
------------------

* [SF1510](https://sourceforge.net/p/passwordsafe/bugs/1510) Fixed truncated text in delete confirmation window.
* [SF1509](https://sourceforge.net/p/passwordsafe/bugs/1509) Cancelling a delete now leaves the selection unchanged.
* [GH634](https://github.com/pwsafe/pwsafe/issues/634),[SF1508](https://sourceforge.net/p/passwordsafe/bugs/1508) Clearing an entry's username no longer causes a revert to default.
* [SF1505](https://sourceforge.net/p/passwordsafe/bugs/1505) No longer crashes when trying to view or copy a shortcut from the "compare databases" results.
* [SF1503](https://sourceforge.net/p/passwordsafe/bugs/1503),[SF1512](https://sourceforge.net/p/passwordsafe/bugs/1512) Recurring password expiration update fixed ([1497](https://sourceforge.net/p/passwordsafe/bugs/1497) redo).
* [SF1495](https://sourceforge.net/p/passwordsafe/bugs/1495) Accepts policy name via Enter as well as click on OK.
