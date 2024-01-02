/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ComapreDlg.h
* 
*/

#ifndef _COMPAREDLG_H_
#define _COMPAREDLG_H_

#include <wx/dialog.h>
#include <wx/collpane.h>
#include "../../core/DBCompareData.h"
#include "../../core/Report.h"

class PWScore;
class PWSAuxCore;
struct SelectionCriteria;
class DbSelectionPanel;
class wxGrid;
struct ComparisonData;
class wxGridEvent;
class wxGridRangeSelectEvent;

class CompareDlg : public wxDialog
{
  void CreateControls();
  wxCollapsiblePane* CreateDBSelectionPanel(wxSizer* sizer);
  wxCollapsiblePane* CreateOptionsPanel(wxSizer* dlgSizer);
  wxCollapsiblePane* CreateDataPanel(wxSizer* dlgSizer, const wxString& title, ComparisonData* cd,
                                              bool customGrid = false);
  void OnCompare(wxCommandEvent& );
  void OnShowReport(wxCommandEvent& );
  void OnGridCellRightClick(wxGridEvent& evt);
  void OnEditInCurrentDB(wxCommandEvent& evt);
  void OnViewInComparisonDB(wxCommandEvent& evt);
  void OnExpandDataPanels(wxCommandEvent& evt);
  void OnCopyItemsToCurrentDB(wxCommandEvent& evt);
  void OnDeleteItemsFromCurrentDB(wxCommandEvent& evt);
  void OnCopyFieldsToCurrentDB(wxCommandEvent& evt);
  void OnSyncItemsWithCurrentDB(wxCommandEvent& evt);
  
  void OnUpdateUI(wxUpdateUIEvent& event);
  
  void WriteReportData();
  void ReportAdvancedOptions();
  void WriteReport();

protected:
  explicit CompareDlg(wxWindow *parent, PWScore* core);
public:
  static CompareDlg* Create(wxWindow *parent, PWScore* core);
  ~CompareDlg();

private:
  struct ContextMenuData {
    ComparisonData* cdata;
    wxArrayInt selectedRows;    //indexes into the grid
    wxArrayInt selectedItems;   //indexes into the table
    CItemData::FieldType field;
  };

  PWScore*            m_currentCore = nullptr;
  PWSAuxCore*         m_otherCore = nullptr;
  SelectionCriteria*  m_selCriteria = nullptr;
  DbSelectionPanel*   m_dbPanel = nullptr;
  wxCollapsiblePane*  m_dbSelectionPane = nullptr;
  wxCollapsiblePane*  m_optionsPane = nullptr;
  ComparisonData      *m_current = nullptr, *m_comparison = nullptr, *m_conflicts = nullptr, *m_identical = nullptr;

  void DoCompare(wxCommandEvent& evt);
  void DoShowReport();
  void DoEditInCurrentDB(ContextMenuData menuContext);
  void DoViewInComparisonDB(ContextMenuData menuContext);
  void DoSyncItemsWithCurrentDB(int menuId, ContextMenuData menuContext);
  
  bool ViewEditEntry(PWScore* core, const pws_os::CUUID& uuid, bool readOnly);
  
  CReport            m_compReport;
  wxButton*          m_compareButton = nullptr;

  DECLARE_EVENT_TABLE()
};

#endif /* _COMPAREDLG_H_ */
