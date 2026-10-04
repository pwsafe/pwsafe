This note describes the new features, fixed bugs and known problems that are specific to versions of *Password Safe* for Mac and Linux. 
For the Windows version, and all common changes, please see [**ReleaseNotes.md**](ReleaseNotes.md).

For a short description of
Password Safe, please see the accompanying [**README.md**](../README.md) file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 1.26.0 Release October 8  2026
===========================================

Bugs fixed in 1.26.0
--------------------
* A security vulnerability related to V4 attachments has been fixed. A maliciously crafted PasswordSafe V4 database could cause out of bounds writes to memory, leading to application crash. No exploits of this have been reported in the wild. This affects Windows, Linux and iOS versions of the program. The android version doesn't support V4 attachments, and is therefore unaffected. Thanks to Fahad Awwad Alotaibi for reporting and helping resolve this issue in a prompt and professional manner.

New features in 1.26.0
----------------------
* Copy Auth Code has been added to the System tray icon's Recently Used Entries menu.
* [GH1938](https://github.com/pwsafe/pwsafe/issues/1938) Alias entries may now have their own 2-factor (TOTP) authentiation key. If present, this will be used instead of the base entry's.
* [GH1873](https://github.com/pwsafe/pwsafe/issues/1873) Added "Auth code" to the draggable items in the dragbar.

PasswordSafe 1.25.0 Release July 18 2026
=========================================

Bugs fixed in 1.25.0
--------------------
* [GH1856](https://github.com/pwsafe/pwsafe/issues/1856) (regression) Clicking on a row in the "conflicting items" grid in the comparison dialog no longer hangs the app.
* [SF1630](https://sourceforge.net/p/passwordsafe/bugs/1630/) Don't clear a non-recurring expiry when the password's changed.


PasswordSafe 1.24.0 Release  May 8 2026
=======================================

New features in 1.24.0
----------------------
* Custom fields can now be added to each entry. That is, fields with user-defined names and values. 

Bugs fixed in 1.24.0
--------------------
* [GH1751](https://github.com/pwsafe/pwsafe/issues/1751) Changing the height of the Add/Edit dialog will cause a vertical scrollbar to be added.
* [GH1685](https://github.com/pwsafe/pwsafe/issues/1685) Attachments are now copied over by Drag & Drop between V3 safes.
* [GH1646](https://github.com/pwsafe/pwsafe/issues/1646) In the "Flattened List" view, sorting by time columns (e.g., entry creation time) now works correctly.
* [GH1661](https://github.com/pwsafe/pwsafe/issues/1661) MacOS specific: The app no longer crashes when changing a filter criteria for Attachment - Media Type.
