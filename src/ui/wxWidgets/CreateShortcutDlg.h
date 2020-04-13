/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file CreateShortcutDlg.h
* 
*/

#ifndef _CREATESHORTCUTDLG_H_
#define _CREATESHORTCUTDLG_H_

/*!
 * Includes
 */

//#include "wx/valgen.h"
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * CreateShortcutDlg class declaration
 */

class CreateShortcutDlg : public wxDialog
{
  DECLARE_CLASS(CreateShortcutDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CreateShortcutDlg(wxWindow* parent, PWScore &core, CItemData *base);

  /// Creation
  bool Create(wxWindow* parent);

  /// Destructor
  virtual ~CreateShortcutDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );

  /// Should we show tooltips?
  static bool ShowToolTips();

private:

  void ItemFieldsToDialog();
  void SetValidators();
  void UpdateControls();

  //(*Handlers(CreateShortcutDlg)
  void OnOk(wxCommandEvent& event);
  //*)

  //(*Identifiers(CreateShortcutDlg)
  static const long ID_COMBOBOX1;
  static const long ID_TEXTCTRL1;
  static const long ID_TEXTCTRL2;
  static const long ID_STATICTEXT7;
  static const long ID_STATICTEXT8;
  static const long ID_STATICTEXT9;
  //*)

  //(*Declarations(CreateShortcutDlg)
  wxComboBox* m_ComboBoxShortcutGroup;
  wxTextCtrl* m_TextCtrlShortcutTitle;
  wxTextCtrl* m_TextCtrlShortcutUsername;

  wxStaticText* m_StaticTextBaseEntryGroup;
  wxStaticText* m_StaticTextBaseEntryTitle;
  wxStaticText* m_StaticTextBaseEntryUsername;
  //*)

  PWScore &m_core;
  CItemData *m_base;

  wxString m_ShortcutGroup;
  wxString m_ShortcutTitle;
  wxString m_ShortcutUsername;

  wxString m_BaseEntryGroup;
  wxString m_BaseEntryTitle;
  wxString m_BaseEntryUsername;
};

#endif // _CREATESHORTCUTDLG_H_
