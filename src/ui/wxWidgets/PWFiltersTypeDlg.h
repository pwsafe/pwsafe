/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "core/PWSFilters.h"
#include "core/PWSprefs.h"
#include "QueryCancelDlg.h"

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

class pwFiltersTypeDlg : public QueryCancelDlg
{
  DECLARE_CLASS(pwFiltersTypeDlg)
  DECLARE_EVENT_TABLE()

public:
  static pwFiltersTypeDlg* Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, CItemData::EntryType *etype);
protected:
  /// Constructors
  pwFiltersTypeDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, CItemData::EntryType *etype);

  /// Destructor
  virtual ~pwFiltersTypeDlg() = default;

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

  void InitDialog() override;
  void SetValidators();

  //(*Handlers(pwFiltersTypeDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChangeRule(wxCommandEvent& event);
  void OnSelectionChangeType(wxCommandEvent& event);
  //*)

  bool IsChanged() const override;

  //(*Declarations(pwFiltersTypeDlg)
  wxComboBox* m_ComboBoxRule = nullptr;
  wxComboBox* m_ComboBoxType = nullptr;
  //*)

  const FieldType m_ftype;
  int m_idx = -1;
  int m_idx_type = -1;
  CItemData::EntryType m_etype;
  // Result parameter
  PWSMatch::MatchRule *m_prule = nullptr;
  CItemData::EntryType *m_petype = nullptr;;

  typedef struct etypeMapItem {
    int msgText;
    CItemData::EntryType typeValue;
  } tEtypeMapItem;
  static const tEtypeMapItem m_mtype[PW_NUM_TYPE_ENUM];
  
  static const PWSMatch::MatchRule m_mrx[PW_NUM_TYPE_RULE_ENUM];
};

#endif // _PWFILTERSTYPEDLG_H_
