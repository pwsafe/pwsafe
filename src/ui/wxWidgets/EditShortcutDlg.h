/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file EditShortcutDlg.h
* 
*/

#ifndef _EDITSHORTCUTDLG_H_
#define _EDITSHORTCUTDLG_H_

/*!
 * Includes
 */

#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "core/ItemData.h"
#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * EditShortcutDlg class declaration
 */

class EditShortcutDlg : public wxDialog
{
  DECLARE_CLASS(EditShortcutDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  EditShortcutDlg(wxWindow* parent, PWScore &core, CItemData *shortcut);

  /// Destructor
  virtual ~EditShortcutDlg();

  /// Creation
  bool Create(wxWindow* parent);

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

////@begin EditShortcutDlg member variables

private:

  void ItemFieldsToDialog();
  void SetValidators();
  void UpdateControls();

  //(*Handlers(CreateShortcutDlg)
  void OnOk(wxCommandEvent& event);
  //*)

  //(*Identifiers(EditShortcutDlgDialog)
  static const wxWindowID ID_COMBOBOX1;
  static const wxWindowID ID_TEXTCTRL2;
  static const wxWindowID ID_TEXTCTRL3;
  static const wxWindowID ID_STATICTEXT6;
  static const wxWindowID ID_STATICTEXT8;
  static const wxWindowID ID_STATICTEXT10;
  static const wxWindowID ID_STATICTEXT2;
  static const wxWindowID ID_STATICTEXT4;
  static const wxWindowID ID_STATICTEXT7;
  //*)

  //(*Declarations(EditShortcutDlgDialog)
  wxComboBox* m_ComboBoxShortcutGroup;
  wxTextCtrl* m_TextCtrlShortcutTitle;
  wxTextCtrl* m_TextCtrlShortcutUsername;

  wxStaticText* m_StaticTextShortcutCreated;
  wxStaticText* m_StaticTextShortcutChanged;
  wxStaticText* m_StaticTextShortcutAccessed;
  wxStaticText* m_StaticTextShortcutAnyChange;

  wxStaticText* m_StaticTextBaseEntryGroup;
  wxStaticText* m_StaticTextBaseEntryTitle;
  wxStaticText* m_StaticTextBaseEntryUsername;
  //*)

  wxString m_ShortcutGroup;
  wxString m_ShortcutTitle;
  wxString m_ShortcutUsername;

  wxString m_ShortcutCreated;
  wxString m_ShortcutChanged;
  wxString m_ShortcutAccessed;
  wxString m_ShortcutAnyChange;

  wxString m_BaseEntryGroup;
  wxString m_BaseEntryTitle;
  wxString m_BaseEntryUsername;

  PWScore &m_Core;
  CItemData *m_Shortcut;
};

#endif // _EDITSHORTCUTDLG_H_
