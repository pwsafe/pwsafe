/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersStringDlg.h
* 
*/

#ifndef _PWFILTERSSTRINGDLG_H_
#define _PWFILTERSSTRINGDLG_H_

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
#define ID_COMBOBOX51 10337
#define ID_TEXTCTRL52 10338
#define ID_CHECKBOX53 10340
////@end control identifiers

#define PW_NUM_PRESENT_ENUM 2
#define PW_NUM_STR_CRITERIA_ENUM 12

/*!
 * pwFiltersStringDlg class declaration
 */

class pwFiltersStringDlg : public QueryCancelDlg
{
  DECLARE_CLASS(pwFiltersStringDlg)
  DECLARE_EVENT_TABLE()

public:
  static pwFiltersStringDlg* Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase);
protected:
  /// Constructors
  pwFiltersStringDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase);

  /// Destructor
  virtual ~pwFiltersStringDlg() = default;

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

  //(*Handlers(pwFiltersStringDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChange(wxCommandEvent& event);
  void OnTextChange(wxCommandEvent& event);
  //*)

  bool IsChanged() const override;

  //(*Declarations(pwFiltersStringDlg)
  wxComboBox* m_ComboBox = nullptr;
  wxTextCtrl* m_TextCtrlValueString = nullptr;
  wxCheckBox* m_CheckBoxFCase = nullptr;
  //*)

  int m_idx = -1;
  wxString m_string;
  bool m_fcase;

  const FieldType m_ftype;
  bool m_add_present = false;
  bool m_controlsReady = false;
  // Result parameter
  PWSMatch::MatchRule *m_prule = nullptr;
  wxString            *m_pvalue = nullptr;
  bool                *m_pfcase = nullptr;
  
  static const PWSMatch::MatchRule m_mrpres[PW_NUM_PRESENT_ENUM];
  static const PWSMatch::MatchRule m_mrcrit[PW_NUM_STR_CRITERIA_ENUM];
};

#endif // _PWFILTERSSTRINGDLG_H_
