/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

class pwFiltersIntegerDlg : public QueryCancelDlg
{
  DECLARE_CLASS(pwFiltersIntegerDlg)
  DECLARE_EVENT_TABLE()
public:
  static pwFiltersIntegerDlg* Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, int *fnum1, int *fnum2, int *funit = nullptr);
protected:
  /// Constructors
  pwFiltersIntegerDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, int *fnum1, int *fnum2, int *funit);

  /// Destructor
  virtual ~pwFiltersIntegerDlg() = default;

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
  bool isRuleSelected(int idx, PWSMatch::MatchRule rule) const;
  void UpdateUnitSelection();

  bool IsChanged() const override;
  bool IsValid(bool showMessage) const;

  //(*Declarations(pwFiltersIntegerDlg)
  wxComboBox* m_ComboBox = nullptr;
  wxSpinCtrl* m_FNum1Ctrl = nullptr;
  wxSpinCtrl* m_FNum2Ctrl = nullptr;
  wxRadioButton* m_UnitByteCtrl = nullptr;
  wxRadioButton* m_UnitKByteCtrl = nullptr;
  wxRadioButton* m_UnitMByteCtrl = nullptr;
  wxStaticText*  m_IntervalTextCtrl = nullptr;
  //*)
  
  wxString infoFmtStr; // Format string for information text

  const FieldType m_ftype;
  bool m_add_present = false;
  
  int m_idx = -1;
  int m_fnum1 = -1;
  int m_fnum2 = -1;
  int m_min = -1;
  int m_max = -1;
  int m_funit = -1;
  
  // Result parameter
  PWSMatch::MatchRule *m_prule = nullptr;
  int                 *m_pfnum1 = nullptr;
  int                 *m_pfnum2 = nullptr;
  int                 *m_pfunit = nullptr;
  
  static const PWSMatch::MatchRule m_mrpres[PW_NUM_PRESENT_ENUM];
  static const PWSMatch::MatchRule m_mrcrit[PW_NUM_INT_CRITERIA_ENUM];
};

#endif // _PWFILTERSINTEGERDLG_H_
