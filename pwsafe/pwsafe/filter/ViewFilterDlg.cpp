/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewFilterDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "ViewFilterDlg.h"
#include "../PWResizeDialog.h"
#include "../corelib/PWSFilters.h"
#include "../corelib/filters.h"
#include "../corelib/match.h"

#include "../resource3.h"
#include "../corelib/corelib.h"
#include "ComboAdder.h"

#include <vector>
#include <map>
#include <algorithm>

// ViewFilterDlg dialog

IMPLEMENT_DYNAMIC(CViewFilterDlg, CPWResizeDialog)

CViewFilterDlg::CViewFilterDlg(CWnd* pParent,
                               st_filters *pfilters,
                               MapFilters &pmapdbfilters,
                               MapFilters &pmapglobalfilters)
  : CPWResizeDialog(CViewFilterDlg::IDD, pParent),
  m_pfilters(pfilters),
  m_pMapDBFilters(pmapdbfilters), m_pMapGlobalFilters(pmapglobalfilters),
  m_selectedstore(m_selectedstore)
{
  // Get DB filter via name and replace m_filters
  MapFilters_Iter mf_iter;

  for (mf_iter = m_pMapDBFilters.begin();
       mf_iter != m_pMapDBFilters.end();
       mf_iter++) {
    m_vcs_db.push_back(mf_iter->first);
  }

  // Get Global filter via name and replace m_filters
  for (mf_iter = m_pMapGlobalFilters.begin();
       mf_iter != m_pMapGlobalFilters.end();
       mf_iter++) {
    m_vcs_gbl.push_back(mf_iter->first);
  }
}

CViewFilterDlg::~CViewFilterDlg()
{
}

void CViewFilterDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_CURRENTFILTERSBTN, m_selectedstore); // only first!
  DDX_Control(pDX, IDC_FILTERNAMECOMBO, m_combo);
  DDX_Control(pDX, IDC_FILTERLC, m_FilterLC);
}

BEGIN_MESSAGE_MAP(CViewFilterDlg, CPWResizeDialog)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_CURRENTFILTERSBTN, OnBnClickedCurrent)
  ON_BN_CLICKED(IDC_DATABASEFILTERSBTN, OnBnClickedDBStore)
  ON_BN_CLICKED(IDC_GLOBALFILTERBTN, OnBnClickedGlobalStore)
  ON_CBN_SELCHANGE(IDC_FILTERNAMECOMBO, OnFilterSelected)
END_MESSAGE_MAP()

// ViewFilter message handlers

BOOL CViewFilterDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);

  AddMainCtrlID(IDC_FILTERLC);
  AddBtnsCtrlIDs(vibottombtns);

  UINT statustext[1] = {IDS_BLANK};
  SetStatusBar(&statustext[0], 1, false);

  CPWResizeDialog::OnInitDialog();

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  GetDlgItem(IDC_CURRENTFILTERSBTN)->EnableWindow(m_pfilters == NULL ? FALSE : TRUE);
  GetDlgItem(IDC_DATABASEFILTERSBTN)->EnableWindow(m_vcs_db.empty() ? FALSE : TRUE);
  GetDlgItem(IDC_GLOBALFILTERBTN)->EnableWindow(m_vcs_gbl.empty() ? FALSE : TRUE);

  if (m_pfilters != NULL) {
    m_selectedstore = VF_CURRENT;
    m_combo.SetWindowText(m_pfilters->fname);
    m_combo.EnableWindow(FALSE);
  } else
  if (!m_vcs_db.empty()) {
    m_selectedstore = VF_DATABASE;
  } else
  if (!m_vcs_gbl.empty()) {
    m_selectedstore = VF_GLOBAL;
  }

  if (m_selectedstore != VF_CURRENT && m_combo.GetCount() == 0) {
    ComboAdder ca(m_combo);
    if (m_selectedstore == VF_DATABASE) {
      ca.doit(m_vcs_db);
    } else {  // VF_GLOBAL
      ca.doit(m_vcs_gbl);
    }
  }

  CString cs_text;
  cs_text = _T(" # ");
  m_FilterLC.InsertColumn(VFLC_FILTER_NUMBER, cs_text);
  cs_text.LoadString(IDS_FILTERACTIVE);
  cs_text += _T("?");
  m_FilterLC.InsertColumn(VFLC_FILTER_ACTIVE, cs_text);
  cs_text.LoadString(IDS_AND_OR);
  m_FilterLC.InsertColumn(VFLC_AND_OR, cs_text);
  cs_text.LoadString(IDS_FILTERFIELD);
  m_FilterLC.InsertColumn(VFLC_FIELD, cs_text);
  cs_text.LoadString(IDS_CRITERIA_DESC);
  m_FilterLC.InsertColumn(VFLC_CRITERIA_TEXT, cs_text);

  // Make first 4 columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;
  m_FilterLC.SetColumn(VFLC_FILTER_NUMBER, &lvc);
  m_FilterLC.SetColumn(VFLC_FILTER_ACTIVE, &lvc);
  m_FilterLC.SetColumn(VFLC_AND_OR, &lvc);

  if (m_selectedstore != VF_CURRENT) {
    m_combo.SetCurSel(0);
    OnFilterSelected();
  } else
    SelectFilter(m_pfilters);

  int itotalwidth = 0;
  for (int i = 0; i < VFLC_NUM_COLUMNS; i++) {
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    itotalwidth += m_FilterLC.GetColumnWidth(i);
  }

  int iMaxWidth = itotalwidth + 16;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);

  UpdateData(FALSE);

  return FALSE;
}

void CViewFilterDlg::OnBnClickedCurrent()
{
  ASSERT(m_pfilters != NULL);
  UpdateData(TRUE);
  m_combo.EnableWindow(FALSE);
  m_combo.ResetContent();
  m_combo.SetWindowText(m_pfilters->fname);
  SelectFilter(m_pfilters);
}
  
void CViewFilterDlg::OnBnClickedDBStore()
{
  UpdateData(TRUE);
  m_combo.EnableWindow(TRUE);
  m_combo.ResetContent();

  ComboAdder ca(m_combo);
  ca.doit(m_vcs_db);

  m_combo.SetCurSel(0);
  m_FilterLC.DeleteAllItems();
  OnFilterSelected();
}

void CViewFilterDlg::OnBnClickedGlobalStore()
{
  UpdateData(TRUE);
  m_combo.EnableWindow(TRUE);
  m_combo.ResetContent();

  ComboAdder ca(m_combo);
  ca.doit(m_vcs_gbl);

  m_combo.SetCurSel(0);
  m_FilterLC.DeleteAllItems();
  OnFilterSelected();
}

void CViewFilterDlg::OnFilterSelected()
{
  int isel = m_combo.GetCurSel();
  if (isel == CB_ERR)
    return;

  m_FilterLC.DeleteAllItems();

  CString cs_selected;
  m_combo.GetLBText(isel, cs_selected);

  MapFilters_Iter mf_iter;
  if (m_selectedstore == VF_DATABASE) {
    mf_iter = m_pMapDBFilters.find(cs_selected);
  } else {
    mf_iter = m_pMapGlobalFilters.find(cs_selected);
  }

  st_filters *pfilters = &mf_iter->second;
  SelectFilter(pfilters);
}

void CViewFilterDlg::SelectFilter(st_filters *pfilters)
{
  CString cs_num, cs_ftype, cs_criteria, cs_ltype, cs_act;

  vfilterdata::iterator Flt_iter;
  bool bHistory(false), bPolicy(false);
  int i(0), iItem(0), n(0);

  // Do the main filters
  for (Flt_iter = pfilters->vMfldata.begin();
       Flt_iter != pfilters->vMfldata.end(); Flt_iter++) {
    st_FilterData &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(_T("%d"), n);
    UINT nID = GetFieldTypeName(st_fldata.ftype);
    cs_ftype.LoadString(nID);
    cs_ftype.TrimRight(_T('\t'));
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata);
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    if (Flt_iter != pfilters->vMfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = _T("");

    iItem = m_FilterLC.InsertItem(i, cs_num);
    m_FilterLC.SetItemText(iItem, 1, cs_act);
    m_FilterLC.SetItemText(iItem, 2, cs_ltype);
    m_FilterLC.SetItemText(iItem, 3, cs_ftype);
    m_FilterLC.SetItemText(iItem, 4, cs_criteria);
 
    if (st_fldata.ftype == FT_PWHIST)
      bHistory = true;
    if (st_fldata.ftype == FT_POLICY)
      bPolicy = true;
  }

  if (bHistory) {
    i++;
    CString cs_history, cs_temp;
    cs_history.LoadString(IDS_SETPWHISTFILTERS);
    iItem = m_FilterLC.InsertItem(i /* VFLC_FILTER_NUMBER */, _T("-"));
    m_FilterLC.SetItemText(iItem, VFLC_FILTER_ACTIVE, _T("---"));
    m_FilterLC.SetItemText(iItem, VFLC_AND_OR, _T("---"));
    m_FilterLC.SetItemText(iItem, VFLC_FIELD, _T("---"));
    cs_temp = _T("---  ") + cs_history + _T("  ---");
    m_FilterLC.SetItemText(iItem, VFLC_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vHfldata.begin();
       Flt_iter != pfilters->vHfldata.end(); Flt_iter++) {
    st_FilterData &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(_T("%d"), n);
    UINT nID = GetFieldTypeName(st_fldata.ftype);
    cs_ftype.LoadString(nID);
    cs_ftype.TrimRight(_T('\t'));
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata);
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    if (Flt_iter != pfilters->vHfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = _T("");

    iItem = m_FilterLC.InsertItem(i /* VFLC_FILTER_NUMBER */, cs_num);
    m_FilterLC.SetItemText(iItem, VFLC_FILTER_ACTIVE, cs_act);
    m_FilterLC.SetItemText(iItem, VFLC_AND_OR, cs_ltype);
    m_FilterLC.SetItemText(iItem, VFLC_FIELD, cs_ftype);
    m_FilterLC.SetItemText(iItem, VFLC_CRITERIA_TEXT, cs_criteria);
  }

  if (bPolicy) {
    i++;
    CString cs_policy, cs_temp;
    cs_policy.LoadString(IDS_SETPWPOLICYFILTER);
    iItem = m_FilterLC.InsertItem(i /* VFLC_FILTER_NUMBER */, _T("-"));
    m_FilterLC.SetItemText(iItem, VFLC_FILTER_ACTIVE, _T("---"));
    m_FilterLC.SetItemText(iItem, VFLC_AND_OR, _T("---"));
    m_FilterLC.SetItemText(iItem, VFLC_FIELD, _T("---"));
    cs_temp = _T("---  ") + cs_policy + _T("  ---");
    m_FilterLC.SetItemText(iItem, VFLC_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vPfldata.begin();
       Flt_iter != pfilters->vPfldata.end(); Flt_iter++) {
    st_FilterData &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(_T("%d"), n);
    UINT nID = GetFieldTypeName(st_fldata.ftype);
    cs_ftype.LoadString(nID);
    cs_ftype.TrimRight(_T('\t'));
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata);
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    if (Flt_iter != pfilters->vPfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = _T("");

    iItem = m_FilterLC.InsertItem(i, cs_num);
    m_FilterLC.SetItemText(iItem, 1, cs_act);
    m_FilterLC.SetItemText(iItem, 2, cs_ltype);
    m_FilterLC.SetItemText(iItem, 3, cs_ftype);
    m_FilterLC.SetItemText(iItem, 4, cs_criteria);
  }

  int itotalwidth = 0;
  for (int i = 0; i < VFLC_CRITERIA_TEXT; i++) {
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int iw1 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int iw2 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, max(iw1, iw2));
    itotalwidth += max(iw1, iw2);
  }

  m_FilterLC.SetColumnWidth(VFLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
  itotalwidth += m_FilterLC.GetColumnWidth(VFLC_CRITERIA_TEXT);

  int iMaxWidth = itotalwidth + 32;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);
}

void CViewFilterDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (!IsWindow(m_FilterLC.GetSafeHwnd()))
    return;

  // As main control is a CListCtrl, need to do this on the last column
  m_FilterLC.SetColumnWidth(VFLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
}

UINT CViewFilterDlg::GetFieldTypeName(const FieldType &ft)
{
  UINT nID(0);
  switch (ft) {
    case FT_GROUP:
      nID = IDSC_EXPHDRGROUP;
      break;
    case FT_TITLE:
      nID = IDSC_EXPHDRTITLE;
      break;
    case FT_GROUPTITLE:
      nID = IDSC_EXPHDRGROUPTITLE;
      break;
    case FT_USER:
      nID = IDSC_EXPHDRUSERNAME;
      break;
    case FT_PASSWORD:
      nID = IDSC_EXPHDRPASSWORD;
      break;
    case FT_NOTES:
      nID = IDSC_EXPHDRNOTES;
      break;
    case FT_URL:
      nID = IDSC_EXPHDRURL;
      break;
    case FT_AUTOTYPE:
      nID = IDSC_EXPHDRAUTOTYPE;
      break;
    case FT_CTIME:
      nID = IDSC_EXPHDRCTIME;
      break;
    case FT_ATIME:
      nID = IDSC_EXPHDRATIME;
      break;
    case FT_PMTIME:
      nID = IDSC_EXPHDRPMTIME;
      break;
    case FT_XTIME:
      nID = IDSC_EXPHDRXTIME;
      break;
    case FT_XTIME_INT:
      nID = IDSC_EXPHDRXTIMEINT;
      break;
    case FT_RMTIME:
      nID = IDSC_EXPHDRRMTIME;
      break;
    case FT_PWHIST:
      nID = IDS_PASSWORDHISTORY;
      break;
    case FT_POLICY:
      nID = IDSC_EXPHDRPWPOLICY;
      break;
    case FT_ENTRYTYPE:
      nID = IDS_ENTRYTYPE;
      break;
    case FT_UNKNOWNFIELDS:
      nID = IDS_UNKNOWNFIELDSFILTER;
      break;
    case HT_PRESENT:
      nID = IDS_PRESENT;
      break;
    case HT_ACTIVE:
      nID = IDS_HACTIVE;
      break;
    case HT_NUM:
      nID = IDS_HNUM;
      break;
    case HT_MAX:
      nID = IDS_HMAX;
      break;
    case HT_CHANGEDATE:
      nID = IDS_HDATE;
      break;
    case HT_PASSWORDS:
      nID = HT_PASSWORDS;
      break;
    case PT_PRESENT:
      nID = IDS_PRESENT;
      break;
    case PT_LENGTH:
      nID = IDS_PLENGTH;
      break;
    case PT_LOWERCASE:
      nID = IDS_PLOWER;
      break;
    case PT_UPPERCASE:
      nID = IDS_PUPPER;
      break;
    case PT_DIGITS:
      nID = IDS_PDIGITS;
      break;
    case PT_SYMBOLS:
      nID = IDS_PSYMBOL;
      break;
    case PT_HEXADECIMAL:
      nID = IDS_PHEXADECIMAL;
      break;
    case PT_EASYVISION:
      nID = IDS_PEASYVISION;
      break;
    case PT_PRONOUNCEABLE:
      nID = IDS_PPRONOUNCEABLE;
      break;
    default:
      ASSERT(0);
  }
  return nID;
}
