/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _COMPAREDLG_H_
#define _COMPAREDLG_H_

#include <wx/dialog.h>
#include <wx/collpane.h>
#include "../../core/DBCompareData.h"

class PWScore;
struct SelectionCriteria;
class DbSelectionPanel;
class wxGrid;
struct ComparisonData;

class CompareDlg: public wxDialog
{
  void CreateControls();
  wxCollapsiblePane* CreateDBSelectionPanel(wxSizer* sizer);
  wxCollapsiblePane* CreateOptionsPanel(wxSizer* dlgSizer);
  wxCollapsiblePane* CreateDataPanel(wxSizer* dlgSizer, const wxString& title, ComparisonData* cd,
                                              bool customGrid = false);
  void OnCompare(wxCommandEvent& );

public:
  CompareDlg(wxWindow* parent, PWScore* core);
  ~CompareDlg();

private:
  PWScore*            m_currentCore;
  PWScore*            m_otherCore;
  SelectionCriteria*  m_selCriteria;
  DbSelectionPanel*   m_dbPanel;
  wxCollapsiblePane*  m_dbSelectionPane;
  wxCollapsiblePane*  m_optionsPane;
  ComparisonData      *m_current, *m_comparison, *m_conflicts, *m_identical;

  void DoCompare();

  DECLARE_EVENT_TABLE()
};

#endif

