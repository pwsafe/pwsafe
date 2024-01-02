/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersBoolDlg.h
* 
*/

#ifndef _PWFILTERSBOOLDLG_H_
#define _PWFILTERSBOOLDLG_H_

/*!
 * Includes
 */

#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "core/PWSFilters.h"
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
#define ID_COMBOBOX53 10339
////@end control identifiers

#define PW_NUM_BOOL_ENUM 2

/*!
 * pwFiltersBoolDlg class declaration
 */

class pwFiltersBoolDlg : public QueryCancelDlg
{
  DECLARE_CLASS(pwFiltersBoolDlg)
  DECLARE_EVENT_TABLE()

public:
  static pwFiltersBoolDlg* Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule);
protected:
  enum BoolType {BT_PRESENT, BT_ACTIVE, BT_SET, BT_IS};

  /// Constructors
  pwFiltersBoolDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule);

  /// Destructor
  virtual ~pwFiltersBoolDlg() = default;

  /// Creates the controls and sizers
  void CreateControls();

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );

  /// Should we show tooltips?
  static bool ShowToolTips();

  static BoolType ConvertType(FieldType ftype);
private:

  void InitDialog() override;
  void SetValidators();

  //(*Handlers(pwFiltersBoolDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChange(wxCommandEvent& event);
  //*)

  bool IsChanged() const override;

  //(*Declarations(pwFiltersBoolDlg)
  wxComboBox* m_ComboBoxBool = nullptr;
  //*)

  const FieldType m_ftype;
  BoolType m_btype;   // Boolean Type is set depending from field type
  int m_idx = -1;
  PWSMatch::MatchRule *m_prule = nullptr; // Pointer to the buffer with result
  
  const PWSMatch::MatchRule *m_pmrx = nullptr; // Pointer to one of the arrays below, depending from boolean type
  
  static const PWSMatch::MatchRule m_mrxp[PW_NUM_BOOL_ENUM];
  static const PWSMatch::MatchRule m_mrxa[PW_NUM_BOOL_ENUM];
  static const PWSMatch::MatchRule m_mrxs[PW_NUM_BOOL_ENUM];
  static const PWSMatch::MatchRule m_mrxi[PW_NUM_BOOL_ENUM];
};

#endif // _PWFILTERSBOOLDLG_H_
