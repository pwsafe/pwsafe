/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersMediaDlg.h
* 
*/

#ifndef _PWFILTERSMEDIATYPESDLG_H_
#define _PWFILTERSMEDIATYPESDLG_H_

/*!
 * Includes
 */

#include <set>

#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "core/PWSFilters.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_COMBOBOX69 10372
#define ID_COMBOBOX70 10373
#define ID_CHECKBOX71 10374
////@end control identifiers

#define PW_NUM_PRESENT_ENUM 2
#define PW_NUM_MEDIA_TYPES_CRITERIA_ENUM 12

/*!
 * pwFiltersMediaTypesDlg class declaration
 */

class pwFiltersMediaTypesDlg : public wxDialog
{
  DECLARE_CLASS(pwFiltersMediaTypesDlg)
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersMediaTypesDlg(wxWindow* parent, FieldType ftype, PWSMatch::MatchRule &rule, wxString &value, bool &fcase, const std::set<StringX> *psMediaTypes);

  /// Destructor
  virtual ~pwFiltersMediaTypesDlg();

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

  //(*Handlers(pwFiltersMediaTypesDlg)
  void OnOk(wxCommandEvent& event);
  void OnSelectionChange(wxCommandEvent& event);
  void OnMediaTypeChange(wxCommandEvent& event);
  //*)

  //(*Declarations(pwFiltersMediaTypesDlg)
  wxComboBox* m_ComboBox;
  wxComboBox* m_MediaTypes;
  wxCheckBox* m_CheckBoxFCase;
  //*)

  int m_idx;
  int m_idx_mt;
  wxString m_string;
  bool m_fcase;

  const FieldType m_ftype;
  const std::set<StringX> *m_psMediaTypes;
  bool m_add_present;
  // Result parameter
  PWSMatch::MatchRule *m_prule;
  wxString            *m_pvalue;
  bool                *m_pfcase;
  
  static const PWSMatch::MatchRule m_mrpres[PW_NUM_PRESENT_ENUM];
  static const PWSMatch::MatchRule m_mrcrit[PW_NUM_MEDIA_TYPES_CRITERIA_ENUM];
};

#endif // _PWFILTERSMEDIATYPESDLG_H_
