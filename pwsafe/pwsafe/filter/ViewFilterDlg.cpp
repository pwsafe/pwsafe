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
#include "../PWDialog.h"
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

IMPLEMENT_DYNAMIC(CViewFilterDlg, CPWDialog)

CViewFilterDlg::CViewFilterDlg(CWnd* pParent,
                               st_filters *pfilters,
                               MapFilters &pmapdbfilters,
                               MapFilters &pmapglobalfilters)
  : CPWDialog(CViewFilterDlg::IDD, pParent),
  m_pfilters(pfilters),
  m_pMapDBFilters(pmapdbfilters), m_pMapGlobalFilters(pmapglobalfilters),
  m_selectedstore(m_selectedstore), m_bInitDone(false), m_bStatusBarOK(false)
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

BEGIN_MESSAGE_MAP(CViewFilterDlg, CPWDialog)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
  ON_BN_CLICKED(IDC_CURRENTFILTERSBTN, OnBnClickedCurrent)
  ON_BN_CLICKED(IDC_DATABASEFILTERSBTN, OnBnClickedDBStore)
  ON_BN_CLICKED(IDC_GLOBALFILTERBTN, OnBnClickedGlobalStore)
  ON_CBN_SELCHANGE(IDC_FILTERNAMECOMBO, OnFilterSelected)
END_MESSAGE_MAP()

// ViewFilter message handlers

BOOL CViewFilterDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  // Add the status bar
  if (m_statusBar.CreateEx(this, SBARS_SIZEGRIP)) {
    UINT statustext[1] = {IDS_BLANK};
    m_statusBar.SetIndicators(statustext, 1);
    m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
    m_statusBar.UpdateWindow();

    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    m_bStatusBarOK = true;
  }

  // Arrange all the controls - needed for resizeable dialog
  CRect sbRect, ctrlRect, dlgRect;
  int xleft, ytop;

  GetClientRect(&dlgRect);
  m_DialogMinWidth = dlgRect.Width();
  m_DialogMinHeight = dlgRect.Height();

  m_statusBar.GetWindowRect(&sbRect);

  m_FilterLC.GetWindowRect(&ctrlRect);
  ScreenToClient(&ctrlRect);

  m_cxBSpace = dlgRect.Size().cx - ctrlRect.Size().cx;
  m_cyBSpace = dlgRect.Size().cy - ctrlRect.Size().cy;
  m_cySBar = sbRect.Size().cy;

  m_FilterLC.SetWindowPos(NULL, NULL, NULL,
                          dlgRect.Size().cx - (2 * ctrlRect.TopLeft().x),
                          dlgRect.Size().cy - m_cyBSpace,
                          SWP_NOMOVE | SWP_NOZORDER);

  GetWindowRect(&dlgRect);

  CWnd *pwnd = GetDlgItem(IDOK);

  pwnd->GetWindowRect(&ctrlRect);
  xleft = (m_DialogMinWidth / 2) - (ctrlRect.Width() / 2);
  ytop = dlgRect.Height() - m_cyBSpace / 2;  // - m_cySBar;

  pwnd->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

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
  m_FilterLC.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_FILTERACTIVE);
  cs_text += _T("?");
  m_FilterLC.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_AND_OR);
  m_FilterLC.InsertColumn(2, cs_text);
  cs_text.LoadString(IDS_FILTERFIELD);
  m_FilterLC.InsertColumn(3, cs_text);
  cs_text.LoadString(IDS_CRITERIA_DESC);
  m_FilterLC.InsertColumn(4, cs_text);

  // Make first 4 columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;
  m_FilterLC.SetColumn(0, &lvc);
  m_FilterLC.SetColumn(1, &lvc);
  m_FilterLC.SetColumn(2, &lvc);

  if (m_selectedstore != VF_CURRENT) {
    m_combo.SetCurSel(0);
    OnFilterSelected();
  } else
    SelectFilter(m_pfilters);

  for (int i = 0; i < 5; i++) {
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  m_bInitDone = true;

  UpdateData(FALSE);

  return TRUE;
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
    iItem = m_FilterLC.InsertItem(i, _T("-"));
    m_FilterLC.SetItemText(iItem, 1, _T("---"));
    m_FilterLC.SetItemText(iItem, 2, _T("---"));
    m_FilterLC.SetItemText(iItem, 3, _T("---"));
    cs_temp = _T("---  ") + cs_history + _T("  ---");
    m_FilterLC.SetItemText(iItem, 4, cs_temp);
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

    iItem = m_FilterLC.InsertItem(i, cs_num);
    m_FilterLC.SetItemText(iItem, 1, cs_act);
    m_FilterLC.SetItemText(iItem, 2, cs_ltype);
    m_FilterLC.SetItemText(iItem, 3, cs_ftype);
    m_FilterLC.SetItemText(iItem, 4, cs_criteria);
  }

  if (bPolicy) {
    i++;
    CString cs_policy, cs_temp;
    cs_policy.LoadString(IDS_SETPWPOLICYFILTER);
    iItem = m_FilterLC.InsertItem(i, _T("-"));
    m_FilterLC.SetItemText(iItem, 1, _T("---"));
    m_FilterLC.SetItemText(iItem, 2, _T("---"));
    m_FilterLC.SetItemText(iItem, 3, _T("---"));
    cs_temp = _T("---  ") + cs_policy + _T("  ---");
    m_FilterLC.SetItemText(iItem, 4, cs_temp);
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

  for (int i = 0; i < 4; i++) {
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int iw1 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int iw2 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, max(iw1, iw2));
  }
  m_FilterLC.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
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

void CViewFilterDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWDialog::OnSize(nType, cx, cy);

  if (!m_bInitDone || !m_bStatusBarOK)
    return;

  SetControls(cx, cy);
}

void CViewFilterDlg::SetControls(int cx, int cy)
{
  if (!m_bInitDone || !m_bStatusBarOK)
    return;

  if (!IsWindow(m_FilterLC.GetSafeHwnd()))
    return;

  CRect ctrlRect, dlgRect, ctrlrect2;
  CPoint pt_top;

  GetWindowRect(&dlgRect);

  // Allow ListCtrl to grow/shrink but leave room for the buttons underneath!
  m_FilterLC.GetWindowRect(&ctrlRect);

  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  m_FilterLC.MoveWindow(pt_top.x, pt_top.y,
                        cx - (2 * pt_top.x), cy - m_cyBSpace, TRUE);

  // Keep buttons in the bottom area
  int xleft, ytop;

  ytop = dlgRect.Height() - m_cyBSpace / 2; // - m_cySBar;

  CWnd *pwnd = GetDlgItem(IDOK);

  pwnd->GetWindowRect(&ctrlRect);
  xleft = (cx / 2) - (ctrlRect.Width() / 2);
  pwnd->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

  // Now move the status bar
  m_statusBar.GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  m_statusBar.MoveWindow(pt_top.x, cy - ctrlRect.Height(),
                         cx - (2 * pt_top.x),
                         ctrlRect.Height(), TRUE);
}

void CViewFilterDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  if (m_bInitDone) {
    lpMMI->ptMinTrackSize = CPoint(m_DialogMinWidth, m_DialogMinHeight);
  } else
    CPWDialog::OnGetMinMaxInfo(lpMMI);
}
