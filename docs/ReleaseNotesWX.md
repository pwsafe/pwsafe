This note describes the new features, fixed bugs and known problems that are specific to versions of *Password Safe* for Mac and Linux. 
For the Windows version, and all common changes, please see **ReleaseNotes.md**.

For a short description of
Password Safe, please see the accompanying **README.md** file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 1.20.0 Release ??? ?? 2024
=======================================

Bugs fixed in 1.20.0
--------------------
* [GH935](https://github.com/pwsafe/pwsafe/issues/935), [GH1149](https://github.com/pwsafe/pwsafe/issues/1149) Fixed several issues with the password expiration controls
* [GH1331](https://github.com/pwsafe/pwsafe/issues/1331) Correctly display long paths with ellipsis (...) in Master Password Entry window
* [GH1332](https://github.com/pwsafe/pwsafe/issues/1332) Correctly display long paths with ellipsis (...) in Unlock Password Database window
* [GH1372](https://github.com/pwsafe/pwsafe/issues/1372) Man page is now installed in the correct location (regression).
* [GH1374](https://github.com/pwsafe/pwsafe/issues/1374) Alt browser command line parameters handling fixed, Misc. help function fixed.

New features in 1.20.0
----------------------
* [GH1301](https://github.com/pwsafe/pwsafe/issues/1301) TOTP authorization code can be used in autotype via '\2'
* [FR890](https://sourceforge.net/p/passwordsafe/feature-requests/890) Set the Recurring checkbox by default when expiration by number of days is selected
* [GH1389](https://github.com/pwsafe/pwsafe/pull/1389) Disable (gray-out) unimplemented configuration options


PasswordSafe 1.19.1 Release 2 July 2024
=======================================

Bugs fixed in 1.19.1
--------------------
* [GH1307](https://github.com/pwsafe/pwsafe/issues/1307), [GH1308](https://github.com/pwsafe/pwsafe/issues/1308) Regression: Clipboard works correctly under Wayland
* [GH1230](https://github.com/pwsafe/pwsafe/issues/1230) The Change Master Password dialog now accepts a blank old password with a Yubikey

New features in 1.19.1
----------------------

Changes to existing features in 1.19.1
--------------------------------------
* [GH1300](https://github.com/pwsafe/pwsafe/issues/1300) Add the password strength check with a Yubikey


PasswordSafe 1.19 Release 8 June 2024
=====================================

Bugs fixed in 1.19
------------------
* [GH1282](https://github.com/pwsafe/pwsafe/issues/1282) Autotype now handles backslashes correctly in all field values, not only password.
* [GH1272](https://github.com/pwsafe/pwsafe/issues/1272) "Remember last N databases" now works correctly with N set to zero.
* [GH1158](https://github.com/pwsafe/pwsafe/issues/1158) Focus on Title field when opening Add/Edit entry.
* [GH1147](https://github.com/pwsafe/pwsafe/issues/1147) Password history grid is no longer editable in Add/Edit Entry's Additional tab.
* [GH1145](https://github.com/pwsafe/pwsafe/issues/1145) Fixed issue with editing a group's name.
* [GH1140](https://github.com/pwsafe/pwsafe/issues/1140) The master password of an empty database can now be changed.
* [GH1132](https://github.com/pwsafe/pwsafe/issues/1132) Handle Cancel button correctly in "Save Changes?" dialog box.
* [GH1128](https://github.com/pwsafe/pwsafe/issues/1128) Correct confirmation text for empty group deletion.
* [GH1115](https://github.com/pwsafe/pwsafe/issues/1115) More helpful handling of incorrect master password entry.
* [GH1085](https://github.com/pwsafe/pwsafe/issues/1085) Unify phrasing across platforms.


New features in 1.19
--------------------
* Added Slovenian language support thanks to grof.

Changes to existing features in 1.19
------------------------------------
* Default polling interval for Yubikey changed from 0.5 to 0.9 seconds to avoid deadlocks on slow systems.
* Texts cleaned up, now simpler and more direct

PasswordSafe 1.18.2 Release January 2024
========================================

Bugs fixed in 1.18.2
--------------------
* [SF1580](https://sourceforge.net/p/passwordsafe/bugs/1580/) Fixed missing help definition.

New Features in 1.18.2
----------------------
* cli support for authentication tokens (TOTP).
* [GH1009](https://github.com/pwsafe/pwsafe/issues/1009) macOS: Unlock Safe from systray menu fully restores the window.
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
