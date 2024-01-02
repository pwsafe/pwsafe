/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

#include <wx/choice.h>
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
 * CreateShortcutDlg class declaration
 */

class CreateShortcutDlg : public QueryCancelDlg
{
  DECLARE_CLASS(CreateShortcutDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Creation
  static CreateShortcutDlg* Create(wxWindow *parent, PWScore &core, CItemData *base);

  /// Destructor
  virtual ~CreateShortcutDlg() = default;

protected:
  /// Constructors
  CreateShortcutDlg(wxWindow *parent, PWScore &core, CItemData *base);

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
  bool SyncAndQueryCancel(bool showDialog) override;

  enum Changes : uint32_t {
    None = 0,
    Group = 1u,
    Title = 1u << 1,
    User = 1u << 2,
  };
  
  uint32_t GetChanges() const;
  bool IsChanged() const override;

  wxString GetDefaultShortcutTitle() const;
  
  //(*Handlers(CreateShortcutDlg)
  void OnOk(wxCommandEvent& event);
  //*)

  //(*Identifiers(CreateShortcutDlg)
  static const wxWindowID ID_COMBOBOX1;
  static const wxWindowID ID_TEXTCTRL1;
  static const wxWindowID ID_TEXTCTRL2;
  static const wxWindowID ID_STATICTEXT7;
  static const wxWindowID ID_STATICTEXT8;
  static const wxWindowID ID_STATICTEXT9;
  //*)

  //(*Declarations(CreateShortcutDlg)
  wxComboBox* m_ComboBoxShortcutGroup = nullptr;
  wxTextCtrl* m_TextCtrlShortcutTitle = nullptr;
  wxTextCtrl* m_TextCtrlShortcutUsername = nullptr;

  wxStaticText* m_StaticTextBaseEntryGroup = nullptr;
  wxStaticText* m_StaticTextBaseEntryTitle = nullptr;
  wxStaticText* m_StaticTextBaseEntryUsername = nullptr;
  //*)

  wxString m_ShortcutGroup;
  wxString m_ShortcutTitle;
  wxString m_ShortcutUsername;

  wxString m_BaseEntryGroup;
  wxString m_BaseEntryTitle;
  wxString m_BaseEntryUsername;

  PWScore &m_Core;
  CItemData *m_Base = nullptr;
};

#endif // _CREATESHORTCUTDLG_H_
