/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "wx/propdlg.h"
#include "wx/valgen.h"
#include "wx/statline.h"
#include "wx/spinctrl.h"
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSpinCtrl;
class wxGridSizer;
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

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
#define ID_CHECKBOX19 10157
#define ID_SPINCTRL10 10158
#define ID_RADIOBOX 10159
#define ID_PANEL2 10133
#define ID_CHECKBOX20 10160
#define ID_CHECKBOX21 10161
#define ID_CHECKBOX22 10162
#define ID_COMBOBOX3 10163
#define ID_CHECKBOX23 10164
#define ID_TEXTCTRL11 10165
#define ID_CHECKBOX24 10166
#define ID_TEXTCTRL12 10167
#define ID_CHECKBOX25 10168
#define ID_TEXTCTRL13 10169
#define ID_BUTTON8 10170
#define ID_TEXTCTRL14 10171
#define ID_PANEL3 10134
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_SPINCTRL5 10126
#define ID_CHECKBOX4 10119
#define ID_SPINCTRL6 10127
#define ID_CHECKBOX5 10120
#define ID_SPINCTRL7 10128
#define ID_CHECKBOX6 10121
#define ID_SPINCTRL8 10129
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define ID_PANEL4 10135
#define ID_CHECKBOX26 10172
#define ID_SPINCTRL11 10173
#define ID_PWHISTNOCHANGE 10178
#define ID_PWHISTSTOP 10175
#define ID_PWHISTSTART 10176
#define ID_PWHISTSETMAX 10177
#define ID_PANEL5 10136
#define ID_CHECKBOX27 10179
#define ID_CHECKBOX 10000
#define ID_CHECKBOX1 10001
#define ID_CHECKBOX2 10002
#define ID_CHECKBOX28 10003
#define ID_CHECKBOX29 10180
#define ID_SPINCTRL12 10181
#define ID_PANEL6 10137
#define ID_CHECKBOX30 10182
#define ID_SPINCTRL13 10183
#define ID_CHECKBOX31 10184
#define ID_SPINCTRL 10004
#define ID_CHECKBOX32 10185
#define ID_CHECKBOX33 10186
#define ID_CHECKBOX34 10005
#define ID_PANEL7 10138
#define ID_GRID1 10187
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
  COptions();
  COptions( wxWindow* parent, wxWindowID id = SYMBOL_COPTIONS_IDNAME, const wxString& caption = SYMBOL_COPTIONS_TITLE, const wxPoint& pos = SYMBOL_COPTIONS_POSITION, const wxSize& size = SYMBOL_COPTIONS_SIZE, long style = SYMBOL_COPTIONS_STYLE );

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

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON6
  void OnBuDirRB( wxCommandEvent& event );

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

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
  void OnPwPolUseClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX26
  void OnPWHistSaveClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PWHISTNOCHANGE
  void OnPWHistRB( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PWHISTNOCHANGE
  void OnPWHistApply( wxCommandEvent& event );

////@end COptions event handler declarations

////@begin COptions member function declarations

  bool GetSaveimmediate() const { return m_saveimmediate ; }
  void SetSaveimmediate(bool value) { m_saveimmediate = value ; }

  bool GetBackupb4save() const { return m_backupb4save ; }
  void SetBackupb4save(bool value) { m_backupb4save = value ; }

  bool GetAlwaysontop() const { return m_alwaysontop ; }
  void SetAlwaysontop(bool value) { m_alwaysontop = value ; }

  bool GetShowusernameintree() const { return m_showusernameintree ; }
  void SetShowusernameintree(bool value) { m_showusernameintree = value ; }

  bool GetShownotesastipsinviews() const { return m_shownotesastipsinviews ; }
  void SetShownotesastipsinviews(bool value) { m_shownotesastipsinviews = value ; }

  bool GetPwshowinedit() const { return m_pwshowinedit ; }
  void SetPwshowinedit(bool value) { m_pwshowinedit = value ; }

  bool GetWordwrapnotes() const { return m_wordwrapnotes ; }
  void SetWordwrapnotes(bool value) { m_wordwrapnotes = value ; }

  int GetInittreeview() const { return m_inittreeview ; }
  void SetInittreeview(int value) { m_inittreeview = value ; }

  bool GetPreexpirywarn() const { return m_preexpirywarn ; }
  void SetPreexpirywarn(bool value) { m_preexpirywarn = value ; }

  bool GetNotesshowinedit() const { return m_notesshowinedit ; }
  void SetNotesshowinedit(bool value) { m_notesshowinedit = value ; }

  bool GetConfirmdelete() const { return m_confirmdelete ; }
  void SetConfirmdelete(bool value) { m_confirmdelete = value ; }

  bool GetMaintaindatetimestamps() const { return m_maintaindatetimestamps ; }
  void SetMaintaindatetimestamps(bool value) { m_maintaindatetimestamps = value ; }

  bool GetEscexits() const { return m_escexits ; }
  void SetEscexits(bool value) { m_escexits = value ; }

  int GetDoubleclickaction() const { return m_doubleclickaction ; }
  void SetDoubleclickaction(int value) { m_doubleclickaction = value ; }

  bool GetMinauto() const { return m_minauto ; }
  void SetMinauto(bool value) { m_minauto = value ; }

  wxString GetAutotypeStr() const { return m_autotypeStr ; }
  void SetAutotypeStr(wxString value) { m_autotypeStr = value ; }

  bool GetUsedefuser() const { return m_usedefuser ; }
  void SetUsedefuser(bool value) { m_usedefuser = value ; }

  bool GetQuerysetdef() const { return m_querysetdef ; }
  void SetQuerysetdef(bool value) { m_querysetdef = value ; }

  wxString GetOtherbrowser() const { return m_otherbrowser ; }
  void SetOtherbrowser(wxString value) { m_otherbrowser = value ; }

  wxString GetOtherbrowserparams() const { return m_otherbrowserparams ; }
  void SetOtherbrowserparams(wxString value) { m_otherbrowserparams = value ; }

  int GetPwdefaultlength() const { return m_pwdefaultlength ; }
  void SetPwdefaultlength(int value) { m_pwdefaultlength = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end COptions member function declarations
  void OnOk(wxCommandEvent& event);
  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin COptions member variables
  wxRadioButton* m_dfltbuprefixRB;
  wxRadioButton* m_usrbuprefixRB;
  wxTextCtrl* m_usrbuprefixTxt;
  wxComboBox* m_busuffixCB;
  wxSpinCtrl* m_bumaxinc;
  wxStaticText* m_suffixExample;
  wxRadioButton* m_dfltbudirRB;
  wxRadioButton* m_usrbudirRB;
  wxTextCtrl* m_usrbudirTxt;
  wxButton* m_buDirBN;
  wxCheckBox* m_showpasswordintreeCB;
  wxCheckBox* m_preexpirywarnCB;
  wxSpinCtrl* m_preexpirywarndaysSB;
  wxComboBox* m_DCACB;
  wxTextCtrl* m_defusernameTXT;
  wxStaticText* m_defusernameLBL;
  wxGridSizer* m_pwMinsGSzr;
  wxCheckBox* m_pwpUseLowerCtrl;
  wxBoxSizer* m_pwNumLCbox;
  wxSpinCtrl* m_pwpLCSpin;
  wxCheckBox* m_pwpUseUpperCtrl;
  wxBoxSizer* m_pwNumUCbox;
  wxSpinCtrl* m_pwpUCSpin;
  wxCheckBox* m_pwpUseDigitsCtrl;
  wxBoxSizer* m_pwNumDigbox;
  wxSpinCtrl* m_pwpDigSpin;
  wxCheckBox* m_pwpSymCtrl;
  wxBoxSizer* m_pwNumSymbox;
  wxSpinCtrl* m_pwpSymSpin;
  wxCheckBox* m_pwpEasyCtrl;
  wxCheckBox* m_pwpPronounceCtrl;
  wxCheckBox* m_pwpHexCtrl;
  wxCheckBox* m_pwhistsaveCB;
  wxSpinCtrl* m_pwhistnumdfltSB;
  wxButton* m_pwhistapplyBN;
  wxString m_otherbrowserparams;
private:
  bool m_saveimmediate;
  bool m_backupb4save;
  bool m_alwaysontop;
  bool m_showusernameintree;
  bool m_shownotesastipsinviews;
  bool m_pwshowinedit;
  bool m_wordwrapnotes;
  int m_inittreeview;
  bool m_preexpirywarn;
  bool m_notesshowinedit;
  bool m_confirmdelete;
  bool m_maintaindatetimestamps;
  bool m_escexits;
  int m_doubleclickaction;
  bool m_minauto;
  wxString m_autotypeStr;
  bool m_usedefuser;
  bool m_querysetdef;
  wxString m_otherbrowser;
  int m_pwdefaultlength;
////@end COptions member variables
 private:
  void PrefsToPropSheet();
  void PropSheetToPrefs();
};

#endif
  // _OPTIONSPROPSHEET_H_
