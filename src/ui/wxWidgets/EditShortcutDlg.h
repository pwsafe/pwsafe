/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "core/ItemData.h"
#include "core/PWScore.h"
#include "QueryCancelDlg.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * EditShortcutDlg class declaration
 */

class EditShortcutDlg : public QueryCancelDlg
{
  DECLARE_CLASS(EditShortcutDlg)
  DECLARE_EVENT_TABLE()

public:

  static EditShortcutDlg* Create(wxWindow *parent, PWScore &core, CItemData *shortcut);

  /// Destructor
  virtual ~EditShortcutDlg() = default;

protected:
  /// Constructors
  EditShortcutDlg(wxWindow *parent, PWScore &core, CItemData *shortcut);

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
  bool SyncAndQueryCancel(bool showDialog) override;

  enum Changes : uint32_t {
    None = 0,
    Group = 1u,
    Title = 1u << 1,
    User = 1u << 2,
  };
  
  uint32_t GetChanges() const;  
  bool IsChanged() const override;

  //(*Handlers(EditShortcutDlgDialog)
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
  wxComboBox* m_ComboBoxShortcutGroup = nullptr;
  wxTextCtrl* m_TextCtrlShortcutTitle = nullptr;
  wxTextCtrl* m_TextCtrlShortcutUsername = nullptr;

  wxStaticText* m_StaticTextShortcutCreated = nullptr;
  wxStaticText* m_StaticTextShortcutChanged = nullptr;
  wxStaticText* m_StaticTextShortcutAccessed = nullptr;
  wxStaticText* m_StaticTextShortcutAnyChange = nullptr;

  wxStaticText* m_StaticTextBaseEntryGroup = nullptr;
  wxStaticText* m_StaticTextBaseEntryTitle = nullptr;
  wxStaticText* m_StaticTextBaseEntryUsername = nullptr;
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
  CItemData *m_Shortcut = nullptr;
};

#endif // _EDITSHORTCUTDLG_H_
