list(APPEND PWSAFE_HDR_LIST
    src/ui/wxWidgets/pwsafeapp.h
    src/ui/wxWidgets/passwordsafeframe.h
    src/ui/wxWidgets/AboutDlg.h
    src/ui/wxWidgets/OptionsPropertySheetDlg.h
    src/ui/wxWidgets/PasswordSafeSearch.h
    src/ui/wxWidgets/ViewReport.h
    src/ui/wxWidgets/GuiInfo.h
    src/ui/wxWidgets/HelpMap.h
    src/ui/wxWidgets/wxMessages.h
    src/ui/wxWidgets/pwsmenushortcuts.h
    src/ui/wxWidgets/safecombinationchange.h
    src/ui/wxWidgets/safecombinationentry.h
    src/ui/wxWidgets/safecombinationsetup.h
    src/ui/wxWidgets/AddEditPropSheetDlg.h
    src/ui/wxWidgets/properties.h
    src/ui/wxWidgets/pwsclip.h
    src/ui/wxWidgets/CreateShortcutDlg.h
    src/ui/wxWidgets/DeleteConfirmationDlg.h
    src/ui/wxWidgets/DragBarGenericCtrl.h
    src/ui/wxWidgets/EditShortcutDlg.h
    src/ui/wxWidgets/FieldSelectionDlg.h
    src/ui/wxWidgets/FieldSelectionPanel.h
    src/ui/wxWidgets/wxutils.h
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
    src/ui/wxWidgets/PWSgrid.h
    src/ui/wxWidgets/PWSgridtable.h
    src/ui/wxWidgets/PWStatusBar.h
    src/ui/wxWidgets/PWStree.h
    src/ui/wxWidgets/PasswordPolicy.h
    src/ui/wxWidgets/PwsSync.h
    src/ui/wxWidgets/PwsToolbarButtons.h
    src/ui/wxWidgets/RecentDBList.h
    src/ui/wxWidgets/SafeCombinationCtrl.h
    src/ui/wxWidgets/SelectionCriteria.h
    src/ui/wxWidgets/SizeRestrictedPanel.h
    src/ui/wxWidgets/SystemTray.h
    src/ui/wxWidgets/SystemTrayMenuId.h
    src/ui/wxWidgets/safecombinationprompt.h
    src/ui/wxWidgets/TimedTaskChain.h
    src/ui/wxWidgets/passwordsubset.h
    src/ui/wxWidgets/CryptKeyEntryDlg.h
    )


list(APPEND PWSAFE_SRC_LIST
    src/ui/wxWidgets/OptionsPropertySheetDlg.cpp
    src/ui/wxWidgets/CompareDlg.cpp
    src/ui/wxWidgets/ImportTextDlg.cpp
    src/ui/wxWidgets/AddEditPropSheetDlg.cpp
    src/ui/wxWidgets/PasswordSafeSearch.cpp
    src/ui/wxWidgets/SystemTray.cpp
    src/ui/wxWidgets/AboutDlg.cpp
    src/ui/wxWidgets/mainManage.cpp
    src/ui/wxWidgets/GuiInfo.cpp
    src/ui/wxWidgets/version.cpp
    src/ui/wxWidgets/pwsclip.cpp
    src/ui/wxWidgets/safecombinationentry.cpp
    src/ui/wxWidgets/mainEdit.cpp
    src/ui/wxWidgets/mainView.cpp
    src/ui/wxWidgets/ViewReport.cpp
    src/ui/wxWidgets/safecombinationprompt.cpp
    src/ui/wxWidgets/passwordsafeframe.cpp
    src/ui/wxWidgets/properties.cpp
    src/ui/wxWidgets/CreateShortcutDlg.cpp
    src/ui/wxWidgets/DeleteConfirmationDlg.cpp
    src/ui/wxWidgets/DragBarGenericCtrl.cpp
    src/ui/wxWidgets/EditShortcutDlg.cpp
    src/ui/wxWidgets/FieldSelectionDlg.cpp
    src/ui/wxWidgets/FieldSelectionPanel.cpp
    src/ui/wxWidgets/pwsmenushortcuts.cpp
    src/ui/wxWidgets/safecombinationchange.cpp
    src/ui/wxWidgets/ExternalKeyboardButton.cpp
    src/ui/wxWidgets/PasswordPolicy.cpp
    src/ui/wxWidgets/DbSelectionPanel.cpp
    src/ui/wxWidgets/pwsafeapp.cpp
    src/ui/wxWidgets/safecombinationsetup.cpp
    src/ui/wxWidgets/wxutils.cpp
    src/ui/wxWidgets/AdvancedSelectionDlg.cpp
    src/ui/wxWidgets/ComparisonGridTable.cpp
    src/ui/wxWidgets/ExportTextWarningDlg.cpp
    src/ui/wxWidgets/ImportXmlDlg.cpp
    src/ui/wxWidgets/ManagePasswordPoliciesDlg.cpp
    src/ui/wxWidgets/MergeDlg.cpp
    src/ui/wxWidgets/OpenFilePickerValidator.cpp
    src/ui/wxWidgets/DragBarCtrl.cpp
    src/ui/wxWidgets/PWSgrid.cpp
    src/ui/wxWidgets/PWSgridtable.cpp
    src/ui/wxWidgets/PWStree.cpp
    src/ui/wxWidgets/PwsSync.cpp
    src/ui/wxWidgets/SafeCombinationCtrl.cpp
    src/ui/wxWidgets/SelectionCriteria.cpp
    src/ui/wxWidgets/SizeRestrictedPanel.cpp
    src/ui/wxWidgets/TimedTaskChain.cpp
    src/ui/wxWidgets/passwordsubset.cpp
    src/ui/wxWidgets/CryptKeyEntryDlg.cpp
    )

if(HAVE_YKPERS_H)
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/yubicfg.cpp src/ui/wxWidgets/YubiMixin.cpp)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/yubicfg.h src/ui/wxWidgets/YubiMixin.h)
endif(HAVE_YKPERS_H)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (NOT NO_QR)
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/pwsqrcodedlg.cpp src/ui/wxWidgets/pwsqrencode.cpp)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/pwsqrencode.h src/ui/wxWidgets/pwsqrcodedlg.h)
endif(NOT NO_QR)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND PWSAFE_SRC_LIST src/ui/wxWidgets/pwsqrcodedlg.cpp src/ui/wxWidgets/pwsqrencode.mm)
  list(APPEND PWSAFE_HDR_LIST src/ui/wxWidgets/pwsqrencode.h src/ui/wxWidgets/pwsqrcodedlg.h)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

set (PWSAFE_SRCS ${PWSAFE_SRC_LIST} PARENT_SCOPE)
set (PWSAFE_HDRS ${PWSAFE_HDR_LIST} PARENT_SCOPE)

