/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersDCADlg.h
* 
*/

#ifndef _PWFILTERSDCADLG_H_
#define _PWFILTERSDCADLG_H_

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
#define ID_COMBOBOX55 10341
#define ID_COMBOBOX56 10342
////@end control identifiers

#define PW_NUM_DCA_RULE_ENUM 4
#define PW_NUM_DCA_ENUM (PWSprefs::maxDCA + 2) // maxDCA is last index, so add 1 for the size and plus 1 for -1 as current default DCA

/*!
 * pwFiltersDCADlg class declaration
 */

class pwFiltersDCADlg : public wxDialog
{
  DECLARE_CLASS(pwFiltersDCADlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersDCADlg(wxWindow* parent, FieldType ftype, PWSMatch::MatchRule &rule, short &fdca);

  /// Destructor
  virtual ~pwFiltersDCADlg();

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

  stringT CurrentDefaultDCAuiString();

  //(*Handlers(pwFiltersDCADlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChangeRule(wxCommandEvent& event);
  void OnSelectionChangeDCA(wxCommandEvent& event);
  //*)

  //(*Declarations(pwFiltersDCADlg)
  wxComboBox* m_ComboBoxRule;
  wxComboBox* m_ComboBoxDCA;
  //*)

  const FieldType m_ftype;
  int m_idx;
  int m_idx_dca;
  short m_fdca;
  // Result parameter
  PWSMatch::MatchRule *m_prule;
  short *m_pfdca;

  typedef struct dcaMapItem {
    int msgText;
    short dcaValue;
  } tDcaMapItem;
  static const tDcaMapItem m_mdca[PW_NUM_DCA_ENUM];
  
  static const PWSMatch::MatchRule m_mrx[PW_NUM_DCA_RULE_ENUM];
};

#endif // _PWFILTERSDCADLG_H_
