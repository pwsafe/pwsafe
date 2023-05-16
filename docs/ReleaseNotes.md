This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying README.md file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.63.0 Release May 2023
====================================

Bugs fixed in 3.63.0
--------------------
* [GH966] (https://github.com/pwsafe/pwsafe/issues/966) Default font sizes for hi-res displays corrected (Windows).
* [SF1572] (https://sourceforge.net/p/passwordsafe/bugs/1572/) [GH959] (https://github.com/pwsafe/pwsafe/issues/959) Opening via system tray icon now focuses correctly on Windows 11. The fix was to change the trigger from double-click to a single click, which is consistent with other system tray icons, but is a change for PasswordSafe.
* [GH946] (https://github.com/pwsafe/pwsafe/issues/946) Expiration dates are now the last second of the specified date, instead of the first minute. This resolves a minor inconsistency.
* [GH945] (https://github.com/pwsafe/pwsafe/issues/945) Document passphrase parameters in pwsafe-cli.

Changes in 3.63.0
-----------------
* Installer executable now has the "PasswordSafe" icon instead of the default NSIS Icon.
* Latvian is now supported, thanks to Jurcuks.

PasswordSafe 3.62.0 Release Feb 2023
====================================

Bugs fixed in 3.62.0
--------------------
* [SF1570] (https://sourceforge.net/p/passwordsafe/bugs/1570/) PasswordSafe no longer crashes when adding a new entry in a filtered view.

Changes in 3.62.0
-----------------
* Text (CSV) import is more lenient in what it accepts, to ease importing data from other password managers.
* When the password is shown, the "Confirm Password:" text is hidden, as the confirmation text
box is used to display the length of the password. Hopefully this makes things less confusing.
* [SF890](https://sourceforge.net/p/passwordsafe/feature-requests/890/) "Recurring" checkbox is now set by default, as this seems to be the more common use-case when setting password expiration in days.

PasswordSafe 3.61.0 Release Dec 2022
====================================

Bugs fixed in 3.61.0
--------------------
* [GH893](https://github.com/pwsafe/pwsafe/issues/893) Removed "Open Another" from system tray menu when a child window (e.g., Add/Edit Entry) is open.
* [SF1566](https://sourceforge.net/p/passwordsafe/bugs/1566/)  Persist Edit Entry and Rename custom shortcuts.

Changes in 3.61.0
-----------------
* [FR906](https://sourceforge.net/p/passwordsafe/feature-requests/906/) Made the ability to browse to an entry's URL with an alternate browser more visible.

PasswordSafe 3.60.0 Release Oct 2022
====================================

Bugs fixed in 3.60.0
--------------------
* [GH883](https://github.com/pwsafe/pwsafe/issues/883) V4: Validation removes orphaned attachments, fixing the database.
* [SF1563](https://sourceforge.net/p/passwordsafe/bugs/1563/) Fixed overlapping controls in Manage->Options->Security tab.
* [SF1560](https://sourceforge.net/p/passwordsafe/bugs/1560/) Flattened list: Width of last column is now retained across invocations.

New features in 3.60.0
----------------------
* [FR904](https://sourceforge.net/p/passwordsafe/feature-requests/904/) Made Edit menu order consistent with context menu (Copy User before copy password).
* [GH887](https://github.com/pwsafe/pwsafe/issues/887) Clipboard history retention is now user-configurable (default off).
* [FR898](https://sourceforge.net/p/passwordsafe/feature-requests/898/) Hovering on a non-empty group icon now shows how many entries under that group.

Changes in 3.60.0
-----------------
* [FR889](https://sourceforge.net/p/passwordsafe/feature-requests/889/) Documented how to work with the Flattened List vew, based on Huw's notes.

PasswordSafe 3.59.0 Release May 2022
====================================

Bugs fixed in 3.59.0
--------------------
* [SF1557](https://sourceforge.net/p/passwordsafe/bugs/1557/) Editing special characters in Password Policy works correctly with selected text.
* [SF1553](https://sourceforge.net/p/passwordsafe/bugs/1553/) Text selection in Notes field works correctly (again).

New features in 3.59.0
----------------------
* [SF903](https://sourceforge.net/p/passwordsafe/feature-requests/903/) When an alternativ browser's defined, allow browsing to an entry's URL with it via a new menu option
in addition to the '[alt]' notation in the URL.
* [SF900](https://sourceforge.net/p/passwordsafe/feature-requests/900/) When duplicating an existing entry, the newly created duplicate is selected, making it easier to edit or delete/undo.

