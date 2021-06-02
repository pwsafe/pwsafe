/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersIntegerDlg.h
* 
*/

#ifndef _PWFILTERSINTEGERDLG_H_
#define _PWFILTERSINTEGERDLG_H_

/*!
 * Includes
 */

#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

#include "PWSFilters.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_COMBOBOX57 10343
#define ID_SPINCTRL58 10344
#define ID_SPINCTRL59 10345
#define ID_RADIO_BT_BY 10346
#define ID_RADIO_BT_KB 10347
#define ID_RADIO_BT_MB 10360
////@end control identifiers

#define PW_NUM_PRESENT_ENUM 2
#define PW_NUM_INT_CRITERIA_ENUM 7

/*!
 * pwFiltersIntegerDlg class declaration
 */

class pwFiltersIntegerDlg : public wxDialog
{
  DECLARE_CLASS(pwFiltersIntegerDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersIntegerDlg(wxWindow* parent, FieldType ftype, PWSMatch::MatchRule &rule, int &fnum1, int &fnum2, int *funit = NULL);

  /// Destructor
  virtual ~pwFiltersIntegerDlg();

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

  //(*Handlers(pwFiltersIntegerDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChange(wxCommandEvent& event);
  void OnFNum1Change(wxSpinEvent& event);
  void OnFNum2Change(wxSpinEvent& event);
  void OnByteRadiobuttonSelected(wxCommandEvent& event);
  void OnKiloByteRadiobuttonSelected(wxCommandEvent& event);
  void OnMegaByteRadiobuttonSelected(wxCommandEvent& event);
  //*)
  
  void CheckControls();
  bool CheckBetween(bool showMessage);
  bool isRuleSelected(PWSMatch::MatchRule rule);
  void UpdateUnitSelection();

  //(*Declarations(pwFiltersIntegerDlg)
  wxComboBox* m_ComboBox;
  wxSpinCtrl* m_FNum1Ctrl;
  wxSpinCtrl* m_FNum2Ctrl;
  wxRadioButton* m_UnitByteCtrl;
  wxRadioButton* m_UnitKByteCtrl;
  wxRadioButton* m_UnitMByteCtrl;
  wxStaticText*  m_IntervalTextCtrl;
  //*)
  
  wxString infoFmtStr; // Format string for information text

  const FieldType m_ftype;
  bool m_add_present;
  
  int m_idx;
  int m_fnum1;
  int m_fnum2;
  int m_min;
  int m_max;
  int m_funit;
  
  // Result parameter
  PWSMatch::MatchRule *m_prule;
  int                 *m_pfnum1;
  int                 *m_pfnum2;
  int                 *m_pfunit;
  
  static const PWSMatch::MatchRule m_mrpres[PW_NUM_PRESENT_ENUM];
  static const PWSMatch::MatchRule m_mrcrit[PW_NUM_INT_CRITERIA_ENUM];
};

#endif // _PWFILTERSINTEGERDLG_H_
