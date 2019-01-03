/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ManageFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "../DboxMain.h"
#include "../PWResizeDialog.h"

#include "ManageFiltersDlg.h"

#include "core/core.h"
#include "core/PWSFilters.h"
#include "core/match.h"

#include "../resource3.h"

#include <vector>
#include <map>
#include <algorithm>

// ManageFiltersDlg dialog

IMPLEMENT_DYNAMIC(CManageFiltersDlg, CPWDialog)

CManageFiltersDlg::CManageFiltersDlg(CWnd* pParent,
                               bool bFilterActive,
                               PWSFilters &mapfilters,
                               bool bCanHaveAttachments)
  : CPWDialog(CManageFiltersDlg::IDD, pParent),
  m_bMFFilterActive(bFilterActive), m_MapMFDFilters(mapfilters),
  m_selectedfilterpool(FPOOL_LAST), m_selectedfiltername(L""),
  m_activefilterpool(FPOOL_LAST), m_activefiltername(L""),
  m_selectedfilter(-1), m_activefilter(-1),
  m_bDBFiltersChanged(false),
  m_num_to_export(0), m_num_to_copy(0),
  m_pCheckImageList(NULL), m_pImageList(NULL),
  m_iSortColumn(-1), m_bSortAscending(-1), m_bDBReadOnly(false),
  m_bCanHaveAttachments(bCanHaveAttachments)
{
  PWSFilters::iterator mf_iter;

  for (mf_iter = m_MapMFDFilters.begin();
       mf_iter != m_MapMFDFilters.end();
       mf_iter++) {
    m_vcs_filters.push_back(mf_iter->first);
  }

  const COLORREF crTransparent = RGB(192, 192, 192);

  // Load all images as list in enum CheckImage and in the order specified in it
  CBitmap bitmap;
  BITMAP bm;
  bitmap.LoadBitmap(IDB_CHECKED);
  bitmap.GetBitmap(&bm); // should be 13 x 13

  m_pCheckImageList = new CImageList;
  VERIFY(m_pCheckImageList->Create(bm.bmWidth, bm.bmHeight,
                                   ILC_MASK | ILC_COLOR, 4, 0));

  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_CHECKED_DISABLED);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_EMPTY);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_EMPTY_DISABLED);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();

  if (m_bCanHaveAttachments) {
    m_sMediaTypes = GetMainDlg()->GetAllMediaTypes();
  }

  m_bDBReadOnly = GetMainDlg()->IsDBReadOnly();
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

  CPWDialog::OnDestroy();
}

void CManageFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_FILTERLC, m_FilterLC);
  DDX_Control(pDX, IDC_FILTERPROPERTIES, m_FilterProperties);
}

BEGIN_MESSAGE_MAP(CManageFiltersDlg, CPWDialog)
  ON_WM_DESTROY()

  ON_COMMAND(IDHELP, OnHelp)

  ON_BN_CLICKED(IDC_FILTERNEW, OnFilterNew)
  ON_BN_CLICKED(IDC_FILTEREDIT, OnFilterEdit)
  ON_BN_CLICKED(IDC_FILTERCOPY, OnFilterCopy)
  ON_BN_CLICKED(IDC_FILTERDELETE, OnFilterDelete)
  ON_BN_CLICKED(IDC_FILTERIMPORT, OnFilterImport)
  ON_BN_CLICKED(IDC_FILTEREXPORT, OnFilterExport)

  ON_NOTIFY(NM_CLICK, IDC_FILTERLC, OnClick)
  ON_NOTIFY(NM_CLICK, IDC_FILTERPROPERTIES, OnClick)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_FILTERLC, OnCustomDraw)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_FILTERLC, OnItemChanging)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_FILTERPROPERTIES, OnItemChanging)
  ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILTERLC, OnColumnClick)
END_MESSAGE_MAP()

// ManageFilters message handlers

BOOL CManageFiltersDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  if (m_bDBReadOnly) {
    GetDlgItem(IDC_FILTERCOPY)->EnableWindow(FALSE);
  }

  // Make some columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  CString cs_text;
  cs_text.LoadString(IDS_FILTERNAME);
  m_FilterLC.InsertColumn(MFLC_FILTER_NAME, cs_text);
  cs_text.LoadString(IDS_FILTERSOURCE);
  m_FilterLC.InsertColumn(MFLC_FILTER_SOURCE, cs_text);
  cs_text.LoadString(IDS_FILTERAPPLY);
  m_FilterLC.InsertColumn(MFLC_INUSE, cs_text);
  cs_text.LoadString(IDS_FILTERCOPY2DB);
  m_FilterLC.InsertColumn(MFLC_COPYTODATABASE, cs_text);
  cs_text.LoadString(IDS_FILTEREXPORT);
  m_FilterLC.InsertColumn(MFLC_EXPORT, cs_text);

  // Make some columns centered
  m_FilterLC.SetColumn(MFLC_FILTER_SOURCE, &lvc);
  m_FilterLC.SetColumn(MFLC_INUSE, &lvc);
  m_FilterLC.SetColumn(MFLC_COPYTODATABASE, &lvc);
  m_FilterLC.SetColumn(MFLC_EXPORT, &lvc);

  // Populate CListCtrl
  UpdateFilterList();

  // if no filters - add a dummy before adjusting item height
  bool bAddDummy(false);
  if (m_FilterLC.GetItemCount() == 0) {
    bAddDummy = true;
    int iItem = m_FilterLC.InsertItem(0 /* MFLC_FILTER_NAME */, L"");
    CString cs_source = GetFilterPoolName(FPOOL_SESSION);

    m_FilterLC.SetItemText(iItem, MFLC_FILTER_SOURCE, cs_source);
    m_FilterLC.SetItemText(iItem, MFLC_INUSE, L".");
    m_FilterLC.SetItemText(iItem, MFLC_COPYTODATABASE, L".");
    m_FilterLC.SetItemText(iItem, MFLC_EXPORT, L".");

    st_FilterItemData *pflt_idata = new st_FilterItemData;
    pflt_idata->flt_key.cs_filtername = L".";
    pflt_idata->flt_key.fpool = FPOOL_SESSION;
    pflt_idata->flt_flags = MFLT_REQUEST_COPY_TO_DB | MFLT_REQUEST_EXPORT | MFLT_INUSE;
    m_FilterLC.SetItemData(iItem, (DWORD_PTR)pflt_idata);
  }

  // Set row height to take image by adding a dummy ImageList
  CRect rect;
  m_FilterLC.GetItemRect(0, &rect, LVIR_BOUNDS);
  IMAGEINFO imageinfo;
  m_pCheckImageList->GetImageInfo(0, &imageinfo);
  int irowheight = std::max(rect.Height(),
                       (int)abs(imageinfo.rcImage.top - imageinfo.rcImage.bottom));
  m_pImageList = new CImageList;
  m_pImageList->Create(1, irowheight, ILC_COLOR4, 1, 1);
  m_FilterLC.SetImageList(m_pImageList, LVSIL_SMALL);

  // If we added a dummy row - delete it
  if (bAddDummy) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(0);
    delete pflt_idata;
    m_FilterLC.DeleteItem(0);
  }

  dwExStyle = m_FilterProperties.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterProperties.SetExtendedStyle(dwExStyle);

  cs_text = L" # ";
  m_FilterProperties.InsertColumn(MFPRP_FILTER_NUMBER, cs_text);
  cs_text.LoadString(IDS_FILTERACTIVE);
  cs_text += L"?";
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

  CHeaderCtrl* pHCtrl;
  pHCtrl = m_FilterLC.GetHeaderCtrl();
  ASSERT(pHCtrl != NULL);
  pHCtrl->SetDlgCtrlID(IDC_FILTERLC_HEADER);
  m_FLCHeader.SubclassWindow(pHCtrl->GetSafeHwnd());

  pHCtrl = m_FilterProperties.GetHeaderCtrl();
  ASSERT(pHCtrl != NULL);
  pHCtrl->SetDlgCtrlID(IDC_FILTERPROP_HEADER);
  m_FPROPHeader.SubclassWindow(pHCtrl->GetSafeHwnd());

  for (int i = 0; i < MFPRP_NUM_COLUMNS; i++) {
    m_FilterProperties.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  m_FLCHeader.SetStopChangeFlag(true);
  m_FPROPHeader.SetStopChangeFlag(true);

  // Nothing is selected yet
  GetDlgItem(IDC_FILTEREDIT)->EnableWindow(FALSE);
  GetDlgItem(IDC_FILTERDELETE)->EnableWindow(FALSE);

  // Don't import if we can't validate
#ifndef USE_XML_LIBRARY
  GetDlgItem(IDC_FILTERIMPORT)->EnableWindow(FALSE);
#endif

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CManageFiltersDlg::OnClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  LPNMITEMACTIVATE pNMLV = reinterpret_cast<LPNMITEMACTIVATE>(pNotifyStruct);

  // Ignore clicks on Properties ListCtrl (doesn't seem to do much though!)
  if (pNMLV->hdr.idFrom == IDC_FILTERPROPERTIES) {
    *pLResult = TRUE;
    return;
  }

  *pLResult = FALSE;

  if (pNMLV->iItem < 0) {
    return;
  }

  GetDlgItem(IDC_FILTEREDIT)->EnableWindow(TRUE);
  GetDlgItem(IDC_FILTERDELETE)->EnableWindow(TRUE);

  st_FilterItemData *pflt_idata;
  m_selectedfilter = pNMLV->iItem;
  m_selectedfiltername = m_FilterLC.GetItemText(m_selectedfilter, 0);
  pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_selectedfilter);
  m_selectedfilterpool = pflt_idata->flt_key.fpool;

  CRect rect, image_rect;
  m_FilterLC.GetSubItemRect(pNMLV->iItem, pNMLV->iSubItem, LVIR_BOUNDS, rect);
  CPoint cpt = rect.CenterPoint();
  image_rect.SetRect(cpt.x - 7, cpt.y - 7, cpt.x + 7, cpt.y + 7);
  image_rect.NormalizeRect();
  BOOL bOnImage = image_rect.PtInRect(pNMLV->ptAction);

  switch (pNMLV->iSubItem) {
    case MFLC_INUSE:
      if (bOnImage == TRUE) {
        if ((pflt_idata->flt_flags & MFLT_INUSE) == MFLT_INUSE) {
          pflt_idata->flt_flags &= ~MFLT_INUSE;
          ClearFilter();
        } else {
          pflt_idata->flt_flags |= MFLT_INUSE;
          SetFilter();
        }
      }
      break;
    case MFLC_COPYTODATABASE:
      if (!m_bDBReadOnly) {
        if (bOnImage == TRUE && pflt_idata->flt_key.fpool != FPOOL_DATABASE) {
          if ((pflt_idata->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB) {
            pflt_idata->flt_flags &= ~MFLT_REQUEST_COPY_TO_DB;
            m_num_to_copy--;
          } else {
            pflt_idata->flt_flags |= MFLT_REQUEST_COPY_TO_DB;
            m_num_to_copy++;
          }
          GetDlgItem(IDC_FILTERCOPY)->EnableWindow(m_num_to_copy > 0);
        }
      }
      break;
    case MFLC_EXPORT:
      if (bOnImage == TRUE) {
        if ((pflt_idata->flt_flags & MFLT_REQUEST_EXPORT) == MFLT_REQUEST_EXPORT) {
          pflt_idata->flt_flags &= ~MFLT_REQUEST_EXPORT;
          m_num_to_export--;
        } else {
          pflt_idata->flt_flags |= MFLT_REQUEST_EXPORT;
          m_num_to_export++;
        }
        GetDlgItem(IDC_FILTEREXPORT)->EnableWindow(m_num_to_export > 0);
      }
      break;
    default:
      break;
  }

  m_FilterProperties.DeleteAllItems();
  GetDlgItem(IDC_STATIC_FILTERNAME)->SetWindowText(m_selectedfiltername);

  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = m_selectedfiltername;

  mf_iter = m_MapMFDFilters.find(flt_key);
  if (mf_iter == m_MapMFDFilters.end())
    return;

  st_filters *pfilters = &mf_iter->second;
  DisplayFilterProperties(pfilters);

  m_FilterLC.Invalidate();
}

void CManageFiltersDlg::OnFilterNew()
{
  st_filters filters;
  st_Filterkey flt_key;
  bool bJustDoIt(false), bCreated;

  flt_key.fpool = FPOOL_SESSION;

do_edit:
  bCreated = GetMainDlg()->EditFilter(&filters, false);

  flt_key.cs_filtername = filters.fname;

  if (bCreated) {
    PWSFilters::const_iterator mf_citer;
    mf_citer = m_MapMFDFilters.find(flt_key);

    // Check if already there (i.e. ask user if to replace)
    if (mf_citer != m_MapMFDFilters.end()) {
      CGeneralMsgBox gmb;
      CString cs_msg(MAKEINTRESOURCE(IDS_REPLACEFILTER));
      CString cs_title(MAKEINTRESOURCE(IDS_FILTEREXISTS));
      INT_PTR rc = gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      // If NO, go to edit again!  Not best practice to jump out of loop
      // to prior call!
      if (rc == IDNO)
        goto do_edit;

      m_MapMFDFilters.erase(flt_key);

      // If this was active, we need to clear it and re-apply
      if (m_bMFFilterActive &&
          m_activefilterpool == FPOOL_SESSION && 
          m_activefiltername == filters.fname.c_str()) {
        bJustDoIt = true;
      }
    }
    m_MapMFDFilters.insert(PWSFilters::Pair(flt_key, filters));

    // Update DboxMain
    GetMainDlg()->SetFilter(FPOOL_SESSION, filters.fname.c_str());
    if (bJustDoIt)
      GetMainDlg()->ApplyFilter(true);

    m_selectedfiltername = flt_key.cs_filtername.c_str();
    m_selectedfilterpool = flt_key.fpool;

    UpdateFilterList();
    DisplayFilterProperties(&filters);
  }
}

void CManageFiltersDlg::OnFilterEdit()
{
  bool bJustDoIt(false), bChanged, bReplacedOther(false);
  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key, flt_otherkey;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = m_selectedfiltername;

  mf_iter = m_MapMFDFilters.find(flt_key);
  if (mf_iter == m_MapMFDFilters.end())
    return;

  // Pass a copy of (not reference to) of the current filter in case 
  // the user cancels the change and the current state is invalid and 
  // corrupts the copy in the map
  st_filters filters = mf_iter->second;

do_edit:
  bChanged = GetMainDlg()->EditFilter(&filters, false);
  if (bChanged) {
    // Has user changed the filter's name?
    // If so, check for conflict.
    if (m_selectedfiltername != filters.fname.c_str()) {
      PWSFilters::const_iterator mf_citer;

      flt_otherkey.fpool = m_selectedfilterpool;
      flt_otherkey.cs_filtername = filters.fname;

      mf_citer = m_MapMFDFilters.find(flt_otherkey);

      // Check if already there (i.e. ask user if to replace)
      if (mf_citer != m_MapMFDFilters.end()) {
        CGeneralMsgBox gmb;
        CString cs_msg(MAKEINTRESOURCE(IDS_REPLACEFILTER));
        CString cs_title(MAKEINTRESOURCE(IDS_FILTEREXISTS));
        INT_PTR rc = gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
        // If NO, go to edit again!  Not best practice to jump out of loop
        // to prior call!
        if (rc == IDNO)
          goto do_edit;

        bReplacedOther = true;
      }
    }

    if (flt_key.fpool == FPOOL_DATABASE)
      m_bDBFiltersChanged = true;

    // If the original was active, we need to clear it and re-apply
    if (m_bMFFilterActive &&
        m_activefilterpool == flt_key.fpool && 
        m_activefiltername == m_selectedfiltername) {
      bJustDoIt = true;
    }

    // User may have changed name (and so key) - delete and add again
    // Have to anyway, even if name not changed.
    m_MapMFDFilters.erase(bReplacedOther ? flt_otherkey : flt_key);
    flt_key.cs_filtername = filters.fname;
    m_MapMFDFilters.insert(PWSFilters::Pair(flt_key, filters));
    m_selectedfiltername = flt_key.cs_filtername.c_str();

    // Update DboxMain's current filter
    GetMainDlg()->SetFilter(flt_key.fpool, filters.fname.c_str());
    if (bJustDoIt)
      GetMainDlg()->ApplyFilter(true);

    UpdateFilterList();
    DisplayFilterProperties(&filters);
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

    mf_iter = m_MapMFDFilters.find(flt_key);
    if (mf_iter == m_MapMFDFilters.end())
      return;

    PWSFilters::const_iterator mf_citer;
    st_Filterkey flt_keydb;
    flt_keydb.fpool = FPOOL_DATABASE;
    flt_keydb.cs_filtername = flt_key.cs_filtername;
    mf_citer = m_MapMFDFilters.find(flt_keydb);

    // Check if already there (i.e. ask user if to replace)
    if (mf_citer != m_MapMFDFilters.end()) {
      CGeneralMsgBox gmb;
      CString cs_msg(MAKEINTRESOURCE(IDS_REPLACEFILTER));
      CString cs_title(MAKEINTRESOURCE(IDS_FILTEREXISTS));
      INT_PTR rc = gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      if (rc == IDNO)
        continue;  // skip this one

      // User agrees to replace
      m_MapMFDFilters.erase(flt_keydb);
    }
    m_MapMFDFilters.insert(PWSFilters::Pair(flt_keydb, mf_iter->second));

    // Turn off copy flag
    pflt_idata->flt_flags &= ~MFLT_REQUEST_COPY_TO_DB;
    m_num_to_copy--;
    bCopied = true;
  }
  if (bCopied) {
    m_bDBFiltersChanged = true;
    GetMainDlg()->ChangeOkUpdate();
  }

  UpdateFilterList();
}

void CManageFiltersDlg::OnFilterDelete()
{
  if (m_selectedfilter < 0)
    return;

  CString cs_pool(L"");
  CString cs_selected = m_FilterLC.GetItemText(m_selectedfilter, 0);

  PWSFilters::iterator mf_iter;
  st_Filterkey flt_key;
  flt_key.fpool = m_selectedfilterpool;
  flt_key.cs_filtername = cs_selected;

  mf_iter = m_MapMFDFilters.find(flt_key);
  if (mf_iter == m_MapMFDFilters.end())
    return;

  cs_pool = GetFilterPoolName(flt_key.fpool);

  // Now to confirm with user:
  CString cs_msg;
  CGeneralMsgBox gmb;
  cs_msg.Format(IDS_CONFIRMFILTERDELETE, static_cast<LPCWSTR>(cs_pool),
                static_cast<LPCWSTR>(cs_selected));
  if (gmb.AfxMessageBox(cs_msg, NULL, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
    return;

  m_MapMFDFilters.erase(flt_key);
  if (m_selectedfilterpool == FPOOL_DATABASE) {
    m_bDBFiltersChanged = true;
    GetMainDlg()->ChangeOkUpdate();
  }

  st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_selectedfilter);
  if ((pflt_idata->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB)
    m_num_to_copy--;
  if ((pflt_idata->flt_flags & MFLT_REQUEST_EXPORT) == MFLT_REQUEST_EXPORT)
    m_num_to_export--;

  delete pflt_idata;
  m_FilterLC.DeleteItem(m_selectedfilter);
  m_FilterProperties.DeleteAllItems();
  if (m_selectedfilter == m_activefilter) {
    m_activefilter = -1;
    m_activefilterpool = FPOOL_LAST;
    m_activefiltername = L"";
    GetMainDlg()->ClearFilter();
  }

  m_selectedfilter = -1;
  m_selectedfilterpool = FPOOL_LAST;
  GetDlgItem(IDC_STATIC_FILTERNAME)->SetWindowText(L"");

  // Nothing selected
  GetDlgItem(IDC_FILTEREDIT)->EnableWindow(FALSE);
  GetDlgItem(IDC_FILTERDELETE)->EnableWindow(FALSE);

  // Update buttons
  GetDlgItem(IDC_FILTERCOPY)->EnableWindow(m_num_to_copy > 0 ? TRUE : FALSE);
  GetDlgItem(IDC_FILTEREXPORT)->EnableWindow(m_num_to_export > 0 ? TRUE : FALSE);
}

void CManageFiltersDlg::OnFilterImport()
{
  GetMainDlg()->ImportFilters();

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
    mf_iter = m_MapMFDFilters.find(pflt_idata->flt_key);
    if (mf_iter == m_MapMFDFilters.end())
      continue;

    Filters.insert(PWSFilters::Pair(pflt_idata->flt_key, mf_iter->second));
  }
  if (!Filters.empty()) {
    GetMainDlg()->ExportFilters(Filters);
    Filters.clear();
  }
}

void CManageFiltersDlg::SetFilter()
{
  GetMainDlg()->SetFilter(m_selectedfilterpool, m_selectedfiltername);
  if (!GetMainDlg()->ApplyFilter(true))
    return;

  m_activefilterpool = m_selectedfilterpool;
  m_activefiltername = m_selectedfiltername;

  st_FilterItemData *pflt_idata;
  // If the selected filter is not the current active one, remove flag
  // from old active filter
  if (m_selectedfilter != m_activefilter && m_activefilter != -1) {
    pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_activefilter);
    pflt_idata->flt_flags &= ~MFLT_INUSE;
  }
  m_activefilter = m_selectedfilter;
  // Now add flag to new selected and active filter
  pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_activefilter);
  pflt_idata->flt_flags |= MFLT_INUSE;
  m_bMFFilterActive = true;

  m_FilterLC.Invalidate();  // Ensure selected statement updated
}

void CManageFiltersDlg::ClearFilter()
{
  GetMainDlg()->ClearFilter();

  m_activefilterpool = FPOOL_LAST;
  m_activefiltername = L"";
  st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(m_activefilter);
  if (pflt_idata != NULL)
    pflt_idata->flt_flags &= ~MFLT_INUSE;
  m_activefilter = -1;
  m_bMFFilterActive = false;

  m_FilterLC.Invalidate();  // Ensure selected statement updated
}

void CManageFiltersDlg::DisplayFilterProperties(st_filters *pfilters)
{
  CString cs_num, cs_ftype, cs_criteria, cs_ltype, cs_act;

  vFilterRows::iterator Flt_iter;
  bool bHistory(false), bPolicy(false), bAttachment(false);
  int i(0), iItem(0), n(0);

  m_FilterProperties.DeleteAllItems();
  m_FilterProperties.SetRedraw(FALSE);

  // Do the main filters
  for (Flt_iter = pfilters->vMfldata.begin();
       Flt_iter != pfilters->vMfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(L"%d", n);
    cs_ftype = GetFieldTypeName(st_fldata.ftype);
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    cs_act.Remove(L'&');  // Remove leading ampersand from Yes/No
    if (Flt_iter != pfilters->vMfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = L"";

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);

    if (st_fldata.ftype == FT_PWHIST)
      bHistory = true;
    if (st_fldata.ftype == FT_POLICY)
      bPolicy = true;
    if (st_fldata.ftype == FT_ATTACHMENT)
      bAttachment = true;
  }

  if (bHistory) {
    i++;
    CString cs_history, cs_temp;
    cs_history.LoadString(IDS_SETPWHISTFILTERS);
    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, L"-");
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, L"---");
    cs_temp = L"---  " + cs_history + L"  ---";
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vHfldata.begin();
       Flt_iter != pfilters->vHfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(L"%d", n);
    cs_ftype = GetFieldTypeName(st_fldata.ftype);
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    cs_act.Remove(L'&');  // Remove leading ampersand from Yes/No
    if (Flt_iter != pfilters->vHfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = L"";

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
    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, L"-");
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, L"---");
    cs_temp = L"---  " + cs_policy + L"  ---";
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vPfldata.begin();
       Flt_iter != pfilters->vPfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(L"%d", n);
    cs_ftype = GetFieldTypeName(st_fldata.ftype);
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    cs_act.Remove(L'&');  // Remove leading ampersand from Yes/No
    if (Flt_iter != pfilters->vPfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = L"";

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);
  }

  if (bAttachment) {
    i++;
    CString cs_attachment, cs_temp;
    cs_attachment.LoadString(IDS_SETATTACHMENTFILTER);
    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, L"-");
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, L"---");
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, L"---");
    cs_temp = L"---  " + cs_attachment + L"  ---";
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_temp);
  }

  n = 0;
  for (Flt_iter = pfilters->vAfldata.begin();
       Flt_iter != pfilters->vAfldata.end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;
    i++;
    n++;

    cs_num.Format(L"%d", n);
    cs_ftype = GetFieldTypeName(st_fldata.ftype);
    cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
    cs_act.LoadString(st_fldata.bFilterActive ? IDS_YES : IDS_NO);
    cs_act.Remove(L'&');  // Remove leading ampersand from Yes/No
    if (Flt_iter != pfilters->vAfldata.begin())
      cs_ltype.LoadString(st_fldata.ltype == LC_AND ? IDSC_AND : IDSC_OR);
    else
      cs_ltype = L"";

    iItem = m_FilterProperties.InsertItem(i /* MFPRP_FILTER_NUMBER */, cs_num);
    m_FilterProperties.SetItemText(iItem, MFPRP_FILTER_ACTIVE, cs_act);
    m_FilterProperties.SetItemText(iItem, MFPRP_AND_OR, cs_ltype);
    m_FilterProperties.SetItemText(iItem, MFPRP_FIELD, cs_ftype);
    m_FilterProperties.SetItemText(iItem, MFPRP_CRITERIA_TEXT, cs_criteria);
  }

  bool bSave = m_FPROPHeader.GetStopChangeFlag();
  m_FPROPHeader.SetStopChangeFlag(false);
  for (int j = 0; j < MFPRP_CRITERIA_TEXT; j++) {
    m_FilterProperties.SetColumnWidth(j, LVSCW_AUTOSIZE);
    int iw1 =  m_FilterProperties.GetColumnWidth(j);
    m_FilterProperties.SetColumnWidth(j, LVSCW_AUTOSIZE_USEHEADER);
    int iw2 =  m_FilterProperties.GetColumnWidth(j);
    m_FilterProperties.SetColumnWidth(j, std::max(iw1, iw2));
  }
  m_FilterProperties.SetColumnWidth(MFPRP_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterProperties.SetRedraw(TRUE);
  m_FPROPHeader.SetStopChangeFlag(bSave);

  GetDlgItem(IDC_STATIC_FILTERNAME)->SetWindowText(pfilters->fname.c_str());
}

void CManageFiltersDlg::UpdateFilterList()
{
  int nCount, iItem, i;

  m_FilterLC.SetRedraw(FALSE);
  nCount = m_FilterLC.GetItemCount();
  for (i = 0; i < nCount; i++) {
    st_FilterItemData *pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(0);
    delete pflt_idata;
    m_FilterLC.DeleteItem(0);
  }
  m_vcs_filters.clear();

  PWSFilters::iterator mf_iter;
  i = 0;
  m_selectedfilter = -1;

  for (mf_iter = m_MapMFDFilters.begin();
       mf_iter != m_MapMFDFilters.end();
       mf_iter++) {
    m_vcs_filters.push_back(mf_iter->first);

    iItem = m_FilterLC.InsertItem(i /* MFLC_FILTER_NAME */,
                                  mf_iter->first.cs_filtername.c_str());
    CString cs_source = GetFilterPoolName(mf_iter->first.fpool);

    m_FilterLC.SetItemText(iItem, MFLC_FILTER_SOURCE, cs_source);
    // Add dummy fields where checkbox images will be. OnCustomDraw will make the colour
    // of this text the same as the background i.e. invisible.
    m_FilterLC.SetItemText(iItem, MFLC_INUSE, L".");
    m_FilterLC.SetItemText(iItem, MFLC_COPYTODATABASE, L".");
    m_FilterLC.SetItemText(iItem, MFLC_EXPORT, L".");

    if (m_bMFFilterActive &&
        mf_iter->first.fpool == m_activefilterpool &&
        mf_iter->first.cs_filtername.c_str() == m_activefiltername) {
      m_activefilter = iItem;
    }
    if (mf_iter->first.fpool == m_selectedfilterpool &&
        mf_iter->first.cs_filtername.c_str() == m_selectedfiltername) {
      m_selectedfilter = iItem;
    }
    st_FilterItemData *pflt_idata = new st_FilterItemData;
    pflt_idata->flt_key = mf_iter->first;
    pflt_idata->flt_flags = (m_activefilter == iItem) ? MFLT_INUSE : 0;
    if (m_selectedfilter == iItem)
      pflt_idata->flt_flags |= MFLT_SELECTED;
    m_FilterLC.SetItemData(iItem, (DWORD_PTR)pflt_idata);
    i++;
  }

  // ResetColumns will set m_FilterLC.SetRedraw(TRUE)
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
  bool bSave = m_FLCHeader.GetStopChangeFlag();
  m_FLCHeader.SetStopChangeFlag(false);

  int iw1, iw2;
  m_FilterLC.SetRedraw(FALSE);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, LVSCW_AUTOSIZE);
  // Add image width - problem under Vista not showing all the text (comdlg32.dll)
  iw1 = m_FilterLC.GetColumnWidth(MFLC_FILTER_NAME) + 6;
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, LVSCW_AUTOSIZE_USEHEADER);
  iw2 = m_FilterLC.GetColumnWidth(MFLC_FILTER_NAME);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_NAME, std::max(iw1, iw2));

  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, LVSCW_AUTOSIZE);
  iw1 = m_FilterLC.GetColumnWidth(MFLC_FILTER_SOURCE);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, LVSCW_AUTOSIZE_USEHEADER);
  iw2 = m_FilterLC.GetColumnWidth(MFLC_FILTER_SOURCE);
  m_FilterLC.SetColumnWidth(MFLC_FILTER_SOURCE, std::max(iw1, iw2));

  m_FilterLC.SetColumnWidth(MFLC_INUSE, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterLC.SetColumnWidth(MFLC_COPYTODATABASE, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterLC.SetColumnWidth(MFLC_EXPORT, LVSCW_AUTOSIZE_USEHEADER);
  m_FilterLC.SetRedraw(TRUE);

  m_FilterLC.Invalidate();
  m_FLCHeader.SetStopChangeFlag(bSave);
}

void CManageFiltersDlg::OnItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLISTVIEW* pNMLV = reinterpret_cast<NMLISTVIEW *>(pNotifyStruct);

  // Ignore clicks on Properties ListCtrl (doesn't seem to do much though!)
  if (pNMLV->hdr.idFrom == IDC_FILTERPROPERTIES) {
    *pLResult = TRUE;
    return;
  }

  *pLResult = FALSE;

  // Has the state changed?
  if (pNMLV->iItem < 0)
    return;

  if ((pNMLV->uChanged & LVIF_STATE) == LVIF_STATE) {
    UINT uiChangedState = pNMLV->uOldState & ~pNMLV->uNewState;
    if ((uiChangedState & LVIS_FOCUSED) == LVIS_FOCUSED) {
      *pLResult = TRUE;
    }
    if ((pNMLV->uNewState & LVIS_SELECTED) == LVIS_SELECTED) {
      m_selectedfilter = pNMLV->iItem;
      *pLResult = TRUE;
    }
    if ((pNMLV->uOldState & LVIS_SELECTED) == LVIS_SELECTED) {
      *pLResult = TRUE;
    }
  }
}

void CManageFiltersDlg::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iItem = (int)pLVCD->nmcd.dwItemSpec;
  const int iSubItem = pLVCD->iSubItem;

  bool bCopy(false), bExport(false), bActive(false);
  st_FilterItemData *pflt_idata(NULL);

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_ITEMPREPAINT:
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      pflt_idata = (st_FilterItemData *)m_FilterLC.GetItemData(iItem);

      bActive = (pflt_idata->flt_flags & MFLT_INUSE) == MFLT_INUSE;
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
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      pLVCD->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
      // Show selected item on light green background
      if (iItem == m_selectedfilter) {
        pLVCD->clrTextBk = crLightGreen;
      } else {
        pLVCD->clrTextBk = m_FilterLC.GetTextBkColor();
      }
      *pLResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
        CRect rect;
        m_FilterLC.GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pLResult = CDRF_SKIPDEFAULT;
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
            break;
          case MFLC_INUSE:
            // Make text 'invisible'
            pLVCD->clrText = pLVCD->clrTextBk;
            // Show selected item on light green background
            if (iItem == m_selectedfilter) {
              pDC->FillSolidRect(&first_rect, crLightGreen);
            }
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            DrawImage(pDC, inner_rect, bActive ? CHECKED : EMPTY);
            *pLResult = CDRF_SKIPDEFAULT;
            break;
          case MFLC_COPYTODATABASE:
            // Make text 'invisible'
            pLVCD->clrText = pLVCD->clrTextBk;
            // Show selected item on light green background
            if (iItem == m_selectedfilter) {
              pDC->FillSolidRect(&first_rect, crLightGreen);
            }
            *pLResult = CDRF_SKIPDEFAULT;
            // If already a database filter - don't need any image
            if (pflt_idata->flt_key.fpool == FPOOL_DATABASE)
              break;
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            // Set image according to DB being R-O or not
            CheckImage nImage;
            if (m_bDBReadOnly)
              nImage = bCopy ? CHECKED_DISABLED : EMPTY_DISABLED;
            else
              nImage = bCopy ? CHECKED : EMPTY;
            DrawImage(pDC, inner_rect, nImage);
            break;
          case MFLC_EXPORT:
            // Make text 'invisible'
            pLVCD->clrText = pLVCD->clrTextBk;
            // Show selected item on light green background
            if (iItem == m_selectedfilter) {
              pDC->FillSolidRect(&first_rect, crLightGreen);
            }
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            DrawImage(pDC, inner_rect, bExport ? CHECKED : EMPTY);
            *pLResult = CDRF_SKIPDEFAULT;
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

void CManageFiltersDlg::DrawImage(CDC *pDC, CRect &rect, CheckImage nImage)
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

        // Due to a bug in VS2010 MFC the following does not work!
        //    m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
        // So do it the hard way!
        IMAGELISTDRAWPARAMS imldp = {0};
        imldp.cbSize = sizeof(imldp);
        imldp.i = nImage;
        imldp.hdcDst = pDC->m_hDC;
        imldp.x = point.x;
        imldp.y = point.y;
        imldp.xBitmap = imldp.yBitmap = 0;
        imldp.cx = size.cx;
        imldp.cy = size.cy;
        imldp.fStyle = ILD_NORMAL;
        imldp.dwRop = SRCCOPY;
        imldp.rgbBk = CLR_DEFAULT;
        imldp.rgbFg = CLR_DEFAULT;
        imldp.fState = ILS_NORMAL;
        imldp.Frame = 0;
        imldp.crEffect = CLR_DEFAULT;

        m_pCheckImageList->DrawIndirect(&imldp);
      }
    }
  }
}

CString CManageFiltersDlg::GetFieldTypeName(FieldType ft)
{
  int nID = IDSC_UNKNOWN;
  switch (ft) {
  case FT_GROUP:        return CItemData::FieldName(CItemData::GROUP).c_str();
  case FT_TITLE:        return CItemData::FieldName(CItemData::TITLE).c_str();
  case FT_GROUPTITLE:   return CItemData::FieldName(CItemData::GROUPTITLE).c_str();
  case FT_USER:         return CItemData::FieldName(CItemData::USER).c_str();
  case FT_PASSWORD:     return CItemData::FieldName(CItemData::PASSWORD).c_str();
  case FT_NOTES:        return CItemData::FieldName(CItemData::NOTES).c_str();
  case FT_AUTOTYPE:     return CItemData::FieldName(CItemData::AUTOTYPE).c_str();
  case FT_URL:          return CItemData::FieldName(CItemData::URL).c_str();
  case FT_RUNCMD:       return CItemData::FieldName(CItemData::RUNCMD).c_str();
  case FT_DCA:          return CItemData::FieldName(CItemData::DCA).c_str();
  case FT_SHIFTDCA:     return CItemData::FieldName(CItemData::SHIFTDCA).c_str();
  case FT_EMAIL:        return CItemData::FieldName(CItemData::EMAIL).c_str();
  case FT_PROTECTED:    return CItemData::FieldName(CItemData::PROTECTED).c_str();
  case FT_SYMBOLS:      return CItemData::FieldName(CItemData::SYMBOLS).c_str();
  case FT_CTIME:        return CItemData::FieldName(CItemData::CTIME).c_str();
  case FT_ATIME:        return CItemData::FieldName(CItemData::ATIME).c_str();
  case FT_PMTIME:       return CItemData::FieldName(CItemData::PMTIME).c_str();
  case FT_XTIME:        return CItemData::FieldName(CItemData::XTIME).c_str();
  case FT_XTIME_INT:    return CItemData::FieldName(CItemData::XTIME_INT).c_str();
  case FT_RMTIME:       return CItemData::FieldName(CItemData::RMTIME).c_str();
  case FT_PWHIST:       return CItemData::FieldName(CItemData::PWHIST).c_str();
  case FT_POLICY:       return CItemData::FieldName(CItemData::POLICY).c_str();
  case FT_POLICYNAME:   return CItemData::FieldName(CItemData::POLICYNAME).c_str();
  case FT_KBSHORTCUT:   return CItemData::FieldName(CItemData::KBSHORTCUT).c_str();

  case FT_PASSWORDLEN:   nID = IDS_PASSWORDLEN; break;
  case FT_ENTRYSIZE:     nID = IDS_ENTRYSIZE; break;
  case FT_ENTRYTYPE:     nID = IDS_ENTRYTYPE; break;
  case FT_ENTRYSTATUS:   nID = IDS_ENTRYSTATUS; break;
  case FT_UNKNOWNFIELDS: nID = IDS_UNKNOWNFIELDSFILTER; break;

  case HT_PRESENT:       nID = IDS_PRESENT; break;
  case HT_ACTIVE:        nID = IDS_HACTIVE; break;
  case HT_NUM:           nID = IDS_HNUM; break;
  case HT_MAX:           nID = IDS_HMAX; break;
  case HT_CHANGEDATE:    nID = IDS_HDATE; break;
  case HT_PASSWORDS:     nID = HT_PASSWORDS; break;

  case PT_PRESENT:       nID = IDS_PRESENT; break;
  case PT_LENGTH:        nID = IDSC_PLENGTH; break;
  case PT_LOWERCASE:     nID = IDS_PLOWER; break;
  case PT_UPPERCASE:     nID = IDS_PUPPER; break;
  case PT_DIGITS:        nID = IDS_PDIGITS; break;
  case PT_SYMBOLS:       nID = IDS_PSYMBOL; break;
  case PT_HEXADECIMAL:   nID = IDSC_PHEXADECIMAL; break;
  case PT_EASYVISION:    nID = IDSC_PEASYVISION; break;
  case PT_PRONOUNCEABLE: nID = IDSC_PPRONOUNCEABLE; break;
  
    case AT_PRESENT:       nID = IDS_PRESENT; break;
    case AT_TITLE:         nID = IDS_FILETITLE; break;
    case AT_CTIME:         nID = IDS_FILENAME; break;
    case AT_MEDIATYPE:     nID = IDS_FILEMEDIATYPE; break;
    case AT_FILENAME:      nID = IDS_FILENAME; break;
    case AT_FILEPATH:      nID = IDS_FILEPATH; break;
    case AT_FILECTIME:     nID = IDS_FILECTIME; break;
    case AT_FILEMTIME:     nID = IDS_FILEMTIME; break;
    case AT_FILEATIME:     nID = IDS_FILEATIME; break;
    case FT_ATTACHMENT:    nID = IDS_ATTACHMENTS; break;

    default:
      ASSERT(0);
  }

  CString retval;
  retval.LoadString(nID);
  retval.TrimRight(L'\t'); // ?? do we still need to trim ??
  return retval;
}

CString CManageFiltersDlg::GetFilterPoolName(FilterPool fp)
{
  UINT uiname(0);
  switch (fp) {
    case FPOOL_DATABASE:
      uiname = IDS_DBPOOLNAME;
      break;
    case FPOOL_AUTOLOAD:
      uiname = IDS_AUTOLOADPOOLNAME;
      break;
    case FPOOL_IMPORTED:
      uiname = IDS_IMPORTEDPOOLNAME;
      break;
    case FPOOL_SESSION:
      uiname = IDS_SESSIONPOOLNAME;
      break;
    default:
      ASSERT(0);
  }
  CString cs_pool(L"");
  if (uiname > 0)
    cs_pool.LoadString(uiname);

  return cs_pool;
}

/*
* Compare function used by m_FilterLC.SortItems()
*/
int CALLBACK CManageFiltersDlg::FLTCompareFunc(LPARAM lParam1,  
                                               LPARAM lParam2,
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

  int iResult(0);
  switch(nSortColumn) {
    case MFLC_FILTER_NAME:
      iResult = pLHS->flt_key.cs_filtername.compare(pRHS->flt_key.cs_filtername);
      break;
    case MFLC_FILTER_SOURCE:
      iResult = GetFilterPoolName(pLHS->flt_key.fpool).Compare(GetFilterPoolName(pRHS->flt_key.fpool));
      break;
    case MFLC_INUSE:
      i1 = (int)(pLHS->flt_flags & MFLT_INUSE);
      i2 = (int)(pRHS->flt_flags & MFLT_INUSE);
      if (i1 != i2)
        iResult = (i1 < i2) ? -1 : 1;
      break;
    case MFLC_COPYTODATABASE:
      // Treat:
      // Database filters (always unticked) as value 0;
      // Non-Database filters unticked as value 1;
      // Non-Database filters ticked as value 2;
      i1 = i2 = 0;
      if (pLHS->flt_key.fpool != FPOOL_DATABASE)
        i1 = ((pLHS->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB) ? 2 : 1;
      if (pRHS->flt_key.fpool != FPOOL_DATABASE)
        i2 = ((pRHS->flt_flags & MFLT_REQUEST_COPY_TO_DB) == MFLT_REQUEST_COPY_TO_DB) ? 2 : 1;
      if (i1 != i2)
        iResult = (i1 < i2) ? -1 : 1;
      break;
    case MFLC_EXPORT:
      i1 = (int)(pLHS->flt_flags & MFLT_REQUEST_EXPORT);
      i2 = (int)(pRHS->flt_flags & MFLT_REQUEST_EXPORT);
      if (i1 != i2)
        iResult = (i1 < i2) ? -1 : 1;
      break;
    default:
      ASSERT(FALSE);
  }
  if (!self->m_bSortAscending && iResult != 0) {
    iResult *= -1;
  }
  return iResult;
}

void CManageFiltersDlg::OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult) 
{
  NMLISTVIEW *pNMLV = reinterpret_cast<NMLISTVIEW *>(pNotifyStruct);

  // Get column index to CItemData value
  int iIndex = pNMLV->iSubItem;

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  if (m_iSortColumn == iIndex) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    // Turn off all previous sort arrows
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
  *pLResult = TRUE;
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

BOOL CManageFiltersDlg::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    OnHelp();
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CManageFiltersDlg::OnHelp()
{
  ShowHelp(L"::/html/filters.html#ManagingFilters");
}
