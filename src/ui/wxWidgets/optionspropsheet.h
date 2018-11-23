/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
////@end forward declarations
class wxBookCtrlEvent;

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

  bool GetAlwaysontop() const { return m_alwaysontop ; }
  void SetAlwaysontop(bool value) { m_alwaysontop = value ; }

  wxString GetAutotypeStr() const { return m_autotypeStr ; }
  void SetAutotypeStr(wxString value) { m_autotypeStr = value ; }

  bool GetBackupb4save() const { return m_backupb4save ; }
  void SetBackupb4save(bool value) { m_backupb4save = value ; }

  bool GetConfirmdelete() const { return m_confirmdelete ; }
  void SetConfirmdelete(bool value) { m_confirmdelete = value ; }

  int GetDoubleclickaction() const { return m_doubleclickaction ; }
  void SetDoubleclickaction(int value) { m_doubleclickaction = value ; }

  bool GetEscexits() const { return m_escexits ; }
  void SetEscexits(bool value) { m_escexits = value ; }

  int GetHashIterSlider() const { return m_hashIterSlider ; }
  void SetHashIterSlider(int value) { m_hashIterSlider = value ; }

  int GetInittreeview() const { return m_inittreeview ; }
  void SetInittreeview(int value) { m_inittreeview = value ; }

  bool GetMaintaindatetimestamps() const { return m_maintaindatetimestamps ; }
  void SetMaintaindatetimestamps(bool value) { m_maintaindatetimestamps = value ; }

  bool GetMinauto() const { return m_minauto ; }
  void SetMinauto(bool value) { m_minauto = value ; }

  bool GetNotesshowinedit() const { return m_notesshowinedit ; }
  void SetNotesshowinedit(bool value) { m_notesshowinedit = value ; }

  wxString GetOtherbrowser() const { return m_otherbrowser ; }
  void SetOtherbrowser(wxString value) { m_otherbrowser = value ; }

  wxString GetOtherbrowserparams() const { return m_otherbrowserparams ; }
  void SetOtherbrowserparams(wxString value) { m_otherbrowserparams = value ; }

  bool GetPreexpirywarn() const { return m_preexpirywarn ; }
  void SetPreexpirywarn(bool value) { m_preexpirywarn = value ; }

  bool GetPutgroups1st() const { return m_putgroups1st ; }
  void SetPutgroups1st(bool value) { m_putgroups1st = value ; }

  int GetPwdefaultlength() const { return m_pwdefaultlength ; }
  void SetPwdefaultlength(int value) { m_pwdefaultlength = value ; }

  bool GetPwHistSave() const { return m_pwhistsave ; }
  void SetPwHistSave(bool value) { m_pwhistsave = value ;}

  int GetPwHistNumDefault() const { return m_pwhistnumdflt ; }
  void SetPwHistNumDefault(int value) { m_pwhistnumdflt = value ; }

  bool GetPwshowinedit() const { return m_pwshowinedit ; }
  void SetPwshowinedit(bool value) { m_pwshowinedit = value ; }

  bool GetQuerysetdef() const { return m_querysetdef ; }
  void SetQuerysetdef(bool value) { m_querysetdef = value ; }

  bool GetSaveimmediate() const { return m_saveimmediate ; }
  void SetSaveimmediate(bool value) { m_saveimmediate = value ; }

  bool GetSecclrclponexit() const { return m_secclrclponexit ; }
  void SetSecclrclponexit(bool value) { m_secclrclponexit = value ; }

  bool GetSecclrclponmin() const { return m_secclrclponmin ; }
  void SetSecclrclponmin(bool value) { m_secclrclponmin = value ; }

  bool GetSecconfrmcpy() const { return m_secconfrmcpy ; }
  void SetSecconfrmcpy(bool value) { m_secconfrmcpy = value ; }

  bool GetSeclockonmin() const { return m_seclockonmin ; }
  void SetSeclockonmin(bool value) { m_seclockonmin = value ; }

  bool GetSeclockonwinlock() const { return m_seclockonwinlock ; }
  void SetSeclockonwinlock(bool value) { m_seclockonwinlock = value ; }

  int GetShiftdoubleclickaction() const { return m_shiftdoubleclickaction ; }
  void SetShiftdoubleclickaction(int value) { m_shiftdoubleclickaction = value ; }

  bool GetShownotesastipsinviews() const { return m_shownotesastipsinviews ; }
  void SetShownotesastipsinviews(bool value) { m_shownotesastipsinviews = value ; }

  bool GetShowusernameintree() const { return m_showusernameintree ; }
  void SetShowusernameintree(bool value) { m_showusernameintree = value ; }

  bool GetSysdefopenro() const { return m_sysdefopenro ; }
  void SetSysdefopenro(bool value) { m_sysdefopenro = value ; }

  int GetSysmaxmru() const { return m_sysmaxmru ; }
  void SetSysmaxmru(int value) { m_sysmaxmru = value ; }

  bool GetSysmruonfilemenu() const { return m_sysmruonfilemenu ; }
  void SetSysmruonfilemenu(bool value) { m_sysmruonfilemenu = value ; }

  bool GetSysmultinst() const { return m_sysmultinst ; }
  void SetSysmultinst(bool value) { m_sysmultinst = value ; }

  bool GetSysstartup() const { return m_sysstartup ; }
  void SetSysstartup(bool value) { m_sysstartup = value ; }

  bool GetUsedefuser() const { return m_usedefuser ; }
  void SetUsedefuser(bool value) { m_usedefuser = value ; }

  bool GetWordwrapnotes() const { return m_wordwrapnotes ; }
  void SetWordwrapnotes(bool value) { m_wordwrapnotes = value ; }

  bool GetUseAltAutoType() const { return m_useAltAutoType ; }
  void SetUseAltAutoType(bool value) { m_useAltAutoType = value ; }

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
  wxComboBox* m_SDCACB;
  wxTextCtrl* m_defusernameTXT;
  wxStaticText* m_defusernameLBL;
  wxCheckBox* m_pwhistsaveCB;
  wxSpinCtrl* m_pwhistnumdfltSB;
  wxSpinCtrl* m_pwhdefexpdaysSB;
  wxRadioButton* m_pwhistnochangeRB;
  wxRadioButton* m_pwhiststopRB;
  wxRadioButton* m_pwhiststartRB;
  wxRadioButton* m_pwhistsetmaxRB;
  wxRadioButton* m_pwhistclearRB;
  wxButton* m_pwhistapplyBN;
  wxCheckBox* m_applytoprotectedCB;
  wxCheckBox* m_seclockonidleCB;
  wxSpinCtrl* m_secidletimeoutSB;
  wxCheckBox* m_sysusesystrayCB;
  wxSpinCtrl* m_sysmaxREitemsSB;
  wxString m_otherbrowserparams;
  wxStaticText *m_systrayWarning;
private:
  bool m_alwaysontop;
  wxString m_autotypeStr;
  bool m_backupb4save;
  bool m_confirmdelete;
  int m_doubleclickaction;
  bool m_escexits;
  int m_hashIterSlider;
  int m_inittreeview;
  bool m_maintaindatetimestamps;
  bool m_minauto;
  bool m_notesshowinedit;
  wxString m_otherbrowser;
  bool m_preexpirywarn;
  bool m_putgroups1st;
  int m_pwdefaultlength;
  bool m_pwhistsave;
  int m_pwhistnumdflt;
  int m_pwhdefexpdays;
  bool m_pwshowinedit;
  bool m_querysetdef;
  bool m_saveimmediate;
  bool m_secclrclponexit;
  bool m_secclrclponmin;
  bool m_secconfrmcpy;
  bool m_seclockonmin;
  bool m_seclockonwinlock;
  int m_shiftdoubleclickaction;
  bool m_shownotesastipsinviews;
  bool m_showusernameintree;
  bool m_sysdefopenro;
  int m_sysmaxmru;
  bool m_sysmruonfilemenu;
  bool m_sysmultinst;
  bool m_sysstartup;
  bool m_usedefuser;
  bool m_wordwrapnotes;
  bool m_useAltAutoType;
  PWScore &m_core;
////@end COptions member variables
  uint32 m_hashIterValue;
#if defined(__X__) || defined(__WXGTK__)
  bool m_usePrimarySelection;
#endif
 private:
  void PrefsToPropSheet();
  void PropSheetToPrefs();
  int GetRequiredPWLength() const;
};

#endif
  // _OPTIONSPROPSHEET_H_
