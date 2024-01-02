/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SetFiltersDlg.h
* 
*/


#ifndef _SETFILTERSDLG_H_
#define _SETFILTERSDLG_H_


/*!
 * Includes
 */
////@begin includes
#include "wx/valtext.h"

#include "core/core.h"

#include "PWFiltersGrid.h"
#include "QueryCancelDlg.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class pwFiltersGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SETFILTERS 10000
#define ID_FILTERNAME 10001
#define ID_FILTERGRID 10002
#define SYMBOL_SETFILTERS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SETFILTERS_TITLE _("Set Display Filters")
#define SYMBOL_SETFILTERS_PWHIST_TITLE _("Set Password History Filters")
#define SYMBOL_SETFILTERS_POLICY_TITLE _("Set Password Policy Filters")
#define SYMBOL_SETFILTERS_ATTACHMENT_TITLE _("Set Attachment Filters")
#define SYMBOL_SETFILTERS_IDNAME ID_SETFILTERS
#define SYMBOL_SETFILTERS_SIZE wxSize(600, 400)
#define SYMBOL_SETFILTERS_POSITION wxDefaultPosition
////@end control identifiers

#define SET_FILTER_WINDOW_OFFSET 10

/*!
 * SetFiltersDlg class declaration
 */

class SetFiltersDlg: public QueryCancelDlg
{    
  DECLARE_DYNAMIC_CLASS( SetFilters )
  DECLARE_EVENT_TABLE()

public:
  static SetFiltersDlg* Create(wxWindow *parent, st_filters *pfilters, st_filters *currentFilters, bool *appliedCalled,
    const FilterType filtertype = DFTYPE_MAIN, FilterPool filterpool = FPOOL_LAST, const bool bCanHaveAttachments = false, const std::set<StringX> *psMediaTypes = nullptr, wxWindowID id = SYMBOL_SETFILTERS_IDNAME, const wxString& caption = SYMBOL_SETFILTERS_TITLE, const wxPoint& pos = SYMBOL_SETFILTERS_POSITION, const wxSize& size = SYMBOL_SETFILTERS_SIZE, long style = SYMBOL_SETFILTERS_STYLE );

  /// Destructor
  ~SetFiltersDlg() = default;
public:
  /// Constructors
  SetFiltersDlg() = default;
  SetFiltersDlg(wxWindow *parent, st_filters *pfilters, st_filters *currentFilters, bool *appliedCalled,
                const FilterType filtertype, FilterPool filterpool, const bool bCanHaveAttachments, const std::set<StringX> *psMediaTypes, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin SetFiltersDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
  void OnApplyClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );
////@end SetFiltersDlg event handler declarations

////@begin SetFiltersDlg member function declarations

  /// A local copy of the filter name is stored as shown in the dialog
  wxString GetFilterName() const { return m_filterName ; }
  void SetFilterName(wxString value) { m_filterName = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end SetFilters member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();
////@end SetFiltersDlg member function declarations

////@begin SetFiltersDlg member variables
  pwFiltersGrid* m_filterGrid = nullptr;
  
private:
  bool VerifyFilters();
  bool SyncAndQueryCancel(bool showDialog) override;
  bool IsChanged() const override;
   
  st_filters *m_pfilters = nullptr;           // The filter to change
  st_filters m_origFilters;                   // Copy of filter to detect changes
  st_filters *m_currentFilters = nullptr;     // The filter to be set for Apply
  const bool m_bCanHaveAttachments = false; // Version V4 includes attachments
  const std::set<StringX> *m_psMediaTypes = nullptr;  // Attachement media types, present in current data base
  const FilterType m_filtertype = DFTYPE_INVALID;    // Type of filter
  const FilterPool m_filterpool = FPOOL_LAST;    // Filter pool
  bool  *m_AppliedCalled = nullptr;           // Mark, when Applied had been called and currentFilters is filled with actual one
  
  wxString m_filterName;
////@end SetFiltersDlg member variables
};

#endif
  // _SETFILTERSDLG_H_
