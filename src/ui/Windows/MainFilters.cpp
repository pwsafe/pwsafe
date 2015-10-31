/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
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
  // Deal with the 2 internal filters before user defined ones
  if (m_bExpireDisplayed) {
    OnShowExpireList();
  } else
  if (m_bUnsavedDisplayed) {
    OnShowUnsavedEntries();
  } else
  if (m_bFilterActive) {
    ApplyFilter();
  }
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

  mf_iter = m_MapFilters.find(fk);
  if (mf_iter == m_MapFilters.end())
    return false;

  m_currentfilter = mf_iter->second;
  bool bActiveFilters = (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive + 
    m_currentfilter.num_Pactive + m_currentfilter.num_Aactive) > 0;

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

  PWSfile::VERSION current_version = m_core.GetReadFileVersion();
  bool bCanHaveAttachments = m_core.GetNumAtts() > 0 && 
    (current_version >= PWSfile::V40 && current_version < PWSfile::NEWFILE);

  std::vector<StringX> vMediaTypes;
  if (bCanHaveAttachments) {
    vMediaTypes = m_core.GetAllMediaTypes();
  }

  CSetFiltersDlg sf(this, &filters, PWS_MSG_EXECUTE_FILTERS, bCanHaveAttachments, &vMediaTypes);

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
      m_currentfilter.num_Pactive + m_currentfilter.num_Aactive) > 0;

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
  PWSfile::VERSION current_version = m_core.GetReadFileVersion();
  bool bCanHaveAttachments = m_core.GetNumAtts() > 0 &&
    (current_version >= PWSfile::V40 && current_version < PWSfile::NEWFILE);

  std::vector<StringX> vMediaTypes;
  if (bCanHaveAttachments) {
    vMediaTypes = m_core.GetAllMediaTypes();
  }

  CSetFiltersDlg sf(this, pfilters, PWS_MSG_EXECUTE_FILTERS, bCanHaveAttachments, &vMediaTypes, bAllowSet);

  INT_PTR rc = sf.DoModal();
  return (rc == IDOK);
}

void DboxMain::ClearFilter()
{
  m_currentfilter.Empty();
  m_bFilterActive = false;
  m_bFilterForStatus = false;
  m_bFilterForType = false;

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
    m_currentfilter.num_Pactive + m_currentfilter.num_Aactive) > 0;
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
    m_currentfilter.num_Pactive + m_currentfilter.num_Aactive) > 0;

  ApplyFilters();

  return 0L;
}

bool DboxMain::PassesFiltering(const CItemData &ci,
                               const st_filters &filters)
{
  bool thistest_rc;
  bool bValue(false);
  const CItemData *pci;

  if ((filters.num_Mactive + filters.num_Hactive + filters.num_Pactive + filters.num_Aactive) == 0)
    return true;

  const CItemData::EntryType entrytype = ci.GetEntryType();

  m_bFilterForStatus = false;
  m_bFilterForType = false;

  vfiltergroups::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vMflgroups.begin();
       Fltgroup_citer != m_vMflgroups.end(); Fltgroup_citer++) {
    const vfiltergroup &group = *Fltgroup_citer;

    int tests(0);
    bool thisgroup_rc = false;
    vfiltergroup::const_iterator Fltnum_citer;
    for (Fltnum_citer = group.begin();
         Fltnum_citer != group.end(); Fltnum_citer++) {
      const int &num = *Fltnum_citer;
      if (num == -1) // Padding to ensure group size is correct for FT_PWHIST & FT_POLICY
        continue;

      const st_FilterRow &st_fldata = filters.vMfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = filters.vMfldata[num].ftype;
      const int ifunction = (int)st_fldata.rule;

      switch (ft) {
        case FT_GROUPTITLE:
        case FT_GROUP:
        case FT_TITLE:
        case FT_USER:
        case FT_NOTES:
        case FT_URL:
        case FT_AUTOTYPE:
        case FT_RUNCMD:
        case FT_EMAIL:
        case FT_SYMBOLS:
        case FT_POLICYNAME:
          mt = PWSMatch::MT_STRING;
          break;
        case FT_PASSWORD:
          mt = PWSMatch::MT_PASSWORD;
          break;
        case FT_DCA:
          mt = PWSMatch::MT_DCA;
          break;
        case FT_SHIFTDCA:
          mt = PWSMatch::MT_SHIFTDCA;
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
        case FT_KBSHORTCUT:
          bValue = !ci.GetKBShortcut().empty();
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_UNKNOWNFIELDS:
          bValue = ci.NumberUnknownFields() > 0;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_PROTECTED:
          bValue = ci.IsProtected();
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_PASSWORDLEN:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_ENTRYTYPE:
          mt = PWSMatch::MT_ENTRYTYPE;
          break;
        case FT_ENTRYSTATUS:
          mt = PWSMatch::MT_ENTRYSTATUS;
          break;
        case FT_ENTRYSIZE:
          mt = PWSMatch::MT_ENTRYSIZE;
          break;
        case FT_ATTACHMENT:
          mt = PWSMatch::MT_ATTACHMENT;
          break;
        default:
          ASSERT(0);
      }

      if (ft == FT_ENTRYSTATUS) {
        m_bFilterForStatus = true;
      }

      if (ft == FT_ENTRYTYPE) {
        m_bFilterForType = true;
      }

      pci = &ci;

      if (ft == FT_PASSWORD && entrytype == CItemData::ET_ALIAS) {
        pci = GetBaseEntry(pci); // This is an alias
      }

      if (entrytype == CItemData::ET_SHORTCUT && !m_bFilterForStatus && !m_bFilterForType) {
        // Only include shortcuts if the filter is on the group, title or user fields
        // Note: "GROUPTITLE = 0x00", "GROUP = 0x02", "TITLE = 0x03", "USER = 0x04"
        //   "UUID = 0x01" but no filter is implemented against this field
        // The following is a simple single test rather than testing against every value
        if (ft > FT_USER) {
          pci = GetBaseEntry(pci); // This is an shortcut
        }
      }

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
        {
          time_t t1(st_fldata.fdate1), t2(st_fldata.fdate2);
          if (st_fldata.fdatetype == 1 /* Relative */) {
            time_t now;
            time(&now);
            t1 = now + (st_fldata.fnum1 * 86400);
            if (ifunction == PWSMatch::MR_BETWEEN)
              t2 = now + (st_fldata.fnum2 * 86400);
          }
          thistest_rc = pci->Matches(t1, t2,
                                     (int)ft, ifunction);
          tests++;
          break;
        }
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
        case PWSMatch::MT_DCA:
        case PWSMatch::MT_SHIFTDCA:
          thistest_rc = pci->Matches(st_fldata.fdca, ifunction, mt == PWSMatch::MT_SHIFTDCA);
          tests++;
          break;
        case PWSMatch::MT_ENTRYSTATUS:
          thistest_rc = pci->Matches(st_fldata.estatus, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ENTRYSIZE:
          thistest_rc = pci->Matches(st_fldata.fnum1, st_fldata.fnum2,
                                     (int)ft, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ATTACHMENT:
          if (filters.num_Aactive != 0) {
            thistest_rc = PassesAttFiltering(pci, filters);
            tests++;
          }
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

bool DboxMain::PassesPWHFiltering(const CItemData *pci,
                                  const st_filters &filters) const
{
  bool thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  size_t pwh_max, err_num;
  PWHistList pwhistlist;
  PWHistList::iterator pwshe_iter;

  bool status = CreatePWHistoryList(pci->GetPWHistory(),
                                    pwh_max, err_num,
                                    pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);

  bPresent = pwh_max > 0 || !pwhistlist.empty();

  vFilterRows::const_iterator Flt_citer;
  vfiltergroups::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vHflgroups.begin();
       Fltgroup_citer != m_vHflgroups.end(); Fltgroup_citer++) {
    const vfiltergroup &group = *Fltgroup_citer;

    int tests(0);
    bool thisgroup_rc = false;
    vfiltergroup::const_iterator Fltnum_citer;
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
          iValue = (int)pwhistlist.size();
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_MAX:
          iValue = (int)pwh_max;
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
          for (pwshe_iter = pwhistlist.begin(); pwshe_iter != pwhistlist.end(); pwshe_iter++) {
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
          for (pwshe_iter = pwhistlist.begin(); pwshe_iter != pwhistlist.end(); pwshe_iter++) {
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

bool DboxMain::PassesPWPFiltering(const CItemData *pci,
                                  const st_filters &filters) const
{
  bool thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  PWPolicy pwp;

  pci->GetPWPolicy(pwp);
  bPresent = pwp.flags != 0;

  vFilterRows::const_iterator Flt_citer;
  vfiltergroups::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vPflgroups.begin();
       Fltgroup_citer != m_vPflgroups.end(); Fltgroup_citer++) {
    const vfiltergroup &group = *Fltgroup_citer;

    int tests(0);
    bool thisgroup_rc = false;
    vfiltergroup::const_iterator Fltnum_citer;
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
          bValue = (pwp.flags & PWPolicy::UseHexDigits) ==
                       PWPolicy::UseHexDigits;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_EASYVISION:
          bValue = (pwp.flags & PWPolicy::UseEasyVision) ==
                       PWPolicy::UseEasyVision;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_PRONOUNCEABLE:
          bValue = (pwp.flags & PWPolicy::MakePronounceable) ==
                       PWPolicy::MakePronounceable;
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

bool DboxMain::PassesAttFiltering(const CItemData *pci,
                                  const st_filters &filters) const
{
  bool thistest_rc, bPresent;
  bool bValue(false);

  bPresent = pci->HasAttRef();
  
  // Only reference att if bPresent is true
  CItemAtt &att = m_core.GetAtt(pci->GetAttUUID());

  vFilterRows::const_iterator Flt_citer;
  vfiltergroups::const_iterator Fltgroup_citer;
  for (Fltgroup_citer = m_vPflgroups.begin();
       Fltgroup_citer != m_vPflgroups.end(); Fltgroup_citer++) {
    const vfiltergroup &group = *Fltgroup_citer;

    int tests(0);
    bool thisgroup_rc = false;
    vfiltergroup::const_iterator Fltnum_citer;
    for (Fltnum_citer = group.begin();
      Fltnum_citer != group.end(); Fltnum_citer++) {
      const int &num = *Fltnum_citer;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = filters.vAfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case AT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case AT_TITLE:
        case AT_FILENAME:
        case AT_FILEPATH:
        case AT_MEDIATYPE:
          mt = PWSMatch::MT_STRING;
          break;
        case AT_CTIME:
        case AT_FILECTIME:
        case AT_FILEMTIME:
        case AT_FILEATIME:
          mt = PWSMatch::MT_DATE;
          break;
        default:
          ASSERT(0);
      }

      const int ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_STRING:
          if (bPresent) {
            thistest_rc = att.Matches(st_fldata.fstring.c_str(), (int)ft,
              st_fldata.fcase == BST_CHECKED ? -ifunction : ifunction);
          } else {
            thistest_rc = false;
          }
          tests++;
          break;
        case PWSMatch::MT_DATE:
          if (bPresent) {
            time_t t1(st_fldata.fdate1), t2(st_fldata.fdate2);
            if (st_fldata.fdatetype == 1 /* Relative */) {
              time_t now;
              time(&now);
              t1 = now + (st_fldata.fnum1 * 86400);
              if (ifunction == PWSMatch::MR_BETWEEN)
                t2 = now + (st_fldata.fnum2 * 86400);
            }
            thistest_rc = att.Matches(t1, t2, (int)ft, ifunction);
          } else {
            thistest_rc = false;
          }
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
  fkl.cs_filtername = L"";

  if (!m_MapFilters.empty()) {
    mf_lower_iter = m_MapFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = L"";
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

  PWSfile::VERSION current_version = m_core.GetReadFileVersion();

  bool bCanHaveAttachments = m_core.GetNumAtts() > 0 && 
    (current_version >= PWSfile::V40 && current_version < PWSfile::NEWFILE);

  CManageFiltersDlg mf(this, m_bFilterActive, m_MapFilters, bCanHaveAttachments);
  mf.SetCurrentData(m_currentfilterpool, m_currentfilter.fname.c_str());
  mf.DoModal();

  // If user has changed the database filters, we need to update the core copy.
  if (!mf.HasDBFiltersChanged())
    return;

  m_core.m_MapFilters.clear();

  if (!m_MapFilters.empty()) {
    mf_lower_iter = m_MapFilters.lower_bound(fkl);

    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
      fku.cs_filtername = L"";
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
  std::wstring XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                  L"filters.xml");
  cs_text.LoadString(IDS_NAMEXMLFILE);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
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
    cs_temp.Format(IDS_CANTOPENWRITING, cs_newfile);
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
    cs_temp.Format(IDSC_MISSINGXSD, XSDfn.c_str());
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }

  std::wstring dir;
  if (m_core.GetCurFile().empty())
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
    rc = m_MapFilters.ImportFilterXMLFile(FPOOL_IMPORTED, L"",
                                          std::wstring(XMLFilename),
                                          XSDFilename.c_str(), strErrors, &q);
    waitCursor.Restore();  // Restore normal cursor

    UINT uiFlags = MB_OK | MB_ICONWARNING;
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
          std::wstring csErrors = strErrors + L"\n";
          cs_temp.Format(IDS_XMLIMPORTWITHERRORS, fd.GetFileName(),
                         csErrors.c_str());
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

bool group_pred (const vfiltergroup& v1, const vfiltergroup& v2)
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
      } else if (st_fldata.ftype == FT_POLICY) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_POLICY entry
        for (int j = 0; j < m_currentfilter.num_Pactive - 1; j++) {
          group.push_back(-1);
        }
      } else if (st_fldata.ftype == FT_ATTACHMENT) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_ATTACHMENT entry
        for (int j = 0; j < m_currentfilter.num_Aactive - 1; j++) {
          group.push_back(-1);
        }
      }
    } // st_fldata.bFilterActive
    i++;
  } // iterate over m_currentfilter.vMfldata
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
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
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
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
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vPflgroups = groups;
  } else
    m_vPflgroups.clear();
}
