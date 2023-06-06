list(APPEND PWSAFE_HDR_LIST
    src/ui/wxWidgets/PWSafeApp.h
    src/ui/wxWidgets/PasswordSafeFrame.h
    src/ui/wxWidgets/AboutDlg.h
    src/ui/wxWidgets/OptionsPropertySheetDlg.h
    src/ui/wxWidgets/PasswordSafeSearch.h
    src/ui/wxWidgets/ViewReportDlg.h
    src/ui/wxWidgets/GuiInfo.h
    src/ui/wxWidgets/HelpMap.h
    src/ui/wxWidgets/wxMessages.h
    src/ui/wxWidgets/GridShortcutsValidator.h
    src/ui/wxWidgets/SafeCombinationChangeDlg.h
    src/ui/wxWidgets/SafeCombinationEntryDlg.h
    src/ui/wxWidgets/SafeCombinationSetupDlg.h
    src/ui/wxWidgets/AddEditPropSheetDlg.h
    src/ui/wxWidgets/SelectAliasDlg.h
    src/ui/wxWidgets/PropertiesDlg.h
    src/ui/wxWidgets/Clipboard.h
    src/ui/wxWidgets/CreateShortcutDlg.h
    src/ui/wxWidgets/DeleteConfirmationDlg.h
    src/ui/wxWidgets/EditShortcutDlg.h
    src/ui/wxWidgets/FieldSelectionDlg.h
    src/ui/wxWidgets/FieldSelectionPanel.h
    src/ui/wxWidgets/wxUtilities.h
    src/ui/wxWidgets/AdvancedSelectionDlg.h
    src/ui/wxWidgets/CompareDlg.h
    src/ui/wxWidgets/ComparisonGridTable.h
    src/ui/wxWidgets/DbSelectionPanel.h
    src/ui/wxWidgets/ExportTextWarningDlg.h
    src/ui/wxWidgets/ExternalKeyboardButton.h
    src/ui/wxWidgets/ImportTextDlg.h
    src/ui/wxWidgets/ImportXmlDlg.h
    src/ui/wxWidgets/ManagePasswordPoliciesDlg.h
    src/ui/wxWidgets/MergeDlg.h
    src/ui/wxWidgets/OpenFilePickerValidator.h
    src/ui/wxWidgets/DragBarCtrl.h
    src/ui/wxWidgets/GridCtrl.h
    src/ui/wxWidgets/GridTable.h
    src/ui/wxWidgets/StatusBar.h
    src/ui/wxWidgets/TreeCtrl.h
    src/ui/wxWidgets/PasswordPolicyDlg.h
    src/ui/wxWidgets/SyncWizard.h
    src/ui/wxWidgets/ToolbarButtons.h
    src/ui/wxWidgets/RecentDbList.h
    src/ui/wxWidgets/SafeCombinationCtrl.h
    src/ui/wxWidgets/SelectionCriteria.h
    src/ui/wxWidgets/SizeRestrictedPanel.h
    src/ui/wxWidgets/SystemTray.h
    src/ui/wxWidgets/SystemTrayMenuId.h
    src/ui/wxWidgets/SafeCombinationPromptDlg.h
    src/ui/wxWidgets/TimedTaskChain.h
    src/ui/wxWidgets/PasswordSubsetDlg.h
    src/ui/wxWidgets/CryptKeyEntryDlg.h
    src/ui/wxWidgets/SetDatabaseIdDlg.h
    src/ui/wxWidgets/SetFiltersDlg.h
    src/ui/wxWidgets/ManageFiltersDlg.h
    src/ui/wxWidgets/ViewAttachmentDlg.h
    src/ui/wxWidgets/DnDFile.h
    src/ui/wxWidgets/DnDPWSafeObject.h
    src/ui/wxWidgets/DnDSupport.h
    src/ui/wxWidgets/DnDDropTarget.h
    src/ui/wxWidgets/PWFiltersGrid.h
    src/ui/wxWidgets/PWFiltersTable.h
    src/ui/wxWidgets/PWFiltersEditor.h
    src/ui/wxWidgets/PWFiltersBoolDlg.h
    src/ui/wxWidgets/PWFiltersStringDlg.h
    src/ui/wxWidgets/PWFiltersDCADlg.h
    src/ui/wxWidgets/PWFiltersIntegerDlg.h
    src/ui/wxWidgets/PWFiltersStatusDlg.h
    src/ui/wxWidgets/PWFiltersTypeDlg.h
    src/ui/wxWidgets/PWFiltersDateDlg.h
    src/ui/wxWidgets/PWFiltersMediaDlg.h
    src/ui/wxWidgets/PWFiltersPasswordDlg.h
    src/ui/wxWidgets/ManageFiltersTable.h
    )


list(APPEND PWSAFE_SRC_LIST
    src/ui/wxWidgets/OptionsPropertySheetDlg.cpp
    src/ui/wxWidgets/CompareDlg.cpp
    src/ui/wxWidgets/ImportTextDlg.cpp
    src/ui/wxWidgets/AddEditPropSheetDlg.cpp
    src/ui/wxWidgets/SelectAliasDlg.cpp
    src/ui/wxWidgets/PasswordSafeSearch.cpp
    src/ui/wxWidgets/SystemTray.cpp
    src/ui/wxWidgets/AboutDlg.cpp
    src/ui/wxWidgets/GuiInfo.cpp
    src/ui/wxWidgets/Version.cpp
    src/ui/wxWidgets/Clipboard.cpp
    src/ui/wxWidgets/SafeCombinationEntryDlg.cpp
    src/ui/wxWidgets/MenuEditHandlers.cpp
    src/ui/wxWidgets/MenuFileHandlers.cpp
    src/ui/wxWidgets/MenuManageHandlers.cpp
    src/ui/wxWidgets/MenuViewHandlers.cpp
    src/ui/wxWidgets/ViewReportDlg.cpp
    src/ui/wxWidgets/SafeCombinationPromptDlg.cpp
    src/ui/wxWidgets/PasswordSafeFrame.cpp
    src/ui/wxWidgets/PropertiesDlg.cpp
    src/ui/wxWidgets/CreateShortcutDlg.cpp
    src/ui/wxWidgets/DeleteConfirmationDlg.cpp
    src/ui/wxWidgets/EditShortcutDlg.cpp
    src/ui/wxWidgets/FieldSelectionDlg.cpp
    src/ui/wxWidgets/FieldSelectionPanel.cpp
    src/ui/wxWidgets/GridShortcutsValidator.cpp
    src/ui/wxWidgets/SafeCombinationChangeDlg.cpp
    src/ui/wxWidgets/ExternalKeyboardButton.cpp
    src/ui/wxWidgets/PasswordPolicyDlg.cpp
    src/ui/wxWidgets/DbSelectionPanel.cpp
    src/ui/wxWidgets/PWSafeApp.cpp
    src/ui/wxWidgets/SafeCombinationSetupDlg.cpp
    src/ui/wxWidgets/wxUtilities.cpp
    src/ui/wxWidgets/AdvancedSelectionDlg.cpp
    src/ui/wxWidgets/ComparisonGridTable.cpp
    src/ui/wxWidgets/ExportTextWarningDlg.cpp
    src/ui/wxWidgets/ImportXmlDlg.cpp
    src/ui/wxWidgets/ManagePasswordPoliciesDlg.cpp
    src/ui/wxWidgets/MergeDlg.cpp
    src/ui/wxWidgets/OpenFilePickerValidator.cpp
    src/ui/wxWidgets/DragBarCtrl.cpp
    src/ui/wxWidgets/GridCtrl.cpp
    src/ui/wxWidgets/GridTable.cpp
    src/ui/wxWidgets/TreeCtrl.cpp
    src/ui/wxWidgets/SyncWizard.cpp
    src/ui/wxWidgets/SafeCombinationCtrl.cpp
    src/ui/wxWidgets/SelectionCriteria.cpp
    src/ui/wxWidgets/SizeRestrictedPanel.cpp
    src/ui/wxWidgets/TimedTaskChain.cpp
    src/ui/wxWidgets/PasswordSubsetDlg.cpp
    src/ui/wxWidgets/CryptKeyEntryDlg.cpp
    src/ui/wxWidgets/SetDatabaseIdDlg.cpp
    src/ui/wxWidgets/SetFiltersDlg.cpp
    src/ui/wxWidgets/ManageFiltersDlg.cpp
    src/ui/wxWidgets/ViewAttachmentDlg.cpp
    src/ui/wxWidgets/DnDFile.cpp
    src/ui/wxWidgets/DnDPWSafeObject.cpp
    src/ui/wxWidgets/DnDSupport.cpp
    src/ui/wxWidgets/DnDDropTarget.cpp
    src/ui/wxWidgets/PWFiltersGrid.cpp
    src/ui/wxWidgets/PWFiltersTable.cpp
    src/ui/wxWidgets/PWFiltersEditor.cpp
    src/ui/wxWidgets/PWFiltersBoolDlg.cpp
    src/ui/wxWidgets/PWFiltersStringDlg.cpp
    src/ui/wxWidgets/PWFiltersDCADlg.cpp
    src/ui/wxWidgets/PWFiltersIntegerDlg.cpp
    src/ui/wxWidgets/PWFiltersStatusDlg.cpp
    src/ui/wxWidgets/PWFiltersTypeDlg.cpp
    src/ui/wxWidgets/PWFiltersDateDlg.cpp
    src/ui/wxWidgets/PWFiltersMediaDlg.cpp
    src/ui/wxWidgets/PWFiltersPasswordDlg.cpp
    src/ui/wxWidgets/ManageFiltersTable.cpp
    src/ui/wxWidgets/QueryCancelDlg.cpp
    )

if(HAVE_YKPERS_H)
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/YubiCfgDlg.cpp src/ui/wxWidgets/YubiMixin.cpp)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/YubiCfgDlg.h src/ui/wxWidgets/YubiMixin.h)
endif(HAVE_YKPERS_H)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
if (NOT NO_QR)
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/QRCodeDlg.cpp src/ui/wxWidgets/QREncode.cpp)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/QREncode.h src/ui/wxWidgets/QRCodeDlg.h)
endif(NOT NO_QR)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/QRCodeDlg.cpp src/ui/wxWidgets/QREncode.mm)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/QREncode.h src/ui/wxWidgets/QRCodeDlg.h)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

set (PWSAFE_SRCS ${PWSAFE_SRC_LIST} PARENT_SCOPE)
set (PWSAFE_HDRS ${PWSAFE_HDR_LIST} PARENT_SCOPE)

