This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying [**README.md**](../README.md) file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.71.1 Release ??? ?? 2026
========================================
Bugs Fixed in 3.71.1
--------------------
* Improved font scale handling -should resolve font size issues on high resolution displays.
* [GH1749](https://github.com/pwsafe/pwsafe/issues/1749) In the Master Password Setup window, "Show Master Password" is no longer truncated on some displays.
* [GH1092](https://github.com/pwsafe/pwsafe/issues/1092)[SF1595](https://sourceforge.net/p/passwordsafe/bugs/1595/) Size and position of main window is now correctly restored on scaled displays.

New Features in 3.71.1
----------------------
* Custom Field support has been added to the more advanced features:
  * Filters
  * XML and Text import and export
  * Comparison, Sync and Merge databases
  * [SF936](https://sourceforge.net/p/passwordsafe/feature-requests/936/) Notes and Custom fields layout now overlap, selectable by tabs, resulting in a more compact and less cluttered layout.

  * [SF935](https://sourceforge.net/p/passwordsafe/feature-requests/935/) Autotype: Specifying '\v{name}' in the autotype text will cause the corresponding value to be autotyped

PasswordSafe 3.71.0 Release March 27 2026
=========================================

Bugs Fixed in 3.71.0
--------------------
* [SF1623](https://sourceforge.net/p/passwordsafe/bugs/1623/) File paths may be enclosed in double-quotes in "Master Password Entry" and File->Compare dialogs.
* [SF933](https://sourceforge.net/p/passwordsafe/feature-requests/933/) 'Back' button has been added to final 'Export' windows, allowing re-exporting with chnaged parameters.

New Features in 3.71.0
----------------------
* Custom fields can now be added to each entry. That is, fields with user-defined names and values. Note that the following functions currently do not support custom fields:
  * Filters
  * XML and Text import and export
  * Comparison, Sync and Merge databases

Support custom fields will be added to the above in a future release.

PasswordSafe 3.70.1 Release Nov 19 2025
=======================================

Bugs Fixed in 3.70.1
--------------------
* The size of an attachment is now accurately preserved instead of being null-padded to a multiple of 8.
* The Attachment tab is now documented in the English help file.

PasswordSafe 3.70.0 Release Nov 8 2025
======================================
New Features in 3.70.0
----------------------
* Password Safe now supports adding a single attachment to an entry. Previously this was only available in the experimental V4 database format. With this release, the ability to add, update, remove and download a single attachment to an entry is now available via a new "attachment" tab in the Add/Edit Entry window.

Bugs Fixed in 3.70.0
--------------------
* Improved support for high contrast mode in Windows.
* "Always on top" now persists correctly across restarts when "Start Password Safe at Login" is selected.
* [GH1534](https://github.com/pwsafe/pwsafe/issues/1534) No longer offer unsupported traditional Chinese in NSIS installer.
* [GH1531](https://github.com/pwsafe/pwsafe/issues/1531) Followup on [GH1524](https://github.com/pwsafe/pwsafe/issues/1524) for Toolbar action.


PasswordSafe 3.69.0 Release Jun 29 2025
=========================================
Bugs Fixed in 3.69.0
-----------------------
* [GH1524](https://github.com/pwsafe/pwsafe/issues/1524) Report selection now works correctly when language isn't English.
* [GH1525](https://github.com/pwsafe/pwsafe/issues/1525) Title bar for alternate notes editor selection is now correct.
* [SF1613](https://sourceforge.net/p/passwordsafe/bugs/1613/) Text in file merge/sync/compare dialog is no longer truncated on some displays.


New features in 3.69.0
----------------------
* [GH1237](https://github.com/pwsafe/pwsafe/issues/1237), [SF823](https://sourceforge.net/p/passwordsafe/feature-requests/823/) A rough password strength meter now displays the strength of the password
for entries and the master password.
* Slovak is now fully supported thanks to Milan. 

PasswordSafe 3.68.0 Release Mar 18 2025
=======================================

New features in 3.68.0
----------------------
* In this release, PasswordSafe will significantly increase the standard (default) unlock difficulty of PasswordSafe databases. This may cause a noticeable slowdown when reading and writing databases.
 This is necessary to maintain the same level of security on today's hardware as when PasswordSafe was originally released.
 * [GH1439](https://github.com/pwsafe/pwsafe/issues/1439) PasswordSafe now uses the system text color and background instead of assuming black and white, respectively.

Bugs Fixed in 3.68.0
-----------------------
* [GH1408](https://github.com/pwsafe/pwsafe/issues/1408) Time fields (such as password modification time) are now copied over correctly in sync operations between databases.

