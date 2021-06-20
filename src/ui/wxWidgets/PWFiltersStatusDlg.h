/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersStatusDlg.h
* 
*/

#ifndef _PWFILTERSSTATUSDLG_H_
#define _PWFILTERSSTATUSDLG_H_

/*!
 * Includes
 */

#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "core/PWSFilters.h"
#include "core/PWSprefs.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_COMBOBOX60 10361
#define ID_COMBOBOX61 10362
////@end control identifiers

#define PW_NUM_STATUS_RULE_ENUM 2
#define PW_NUM_STATUS_ENUM      3

/*!
 * pwFiltersStatusDlg class declaration
 */

class pwFiltersStatusDlg : public wxDialog
{
  DECLARE_CLASS(pwFiltersStatusDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersStatusDlg(wxWindow* parent, FieldType ftype, PWSMatch::MatchRule &rule, CItemData::EntryStatus &estatus);

  /// Destructor
  virtual ~pwFiltersStatusDlg();

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

private:

  void InitDialog();
  void SetValidators();

  //(*Handlers(pwFiltersStatusDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChangeRule(wxCommandEvent& event);
  void OnSelectionChangeStatus(wxCommandEvent& event);
  //*)

  //(*Declarations(pwFiltersStatusDlg)
  wxComboBox* m_ComboBoxRule;
  wxComboBox* m_ComboBoxStatus;
  //*)

  const FieldType m_ftype;
  int m_idx;
  int m_idx_status;
  CItemData::EntryStatus m_estatus;
  // Result parameter
  PWSMatch::MatchRule *m_prule;
  CItemData::EntryStatus *m_pestatus;

  typedef struct estatusMapItem {
    int msgText;
    CItemData::EntryStatus statusValue;
  } tEstatusMapItem;
  static const tEstatusMapItem m_mstatus[PW_NUM_STATUS_ENUM];
  
  static const PWSMatch::MatchRule m_mrx[PW_NUM_STATUS_RULE_ENUM];
};

#endif // _PWFILTERSSTATUSDLG_H_
