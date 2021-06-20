/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersTypeDlg.h
* 
*/

#ifndef _PWFILTERSTYPEDLG_H_
#define _PWFILTERSTYPEDLG_H_

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
#define ID_COMBOBOX62 10363
#define ID_COMBOBOX63 10364
////@end control identifiers

#define PW_NUM_TYPE_RULE_ENUM 2
#define PW_NUM_TYPE_ENUM      5

/*!
 * pwFiltersTypeDlg class declaration
 */

class pwFiltersTypeDlg : public wxDialog
{
  DECLARE_CLASS(pwFiltersTypeDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersTypeDlg(wxWindow* parent, FieldType ftype, PWSMatch::MatchRule &rule, CItemData::EntryType &etype);

  /// Destructor
  virtual ~pwFiltersTypeDlg();

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

  //(*Handlers(pwFiltersTypeDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChangeRule(wxCommandEvent& event);
  void OnSelectionChangeType(wxCommandEvent& event);
  //*)

  //(*Declarations(pwFiltersTypeDlg)
  wxComboBox* m_ComboBoxRule;
  wxComboBox* m_ComboBoxType;
  //*)

  const FieldType m_ftype;
  int m_idx;
  int m_idx_type;
  CItemData::EntryType m_etype;
  // Result parameter
  PWSMatch::MatchRule *m_prule;
  CItemData::EntryType *m_petype;

  typedef struct etypeMapItem {
    int msgText;
    CItemData::EntryType typeValue;
  } tEtypeMapItem;
  static const tEtypeMapItem m_mtype[PW_NUM_TYPE_ENUM];
  
  static const PWSMatch::MatchRule m_mrx[PW_NUM_TYPE_RULE_ENUM];
};

#endif // _PWFILTERSTYPEDLG_H_
