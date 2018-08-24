/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainFilters.cpp
//
// Filter-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "MFCMessages.h"
#include "GeneralMsgBox.h"
#include "PWFileDialog.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "filter/SetFiltersDlg.h"
#include "filter/ManageFiltersDlg.h"

#include "core/core.h"
#include "core/PWSFilters.h"
#include "core/PWHistory.h"
#include "core/pwsprefs.h"
#include "core/match.h"
#include "core/PWSfile.h"
#include "core/PWSdirs.h"
#include "core/XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#include "os/file.h"
#include "os/dir.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DboxMain::OnCancelFilter()
{
  // Deal with the 3 internal filters before user defined ones

  // Save currently selected items
  SaveGUIStatus();

  if (m_bExpireDisplayed) {
    OnShowExpireList();
  } else
  if (m_bUnsavedDisplayed) {
    OnShowUnsavedEntries();
  } else
  if (m_bFindFilterDisplayed) {
    OnShowFoundEntries();
  } else
  if (m_bFilterActive) {
    ApplyFilter();
  }

  // Now try to restore selection
  RestoreGUIStatus();
}

void DboxMain::OnApplyFilter()
{
  ApplyFilter();
}

bool DboxMain::ApplyFilter(bool bJustDoIt)
{
  // bJustDoIt implies apply filters even if one already applied - ie. without
  // the intervening Clear.
  // If there are no active filters - this becomes just a Clear.
  PWSFilters::iterator mf_iter;
  st_Filterkey fk;
  fk.fpool = (FilterPool)m_currentfilterpool;
  fk.cs_filtername = m_selectedfiltername;

  mf_iter = m_MapAllFilters.find(fk);
  if (mf_iter == m_MapAllFilters.end())
    return false;

  CurrentFilter() = mf_iter->second;
  bool bActiveFilters = CurrentFilter().IsActive();

  if (!bJustDoIt && !m_bFilterActive && !bActiveFilters) {
    // Nothing to do!
    return false;
  }

  // If we are told to "Just do it" - we do, otherwise we toggle!
  if (bJustDoIt)
    m_bFilterActive = bActiveFilters;
  else
    m_bFilterActive = !m_bFilterActive;

  ApplyFilters();
  return true;
}

void DboxMain::OnSetFilter()
{
  st_filters filters(CurrentFilter());
  bool bCanHaveAttachments = m_core.GetNumAtts() > 0;
  const std::set<StringX> sMediaTypes = m_core.GetAllMediaTypes();

  CSetFiltersDlg sf(this, &filters, PWS_MSG_EXECUTE_FILTERS, bCanHaveAttachments, &sMediaTypes);

  INT_PTR rc = sf.DoModal();

  if (rc == IDOK || (rc == IDCANCEL && m_bFilterActive)) {
    // If filters currently active - update and re-apply
    // If not, just update

    // User can apply the filter in SetFiltersDlg and then press Cancel button
    // and so still process
    CurrentFilter().Empty();
    CurrentFilter() = filters;

    st_Filterkey fk;
    fk.fpool = FPOOL_SESSION;
    fk.cs_filtername = CurrentFilter().fname;
    m_MapAllFilters.erase(fk);
    m_MapAllFilters.insert(PWSFilters::Pair(fk, CurrentFilter()));

    m_currentfilterpool = fk.fpool;
    m_selectedfiltername = fk.cs_filtername.c_str();

    bool bFilters = CurrentFilter().IsActive();

    if (m_bFilterActive) {
      m_bFilterActive = bFilters;
      ApplyFilters();
    } else if (bFilters) {
      m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER, 
                                                  bFilters ? TRUE : FALSE);
    }
  }
}

bool DboxMain::EditFilter(st_filters *pfilters, const bool &bAllowSet)
{
  bool bCanHaveAttachments = m_core.GetNumAtts() > 0;
  std::set<StringX> sMediaTypes = m_core.GetAllMediaTypes();

  CSetFiltersDlg sf(this, pfilters, PWS_MSG_EXECUTE_FILTERS, bCanHaveAttachments, &sMediaTypes, bAllowSet);

  INT_PTR rc = sf.DoModal();
  return (rc == IDOK);
}

void DboxMain::ClearFilter()
{
  CurrentFilter().Empty();

  m_bFilterActive = false;
  m_ctlItemTree.SetFilterState(m_bFilterActive);
  m_ctlItemList.SetFilterState(m_bFilterActive);
  m_StatusBar.SetFilterStatus(m_bFilterActive);

  if (m_bOpen)
    ApplyFilters();
}

void DboxMain::ApplyFilters()
{
  m_ctlItemTree.SetFilterState(m_bFilterActive);
  m_ctlItemList.SetFilterState(m_bFilterActive);
  m_StatusBar.SetFilterStatus(m_bFilterActive);

  m_ctlItemTree.Invalidate();
  m_ctlItemList.Invalidate();
  m_StatusBar.Invalidate();
  m_StatusBar.UpdateWindow();

  m_FilterManager.CreateGroups();

  RefreshViews();

  bool bFilters = CurrentFilter().IsActive();
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER, 
                                              bFilters ? TRUE : FALSE);

  if (m_bFilterActive)
    m_ctlItemTree.OnExpandAll();

  // m_LastFoundTreeItem might be invalid if filter activated or cleared
  pws_os::CUUID entry_uuid;
  int iLastShown = m_FindToolBar.GetLastSelectedFoundItem(entry_uuid);
  if (iLastShown >= 0) {
    CItemData *pci = &m_core.Find(entry_uuid)->second;
    DisplayInfo *pdi = GetEntryGUIInfo(*pci);
    m_LastFoundTreeItem = pdi->tree_item;
    m_LastFoundListItem = pdi->list_index;

    UpdateToolBarForSelectedItem(pci);
    SetDCAText(pci);
  }

  if (iLastShown < 0 || m_LastFoundListItem < 0) {
    // No item selected by Find or found item not in this view - select first entry
    SelectFirstEntry();
  }

  // Update Status Bar
  UpdateStatusBar();
}

LRESULT DboxMain::OnExecuteFilters(WPARAM wParam, LPARAM /* lParam */)
{
  // Called when user presses "Apply" on main SetFilters dialog
  st_filters *pfilters = reinterpret_cast<st_filters *>(wParam);
  CurrentFilter().Empty();

  CurrentFilter() = (*pfilters);

  m_bFilterActive = CurrentFilter().IsActive();

  ApplyFilters();

  return 0L;
}

// functor for Copy subset of map entries back to the database
struct CopyDBFilters {
  CopyDBFilters(PWSFilters &core_mapFilters) :
  m_CoreMapFilters(core_mapFilters)
  {}

  // operator
  void operator()(pair<const st_Filterkey, st_filters> p)
  {
    m_CoreMapFilters.insert(PWSFilters::Pair(p.first, p.second));
  }

private:
  PWSFilters &m_CoreMapFilters;
};

void DboxMain::OnManageFilters()
{
  PWSFilters::iterator mf_iter, mf_lower_iter, mf_upper_iter;

  st_Filterkey fkl, fku;
  fkl.fpool = FPOOL_DATABASE;
  fkl.cs_filtername = L"";

  // Find & delete DB filters only
  if (!m_MapAllFilters.empty()) {
    mf_lower_iter = m_MapAllFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = L"";
      mf_upper_iter = m_MapAllFilters.upper_bound(fku);

      // Delete existing database filters (if any)
      m_MapAllFilters.erase(mf_lower_iter, mf_upper_iter);
    }
  }

  // Get current core filters
  PWSFilters core_filters = m_core.GetDBFilters();
  const PWSFilters original_core_filters = m_core.GetDBFilters();

  // Now add any existing database filters
  for (mf_iter = core_filters.begin();
    mf_iter != core_filters.end(); mf_iter++) {
    m_MapAllFilters.insert(PWSFilters::Pair(mf_iter->first, mf_iter->second));
  }

  bool bCanHaveAttachments = m_core.GetNumAtts() > 0;

  // m_MapAllFilters will be updated by this dialog if user adds, deletes or changes a filter
  CManageFiltersDlg mf(this, m_bFilterActive, m_MapAllFilters, bCanHaveAttachments);
  mf.SetCurrentData(m_currentfilterpool, CurrentFilter().fname.c_str());
  mf.DoModal();

  // If user has changed the database filters, we need to update the core copy.
  if (!mf.HasDBFiltersChanged())
    return;

  // Clear core filters ready to replace with new ones
  core_filters.clear();

  // Get DB filters populated via CManageFiltersDlg
  if (!m_MapAllFilters.empty()) {
    mf_lower_iter = m_MapAllFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = L"";
      mf_upper_iter = m_MapAllFilters.upper_bound(fku);

      // Copy database filters (if any) to the core
      CopyDBFilters copy_db_filters(core_filters);
      for_each(mf_lower_iter, mf_upper_iter, copy_db_filters);
    }
  }

  // However, we need to check as user may have edited the filter more thn once 
  // and reverted any changes!
  if (core_filters != original_core_filters) {
    // Now update DB filters in core
    Command *pcmd = DBFiltersCommand::Create(&m_core, core_filters);

    // Do it
    Execute(pcmd);
  }
}

void DboxMain::ExportFilters(PWSFilters &Filters)
{
  CString cs_text, cs_temp, cs_title, cs_newfile;
  INT_PTR rc;

  // do the export
  // SaveAs-type dialog box
  std::wstring XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                  L"filters.xml");
  cs_text.LoadString(IDS_NAMEXMLFILE);
  std::wstring dir;
  if (!m_core.IsDbOpen())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     L"xml",
                     XMLFileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_X_ALL)),
                     this);

    fd.m_ofn.lpstrTitle = cs_text;
  
    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();

    rc = fd.DoModal();

    if (m_inExit) {
      // If U3ExitNow called while in CPWFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling
      PostQuitMessage(0);
      return;
    }
    if (rc == IDOK) {
      cs_newfile = fd.GetPathName();
      break;
    } else
      return;
  } // while (1)

  PWSfileHeader hdr = m_core.GetHeader();
  StringX currentfile = m_core.GetCurFile();
  rc = Filters.WriteFilterXMLFile(LPCWSTR(cs_newfile), hdr, currentfile);

  CGeneralMsgBox gmb;
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, static_cast<LPCWSTR>(cs_newfile));
    cs_title.LoadString(IDS_FILEWRITEERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
  } else {
    gmb.AfxMessageBox(IDS_FILTERSEXPORTEDOK, MB_OK | MB_ICONINFORMATION);
  }
}

void DboxMain::ImportFilters()
{
  CString cs_title, cs_temp, cs_text;
  cs_text.LoadString(IDS_PICKXMLFILE);
  const std::wstring XSDfn(L"pwsafe_filter.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

  if (!pws_os::FileExists(XSDFilename)) {
    CGeneralMsgBox gmb;
    cs_temp.Format(IDSC_MISSINGXSD, static_cast<LPCWSTR>(XSDfn.c_str()));
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }

  std::wstring dir;
  if (!m_core.IsDbOpen())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"xml",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_XML)),
                   this);

  fd.m_ofn.lpstrTitle = cs_text;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return;
  }
  if (rc == IDCANCEL)
    return;

  if (rc == IDOK) {
    std::wstring strErrors;
    CString XMLFilename = fd.GetPathName();
    CWaitCursor waitCursor;  // This may take a while!

    MFCAsker q;
    rc = m_MapAllFilters.ImportFilterXMLFile(FPOOL_IMPORTED, L"",
                                          std::wstring(XMLFilename),
                                          XSDFilename.c_str(), strErrors, &q);
    waitCursor.Restore();  // Restore normal cursor

    UINT uiFlags = MB_OK | MB_ICONWARNING;
    switch (rc) {
      case PWScore::XML_FAILED_VALIDATION:
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, static_cast<LPCWSTR>(fd.GetFileName()),
                       static_cast<LPCWSTR>(strErrors.c_str()));
        break;
      case PWScore::XML_FAILED_IMPORT:
        cs_temp.Format(IDS_XMLERRORS, static_cast<LPCWSTR>(fd.GetFileName()),
                       static_cast<LPCWSTR>(strErrors.c_str()));
        break;
      case PWScore::SUCCESS:
        if (!strErrors.empty()) {
          std::wstring csErrors = strErrors + L"\n";
          cs_temp.Format(IDS_XMLIMPORTWITHERRORS, static_cast<LPCWSTR>(fd.GetFileName()),
                         static_cast<LPCWSTR>(csErrors.c_str()));
        } else {
          cs_temp.LoadString(IDS_FILTERSIMPORTEDOK);
          uiFlags = MB_OK | MB_ICONINFORMATION;
        }
        break;
      default:
        ASSERT(0);
    } // switch

    CGeneralMsgBox gmb;
    cs_title.LoadString(IDS_STATUS);
    gmb.MessageBox(cs_temp, cs_title, uiFlags);
  }
}
