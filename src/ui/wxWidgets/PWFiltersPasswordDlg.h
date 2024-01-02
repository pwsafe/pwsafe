/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersPasswordDlg.h
* 
*/

#ifndef _PWFILTERSPASSWORDDLG_H_
#define _PWFILTERSPASSWORDDLG_H_

/*!
 * Includes
 */

#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

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
#define ID_COMBOBOX72 10375
#define ID_TEXTCTRL73 10376
#define ID_CHECKBOX74 10377
#define ID_SPINCTRL75 10378
////@end control identifiers

#define PW_NUM_PASSWORD_CRITERIA_ENUM 14

/*!
 * pwFiltersPasswordDlg class declaration
 */

class pwFiltersPasswordDlg : public QueryCancelDlg
{
  DECLARE_CLASS(pwFiltersPasswordDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  static pwFiltersPasswordDlg* Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, int *fnum1);

protected:
  /// Constructors
  pwFiltersPasswordDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, int *fnum1);

  /// Destructor
  virtual ~pwFiltersPasswordDlg() = default;

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

  //(*Handlers(pwFiltersPasswordDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChange(wxCommandEvent& event);
  void OnTextChange(wxCommandEvent& event);
  void OnFNum1Change(wxSpinEvent& event);
  //*)

  bool IsChanged() const override;

  //(*Declarations(pwFiltersPasswordDlg)
  wxComboBox* m_ComboBox = nullptr;
  wxTextCtrl* m_TextCtrlValueString = nullptr;
  wxCheckBox* m_CheckBoxFCase = nullptr;
  wxSpinCtrl* m_FNum1Ctrl = nullptr;
  //*)

  int m_idx = -1;
  wxString m_string;
  bool m_fcase;
  int m_fnum1;
  int m_min;
  int m_max;
  
  wxString infoFmtStr; // Format string for information text

  const FieldType m_ftype;
  // Result parameter
  PWSMatch::MatchRule *m_prule = nullptr;
  wxString            *m_pvalue = nullptr;
  bool                *m_pfcase = nullptr;
  int                 *m_pfnum1 = nullptr;
  
  static const PWSMatch::MatchRule m_mrcrit[PW_NUM_PASSWORD_CRITERIA_ENUM];
};

#endif // _PWFILTERSPASSWORDDLG_H_
