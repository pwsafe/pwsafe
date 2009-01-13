/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "DboxMain.h"
#include "filter/SetFiltersDlg.h"
#include "filter/ManageFiltersDlg.h"
#include "GeneralMsgBox.h"
#include "MFCMessages.h"

#include "corelib/corelib.h"
#include "corelib/PWSFilters.h"
#include "corelib/PWHistory.h"
#include "corelib/pwsprefs.h"
#include "corelib/match.h"
#include "corelib/PWSfile.h"
#include "corelib/PWSdirs.h"

#include "os/file.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

  mf_iter = m_MapFilters.find(fk);
  if (mf_iter == m_MapFilters.end())
    return false;

  m_currentfilter = mf_iter->second;
  bool bActiveFilters = (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive + 
                                     m_currentfilter.num_Pactive) > 0;

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
  st_filters filters(m_currentfilter);
  CSetFiltersDlg sf(this, &filters, WM_EXECUTE_FILTERS);

  INT_PTR rc = sf.DoModal();
  if (rc == IDOK) {
    // If filters currently active - update and re-apply
    // If not, just update
    m_currentfilter.Empty();
    m_currentfilter = filters;

    st_Filterkey fk;
    fk.fpool = FPOOL_SESSION;
    fk.cs_filtername = m_currentfilter.fname;
    m_MapFilters.erase(fk);
    m_MapFilters.insert(PWSFilters::Pair(fk, m_currentfilter));

    m_currentfilterpool = fk.fpool;
    m_selectedfiltername = fk.cs_filtername.c_str();

    bool bFilters = (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive + 
                                                   m_currentfilter.num_Pactive) > 0;

    if (m_bFilterActive) {
      if (bFilters)
        m_bFilterActive = true;
      else
        m_bFilterActive = false;

      ApplyFilters();
    }
    else if (bFilters) {
      m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER, 
                                                  bFilters ? TRUE : FALSE);
    }
  }
}

bool DboxMain::EditFilter(st_filters *pfilters, const bool &bAllowSet)
{
  CSetFiltersDlg sf(this, pfilters, WM_EXECUTE_FILTERS, bAllowSet);

  INT_PTR rc = sf.DoModal();
  return (rc == IDOK);
}

void DboxMain::ClearFilter()
{
  m_currentfilter.Empty();
  m_bFilterActive = false;

  ApplyFilters();
}

void DboxMain::ApplyFilters()
{
  m_statusBar.SetFilterStatus(m_bFilterActive);

  m_statusBar.Invalidate();
  m_statusBar.UpdateWindow();

  m_ctlItemTree.SetFilterState(m_bFilterActive);
  m_ctlItemList.SetFilterState(m_bFilterActive);
  m_ctlItemTree.Invalidate();
  m_ctlItemList.Invalidate();

  CreateGroups();

  RefreshViews();

  bool bFilters = (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive + 
                                                 m_currentfilter.num_Pactive) > 0;
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER, 
                                              bFilters ? TRUE : FALSE);

  // Clear Find as old entries might not now be in the List View (which is how
  // Find works).  Also, hide it if visible.
  m_FindToolBar.ClearFind();
  if (m_FindToolBar.IsVisible())
    OnHideFindToolBar();

  if (m_bFilterActive)
    m_ctlItemTree.OnExpandAll();

  // Update Status Bar
  UpdateStatusBar();
}

LRESULT DboxMain::OnExecuteFilters(WPARAM wParam, LPARAM /* lParam */)
{
  // Called when user presses "Apply" on main SetFilters dialog
  st_filters *pfilters = reinterpret_cast<st_filters *>(wParam);
  m_currentfilter.Empty();

  m_currentfilter = (*pfilters);

  m_bFilterActive = (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive +
                                                   m_currentfilter.num_Pactive) > 0;

  ApplyFilters();

  return 0L;
}

bool DboxMain::PassesFiltering(CItemData &ci, const st_filters &filters)
{
  bool thistest_rc, thisgroup_rc;
  bool bValue(false);
  CItemData *pci;

  if ((filters.num_Mactive + filters.num_Hactive + filters.num_Pactive) == 0)
    return true;

  const CItemData::EntryType entrytype = ci.GetEntryType();

  std::vector<std::vector<int> >::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vMflgroups.begin();
       Fltgroup_citer != m_vMflgroups.end(); Fltgroup_citer++) {
    const std::vector<int> &group = *Fltgroup_citer;

    int tests(0);
    thisgroup_rc = false;
    std::vector<int>::const_iterator Fltnum_citer;
    for (Fltnum_citer = group.begin();
         Fltnum_citer != group.end(); Fltnum_citer++) {
      const int &num = *Fltnum_citer;
      if (num == -1) // Padding to ensure group size is correct for FT_PWHIST & FT_POLICY
        continue;

      const st_FilterRow &st_fldata = filters.vMfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = filters.vMfldata[num].ftype;

      switch (ft) {
        case FT_GROUPTITLE:
        case FT_GROUP:
        case FT_TITLE:
        case FT_USER:
        case FT_NOTES:
        case FT_URL:
        case FT_AUTOTYPE:
          mt = PWSMatch::MT_STRING;
          break;
        case FT_PASSWORD:
          mt = PWSMatch::MT_PASSWORD;
          break;
        case FT_CTIME:
        case FT_PMTIME:
        case FT_ATIME:
        case FT_XTIME:
        case FT_RMTIME:
          mt = PWSMatch::MT_DATE;
          break;
        case FT_PWHIST:
          mt = PWSMatch::MT_PWHIST;
          break;
        case FT_POLICY:
          mt = PWSMatch::MT_POLICY;
          break;
        case FT_XTIME_INT:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_UNKNOWNFIELDS:
          bValue = ci.NumberUnknownFields() > 0;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_ENTRYTYPE:
          mt = PWSMatch::MT_ENTRYTYPE;
          break;
        default:
          ASSERT(0);
      }

      pci = &ci;

      if (ft == FT_PASSWORD && entrytype == CItemData::ET_ALIAS) {
        // This is an alias
        uuid_array_t entry_uuid, base_uuid;
        ci.GetUUID(entry_uuid);
        m_core.GetAliasBaseUUID(entry_uuid, base_uuid);

        ItemListIter iter = m_core.Find(base_uuid);
        if (iter != End()) {
          pci = &(iter->second);
        }
      }

      if (entrytype == CItemData::ET_SHORTCUT) {
        // Only include shortcuts if the filter is on the group, title or user fields
        // Note: "GROUPTITLE = 0x00", "GROUP = 0x02", "TITLE = 0x03", "USER = 0x04"
        //   "UUID = 0x01" but no filter is implemented against this field
        // The following is a simple single test rather than testing against every value
        if (ft > FT_USER) {
          // This is an shortcut
          uuid_array_t entry_uuid, base_uuid;
          ci.GetUUID(entry_uuid);
          m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

          ItemListIter iter = m_core.Find(base_uuid);
          if (iter != End()) {
            pci = &(iter->second);
          }
        }
      }

      const int ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_PASSWORD:
          if (ifunction == PWSMatch::MR_EXPIRED) {
            // Special Password "string" case
            thistest_rc = pci->IsExpired();
            tests++;
            break;
          } else if (ifunction == PWSMatch::MR_WILLEXPIRE) {
            // Special Password "string" case
            thistest_rc = pci->WillExpire(st_fldata.fnum1);
            tests++;
            break;
          }
          // Note: purpose drop through to standard 'string' processing
        case PWSMatch::MT_STRING:
          thistest_rc = pci->Matches(st_fldata.fstring.c_str(), (int)ft,
                                 st_fldata.fcase == BST_CHECKED ? -ifunction : ifunction);
          tests++;
          break;
        case PWSMatch::MT_INTEGER:
          thistest_rc = pci->Matches(st_fldata.fnum1, st_fldata.fnum2,
                                     (int)ft, ifunction);
          tests++;
          break;
        case PWSMatch::MT_DATE:
          thistest_rc = pci->Matches(st_fldata.fdate1, st_fldata.fdate2,
                                     (int)ft, ifunction);
          tests++;
          break;
        case PWSMatch::MT_PWHIST:
          if (filters.num_Hactive != 0) {
            thistest_rc = PassesPWHFiltering(pci, filters);
            tests++;
          }
          break;
        case PWSMatch::MT_POLICY:
          if (filters.num_Pactive != 0) {
            thistest_rc = PassesPWPFiltering(pci, filters);
            tests++;
          }
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ENTRYTYPE:
          thistest_rc = pci->Matches(st_fldata.etype, ifunction);
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool DboxMain::PassesPWHFiltering(CItemData *pci, const st_filters &filters)
{
  bool thisgroup_rc, thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  size_t pwh_max, err_num;
  PWHistList PWHistList;
  PWHistList::iterator pwshe_iter;

  bool status = CreatePWHistoryList(pci->GetPWHistory(),
                                    pwh_max, err_num,
                                    PWHistList, TMC_EXPORT_IMPORT);

  bPresent = pwh_max > 0 || !PWHistList.empty();

  vFilterRows::const_iterator Flt_citer;
  std::vector<std::vector<int> >::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vHflgroups.begin();
       Fltgroup_citer != m_vHflgroups.end(); Fltgroup_citer++) {
    const std::vector<int> &group = *Fltgroup_citer;

    int tests(0);
    thisgroup_rc = false;
    std::vector<int>::const_iterator Fltnum_citer;
    for (Fltnum_citer = group.begin();
         Fltnum_citer != group.end(); Fltnum_citer++) {
      const int &num = *Fltnum_citer;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = filters.vHfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case HT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_ACTIVE:
          bValue = status == TRUE;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_NUM:
          iValue = PWHistList.size();
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_MAX:
          iValue = pwh_max;
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_CHANGEDATE:
          mt = PWSMatch::MT_DATE;
          break;
        case HT_PASSWORDS:
          mt = PWSMatch::MT_STRING;
          break;
        default:
          ASSERT(0);
      }

      const int ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_STRING:
          for (pwshe_iter = PWHistList.begin(); pwshe_iter != PWHistList.end(); pwshe_iter++) {
            PWHistEntry pwshe = *pwshe_iter;
            thistest_rc = PWSMatch::Match(st_fldata.fstring, pwshe.password,
                                  st_fldata.fcase == BST_CHECKED ? -ifunction : ifunction);
            tests++;
            if (thistest_rc)
              break;
          }
          break;
        case PWSMatch::MT_INTEGER:
          thistest_rc = PWSMatch::Match(st_fldata.fnum1, st_fldata.fnum2,
                                        iValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_DATE:
          for (pwshe_iter = PWHistList.begin(); pwshe_iter != PWHistList.end(); pwshe_iter++) {
            const PWHistEntry pwshe = *pwshe_iter;
            CTime ct(pwshe.changetttdate);
            CTime ct2;
            ct2 = CTime(ct.GetYear(), ct.GetMonth(), ct.GetDay(), 0, 0, 0);
            time_t changetime;
            changetime = (time_t)ct2.GetTime();
            thistest_rc = PWSMatch::Match(st_fldata.fdate1, st_fldata.fdate2,
                                          changetime, ifunction);
            tests++;
            if (thistest_rc)
              break;
          }
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool DboxMain::PassesPWPFiltering(CItemData *pci, const st_filters &filters)
{
  bool thisgroup_rc, thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  PWPolicy pwp;

  pci->GetPWPolicy(pwp);
  bPresent = pwp.flags != 0;

  vFilterRows::const_iterator Flt_citer;
  std::vector<std::vector<int> >::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vPflgroups.begin();
       Fltgroup_citer != m_vPflgroups.end(); Fltgroup_citer++) {
    const std::vector<int> &group = *Fltgroup_citer;

    int tests(0);
    thisgroup_rc = false;
    std::vector<int>::const_iterator Fltnum_citer;
    for (Fltnum_citer = group.begin();
         Fltnum_citer != group.end(); Fltnum_citer++) {
      const int &num = *Fltnum_citer;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = filters.vPfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case PT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_LENGTH:
          iValue = pwp.length;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_LOWERCASE:
          iValue = pwp.lowerminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_UPPERCASE:
          iValue = pwp.upperminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_DIGITS:
          iValue = pwp.digitminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_SYMBOLS:
          iValue = pwp.symbolminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_HEXADECIMAL:
          bValue = (pwp.flags & PWSprefs::PWPolicyUseHexDigits) ==
                       PWSprefs::PWPolicyUseHexDigits;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_EASYVISION:
          bValue = (pwp.flags & PWSprefs::PWPolicyUseEasyVision) ==
                       PWSprefs::PWPolicyUseEasyVision;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_PRONOUNCEABLE:
          bValue = (pwp.flags & PWSprefs::PWPolicyMakePronounceable) ==
                       PWSprefs::PWPolicyMakePronounceable;
          mt = PWSMatch::MT_BOOL;
          break;
        default:
          ASSERT(0);
      }

      const int ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_INTEGER:
          thistest_rc = PWSMatch::Match(st_fldata.fnum1, st_fldata.fnum2,
                                        iValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
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
  fkl.cs_filtername = _T("");

  if (m_MapFilters.size() != 0) {
    mf_lower_iter = m_MapFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = _T("");
      mf_upper_iter = m_MapFilters.upper_bound(fku);

      // Delete existing database filters (if any)
      m_MapFilters.erase(mf_lower_iter, mf_upper_iter);
    }
  }

  // Now add any existing database filters
  for (mf_iter = m_core.m_MapFilters.begin();
       mf_iter != m_core.m_MapFilters.end(); mf_iter++) {
    m_MapFilters.insert(PWSFilters::Pair(mf_iter->first, mf_iter->second));
  }

  CManageFiltersDlg mf(this, m_bFilterActive, m_MapFilters);
  mf.SetCurrentData(m_currentfilterpool, m_currentfilter.fname.c_str());
  mf.DoModal();

  // If user has changed the database filters, we need to update the core copy.
  if (!mf.HasDBFiltersChanged())
    return;

  m_core.m_MapFilters.clear();

  if (m_MapFilters.size() != 0) {
    mf_lower_iter = m_MapFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = _T("");
      mf_upper_iter = m_MapFilters.upper_bound(fku);

      // Copy database filters (if any) to the core
      CopyDBFilters copy_db_filters(m_core.m_MapFilters);
      for_each(mf_lower_iter, mf_upper_iter, copy_db_filters);
    }
  }
}

void DboxMain::ExportFilters(PWSFilters &Filters)
{
  CString cs_text, cs_temp, cs_title, cs_newfile;
  INT_PTR rc;

  // do the export
  //SaveAs-type dialog box
  stringT XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                  _T("filters.xml"));
  cs_text.LoadString(IDS_NAMEXMLFILE);
  while (1) {
    CFileDialog fd(FALSE,
                   _T("xml"),
                   XMLFileName.c_str(),
                   OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                   OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                   _T("XML files (*.xml)|*.xml|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    rc = fd.DoModal();
    if (m_inExit) {
      // If U3ExitNow called while in CFileDialog,
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

  PWSfile::HeaderRecord hdr = m_core.GetHeader();
  StringX currentfile = m_core.GetCurFile();
  rc = Filters.WriteFilterXMLFile(LPCTSTR(cs_newfile), hdr, currentfile);

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, cs_newfile);
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
  } else {
    AfxMessageBox(IDS_FILTERSEXPORTEDOK, MB_OK);
  }
}

void DboxMain::ImportFilters()
{
  CString cs_title, cs_temp, cs_text;
  const stringT XSDfn(_T("pwsafe_filter.xsd"));
  stringT XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  // Expat is a non-validating parser - no use for Schema!
  if (!pws_os::FileExists(XSDFilename)) {
    cs_temp.Format(IDSC_MISSINGXSD, XSDfn.c_str());
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }
#endif

  CFileDialog fd(TRUE,
                 _T("xml"),
                 NULL,
                 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                 _T("XML files (*.xml)|*.xml||"),
                 this);
  cs_text.LoadString(IDS_PICKXMLFILE);
  fd.m_ofn.lpstrTitle = cs_text;

  INT_PTR rc = fd.DoModal();
  if (m_inExit) {
    // If U3ExitNow called while in CFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return;
  }
  if (rc == IDCANCEL)
    return;

  if (rc == IDOK) {
    stringT strErrors;
    CString XMLFilename = fd.GetPathName();
    CWaitCursor waitCursor;  // This may take a while!

    MFCAsker q;
    rc = m_MapFilters.ImportFilterXMLFile(FPOOL_IMPORTED, _T(""),
                                          stringT(XMLFilename),
                                          XSDFilename.c_str(), strErrors, &q);
    waitCursor.Restore();  // Restore normal cursor

    switch (rc) {
      case PWScore::XML_FAILED_VALIDATION:
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, fd.GetFileName(),
                       strErrors.c_str());
        break;
      case PWScore::XML_FAILED_IMPORT:
        cs_temp.Format(IDS_XMLERRORS, fd.GetFileName(), strErrors.c_str());
        break;
      case PWScore::SUCCESS:
        if (!strErrors.empty()) {
          stringT csErrors = strErrors + _T("\n");
          cs_temp.Format(IDS_XMLIMPORTWITHERRORS, fd.GetFileName(),
                         csErrors.c_str());
        } else {
          cs_temp.LoadString(IDS_FILTERSIMPORTEDOK);
        }
        break;
      default:
        ASSERT(0);
    } // switch

    cs_title.LoadString(IDS_STATUS);
    MessageBox(cs_temp, cs_title, MB_ICONINFORMATION | MB_OK);
  }
}

bool group_pred (const std::vector<int>& v1, const std::vector<int>& v2)
{
  return v1.size() < v2.size();
}

void DboxMain::CreateGroups()
{
  int i(0);
  vFilterRows::iterator Flt_iter;
  vfiltergroup group;
  vfiltergroups groups;

  // Do the main filters
  for (Flt_iter = m_currentfilter.vMfldata.begin();
       Flt_iter != m_currentfilter.vMfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // This active filter is in a new group!
        groups.push_back(group);
        group.clear();
      }

      group.push_back(i);

      if (st_fldata.ftype == FT_PWHIST) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_PWHIST entry
        for (int j = 0; j < m_currentfilter.num_Hactive - 1; j++) {
          group.push_back(-1);
         }
      }

      if (st_fldata.ftype == FT_POLICY) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_POLICY entry
        for (int j = 0; j < m_currentfilter.num_Pactive - 1; j++) {
          group.push_back(-1);
        }
      }
    }
    i++;
  }
  if (group.size() > 0)
    groups.push_back(group);

  if (groups.size() > 0) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vMflgroups = groups;
  } else
    m_vMflgroups.clear();

  // Now do the History filters
  i = 0;
  group.clear();
  groups.clear();
  for (Flt_iter = m_currentfilter.vHfldata.begin();
       Flt_iter != m_currentfilter.vHfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // Next active is in a new group!
        groups.push_back(group);
        group.clear();
      }
      group.push_back(i);
    }
    i++;
  }
  if (group.size() > 0)
    groups.push_back(group);

  if (groups.size() > 0) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vHflgroups = groups;
  } else
    m_vHflgroups.clear();

  // Now do the Policy filters
  i = 0;
  group.clear();
  groups.clear();
  for (Flt_iter = m_currentfilter.vPfldata.begin();
       Flt_iter != m_currentfilter.vPfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // Next active is in a new group!
        groups.push_back(group);
        group.clear();
      }
      group.push_back(i);
    }
    i++;
  }
  if (group.size() > 0)
    groups.push_back(group);

  if (groups.size() > 0) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vPflgroups = groups;
  } else
    m_vPflgroups.clear();
}
