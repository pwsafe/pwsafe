/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file AddEditPropSheetDlg.h
 *
 */

#ifndef _ADDEDITPROPSHEETDLG_H_
#define _ADDEDITPROPSHEETDLG_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/propdlg.h>
#include <wx/valgen.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/statline.h>
////@end includes
#include "core/ItemData.h"
#include "core/PWScore.h"

#include "wxUtilities.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
class wxSpinCtrl;
class wxGrid;
class wxDatePickerCtrl;
class wxGridSizer;
class wxBoxSizer;
////@end forward declarations

#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ADDEDITPROPSHEETDLG 10083
#define ID_PANEL_BASIC 10084
#define ID_COMBOBOX_GROUP 10095
#define ID_TEXTCTRL_TITLE 10096
#define ID_TEXTCTRL_USERNAME 10088
#define ID_TEXTCTRL_PASSWORD 10089
#define ID_BUTTON_SHOWHIDE 10090
#define ID_BUTTON_GENERATE 10097
#define ID_TEXTCTRL_PASSWORD2 10091
#define ID_TEXTCTRL_URL 10092
#define ID_GO_BTN 10093
#define ID_TEXTCTRL_EMAIL 10100
#define ID_SEND_BTN 10214
#define ID_TEXTCTRL_NOTES 10098
#define ID_PANEL_ADDITIONAL 10085
#define ID_TEXTCTRL_AUTOTYPE 10094
#define ID_TEXTCTRL_RUN_CMD 10099
#define ID_COMBOBOX_DBC_ACTION 10101
#define ID_COMBOBOX_SDBC_ACTION 10000
#define ID_CHECKBOX_KEEP 10102
#define ID_SPINCTRL_MAX_PW_HIST 10103
#define ID_GRID_PW_HIST 10104
#define ID_BUTTON_CLEAR_HIST 10105
#define ID_BUTTON_COPY_ALL 10106
#define ID_PANEL_DTIME 10086
#define ID_RADIOBUTTON_ON 10107
#define ID_DATECTRL_EXP_DATE 10108
#define ID_RADIOBUTTON_IN 10110
#define ID_SPINCTRL_EXP_TIME 10111
#define ID_CHECKBOX_RECURRING 10112
#define ID_RADIOBUTTON_NEVER 10001
#define ID_PANEL_PPOLICY 10087
#define ID_CHECKBOX42 10115
#define ID_POLICYLIST 10063
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_SPINCTRL5 10126
#define ID_CHECKBOX4 10119
#define ID_SPINCTRL6 10127
#define ID_CHECKBOX5 10120
#define ID_SPINCTRL7 10128
#define ID_CHECKBOX6 10121
#define ID_SPINCTRL8 10129
#define IDC_OWNSYMBOLS 10212
#define ID_RESET_SYMBOLS 10109
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define ID_STATICTEXT_DAYS 11125
#define SYMBOL_ADDEDITPROPSHEETDLG_STYLE wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX | wxDIALOG_MODAL
#define SYMBOL_ADDEDITPROPSHEETDLG_TITLE _("Edit Entry")
#define SYMBOL_ADDEDITPROPSHEETDLG_IDNAME ID_ADDEDITPROPSHEETDLG
#define SYMBOL_ADDEDITPROPSHEETDLG_SIZE wxSize(400, 300)
#define SYMBOL_ADDEDITPROPSHEETDLG_POSITION wxDefaultPosition
////@end control identifiers
#define SYMBOL_ADDPROPSHEETDLG_TITLE _("Add Entry")
#define SYMBOL_EDITPROPSHEETDLG_TITLE _("Edit Entry")
#define SYMBOL_VIEWPROPSHEETDLG_TITLE _("View Entry")
#define SYMBOL_AUTOPROPSHEETDLG_TITLE _("Add, Edit or View Entry")

/*!
 * AddEditPropSheetDlg class declaration
 */

class AddEditPropSheetDlg : public wxPropertySheetDialog
{
  DECLARE_CLASS(AddEditPropSheetDlg)
  DECLARE_EVENT_TABLE()

public:
  enum class SheetType
  {
    ADD,
    EDIT,
    VIEW
  }; // to tweak UI, mainly

  /// Constructor
  // item is nullptr for ADD, otherwise its values are retrieved and displayed
  AddEditPropSheetDlg(wxWindow *parent, PWScore &core,
                      SheetType type, const CItemData *item = nullptr,
                      const wxString &selectedGroup = wxEmptyString,
                      wxWindowID id = SYMBOL_ADDEDITPROPSHEETDLG_IDNAME,
                      const wxString &caption = SYMBOL_AUTOPROPSHEETDLG_TITLE,
                      const wxPoint &pos = SYMBOL_ADDEDITPROPSHEETDLG_POSITION,
                      const wxSize &size = SYMBOL_ADDEDITPROPSHEETDLG_SIZE,
                      long style = SYMBOL_ADDEDITPROPSHEETDLG_STYLE);

  /// Creation
  bool Create(wxWindow *parent, wxWindowID id = SYMBOL_ADDEDITPROPSHEETDLG_IDNAME, const wxString &caption = SYMBOL_AUTOPROPSHEETDLG_TITLE, const wxPoint &pos = SYMBOL_ADDEDITPROPSHEETDLG_POSITION, const wxSize &size = SYMBOL_ADDEDITPROPSHEETDLG_SIZE, long style = SYMBOL_ADDEDITPROPSHEETDLG_STYLE);

  /// Destructor
  ~AddEditPropSheetDlg();

  ////@begin AddEditPropSheetDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
  void OnShowHideClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
  void OnGenerateButtonClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN
  void OnGoButtonClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEND_BTN
  void OnSendButtonClick(wxCommandEvent &event);

  /// wxEVT_SET_FOCUS event handler for ID_TEXTCTRL_NOTES
  void OnNoteSetFocus(wxFocusEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_KEEP
  void OnKeepHistoryClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
  void OnExpRadiobuttonSelected(wxCommandEvent &event);

  /// wxEVT_DATE_CHANGED event handler for ID_DATECTRL_EXP_DATE
  void OnExpDateChanged(wxDateEvent &event);

  /// wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_SPINCTRL_EXP_TIME
  void OnExpIntervalChanged(wxSpinEvent &event);

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CHECKBOX42
  void OnPasswordPolicySelected(wxCommandEvent &event);

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_POLICYLIST
  void OnPolicylistSelected(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
  void OnLowercaseCB(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
  void OnUppercaseCB(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX5
  void OnDigitsCB(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
  void OnSymbolsCB(wxCommandEvent &event);

  /// wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS
  void OnOwnSymSetFocus(wxFocusEvent &event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESET_SYMBOLS
  void OnResetSymbolsClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
  void OnEasyReadCBClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX8
  void OnPronouceableCBClick(wxCommandEvent &event);

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX9
  void OnUseHexCBClick(wxCommandEvent &event);

  /// wxEVT_UPDATE_UI event handler for all command ids
  void OnUpdateUI(wxUpdateUIEvent &event);

  ////@end AddEditPropSheetDlg event handler declarations
  void OnEasyReadOrPronounceable(wxCommandEvent &event);
  void OnClearPasswordHistory(wxCommandEvent &event);
  void OnOk(wxCommandEvent &event);

  /// wxEVT_SPINCTRL event handler for ID_SPINCTRL5, ID_SPINCTRL6, ID_SPINCTRL7, ID_SPINCTRL8
  void OnAtLeastPasswordChars(wxSpinEvent &event);
  ////@begin AddEditPropSheetDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource(const wxString &name);

  /// Retrieves icon resources
  wxIcon GetIconResource(const wxString &name);
  ////@end AddEditPropSheetDlg member function declarations

  const CItemData &GetItem() const { return m_Item; } // for ADD mode

  /// Should we show tooltips?
  static bool ShowToolTips();

  ////@begin AddEditPropSheetDlg member variables

private:
  // Initialises member variables
  void Init();

  // Creates the controls and sizers
  void CreateControls();

  // Creates the controls of 'Basic' tab
  wxPanel *CreateBasicPanel();

  // Creates the controls of 'Additional' tab
  wxPanel *CreateAdditionalPanel();

  // Creates the controls of 'DatesTimes' tab
  wxPanel *CreateDatesTimesPanel();

  // Creates the controls of 'Password Policy' tab
  wxPanel *CreatePasswordPolicyPanel();

  // Creates the controls of 'Attachment' tab
  wxPanel *CreateAttachmentPanel();

  void InitAttachmentTab();
  void ShowAttachmentData(const CItemAtt &attachmentItem);
  void ResetAttachmentData();
  bool LoadImagePreview(const CItemAtt &itemAttachment);
  void ShowImagePreview();
  void HideImagePreview(const wxString &reason = _("No preview available"));
  bool IsFileMimeTypeImage(const wxString &filename);
  bool IsMimeTypeImage(const stringT &mimeTypeDescription);
  wxString GetMimeTypeExtension(const stringT &mimeTypeDescription);
  void EnableImport();
  void DisableImport();

  // Applies font preferences to corresponding controls
  void ApplyFontPreferences();

  void ItemFieldsToPropSheet();
  void SetupDCAComboBoxes(wxComboBox *pcbox, short &iDCA, bool isShift);
  void UpdateExpTimes();        // entry -> controls
  void SetXTime(wxObject *src); // sync controls + controls -> entry
  void UpdatePWPolicyControls(const PWPolicy &pwp);
  void EnablePWPolicyControls(bool enable);
  PWPolicy GetPWPolicyFromUI();
  PWPolicy GetSelectedPWPolicy();
  void ShowPWPSpinners(bool show);
  void EnableNonHexCBs(bool enable);
  void ShowPassword();
  void HidePassword();
  int GetRequiredPWLength() const;

  // Tab: "Basic"
  wxPanel *m_BasicPanel;
  wxGridBagSizer *m_BasicSizer;
  wxComboBox *m_BasicGroupNamesCtrl;
  wxTextCtrl *m_BasicTitleTextCtrl;
  wxTextCtrl *m_BasicUsernameTextCtrl;
  wxTextCtrl *m_BasicPasswordTextCtrl;
  wxButton *m_BasicShowHideCtrl;
  wxTextCtrl *m_BasicPasswordConfirmationTextCtrl;
  wxTextCtrl *m_BasicUrlTextCtrl;
  wxTextCtrl *m_BasicEmailTextCtrl;
  wxTextCtrl *m_BasicNotesTextCtrl;

  wxString m_Title;
  wxString m_User;
  wxString m_Url;
  wxString m_Email;
  wxString m_Notes;
  bool m_IsNotesHidden;
  StringX m_Password;
  bool m_IsPasswordHidden;

  // Tab: "Additional"
  wxPanel *m_AdditionalPanel;
  wxComboBox *m_AdditionalDoubleClickActionCtrl;
  wxComboBox *m_AdditionalShiftDoubleClickActionCtrl;
  wxSpinCtrl *m_AdditionalMaxPasswordHistoryCtrl;
  wxGrid *m_AdditionalPasswordHistoryGrid;

  wxString m_Autotype;
  wxString m_RunCommand;
  wxString m_PasswordHistory; // String as stored in CItemData
  bool m_KeepPasswordHistory;
  int m_MaxPasswordHistory; // How many passwords to keep
  short m_DoubleClickAction;
  short m_ShiftDoubleClickAction;

  // Tab: "Dates and Times"
  wxRadioButton *m_DatesTimesExpireOnCtrl;
  wxDatePickerCtrl *m_DatesTimesExpiryDateCtrl;
  wxRadioButton *m_DatesTimesExpireInCtrl;
  wxSpinCtrl *m_DatesTimesExpiryTimeCtrl;
  wxCheckBox *m_DatesTimesRecurringExpiryCtrl;
  wxRadioButton *m_DatesTimesNeverExpireCtrl;

  wxString m_RMTime; // Any field modification time
  wxString m_AccessTime;
  wxString m_CreationTime;
  wxString m_CurrentExpirationTime;
  wxString m_ModificationTime;
  bool m_Recurring;
  wxString m_ExpirationTime;
  int m_ExpirationTimeInterval; // Password expiration interval in days
  time_t m_tttExpirationTime;   // Password expiration date in time_t

  // Tab: "Password Policy"
  wxPanel *m_PasswordPolicyPanel;
  wxCheckBox *m_PasswordPolicyUseDatabaseCtrl;
  wxComboBox *m_PasswordPolicyNamesCtrl;
  wxSpinCtrl *m_PasswordPolicyPasswordLengthCtrl;
  wxGridSizer *m_PasswordPolicySizer;
  wxCheckBox *m_PasswordPolicyUseLowerCaseCtrl;
  wxBoxSizer *m_PasswordPolicyLowerCaseMinSizer;
  wxSpinCtrl *m_PasswordPolicyLowerCaseMinCtrl;
  wxCheckBox *m_PasswordPolicyUseUpperCaseCtrl;
  wxBoxSizer *m_PasswordPolicyUpperCaseMinSizer;
  wxSpinCtrl *m_PasswordPolicyUpperCaseMinCtrl;
  wxCheckBox *m_PasswordPolicyUseDigitsCtrl;
  wxBoxSizer *m_PasswordPolicyDigitsMinSizer;
  wxSpinCtrl *m_PasswordPolicyDigitsMinCtrl;
  wxCheckBox *m_PasswordPolicyUseSymbolsCtrl;
  wxBoxSizer *m_PasswordPolicySymbolsMinSizer;
  wxSpinCtrl *m_PasswordPolicySymbolsMinCtrl;
  wxTextCtrl *m_PasswordPolicyOwnSymbolsTextCtrl;
  wxCheckBox *m_PasswordPolicyUseEasyCtrl;
  wxCheckBox *m_PasswordPolicyUsePronounceableCtrl;
  wxCheckBox *m_PasswordPolicyUseHexadecimalOnlyCtrl;

  wxString m_Symbols;

  // Tab: "Attachment"

  //(*Handlers(AttachmentTab)
  void OnImport(wxCommandEvent& event);
  void OnExport(wxCommandEvent& event);
  void OnRemove(wxCommandEvent& event);
  //*)

  //(*Identifiers(AttachmentTab)
  static const long ID_IMAGEPANEL1;
  static const long ID_STATICTEXT1;
  static const long ID_BUTTON_IMPORT;
  static const long ID_BUTTON_EXPORT;
  static const long ID_BUTTON_REMOVE;
  static const long ID_TEXTCTRL2;
  static const long ID_STATICTEXT4;
  static const long ID_STATICTEXT5;
  static const long ID_STATICTEXT6;
  static const long ID_STATICTEXT8;
  static const long ID_STATICTEXT10;
  //*)

  //(*Declarations(AttachmentTab)
  wxStaticBoxSizer *StaticBoxSizerPreview;
  wxPanel *m_AttachmentPanel;
  ImagePanel *m_AttachmentImagePanel;
  wxButton *m_AttachmentButtonImport;
  wxButton *m_AttachmentButtonExport;
  wxButton *m_AttachmentButtonRemove;
  wxStaticText *m_AttachmentFilePath;
  wxTextCtrl *m_AttachmentTitle;
  wxStaticText *m_AttachmentMediaType;
  wxStaticText *m_AttachmentCreationDate;
  wxStaticText *m_AttachmentFileSize;
  wxStaticText *m_AttachmentFileCreationDate;
  wxStaticText *m_AttachmentFileLastModifiedDate;
  wxStaticText *m_AttachmentPreviewStatus;
  //*)
  ////@end AddEditPropSheetDlg member variables

  PWScore &m_Core;
  wxString m_SelectedGroup; // Group title in tree view user right-clicked on to add an item

  SheetType m_Type;
  CItemData m_Item;
  CItemAtt  m_ItemAttachment;
};

#endif // _ADDEDITPROPSHEETDLG_H_
