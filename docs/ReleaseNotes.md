This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying README.md file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.68.0pre Release ???
==================================

Bugs Fixed in 3.68.0pre
-----------------------
* [GH1408](https://github.com/pwsafe/pwsafe/issues/1408) Time fields (such as password modification time) are now copied over correctly in sync operations between databases.

PasswordSafe 3.67.0 Release Oct 20 2024
=======================================

Bugs Fixed in 3.67.0
----------------------
* [GH1390](https://github.com/pwsafe/pwsafe/issues/1390), [SF1602](https://sourceforge.net/p/passwordsafe/bugs/1602/) Fixed regression, an entry's password history list now responds to mouse clicks.
* [GH1371](https://github.com/pwsafe/pwsafe/issues/1371) Fixed regresssion, recently used files are now remembered correctly.
* [GH1351](https://github.com/pwsafe/pwsafe/issues/1351) Fixed crash in pwsafe-cli when exporting database with named policy to XML.

New features in 3.67.0
----------------------
* [GH1301](https://github.com/pwsafe/pwsafe/issues/1301), [SF918](https://sourceforge.net/p/passwordsafe/feature-requests/918/) TOTP authorization code can be used in autotype via '\2'
* [SF921](https://sourceforge.net/p/passwordsafe/feature-requests/921/) Ctrl-Backspace now clears password fields, both for entries and master passwords.
* [SF912](https://sourceforge.net/p/passwordsafe/feature-requests/912/) If a view filter is active when a database is closed, it will be activated the next time the database is opened.


PasswordSafe 3.66.1 Release Jun 4 2024
======================================

Bugs fixed in 3.66.1
--------------------

* [SF1599](https://sourceforge.net/p/passwordsafe/bugs/1599/) Do not crash if pwsafe.cfg is missing (regression).
* [SF1597](https://sourceforge.net/p/passwordsafe/bugs/1597/) Group control no longer appears focused (green background) in initial Add/Edit window.
* [GH1282](https://github.com/pwsafe/pwsafe/issues/1282) Autotype now handles backslashes correctly in all field values, not only password.

PasswordSafe 3.66.0 Release May 27 2024
=======================================
Bugs fixed in 3.66.0
--------------------

* [GH1272](https://github.com/pwsafe/pwsafe/issues/1272) "Remember last N databases" now works correctly with N set to zero.
* [GH1203](https://github.com/pwsafe/pwsafe/issues/1203) The initial focus for new entries is now on the Title control, which is more useful.
* Saving intermediate backups should no longer fail due to different security attributes.

PasswordSafe 3.65.1 Release Feb 2024
====================================

Bugs fixed in 3.65.1
--------------------

* [SF551](https://sourceforge.net/p/passwordsafe/support-requests/551/) Document the fact that Screen Capture Protection isn't available before Windows 10 Version 2004, as well as the admin registry key used to disable it.
* [GH1092](https://github.com/pwsafe/pwsafe/issues/1092) Size and position of main window is now restored correctly on displays with scale > 100%.

Bugs fixed in 3.65.0
--------------------

* Modality of dialog windows is now preserved across lock/unlock.
* [GH1066](https://github.com/pwsafe/pwsafe/issues/1066) Signature verification no longer triggers network traffic.


New features in 3.65.0
----------------------
* [GH1075](https://github.com/pwsafe/pwsafe/issues/1075) PasswordSafe can now generate authentication tokens (one time password codes) for entries for site that require those for stronger security.
