This note describes the new features, fixed bugs and known problems that are specific to versions of *Password Safe* for Mac and Linux. 
For the Windows version, and all common changes, please see **ReleaseNotes.md**.

For a short description of
Password Safe, please see the accompanying **README.md** file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.


PasswordSafe 1.22.0 Release 11 July 2025
========================================

Bugs fixed in 1.22.0
--------------------
[GH1227](https://github.com/pwsafe/pwsafe/issues/1227), [GH1228](https://github.com/pwsafe/pwsafe/issues/1228) Irrelevant functions are now disabled when databased is locked.
[GH1472](https://github.com/pwsafe/pwsafe/issues/1472) 'Go' button in Add/Edit window now respects "alt" browser specification.

New features in 1.22.0
----------------------
* macOS: File type associations are now supported for .psafe3 and .ibak files.  The Pwsafe icon will be used in Finder and the app can be launched by double-clicking a file.
* [GH1237](https://github.com/pwsafe/pwsafe/issues/1237), [SF823](https://sourceforge.net/p/passwordsafe/feature-requests/823/) A rough password strength meter now displays the strength of the password
for entries and the master password.


PasswordSafe 1.21.0 Release 28 March 2025
=========================================

* In this release, PasswordSafe will significantly increase the standard (default) unlock difficulty of PasswordSafe databases. This may cause a noticeable slowdown when reading and writing databases.
 This is necessary to maintain the same level of security on today's hardware as when PasswordSafe was originally released.
* Authenticator codes (One-time passwords, OTP, TOTP) can be configured and used via the UI.

Bugs fixed in 1.21.0
--------------------
* [GH1317](https://github.com/pwsafe/pwsafe/issues/1317) Aliases can now be created from new or edited entries.
* [GH1408](https://github.com/pwsafe/pwsafe/issues/1408) Time fields (such as password modification time) are now copied over correctly in sync operations between databases.
