/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ManageFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "ManageFiltersDlg.h"
#include "../DboxMain.h"
#include "../PWResizeDialog.h"
#include "../corelib/PWSFilters.h"
#include "../corelib/match.h"

#include "../resource3.h"
#include "../corelib/corelib.h"

#include <vector>
#include <map>
#include <algorithm>

// ManageFiltersDlg dialog

IMPLEMENT_DYNAMIC(CManageFiltersDlg, CPWResizeDialog)

CManageFiltersDlg::CManageFiltersDlg(CWnd* pParent,
                               bool bFilterActive,
                               PWSFilters &mapfilters)
  : CPWResizeDialog(CManageFiltersDlg::IDD, pParent),
  m_bFilterActive(bFilterActive), m_MapFilters(mapfilters),
  m_selectedfilterpool(FPOOL_LAST), m_selectedfiltername(_T("")),
  m_activefilterpool(FPOOL_LAST), m_activefiltername(_T("")),
  m_selectedfilter(-1), m_inusefilter(-1),
  m_bStopChange(false), m_bDBFiltersChanged(false),
  m_num_to_export(0), m_num_to_copy(0),
  m_pCheckImageList(NULL), m_pImageList(NULL),
  m_iSortColumn(-1), m_bSortAscending(-1)
{
  m_pDbx = static_cast<DboxMain *>(pParent);

  PWSFilters::iterator mf_iter;

  for (mf_iter = m_MapFilters.begin();
       mf_iter != m_MapFilters.end();
       mf_iter++) {
    m_vcs_filters.push_back(mf_iter->first);
  }

  const COLORREF crTransparent = RGB(192, 192, 192);
  CBitmap bitmap;
  BITMAP bm;
  bitmap.LoadBitmap(IDB_CHECKED);
  bitmap.GetBitmap(&bm); // should be 13 x 13

  m_pCheckImageList = new CImageList;
  BOOL status = m_pCheckImageList->Create(bm.bmWidth, bm.bmHeight,
                                     ILC_MASK | ILC_COLOR, 3, 0);
  ASSERT(status != 0);

  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_UNCHECKED);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_EMPTY);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_BLANK);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
}

CManageFiltersDlg::~CManageFiltersDlg()
{
}

void CManageFiltersDlg::OnDestroy()
{
  // Tidy up filter ItemData
  int nCount = m_FilterLC.GetItemCount();
  for (int i = 0; i < nCount; i++) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(0);
    delete pflt_idata;
    m_FilterLC.DeleteItem(0);
  }

  // Tidy up images
  m_pImageList->DeleteImageList();
  delete m_pImageList;

  m_pCheckImageList->DeleteImageList();
  delete m_pCheckImageList;

  CPWResizeDialog::OnDestroy();
}

void CManageFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FILTERLC, m_FilterLC);
  DDX_Control(pDX, IDC_FILTERPROPERTIES, m_FilterProperties);
}

BEGIN_MESSAGE_MAP(CManageFiltersDlg, CPWResizeDialog)
  ON_WM_DESTROY()
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_FILTERAPPLY, OnFilterApply)
  ON_BN_CLICKED(IDC_FILTERUNAPPLY, OnFilterUnApply)
  ON_BN_CLICKED(IDC_FILTERNEW, OnFilterNew)
  ON_BN_CLICKED(IDC_FILTEREDIT, OnFilterEdit)
  ON_BN_CLICKED(IDC_FILTERCOPY, OnFilterCopy)
  ON_BN_CLICKED(IDC_FILTERDELETE, OnFilterDelete)
  ON_BN_CLICKED(IDC_FILTERIMPORT, OnFilterImport)
  ON_BN_CLICKED(IDC_FILTEREXPORT, OnFilterExport)
  ON_NOTIFY(NM_CLICK, IDC_FILTERLC, OnClick)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_FILTERLC, OnCustomDraw)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_FILTERLC, OnItemChanging)
  ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILTERLC, OnColumnClick)
  ON_NOTIFY(HDN_BEGINTRACK, IDC_FILTERLC_HEADER, OnHDRBeginTrack)
  ON_NOTIFY(HDN_ITEMCHANGING, IDC_FILTERLC_HEADER, OnHDRItemChanging)
  ON_NOTIFY(HDN_BEGINTRACK, IDC_FILTERPROP_HEADER, OnHDRBeginTrack)
  ON_NOTIFY(HDN_ITEMCHANGING, IDC_FILTERPROP_HEADER, OnHDRItemChanging)
END_MESSAGE_MAP()

// ManageFilters message handlers

BOOL CManageFiltersDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDC_FILTERAPPLY);
  vibottombtns.push_back(IDC_FILTERUNAPPLY);
  vibottombtns.push_back(IDC_FILTERNEW);
  vibottombtns.push_back(IDC_FILTEREDIT);
  vibottombtns.push_back(IDC_FILTERCOPY);
  vibottombtns.push_back(IDC_FILTERDELETE);
  vibottombtns.push_back(IDC_FILTERIMPORT);
  vibottombtns.push_back(IDC_FILTEREXPORT);
  vibottombtns.push_back(IDOK);

  AddMainCtrlID(IDC_FILTERPROPERTIES);
  AddBtnsCtrlIDs(vibottombtns);

  UINT statustext[1] = {IDS_BLANK};
  SetStatusBar(&statustext[0], 1, false);

  CPWResizeDialog::OnInitDialog();

  // Make some columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  CString cs_text;
  cs_text = _T("Name");
  m_FilterLC.InsertColumn(MFLC_FILTER_NAME, cs_text);
  cs_text = _T("Source");
  m_FilterLC.InsertColumn(MFLC_FILTER_SOURCE, cs_text);
  cs_text = _T("Copy to DB");
  m_FilterLC.InsertColumn(MFLC_COPYTODATABASE, cs_text);
  cs_text = _T("Export");
  m_FilterLC.InsertColumn(MFLC_EXPORT, cs_text);
  cs_text = _T("Currently Active");
  m_FilterLC.InsertColumn(MFLC_INUSE, cs_text);

  // Make some columns centered
  m_FilterLC.SetColumn(MFLC_FILTER_SOURCE, &lvc);
  m_FilterLC.SetColumn(MFLC_COPYTODATABASE, &lvc);
  m_FilterLC.SetColumn(MFLC_EXPORT, &lvc);
  m_FilterLC.SetColumn(MFLC_INUSE, &lvc);

  // Populate CListCtrl
  UpdateFilterList();

  // Set row height to take image by adding a dummy ImageList
  CRect rect;
  m_FilterLC.GetItemRect(0, &rect, LVIR_BOUNDS);
  IMAGEINFO imageinfo;
  m_pCheckImageList->GetImageInfo(0, &imageinfo);
  int irowheight = max(rect.Height(),
                       abs(imageinfo.rcImage.top - imageinfo.rcImage.bottom));
  m_pImageList = new CImageList;
  m_pImageList->Create(1, irowheight, ILC_COLOR4, 1, 1);
  m_FilterLC.SetImageList(m_pImageList, LVSIL_SMALL);

  dwExStyle = m_FilterProperties.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterProperties.SetExtendedStyle(dwExStyle);

  cs_text = _T(" # ");
  m_FilterProperties.InsertColumn(MFPRP_FILTER_NUMBER, cs_text);
  cs_text.LoadString(IDS_FILTERACTIVE);
  cs_text += _T("?");
  m_FilterProperties.InsertColumn(MFPRP_FILTER_ACTIVE, cs_text);
  cs_text.LoadString(IDS_AND_OR);
  m_FilterProperties.InsertColumn(MFPRP_AND_OR, cs_text);
  cs_text.LoadString(IDS_FILTERFIELD);
  m_FilterProperties.InsertColumn(MFPRP_FIELD, cs_text);
  cs_text.LoadString(IDS_CRITERIA_DESC);
  m_FilterProperties.InsertColumn(MFPRP_CRITERIA_TEXT, cs_text);

  // Make first 3 columns centered
  m_FilterProperties.SetColumn(MFPRP_FILTER_NUMBER, &lvc);
  m_FilterProperties.SetColumn(MFPRP_FILTER_ACTIVE, &lvc);
  m_FilterProperties.SetColumn(MFPRP_AND_OR, &lvc);

  int itotalwidth = 0;
  for (int i = 0; i < MFPRP_NUM_COLUMNS; i++) {
    m_FilterProperties.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    itotalwidth += m_FilterProperties.GetColumnWidth(i);
  }

  BOOL bEnable = (m_selectedfilter != -1 && 
                  m_selectedfilter == m_inusefilter &&
                  m_bFilterActive) ? TRUE : FALSE;
  GetDlgItem(IDC_FILTERUNAPPLY)->EnableWindow(bEnable);

  int iMaxWidth = itotalwidth + 16;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);

  m_FilterLC.GetHeaderCtrl()->SetDlgCtrlID(IDC_FILTERLC_HEADER);
  m_FilterProperties.GetHeaderCtrl()->SetDlgCtrlID(IDC_FILTERPROP_HEADER);

  m_bStopChange = true;

  UpdateData(FALSE);

  return FALSE;
}

void CManageFiltersDlg::OnClick(NMHDR *pNotifyStruct, LRESULT *pResult)
{
  LPNMITEMACTIVATE pNMLV = reinterpret_cast<LPNMITEMACTIVATE>(pNotifyStruct);
  *pResult = FALSE;

  if (pNMLV->iItem < 0) {
    m_selectedfilter = -1;
    m_selectedfiltername = _T("");
    m_selectedfilterpool = FPOOL_LAST;
    return;
  }

  st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(pNMLV->iItem);
  switch (pNMLV->iSubItem) {
    case MFLC_COPYTODATABASE:
      if ((pflt_idata->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB) {
        pflt_idata->flt_flags &= ~MFLT_REQUEST_COPY_TO_DB;
        m_num_to_copy--;
      } else {
        pflt_idata->flt_flags |= MFLT_REQUEST_COPY_TO_DB;
        m_num_to_copy++;
      }
      m_FilterLC.SetItemData(pNMLV->iItem, (DWORD)pflt_idata);
      GetDlgItem(IDC_FILTERCOPY)->EnableWindow(m_num_to_copy > 0);
      break;
    case MFLC_EXPORT:
      if ((pflt_idata->flt_flags & MFLT_REQUEST_EXPORT) == MFLT_REQUEST_EXPORT) {
        pflt_idata->flt_flags &= ~MFLT_REQUEST_EXPORT;
        m_num_to_export--;
      } else {
        pflt_idata->flt_flags |= MFLT_REQUEST_EXPORT;
        m_num_to_export++;
      }
      m_FilterLC.SetItemData(pNMLV->iItem, (DWORD)pflt_idata);
      GetDlgItem(IDC_FILTEREXPORT)->EnableWindow(m_num_to_export > 0);
      break;
    default:
      break;
  }

  m_selectedfilter = pNMLV->iItem;
  m_FilterProperties.DeleteAllItems();

  m_selectedfiltername = m_FilterLC.GetItemText(m_selectedfilter, 0);
  pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_selectedfilter);
  m_selectedfilterpool = pflt_idata->flt_key.fpool;
  GetDlgItem(IDC_STATIC_FILTERNAME)->SetWindowText(m_selectedfiltername);

  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = m_selectedfiltername;

  mf_iter = m_MapFilters.find(flt_key);
  if (mf_iter == m_MapFilters.end())
    return;

  st_filters *pfilters = &mf_iter->second;
  DisplayFilterProperties(pfilters);

  BOOL bEnable = (m_selectedfilter != -1 && 
                  m_selectedfilter == m_inusefilter &&
                  m_bFilterActive) ? TRUE : FALSE;
  GetDlgItem(IDC_FILTERUNAPPLY)->EnableWindow(bEnable);

  m_FilterLC.Invalidate();
}

void CManageFiltersDlg::OnFilterNew()
{
  st_filters filters;
  if (m_pDbx->EditFilter(&filters)) {
    st_Filterkey flt_key;
    flt_key.fpool = FPOOL_SESSION;
    flt_key.cs_filtername = filters.fname;

    m_MapFilters.insert(PWSFilters::Pair(flt_key, filters));

    UpdateFilterList();
    DisplayFilterProperties(&filters);
  }
}

void CManageFiltersDlg::OnFilterEdit()
{
  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = m_selectedfiltername;

  mf_iter = m_MapFilters.find(flt_key);
  if (mf_iter == m_MapFilters.end())
    return;

  st_filters *pfilters = &mf_iter->second;
  if (m_pDbx->EditFilter(pfilters)) {
    if (flt_key.fpool == FPOOL_DATABASE)
      m_bDBFiltersChanged = true;

    UpdateFilterList();
    DisplayFilterProperties(pfilters);
  }
}

void CManageFiltersDlg::OnFilterCopy()
{
  int numfilters = m_FilterLC.GetItemCount();
  bool bCopied(false);

  for (int i = 0; i < numfilters; i++) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(i);
    if ((pflt_idata->flt_flags & MFLT_REQUEST_COPY_TO_DB) != MFLT_REQUEST_COPY_TO_DB)
      continue;

    PWSFilters::iterator mf_iter;
    st_Filterkey flt_key;
    flt_key = pflt_idata->flt_key;

    mf_iter = m_MapFilters.find(flt_key);
    if (mf_iter == m_MapFilters.end())
      return;

    PWSFilters::const_iterator mf_citer;
    st_Filterkey flt_keydb;
    flt_keydb.fpool = FPOOL_DATABASE;
    flt_keydb.cs_filtername = flt_key.cs_filtername;
    mf_citer = m_MapFilters.find(flt_keydb);

    // Check if already there (i.e. ask user if to replace)
    if (mf_citer != m_MapFilters.end()) {
      CString cs_msg(MAKEINTRESOURCE(IDS_REPLACEFILTER));
      CString cs_title(MAKEINTRESOURCE(IDS_FILTEREXISTS));
      int rc = MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      if (rc == IDNO)
        continue;  // skip this one

      // User agrees to replace
      m_MapFilters.erase(flt_keydb);
    }
    m_MapFilters.insert(PWSFilters::Pair(flt_keydb, mf_iter->second));

    // Turn off copy flag
    pflt_idata->flt_flags &= ~MFLT_REQUEST_COPY_TO_DB;
    m_FilterLC.SetItemData(i, (DWORD)pflt_idata);
    m_num_to_copy--;
    bCopied = true;
  }
  if (bCopied) {
    m_bDBFiltersChanged = true;
    m_pDbx->SetChanged(DboxMain::Data);
    m_pDbx->ChangeOkUpdate();
  }

  UpdateFilterList();
}

void CManageFiltersDlg::OnFilterDelete()
{
  if (m_selectedfilter < 0)
    return;

  CString cs_pool(_T(""));
  CString cs_selected = m_FilterLC.GetItemText(m_selectedfilter, 0);

  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = cs_selected;

  mf_iter = m_MapFilters.find(flt_key);
  if (mf_iter == m_MapFilters.end())
    return;

  switch (flt_key.fpool) {
    case FPOOL_DATABASE:
      cs_pool = _T("Database");
      break;
    case FPOOL_AUTOLOAD:
      cs_pool = _T("Autoload");
      break;
    case FPOOL_IMPORTED:
      cs_pool = _T("Imported");
      break;
    case FPOOL_SESSION:
      cs_pool = _T("Session");
      break;
    default:
      ASSERT(0);
  }
  // Now to confirm with user:
  CString cs_msg;
  cs_msg.Format(IDS_CONFIRMFILTERDELETE, cs_pool, cs_selected);
  if (AfxMessageBox(cs_msg, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES)
    return;

  m_MapFilters.erase(flt_key);
  if (m_selectedfilterpool == FPOOL_DATABASE) {
    m_bDBFiltersChanged = true;
    m_pDbx->SetChanged(DboxMain::Data);
    m_pDbx->ChangeOkUpdate();
  }

  st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_selectedfilter);
  delete pflt_idata;
  m_FilterLC.DeleteItem(m_selectedfilter);
  m_FilterProperties.DeleteAllItems();
  if (m_selectedfilter == m_inusefilter)
    m_inusefilter = -1;

  m_selectedfilter = -1;
  m_selectedfilterpool = FPOOL_LAST;
  GetDlgItem(IDC_STATIC_FILTERNAME)->SetWindowText(_T(""));
}

void CManageFiltersDlg::OnFilterImport()
{
  m_pDbx->ImportFilters();

  UpdateFilterList();
}

void CManageFiltersDlg::OnFilterExport()
{
  PWSFilters Filters;
  int numfilters = m_FilterLC.GetItemCount();

  for (int i = 0; i < numfilters; i++) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(i);
    if ((pflt_idata->flt_flags & MFLT_REQUEST_EXPORT) != MFLT_REQUEST_EXPORT)
      continue;

    PWSFilters::iterator mf_iter;
    mf_iter = m_MapFilters.find(pflt_idata->flt_key);
    if (mf_iter == m_MapFilters.end())
      continue;

    Filters.insert(PWSFilters::Pair(pflt_idata->flt_key, mf_iter->second));
  }
  if (Filters.size() > 0) {
    m_pDbx->ExportFilters(Filters);
    Filters.clear();
  }
}

void CManageFiltersDlg::OnFilterApply()
{
  m_pDbx->SetFilter(m_selectedfilterpool, m_selectedfiltername);
  if (!m_pDbx->ApplyFilter(true))
    return;

  m_FilterLC.SetItemText(m_selectedfilter, MFLC_INUSE, _T("Yes"));
  m_activefilterpool = m_selectedfilterpool;
  m_activefiltername = m_selectedfiltername;

  st_FilterItemData *pflt_idata;
  if (m_selectedfilter != m_inusefilter && m_inusefilter != -1) {
    m_FilterLC.SetItemText(m_inusefilter, MFLC_INUSE, _T(" "));
    pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_inusefilter);
    pflt_idata->flt_flags &= ~MFLT_INUSE;
    m_FilterLC.SetItemData(m_inusefilter, (DWORD)pflt_idata);
  }
  m_inusefilter = m_selectedfilter;
  pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_selectedfilter);
  pflt_idata->flt_flags |= MFLT_INUSE;
  m_FilterLC.SetItemData(m_selectedfilter, (DWORD)pflt_idata);
  m_bFilterActive = true;

  m_FilterLC.Invalidate();  // Ensure selected statement updated
  GetDlgItem(IDC_FILTERUNAPPLY)->EnableWindow(TRUE);
}

void CManageFiltersDlg::OnFilterUnApply()
{
  m_pDbx->ClearFilter();

  m_FilterLC.SetItemText(m_inusefilter, MFLC_INUSE, _T(" "));
  m_activefilterpool = FPOOL_LAST;
  m_activefiltername = _T("");
  st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_inusefilter);
  pflt_idata->flt_flags &= ~MFLT_INUSE;
  m_FilterLC.SetItemData(m_inusefilter, (DWORD)pflt_idata);
  m_bFilterActive = false;

  m_FilterLC.Invalidate();  // Ensure selected statement updated
  GetDlgItem(IDC_FILTERUNAPPLY)->EnableWindow(FALSE);
}

void CManageFiltersDlg::DisplayFilterProperties(st_filters *pfilters)
{
  CString cs_num, cs_ftype, cs_criteria, cs_ltype, cs_act;

  vFilterRows::iterator Flt_iter;
  bool bHistory(false), bPolicy(false);
  int i(0), iItem(0), n(0);

  // Do the main filters
  for (Flt_iter = pfilters->vMfldata.begin();
       Flt_iter != pfilters->vMfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
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

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);

    if (st_fldata.ftype == FT_PWHIST)
      bHistory = true;
    if (st_fldata.ftype == FT_POLICY)
      bPolicy = true;
  }

  if (bHistory) {
    i++;
    CString cs_history, cs_temp;
    cs_history.LoadString(IDS_SETPWHISTFILTERS);
    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, _T("-"));
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, _T("---"));
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, _T("---"));
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, _T("---"));
    cs_temp = _T("---  ") + cs_history + _T("  ---");
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vHfldata.begin();
       Flt_iter != pfilters->vHfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
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

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);
  }

  if (bPolicy) {
    i++;
    CString cs_policy, cs_temp;
    cs_policy.LoadString(IDS_SETPWPOLICYFILTER);
    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, _T("-"));
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, _T("---"));
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, _T("---"));
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, _T("---"));
    cs_temp = _T("---  ") + cs_policy + _T("  ---");
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vPfldata.begin();
       Flt_iter != pfilters->vPfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
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

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);
  }

  int itotalwidth = 0;
  for (int i = 0; i < MFPRP_CRITERIA_TEXT; i++) {
    m_FilterProperties.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int iw1 =  m_FilterProperties.GetColumnWidth(i);
    m_FilterProperties.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int iw2 =  m_FilterProperties.GetColumnWidth(i);
    m_FilterProperties.SetColumnWidth(i, max(iw1, iw2));
    itotalwidth += max(iw1, iw2);
  }

  m_FilterProperties.SetColumnWidth(MFPRP_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
  itotalwidth += m_FilterProperties.GetColumnWidth(MFPRP_CRITERIA_TEXT);

  int iMaxWidth = itotalwidth + 32;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);
}

void CManageFiltersDlg::UpdateFilterList()
{
  int iItem;
  int nCount = m_FilterLC.GetItemCount();
  for (int i = 0; i < nCount; i++) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(0);
    delete pflt_idata;
    m_FilterLC.DeleteItem(0);
  }
  m_vcs_filters.clear();

  PWSFilters::iterator mf_iter;
  int i(0);
  m_selectedfilter = -1;
  for (mf_iter = m_MapFilters.begin();
       mf_iter != m_MapFilters.end();
       mf_iter++) {
    m_vcs_filters.push_back(mf_iter->first);

    iItem = m_FilterLC.InsertItem(i /* MFLC_FILTER_NAME */, mf_iter->first.cs_filtername);
    CString cs_source(_T(""));
    switch (mf_iter->first.fpool) {
      case FPOOL_DATABASE:
        cs_source = _T("Database");
        break;
      case FPOOL_AUTOLOAD:
        cs_source = _T("Autoload");
        break;
      case FPOOL_IMPORTED:
        cs_source = _T("Imported");
        break;
      case FPOOL_SESSION:
        cs_source = _T("Session");
        break;
      default:
        ASSERT(0);
    }
    m_FilterLC.SetItemText(iItem, MFLC_FILTER_SOURCE, cs_source);
    if (m_bFilterActive &&
        mf_iter->first.fpool == m_activefilterpool &&
        mf_iter->first.cs_filtername == m_activefiltername) {
      m_FilterLC.SetItemText(iItem, MFLC_INUSE, _T("Yes"));
      m_inusefilter = iItem;
    } else {
      m_FilterLC.SetItemText(iItem, MFLC_INUSE, _T(" "));
    }
    if (mf_iter->first.fpool == m_selectedfilterpool &&
        mf_iter->first.cs_filtername == m_selectedfiltername) {
      m_FilterLC.SetItemText(iItem, MFLT_SELECTED, _T("Yes"));
      m_selectedfilter = iItem;
    } else {
      m_FilterLC.SetItemText(iItem, MFLT_SELECTED, _T(" "));
    }
    st_FilterItemData *pflt_idata = new st_FilterItemData;
    pflt_idata->flt_key = mf_iter->first;
    pflt_idata->flt_flags = (m_inusefilter == iItem) ? MFLT_INUSE : 0;
    if (m_selectedfilter == iItem)
      pflt_idata->flt_flags |= MFLT_SELECTED;
    m_FilterLC.SetItemData(iItem, (DWORD)pflt_idata);
    i++;
  }

  ResetColumns();

  if (m_selectedfilter != -1)
    m_FilterLC.SetItem(m_selectedfilter, 0, LVIF_STATE, NULL, 0, 
                           LVIS_SELECTED, LVIS_SELECTED, 0);

  // None selected for copy/export at first go
  GetDlgItem(IDC_FILTERCOPY)->EnableWindow(m_num_to_copy > 0 ? TRUE : FALSE);
  GetDlgItem(IDC_FILTEREXPORT)->EnableWindow(m_num_to_export > 0 ? TRUE : FALSE);

  // Sort them
  SortFilterView();
}

void CManageFiltersDlg::ResetColumns()
{
  bool bSave = m_bStopChange;
  m_bStopChange = false;
  int iw1, iw2;
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, LVSCW_AUTOSIZE);
  iw1 = m_FilterLC.GetColumnWidth(MFLC_FILTER_NAME);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, LVSCW_AUTOSIZE_USEHEADER);
  iw2 = m_FilterLC.GetColumnWidth(MFLC_FILTER_NAME);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, max(iw1, iw2));

  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, LVSCW_AUTOSIZE);
  iw1 = m_FilterLC.GetColumnWidth(MFLC_FILTER_SOURCE);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, LVSCW_AUTOSIZE_USEHEADER);
  iw2 = m_FilterLC.GetColumnWidth(MFLC_FILTER_SOURCE);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, max(iw1, iw2));

  m_FilterLC.SetColumnWidth(MFLC_COPYTODATABASE, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterLC.SetColumnWidth(MFLC_EXPORT, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterLC.SetColumnWidth(MFLC_INUSE, LVSCW_AUTOSIZE_USEHEADER);
  m_bStopChange = bSave;
}

void CManageFiltersDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (!IsWindow(m_FilterLC.GetSafeHwnd()))
    return;

  // As main control is a CListCtrl, need to do this on the last column
  m_FilterProperties.SetColumnWidth(MFPRP_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
}

void CManageFiltersDlg::OnItemChanging(NMHDR* pNotifyStruct, LRESULT* pResult)
{
  NMLISTVIEW* pNMLV = reinterpret_cast<NMLISTVIEW *>(pNotifyStruct);
  *pResult = FALSE;

  // Has the state changed?
  if (pNMLV->iItem < 0)
    return;

  if ((pNMLV->uChanged & LVIF_STATE) == LVIF_STATE) {
    UINT uiChangedState = pNMLV->uOldState & ~pNMLV->uNewState;
    if ((uiChangedState & LVIS_FOCUSED) == LVIS_FOCUSED) {
      *pResult = TRUE;
    }
    if ((pNMLV->uNewState & LVIS_SELECTED) == LVIS_SELECTED) {
      m_selectedfilter = pNMLV->iItem;
      *pResult = TRUE;
    }
    if ((pNMLV->uOldState & LVIS_SELECTED) == LVIS_SELECTED) {
      *pResult = TRUE;
    }
  }
}

void CManageFiltersDlg::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pResult = CDRF_DODEFAULT;
  const int iItem = pLVCD->nmcd.dwItemSpec;
  const int iSubItem = pLVCD->iSubItem;

  bool bDatabase(false), bCopy(false), bExport(false);
  st_FilterItemData *pflt_idata(NULL);

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_ITEMPREPAINT:
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(iItem);

      bDatabase = (pflt_idata->flt_flags & FPOOL_DATABASE) == FPOOL_DATABASE;
      bCopy = (pflt_idata->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB;
      bExport = (pflt_idata->flt_flags & MFLT_REQUEST_EXPORT) == MFLT_REQUEST_EXPORT;
      break;
    default:
      break;
  }

  int ix, iy;
  const COLORREF crLightGreen = RGB(222, 255, 222);

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      pLVCD->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
      if (iItem == m_selectedfilter) {
        pLVCD->clrTextBk = crLightGreen;
      } else {
        pLVCD->clrTextBk = m_FilterLC.GetTextBkColor();
      }
      *pResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
        CRect rect;
        m_FilterLC.GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pResult = CDRF_SKIPDEFAULT;
          break;
        }
        if (iSubItem == 0) {
          CRect rect1;
          m_FilterLC.GetSubItemRect(iItem, 1, LVIR_BOUNDS, rect1);
          rect.right = rect1.left;
        }
        CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
        CRect inner_rect(rect), first_rect(rect);
        inner_rect.DeflateRect(2, 2);
        switch (iSubItem) {
          case MFLC_FILTER_NAME:
          case MFLC_FILTER_SOURCE:
          case MFLC_INUSE:
            break;
          case MFLC_COPYTODATABASE:
            if (iItem == m_selectedfilter) {
              pDC->FillSolidRect(&first_rect, crLightGreen);
            }
            *pResult = CDRF_SKIPDEFAULT;
            if (bDatabase)
              break;
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            DrawImage(pDC, inner_rect, bCopy ? 0 : 2);
            break;
          case MFLC_EXPORT:
            if (iItem == m_selectedfilter) {
              pDC->FillSolidRect(&first_rect, crLightGreen);
            }
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            DrawImage(pDC, inner_rect, bExport ? 0 : 2);
            *pResult = CDRF_SKIPDEFAULT;
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
}

void CManageFiltersDlg::DrawImage(CDC *pDC, CRect &rect, int nImage)
{
  // Draw check image in given rectangle
  if (rect.IsRectEmpty() || nImage < 0) {
    return;
  }

  if (m_pCheckImageList) {
    SIZE sizeImage = {0, 0};
    IMAGEINFO info;

    if (m_pCheckImageList->GetImageInfo(nImage, &info)) {
      sizeImage.cx = info.rcImage.right - info.rcImage.left;
      sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
    }

    if (sizeImage.cx > 0 && nImage >= 0) {
      if (rect.Width() > 0) {
        POINT point;

        point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
        point.x = rect.left;

        SIZE size;
        size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
        size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;
        m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
      }
    }
  }
}

void CManageFiltersDlg::OnHDRBeginTrack(NMHDR * /*pNotifyStruct*/, LRESULT* pResult)
{
  // Don't allow user to change the size of any columns!
  *pResult = TRUE;
}

void CManageFiltersDlg::OnHDRItemChanging(NMHDR * pNotifyStruct, LRESULT* pResult)
{
  NMHEADER *pNMHDR = reinterpret_cast<NMHEADER *>(pNotifyStruct);

  // Only allow last column to resize - it has variable data (filter description)
  if (pNMHDR->hdr.idFrom == IDC_FILTERPROP_HEADER &&pNMHDR->iItem == MFPRP_CRITERIA_TEXT)
    *pResult = FALSE;
  else
    *pResult = m_bStopChange ? TRUE : FALSE;
}

UINT CManageFiltersDlg::GetFieldTypeName(const FieldType &ft)
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
/*
* Compare function used by m_FilterLC.SortItems()
*/
int CALLBACK CManageFiltersDlg::FLTCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                            LPARAM pSelf)
{
  // pSelf is "this" of the calling CManageFiltersDlg, from which we use:
  // m_iSortColumn to determine which column is getting sorted
  // m_bSortAscending to determine the direction of the sort (duh)

  CManageFiltersDlg *self = (CManageFiltersDlg *)pSelf;
  const int nSortColumn = self->m_iSortColumn;
  st_FilterItemData *pLHS = (st_FilterItemData *)lParam1;
  st_FilterItemData *pRHS = (st_FilterItemData *)lParam2;
  int i1, i2;

  int iResult;
  switch(nSortColumn) {
    case MFLC_FILTER_NAME:
      iResult = pLHS->flt_key.cs_filtername.Compare(pRHS->flt_key.cs_filtername);
      break;
    case MFLC_FILTER_SOURCE:
      iResult = ((int)pLHS->flt_key.fpool < (int)pRHS->flt_key.fpool) ? -1 : 1;
      break;
    case MFLC_COPYTODATABASE:
      i1 = (int)(pLHS->flt_flags & MFLT_REQUEST_COPY_TO_DB);
      i2 = (int)(pRHS->flt_flags & MFLT_REQUEST_COPY_TO_DB);
      iResult = (i1 < i2) ? -1 : 1;
      break;
    case MFLC_EXPORT:
      i1 = (int)(pLHS->flt_flags & MFLT_REQUEST_EXPORT);
      i2 = (int)(pRHS->flt_flags & MFLT_REQUEST_EXPORT);
      iResult = (i1 < i2) ? -1 : 1;
      break;
    case MFLC_INUSE:
      i1 = (int)(pLHS->flt_flags & MFLT_INUSE);
      i2 = (int)(pRHS->flt_flags & MFLT_INUSE);
      iResult = (i1 < i2) ? -1 : 1;
      break;
    default:
      iResult = 0; // should never happen - just keep compiler happy
      ASSERT(FALSE);
  }
  if (!self->m_bSortAscending) {
    iResult *= -1;
  }
  return iResult;
}

void CManageFiltersDlg::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMLV = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);

  // Get column index to CItemData value
  int iIndex = pNMLV->iSubItem;

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  if (m_iSortColumn == iIndex) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    // Turn off all previous sort arrrows
    CHeaderCtrl *phctrl = m_FilterLC.GetHeaderCtrl();
    for (int i = 0; i < phctrl->GetItemCount(); i++) {
      phctrl->GetItem(i, &hdi);
      if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        phctrl->SetItem(i, &hdi);
      }
    }
    m_iSortColumn = iIndex;
    m_bSortAscending = true;
  }

  SortFilterView();
  *pResult = TRUE;
}

void CManageFiltersDlg::SortFilterView()
{
  if (m_iSortColumn < 0)
    return;

  m_FilterLC.SortItems(FLTCompareFunc, (LPARAM)this);

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  CHeaderCtrl *phctrl = m_FilterLC.GetHeaderCtrl();
  phctrl->GetItem(m_iSortColumn, &hdi);
  // Turn off all arrows in sorted column
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  phctrl->SetItem(m_iSortColumn, &hdi);
}
