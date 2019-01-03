/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
class PWSAuxCore;
struct SelectionCriteria;
class DbSelectionPanel;
class wxGrid;
struct ComparisonData;
class wxGridEvent;
class wxGridRangeSelectEvent;

class CompareDlg: public wxDialog
{
  void CreateControls();
  wxCollapsiblePane* CreateDBSelectionPanel(wxSizer* sizer);
  wxCollapsiblePane* CreateOptionsPanel(wxSizer* dlgSizer);
  wxCollapsiblePane* CreateDataPanel(wxSizer* dlgSizer, const wxString& title, ComparisonData* cd,
                                              bool customGrid = false);
  void OnCompare(wxCommandEvent& );
  void OnGridCellRightClick(wxGridEvent& evt);
  void OnEditInCurrentDB(wxCommandEvent& evt);
  void OnViewInComparisonDB(wxCommandEvent& evt);
  void OnExpandDataPanels(wxCommandEvent& evt);
  void OnCopyItemsToCurrentDB(wxCommandEvent& evt);
  void OnDeleteItemsFromCurrentDB(wxCommandEvent& evt);
  void OnCopyFieldsToCurrentDB(wxCommandEvent& evt);
  void OnSyncItemsWithCurrentDB(wxCommandEvent& evt);

public:
  CompareDlg(wxWindow* parent, PWScore* core);
  ~CompareDlg();

private:
  PWScore*            m_currentCore;
  PWSAuxCore*         m_otherCore;
  SelectionCriteria*  m_selCriteria;
  DbSelectionPanel*   m_dbPanel;
  wxCollapsiblePane*  m_dbSelectionPane;
  wxCollapsiblePane*  m_optionsPane;
  ComparisonData      *m_current, *m_comparison, *m_conflicts, *m_identical;

  void DoCompare(wxCommandEvent& evt);
  bool ViewEditEntry(PWScore* core, const pws_os::CUUID& uuid, bool readOnly);

  DECLARE_EVENT_TABLE()
};

#endif
