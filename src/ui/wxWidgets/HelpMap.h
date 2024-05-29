/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef _HELPMAP_H_
#define _HELPMAP_H_

//For both export text and export XML
DLG_HELP(ExportTextWarningDlgBase,                      html/export.html)
DLG_HELP(SafeCombinationChangeDlg,                      html/change_combo.html)
DLG_HELP(MergeDlg,                                      html/file_menu.html)
DLG_HELP(SafeCombinationSetupDlg,                       html/about_combinations.html)
DLG_HELP(ImportTextDlg,                                 html/import.html)
DLG_HELP(ImportXmlDlg,                                  html/import.html#XML)
DLG_HELP(SafeCombinationEntryDlg,                       html/create_new_db.html)

//The Contents page from main toolbar
DLG_HELP(wxToolBar,                                     html/Welcome.html)

//AdvancedSelectionDlg template
DLG_HELP(AdvancedSelectionDlg<ExportFullXml>,           html/export.html#xml)
DLG_HELP(AdvancedSelectionDlg<ExportFullText>,          html/export.html#text)
DLG_HELP(AdvancedSelectionDlg<FindDlgType>,             html/searching.html)

//The help for Manage Password Policies dialog
DLG_HELP(ManagePasswordPoliciesDlg,                     html/named_password_policies.html)

#ifndef NO_YUBI
//The Yubikey Configuration dialog
DLG_HELP(YubiCfgDlg,                                    html/manage_menu.html#yubikey)
#endif

//
//for property sheet help, note that the second parameter is a locale-specific string
//and must be translated at runtime, so make sure you use _(...) and not wxT(...)
//

//Options dialog
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Backups"),          html/backups_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Display"),          html/display_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Misc."),            html/misc_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Password Policy"),  html/password_policies.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Password History"), html/password_history_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Security"),         html/security_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("System"),           html/system_tab.html)
PROPSHEET_HELP(OptionsPropertySheetDlg,            _("Shortcuts"),        html/shortcuts_tab.html)

PROPSHEET_HELP(AddEditPropSheetDlg, _("Basic"),            html/entering_pwd.html)
PROPSHEET_HELP(AddEditPropSheetDlg, _("Dates and Times"),  html/entering_pwd_date.html)
PROPSHEET_HELP(AddEditPropSheetDlg, _("Password Policy"),  html/entering_pwd_pp.html)
PROPSHEET_HELP(AddEditPropSheetDlg, _("Additional"),       html/entering_pwd_add.html)

DLG_HELP(EditShortcutDlg,                  html/edit_menu.html)

DLG_HELP(SetFiltersDlg,                    html/filters.html)
DLG_HELP(ManageFiltersDlg,                 html/filters.html)
DLG_HELP(pwFiltersBoolDlg,                 html/filters.html)
DLG_HELP(pwFiltersStringDlg,               html/filters.html)
DLG_HELP(pwFiltersDCADlg,                  html/filters.html)
DLG_HELP(pwFiltersIntegerDlg,              html/filters.html)
DLG_HELP(pwFiltersTypeDlg,                 html/filters.html)
DLG_HELP(pwFiltersDateDlg,                 html/filters.html)
DLG_HELP(pwFiltersMediaDlg,                html/filters.html)
DLG_HELP(pwFiltersPasswordDlg,             html/filters.html)

DLG_HELP(SelectAliasDlg,                   html/aliases.html)

#endif // _HELPMAP_H_
