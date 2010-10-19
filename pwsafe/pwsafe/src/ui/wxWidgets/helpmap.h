#ifndef __HELPMAP_H__
#define __HELPMAP_H__

//For both export text and export XML
DLG_HELP(CExportTextWarningDlgBase,                     html/export.html)
DLG_HELP(CSafeCombinationChange,                        html/change_combo.html)
DLG_HELP(MergeDlg,                                      html/file_menu.html)
DLG_HELP(CSafeCombinationSetup,                         html/about_combinations.html)
DLG_HELP(CImportTextDlg,                                html/import.html)
DLG_HELP(CImportXMLDlg,                                 html/import.html#XML)
DLG_HELP(CSafeCombinationEntry,                         html/create_new_db.html)

//The Contents page from main toolbar
DLG_HELP(wxToolBar,                                     html/Welcome.html)

//AdvancedSelectionDlg template
DLG_HELP(AdvancedSelectionDlg<ExportFullXml>,           html/export.html#xml)
DLG_HELP(AdvancedSelectionDlg<ExportFullText>,          html/export.html#te)
DLG_HELP(AdvancedSelectionDlg<FindDlgType>,             html/searching.html)

//
//for property sheet help, note that the second parameter is a locale-specific string
//and must be translated at runtime, so make sure you use _(...) and not wxT(...)
//

//Options dialog
PROPSHEET_HELP(COptions,         _("Backups"),          html/backups_tab.html)
PROPSHEET_HELP(COptions,         _("Display"),          html/display_tab.html)
PROPSHEET_HELP(COptions,         _("Misc."),            html/misc_tab.html)
PROPSHEET_HELP(COptions,         _("Password Policy"),  html/password_policies.html)
PROPSHEET_HELP(COptions,         _("Password History"), html/password_history_tab.html)
PROPSHEET_HELP(COptions,         _("Security"),         html/security_tab.html)
PROPSHEET_HELP(COptions,         _("System"),           html/system_tab.html)
PROPSHEET_HELP(COptions,         _("Shortcuts"),        html/shortcuts_tab.html)

PROPSHEET_HELP(AddEditPropSheet, _("Basic"),            html/entering_pwd.html)
PROPSHEET_HELP(AddEditPropSheet, _("Dates and Times"),  html/entering_pwd_date.html)
PROPSHEET_HELP(AddEditPropSheet, _("Password Policy"),  html/entering_pwd_pp.html)
PROPSHEET_HELP(AddEditPropSheet, _("Additional"),       html/entering_pwd_add.html)

#endif
