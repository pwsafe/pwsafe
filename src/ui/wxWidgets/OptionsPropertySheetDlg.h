/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _OPTIONSPROPSHEET_H_
#define _OPTIONSPROPSHEET_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/propdlg.h>
#include <wx/valgen.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include "core/PWScore.h" // for password history actions
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSpinCtrl;
class wxBookCtrlEvent;
////@end forward declarations

/*!
 * Control identifiers
 */

#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

////@begin control identifiers
#define ID_OPTIONS 10130
#define ID_PANEL 10131
#define ID_CHECKBOX10 10139
#define ID_CHECKBOX11 10140
#define ID_RADIOBUTTON4 10141
#define ID_RADIOBUTTON5 10142
#define ID_TEXTCTRL9 10143
#define ID_COMBOBOX2 10144
#define ID_SPINCTRL9 10145
#define ID_RADIOBUTTON6 10146
#define ID_RADIOBUTTON7 10147
#define ID_TEXTCTRL10 10148
#define ID_BUTTON 10149
#define ID_PANEL1 10132
#define ID_CHECKBOX12 10150
#define ID_CHECKBOX13 10151
#define ID_CHECKBOX14 10152
#define ID_CHECKBOX15 10153
#define ID_CHECKBOX16 10154
#define ID_CHECKBOX17 10155
#define ID_CHECKBOX18 10156
#define ID_CHECKBOX38 10209
#define ID_CHECKBOX19 10157
#define ID_SPINCTRL10 10158
#define ID_RADIOBOX 10159
#define ID_PANEL2 10133
#define ID_CHECKBOX20 10160
#define ID_CHECKBOX21 10161
#define ID_CHECKBOX22 10162
#define ID_COMBOBOX3 10163
#define ID_COMBOBOX 10006
#define ID_CHECKBOX23 10164
#define ID_TEXTCTRL11 10165
#define ID_CHECKBOX24 10166
#define ID_TEXTCTRL12 10167
#define ID_CHECKBOX25 10168
#define ID_TEXTCTRL13 10169
#define ID_BUTTON8 10170
#define ID_TEXTCTRL14 10171
#define ID_PANEL4 10135
#define ID_CHECKBOX26 10172
#define ID_SPINCTRL11 10173
#define ID_PWHISTNOCHANGE 10178
#define ID_PWHISTSTOP 10175
#define ID_PWHISTSTART 10176
#define ID_PWHISTSETMAX 10177
#define ID_PWHISTCLEAR 10188
#define ID_APPLYTOPROTECTED 10189
#define ID_PANEL5 10136
#define ID_CHECKBOX27 10179
#define ID_CHECKBOX 10000
#define ID_CHECKBOX1 10001
#define ID_CHECKBOX2 10002
#define ID_CHECKBOX28 10003
#define ID_CHECKBOX29 10180
#define ID_SPINCTRL12 10181
#define ID_SLIDER 10059
#define ID_PANEL6 10137
#define ID_CHECKBOX30 10182
#define ID_SPINCTRL13 10183
#define ID_SPINCTRL14 11183
#define ID_CHECKBOX31 10184
#define ID_SPINCTRL 10004
#define ID_CHECKBOX32 10185
#define ID_CHECKBOX33 10186
#define ID_CHECKBOX34 10005
#define ID_CHECKBOX35 10336
#define ID_CHECKBOX39 10010
#define ID_CHECKBOX40 10114
#define ID_PANEL7 10138
#define ID_GRID1 10187
#define ID_STATICTEXT_1 10190
#define ID_STATICTEXT_2 10191
#define ID_STATICTEXT_3 10192
#define ID_STATICTEXT_4 10193
#define ID_STATICTEXT_5 10194
#define ID_STATICTEXT_7 10196
#define ID_STATICTEXT_8 10197
#define ID_STATICTEXT_9 11197
#define ID_STATICTEXT_10 11198
#define ID_STATICBOX_1 10198
#define ID_PWHISTAPPLY 10199
#define SYMBOL_COPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_COPTIONS_TITLE _("Options")
#define SYMBOL_COPTIONS_IDNAME ID_OPTIONS
#define SYMBOL_COPTIONS_SIZE wxSize(400, 300)
#define SYMBOL_COPTIONS_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Options class declaration
 */

class COptions: public wxPropertySheetDialog
{
  DECLARE_CLASS( COptions )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  COptions(PWScore &core);
  COptions( wxWindow* parent, PWScore &core, wxWindowID id = SYMBOL_COPTIONS_IDNAME, const wxString& caption = SYMBOL_COPTIONS_TITLE, const wxPoint& pos = SYMBOL_COPTIONS_POSITION, const wxSize& size = SYMBOL_COPTIONS_SIZE, long style = SYMBOL_COPTIONS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COPTIONS_IDNAME, const wxString& caption = SYMBOL_COPTIONS_TITLE, const wxPoint& pos = SYMBOL_COPTIONS_POSITION, const wxSize& size = SYMBOL_COPTIONS_SIZE, long style = SYMBOL_COPTIONS_STYLE );

  /// Destructor
  ~COptions();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin COptions event handler declarations

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX11
  void OnBackupB4SaveClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON4
  void OnBuPrefix( wxCommandEvent& event );

  /// wxEVT_SET_FOCUS event handler for ID_TEXTCTRL9
  void OnBuPrefixTxtSetFocus( wxFocusEvent& event );

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX2
  void OnSuffixCBSet( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
  void OnBuDirBrowseClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
  void OnShowUsernameInTreeCB( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX19
  void OnPreExpiryWarnClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX24
  void OnUseDefaultUserClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
  void OnBrowseLocationClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PWHISTAPPLY
  void OnPWHistApply( wxCommandEvent& event );

  /// wxEVT_UPDATE_UI event handler for all command ids
  void OnUpdateUI(wxUpdateUIEvent& evt);

////@end COptions event handler declarations

  /// wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGING event handler for all pages (wxID_ANY)
  void OnPageChanging(wxBookCtrlEvent& evt);

////@begin COptions member function declarations
  uint32 GetHashItersValue() const { return m_hashIterValue; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end COptions member function declarations
  void OnOk(wxCommandEvent& evt);
  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin COptions member variables
private:
  wxRadioButton*  m_Backup_DefaultPrefixRB;
  wxRadioButton*  m_Backup_UserPrefixRB;
  wxTextCtrl*     m_Backup_UserPrefixTXT;
  wxComboBox*     m_Backup_SuffixCB;
  wxSpinCtrl*     m_Backup_MaxIncrSB;
  wxStaticText*   m_Backup_SuffixExampleST;
  wxRadioButton*  m_Backup_DefaultDirRB;
  wxRadioButton*  m_Backup_UserDirRB;
  wxTextCtrl*     m_Backup_UserDirTXT;
  wxButton*       m_Backup_DirBN;

  wxCheckBox*     m_Display_ShowPasswordInTreeCB;
  wxCheckBox*     m_Display_PreExpiryWarnCB;
  wxSpinCtrl*     m_Display_PreExpiryWarnDaysSB;

  wxComboBox*     m_Misc_DoubleClickActionCB;
  wxComboBox*     m_Misc_ShiftDoubleClickActionCB;
  wxTextCtrl*     m_Misc_DefaultUsernameTXT;
  wxStaticText*   m_Misc_DefaultUsernameLBL;
  wxString        m_Misc_OtherBrowserLocationparams;

  wxCheckBox*     m_Security_LockOnIdleTimeoutCB;
  wxSpinCtrl*     m_Security_IdleTimeoutSB;

  wxCheckBox*     m_System_UseSystemTrayCB;
  wxSpinCtrl*     m_System_MaxREItemsSB;
  wxStaticText*   m_System_SystemTrayWarningST;

  wxCheckBox*     m_PasswordHistory_SaveCB;
  wxSpinCtrl*     m_PasswordHistory_NumDefaultSB;
  wxSpinCtrl*     m_PasswordHistory_DefaultExpiryDaysSB;
  wxRadioButton*  m_PasswordHistory_NoChangeRB;
  wxRadioButton*  m_PasswordHistory_StopRB;
  wxRadioButton*  m_PasswordHistory_StartRB;
  wxRadioButton*  m_PasswordHistory_SetMaxRB;
  wxRadioButton*  m_PasswordHistory_ClearRB;
  wxButton*       m_PasswordHistory_ApplyBN;
  wxCheckBox*     m_PasswordHistory_Apply2ProtectedCB;

  uint32 m_hashIterValue;
  int m_DoubleClickAction;
  int m_ShiftDoubleClickAction;

  // Backups Preferences
  bool m_Backup_SaveImmediately;
  bool m_Backup_BackupBeforeSave;

  // Display Preferences
  bool m_Display_AlwaysOnTop;
  bool m_Display_ShowUsernameInTree;
  bool m_Display_ShowNotesAsTipsInViews;
  bool m_Display_ShowPasswordInEdit;
  bool m_Display_ShowNotesInEdit;
  bool m_Display_WordWrapNotes;
  bool m_Display_GroupsFirst;
  bool m_Display_PreExpiryWarn;
  int  m_Display_TreeDisplayStatusAtOpen;

  // Misc. Preferences
  bool m_Misc_ConfirmDelete;
  bool m_Misc_MaintainDatetimeStamps;
  bool m_Misc_EscExits;
  bool m_Misc_AutotypeMinimize;
  wxString m_Misc_AutotypeString;
  bool m_Misc_UseDefUsername;
  bool m_Misc_QuerySetDefUsername;
  wxString m_Misc_OtherBrowserLocation;

  // Security Preferences
  bool m_Security_ClearClipboardOnMinimize;
  bool m_Security_ClearClipboardOnExit;
  bool m_Security_ConfirmCopy;
  bool m_Security_CopyPswdBrowseURL;
  bool m_Security_LockOnMinimize;
  bool m_Security_LockOnWindowLock;
  bool m_Security_LockOnIdleTimeout;
  int  m_Security_HashIterSlider;

  // System Preferences
  bool m_System_Startup;
  int  m_System_MaxMRUItems;
  bool m_System_MRUOnFileMenu;
  bool m_System_DefaultOpenRO;
  bool m_System_MultipleInstances;
  bool m_System_UseAltAutoType;
#if defined(__X__) || defined(__WXGTK__)
  bool m_System_UsePrimarySelection;
#endif

  // Password History Preferences
  bool m_PasswordHistory_Save;
  int  m_PasswordHistory_NumDefault;
  int  m_PasswordHistory_DefaultExpiryDays;

  PWScore &m_core;
////@end COptions member variables

private:
  void PrefsToPropSheet();
  void PropSheetToPrefs();
  int GetRequiredPWLength() const;
};

#endif // _OPTIONSPROPSHEET_H_
