This note describes the new features, fixed bugs and known problems with the latest versions of Password Safe. For a short description of
Password Safe, please see the accompanying [**README.md**](../README.md) file. For more information on the product and the project, please visit
https://pwsafe.org/. Details about changes to older releases may be found in the file ChangeLog.txt.

In the following, SFxxxx refers to Bug Reports, Feature Requests and Service Requests in PasswordSafe SourceForge Project tickets, and GHxxxx refers to issues in the PasswordSafe GitHub project.

PasswordSafe 3.72.0pre Release June 12 2026
===========================================
Bugs Fixed in 3.72.0pre
-----------------------
* Improved font scale handling -should resolve font size issues on high resolution displays.
* [GH1749](https://github.com/pwsafe/pwsafe/issues/1749) In the Master Password Setup window, "Show Master Password" is no longer truncated on some displays.
* [GH1092](https://github.com/pwsafe/pwsafe/issues/1092)[SF1595](https://sourceforge.net/p/passwordsafe/bugs/1595/) Size and position of main window is now correctly restored on scaled displays.
* [SF1628](https://sourceforge.net/p/passwordsafe/bugs/1628/) Custom values can now be copied to the clipboard in read-only mode via Ctrl-C and right-click->Copy Value.

New Features in 3.72.0pre
-------------------------
* [GH1196](https://github.com/pwsafe/pwsafe/issues/1196) Dark display mode support: Password Safe now supports the system display mode, as well as setting the mode directly via Manage->Options->Display->Dsiplay Mode. This change also updates the general "look & feel" of the app to the current Windows theme. Known limitations:
  * The Date picker and keyboard shortcut controls do not switch to dark theme
  * The Customize Toolbar dialog does not switch to dark theme
* Custom Field support has been added to the more advanced features:
  * Filters
  * XML and Text import and export
  * Comparison, Sync and Merge databases
  * [SF938](https://sourceforge.net/p/passwordsafe/feature-requests/938/) Custom field values may now be selected by name and copied via a "Copy Custom Field Value..." submenu in the entry context popup menu.
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
