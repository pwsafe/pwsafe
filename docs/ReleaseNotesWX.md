This note describes the new features, fixed bugs and known problems that are specific to versions of *Password Safe* for Mac and Linux. 
For the Windows version, and all common changes, please see **ReleaseNotes.md**.

For a short description of
Password Safe, please see the accompanying **README.md** file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 1.18.2 Release ??? 2023?
=====================================

New Features in 1.18.2
----------------------
* [GH1056](https://github.com/pwsafe/pwsafe/issues/1056) macOS: Add Yubikey support.
* [GH1063](https://github.com/pwsafe/pwsafe/issues/1063) macOS: Add language translations and re-work packaging.

PasswordSafe 1.18.1 Release November 2023
=========================================

Bugs fixed in 1.18.1
--------------------
* [GH1010](https://github.com/pwsafe/pwsafe/issues/1010) No more spurious change warnings on entries with password history.

PasswordSafe 1.18 Release October 2023
======================================

Bugs fixed in 1.18
--------------------
* [SF1578](https://sourceforge.net/p/passwordsafe/bugs/1578) Import text report file now named correctly.
* [GH996](https://github.com/pwsafe/pwsafe/pull/996) The "Clear clipboard upon minimize" option only worked if "Lock password database on Minimize" was also set.
* [GH998](https://github.com/pwsafe/pwsafe/pull/998) Eliminate a spurious warning when the password policy has a custom character list.
* [GH1005](https://github.com/pwsafe/pwsafe/issues/1005) Backslash in passwords are now passed as-is in autotype.
* [GH1008](https://github.com/pwsafe/pwsafe/pull/1008) Tree view was not properly restored when a safe is locked and then unlocked.
  
Changes in 1.18
-----------------
* [GH995](https://github.com/pwsafe/pwsafe/pull/995) [SF433](https://sourceforge.net/p/passwordsafe/support-requests/433/) macOS only - The Password Safe program version number now appears in the Finder "Get Info" display.
* [GH1007](https://github.com/pwsafe/pwsafe/pull/1007) macOS only - A hidden or minimized window can now be restored with a left-click on the dock icon.

New Features in 1.18
--------------------
* [GH882](https://github.com/pwsafe/pwsafe/issues/882) Yubikey polling can now be adjusted/disabled via new --yubi-polling-interval command line argument (0 disables, other value is polling interval in milliseconds, default is 500ms).
* [SF909](https://sourceforge.net/p/passwordsafe/feature-requests/909/) Search (Find) bar visibility is now persistent.

Known Issues in 1.18
--------------------
* [GH1009](https://github.com/pwsafe/pwsafe/issues/1009) macOS only: Unlock Safe from systray menu opens an empty window
* [README.MAC.DEVELOPERS.md](https://github.com/pwsafe/pwsafe/blob/master/README.MAC.DEVELOPERS.md#known-issues-with-macos-install) macOS only: On an initial install, the language may default to German.

PasswordSafe 1.17 Release June 2023
===================================

* [GH957](https://github.com/pwsafe/pwsafe/issues/) [GH988](https://github.com/pwsafe/pwsafe/issues/988) Fixed crash when checking for new version.
* [GH952](https://github.com/pwsafe/pwsafe/issues/952) Mac - Update to wxWidgets 3.2.2.1 due to font issues with earlier versions.
* [GH949](https://github.com/pwsafe/pwsafe/issues/949) Added Wx-specific release notes for non-Windows releases.
* [GH945](https://github.com/pwsafe/pwsafe/issues/945) Document missing flags in pwsafe-cli usage text.
* [GH942](https://github.com/pwsafe/pwsafe/issues/942) Mac - Now built as a Universal Binary targeting Intel and Apple Silicon Macs with macOS 10.14 and up.
