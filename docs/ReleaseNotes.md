This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying README.md file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.65.1 Release Feb 2024
====================================

Bugs fixed in 3.65.1
--------------------
* [SF551](https://sourceforge.net/p/passwordsafe/support-requests/551/) Document the fact that Scrren Capture Protection isn't available before Windows 10 Version 2004, as well as the admin registry key used to disable it.
* [GH1092](https://github.com/pwsafe/pwsafe/issues/1092) Size and position of main window is now restored correctly on displays with scale > 100%.

Bugs fixed in 3.65.0
--------------------

* Modality of dialog windows is now preserved across lock/unlock.
* [GH1066](https://github.com/pwsafe/pwsafe/issues/1066) Signature verification no longer triggers network traffic.


New features in 3.65.0
----------------------
* [GH1075](https://github.com/pwsafe/pwsafe/issues/1075) PasswordSafe can now generate authentication tokens (one time password codes) for entries for site that require those for stronger security.


PasswordSafe 3.64.1 Release Nov 2023
====================================

Bugs fixed in 3.64.1
--------------------

* Support control of screen capture protection feature at installation time, via checkbox in exe installer, SCREENCAPTUREPROTECTION=False option via msiexec.
* [GH1041](https://github.com/pwsafe/pwsafe/issues/1041) PasswordSafe now verifies the signature when loading an app-specific dll.


PasswordSafe 3.64.0 Release Sept 2023
=====================================

Bugs fixed in 3.64.0
--------------------

* [GH1025](https://github.com/pwsafe/pwsafe/issues/1025) PasswordSafe database-specific options now reset to default when a database is closed.
* [SF1577](https://sourceforge.net/p/passwordsafe/bugs/1577/) Changes to font preferences are now saved immediately.
* [GH1021](https://github.com/pwsafe/pwsafe/issues/1021) Invalid values in System Option no longer trigger double error messages and crash.
* [GH1014](https://github.com/pwsafe/pwsafe/issues/1014) Focus is now set correctly after unlock when not using system tray.
* [GH1005](https://github.com/pwsafe/pwsafe/issues/1005) Backslash in passwords are now passed as-is in autotype.

New features in 3.64.0
----------------------
* [GH1022](https://github.com/pwsafe/pwsafe/issues/1022) By default, PasswordSafe windows no longer appear in Windows screen captures. This is user-configurable, see online help for details.
* [SF909](https://sourceforge.net/p/passwordsafe/feature-requests/909/) Search (Find) bar visibility is now persistent.

