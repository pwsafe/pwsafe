/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

/// CompareResultsDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"  // For WM_VIEW_COMPARE_RESULT
#include "CompareResultsDlg.h"
#include "corelib/PWScore.h"
#include <vector>
#include <algorithm>
#include <functional>
#include "resource.h"
#include "resource2.h"
#include "resource3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCompareResultsDlg, CDialog)

//-----------------------------------------------------------------------------
CCompareResultsDlg::CCompareResultsDlg(CWnd* pParent,
  CompareData &OnlyInCurrent, CompareData &OnlyInComp,
  CompareData &Conflicts, CompareData &Identical,
  CItemData::FieldBits &bsFields, PWScore *pcore0, PWScore *pcore1)
  : CDialog(CCompareResultsDlg::IDD, pParent),
  m_OnlyInCurrent(OnlyInCurrent), m_OnlyInComp(OnlyInComp),
  m_Conflicts(Conflicts), m_Identical(Identical),
  m_bsFields(bsFields), m_pcore0(pcore0), m_pcore1(pcore1),
  m_bSortAscending(true), m_iSortedColumn(-1),
  m_OriginalDBChanged(false), m_ComparisonDBChanged(false),
  m_ShowIdenticalEntries(BST_UNCHECKED),
  m_DialogMinWidth(455), m_DialogMinHeight(415),
  m_DialogMaxWidth(999), m_DialogMaxHeight(1024)
  // Need to set default values for MinWidth & MinHeight as OnGetMinMaxInfo is 
  // called during Create before set in InitDialog
{
}

BOOL CCompareResultsDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  m_LCResults.GetHeaderCtrl()->SetDlgCtrlID(IDC_RESULTLISTHDR);

  DWORD dwExtendedStyle = m_LCResults.GetExtendedStyle();
  dwExtendedStyle |= LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
  m_LCResults.SetExtendedStyle(dwExtendedStyle);

  CString cs_header;
  cs_header.LoadString(IDS_ORIGINALDB);
  m_LCResults.InsertColumn(CURRENT, cs_header);
  cs_header.LoadString(IDS_COMPARISONDB);
  m_LCResults.InsertColumn(COMPARE, cs_header);

  cs_header.LoadString(IDS_GROUP);
  m_LCResults.InsertColumn(GROUP, cs_header);
  cs_header.LoadString(IDS_TITLE);
  m_LCResults.InsertColumn(TITLE, cs_header);
  cs_header.LoadString(IDS_USERNAME);
  m_LCResults.InsertColumn(USER, cs_header);

  if (m_bsFields.test(CItemData::PASSWORD)) {
    cs_header.LoadString(IDS_PASSWORD);
    m_LCResults.InsertColumn(PASSWORD, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::NOTES)) {
    cs_header.LoadString(IDS_NOTES);
    m_LCResults.InsertColumn(NOTES, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::URL)) {
    cs_header.LoadString(IDS_URL);
    m_LCResults.InsertColumn(URL, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::AUTOTYPE)) {
    cs_header.LoadString(IDS_AUTOTYPE);
    m_LCResults.InsertColumn(AUTOTYPE, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::PWHIST)) {
    cs_header.LoadString(IDS_PWHIST);
    m_LCResults.InsertColumn(PWHIST, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::CTIME)) {
    cs_header.LoadString(IDS_CREATED);
    m_LCResults.InsertColumn(CTIME, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::ATIME)) {
    cs_header.LoadString(IDS_LASTACCESSED);
    m_LCResults.InsertColumn(ATIME, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::LTIME)) {
    cs_header.LoadString(IDS_PASSWORDEXPIRYDATE);
    m_LCResults.InsertColumn(LTIME, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::PMTIME)) {
    cs_header.LoadString(IDS_PASSWORDMODIFIED);
    m_LCResults.InsertColumn(PMTIME, cs_header, LVCFMT_CENTER);
  }
  if (m_bsFields.test(CItemData::RMTIME)) {
    cs_header.LoadString(IDS_LASTMODIFIED);
    m_LCResults.InsertColumn(RMTIME, cs_header, LVCFMT_CENTER);
  }
  m_nCols = m_LCResults.GetHeaderCtrl()->GetItemCount();

  m_LCResults.SetItemCount(m_OnlyInCurrent.size() +
                           m_OnlyInComp.size() +
                           m_Conflicts.size() +
                           m_Identical.size());

  int i, iItem = 0;
  CompareData::iterator cd_iter;
  m_numOnlyInCurrent = m_OnlyInCurrent.size();
  m_numOnlyInComp = m_OnlyInComp.size();
  m_numConflicts = m_Conflicts.size();
  m_numIdentical = m_Identical.size();

  if (m_numOnlyInCurrent > 0) {
    for (cd_iter = m_OnlyInCurrent.begin(); cd_iter != m_OnlyInCurrent.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      if (st_data.unknflds0)
        m_LCResults.InsertItem(iItem, _T("Y*"));
      else
        m_LCResults.InsertItem(iItem, _T("Y"));

      m_LCResults.SetItemText(iItem, COMPARE, _T("-"));
      m_LCResults.SetItemText(iItem, GROUP, st_data.group);
      m_LCResults.SetItemText(iItem, TITLE, st_data.title);
      m_LCResults.SetItemText(iItem, USER, st_data.user);
      for (i = USER + 1; i < m_nCols; i++)
        m_LCResults.SetItemText(iItem, i, _T("-"));

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, (DWORD)&st_data);
      iItem++;
    }
  }

  if (m_numOnlyInComp > 0) {
    for (cd_iter = m_OnlyInComp.begin(); cd_iter != m_OnlyInComp.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      m_LCResults.InsertItem(iItem, _T("-"));
      if (st_data.unknflds1)
        m_LCResults.SetItemText(iItem, COMPARE, _T("Y*"));
      else
        m_LCResults.SetItemText(iItem, COMPARE, _T("Y"));

      m_LCResults.SetItemText(iItem, GROUP, st_data.group);
      m_LCResults.SetItemText(iItem, TITLE, st_data.title);
      m_LCResults.SetItemText(iItem, USER, st_data.user);
      for (i = USER + 1; i < m_nCols; i++)
        m_LCResults.SetItemText(iItem, i, _T("-"));

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, (DWORD)&st_data);
      iItem++;
    }
  }

  if (m_numConflicts > 0) {
    int icol;
    for (cd_iter = m_Conflicts.begin(); cd_iter != m_Conflicts.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      if (st_data.unknflds0)
        m_LCResults.InsertItem(iItem, _T("Y*"));
      else
        m_LCResults.InsertItem(iItem, _T("Y"));
      if (st_data.unknflds1)
        m_LCResults.SetItemText(iItem, COMPARE, _T("Y*"));
      else
        m_LCResults.SetItemText(iItem, COMPARE, _T("Y"));

      m_LCResults.SetItemText(iItem, GROUP, st_data.group);
      m_LCResults.SetItemText(iItem, TITLE, st_data.title);
      m_LCResults.SetItemText(iItem, USER, st_data.user);

      icol = 4;
      if (m_bsFields.test(CItemData::PASSWORD))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PASSWORD) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::NOTES))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::NOTES) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::URL))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::URL) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::AUTOTYPE))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::AUTOTYPE) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::PWHIST))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PWHIST) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::CTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::CTIME) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::ATIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::ATIME) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::LTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::LTIME) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::PMTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PMTIME) ? _T("X") : _T("-"));
      if (m_bsFields.test(CItemData::RMTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::RMTIME) ? _T("X") : _T("-"));

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, (DWORD)&st_data);
      iItem++;
    }
  }

  m_LCResults.SetRedraw(FALSE);
  for (i = 0; i < m_nCols - 1; i++) {
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int header_width = m_LCResults.GetColumnWidth(i);
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int data_width = m_LCResults.GetColumnWidth(i);
    if (header_width > data_width)
      m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }
  m_LCResults.SetColumnWidth(m_nCols - 1, LVSCW_AUTOSIZE_USEHEADER);
  m_LCResults.SetRedraw(TRUE);
  m_LCResults.Invalidate();

  // setup status bar for gripper only
  if (m_statusBar.CreateEx(this, SBARS_SIZEGRIP)) {
    statustext[0] = IDS_STATCOMPANY;
    m_statusBar.SetIndicators(statustext, 1);
    UpdateStatusBar();
  } else {
    TRACE(_T("Could not create status bar\n"));
  }

  // Put on StatusBar
  CRect rcClientStart;
  CRect rcClientNow;
  GetClientRect(rcClientStart);
  RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

  // Arrange all the controls - needed for resizeable dialog
  CWnd *pwndListCtrl = GetDlgItem(IDC_RESULTLIST);
  CWnd *pwndOKButton = GetDlgItem(IDOK);
  CWnd *pwndCPYButton = GetDlgItem(IDC_COPYTOCLIPBOARD);

  CRect sbRect, ctrlRect, dlgRect;
  int xleft, ytop;

  int itotalwidth = 0;
  for (int i = 0; i < m_nCols; i++)
    itotalwidth += m_LCResults.GetColumnWidth(i);

  GetClientRect(&dlgRect);
  m_DialogMinWidth = dlgRect.Width();
  m_DialogMinHeight = dlgRect.Height();
  m_DialogMaxWidth = itotalwidth + 16;
  m_DialogMaxHeight = 1024;
  if (m_DialogMaxWidth < m_DialogMinWidth)
    m_DialogMaxWidth = m_DialogMinWidth;
  if (m_DialogMaxHeight < m_DialogMinHeight)
    m_DialogMaxHeight = m_DialogMinHeight; 

  m_statusBar.GetWindowRect(&sbRect);

  pwndListCtrl->GetWindowRect(&ctrlRect);
  ScreenToClient(&ctrlRect);

  m_cxBSpace = dlgRect.Size().cx - ctrlRect.Size().cx;
  m_cyBSpace = dlgRect.Size().cy - ctrlRect.Size().cy;
  m_cySBar = sbRect.Size().cy;

  pwndListCtrl->SetWindowPos(NULL, NULL, NULL,
                        dlgRect.Size().cx - (2 * ctrlRect.TopLeft().x),
                        dlgRect.Size().cy - m_cyBSpace,
                        SWP_NOMOVE | SWP_NOZORDER);

  GetWindowRect(&dlgRect);
  pwndCPYButton->GetWindowRect(&ctrlRect);
  xleft = (m_DialogMinWidth / 4) - (ctrlRect.Width() / 2);
  ytop = dlgRect.Height() - m_cyBSpace/2 - m_cySBar;

  pwndCPYButton->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

  pwndOKButton->GetWindowRect(&ctrlRect);
  xleft = (3 * m_DialogMinWidth / 4) - (ctrlRect.Width() / 2);

  pwndOKButton->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

  GetDlgItem(IDC_COMPAREORIGINALDB)->SetWindowText(m_cs_Filename1);
  GetDlgItem(IDC_COMPARECOMPARISONDB)->SetWindowText(m_cs_Filename2);

  this->SetWindowPos(NULL, NULL, NULL, m_DialogMinWidth, m_DialogMinHeight, 
                     SWP_NOMOVE | SWP_NOZORDER);

  return TRUE;
}

void CCompareResultsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_RESULTLIST, m_LCResults);
}

BEGIN_MESSAGE_MAP(CCompareResultsDlg, CDialog)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
  ON_NOTIFY(NM_DBLCLK, IDC_RESULTLIST, OnItemDoubleClick)
  ON_NOTIFY(NM_RCLICK, IDC_RESULTLIST, OnItemRightClick)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDC_COPYTOCLIPBOARD, OnCopyToClipboard)
  ON_BN_CLICKED(IDC_SHOW_IDENTICAL_ENTRIES, OnShowIdenticalEntries)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_RESULTLISTHDR, OnColumnClick)
  ON_COMMAND(ID_MENUITEM_COMPVIEWEDIT, OnCompareViewEdit)
  ON_COMMAND(ID_MENUITEM_COPY_TO_ORIGINAL, OnCompareCopyToOriginalDB)
  ON_COMMAND(ID_MENUITEM_COPY_TO_COMPARISON, OnCompareCopyToComparisonDB)
END_MESSAGE_MAP()

void
CCompareResultsDlg::OnShowIdenticalEntries()
{
  m_ShowIdenticalEntries = ((CButton*)GetDlgItem(IDC_SHOW_IDENTICAL_ENTRIES))->GetCheck();

  m_LCResults.SetRedraw(FALSE);

  CompareData::iterator cd_iter;
  int i, iItem = 0;
  if (m_ShowIdenticalEntries == BST_CHECKED) {
    if (m_numIdentical > 0) {
      for (cd_iter = m_Identical.begin(); cd_iter != m_Identical.end(); cd_iter++) {
        st_CompareData &st_data = *cd_iter;

        if (st_data.unknflds0)
          m_LCResults.InsertItem(iItem, _T("=*"));
        else
          m_LCResults.InsertItem(iItem, _T("="));
        if (st_data.unknflds1)
          m_LCResults.SetItemText(iItem, COMPARE, _T("=*"));
        else
          m_LCResults.SetItemText(iItem, COMPARE, _T("="));

        m_LCResults.SetItemText(iItem, GROUP, st_data.group);
        m_LCResults.SetItemText(iItem, TITLE, st_data.title);
        m_LCResults.SetItemText(iItem, USER, st_data.user);
        for (i = USER + 1; i < m_nCols; i++)
          m_LCResults.SetItemText(iItem, i, _T("-"));

        st_data.listindex = iItem;
        m_LCResults.SetItemData(iItem, (DWORD)&st_data);
        iItem++;
      }
    }
  } else {
    if (m_numIdentical > 0) {
      for (i = m_LCResults.GetItemCount() - 1; i >= 0; i--) {
        if (m_LCResults.GetItemText(i, 0).Left(1) == _T("="))
          m_LCResults.DeleteItem(i);
      }
    }
  }

  for (i = 0; i < m_nCols - 1; i++) {
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int header_width = m_LCResults.GetColumnWidth(i);
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int data_width = m_LCResults.GetColumnWidth(i);
    if (header_width > data_width)
      m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }
  m_LCResults.SetColumnWidth(m_nCols - 1, LVSCW_AUTOSIZE_USEHEADER);

  m_LCResults.SetRedraw(TRUE);
  m_LCResults.Invalidate();
}

void
CCompareResultsDlg::OnCancel()
{
  CDialog::OnCancel();
}

void
CCompareResultsDlg::OnOK()
{
  CDialog::OnOK();
}

void
CCompareResultsDlg::OnHelp()
{
  CString cs_HelpTopic = app.GetHelpFileName() + _T("::/html/compare_results.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void
CCompareResultsDlg::UpdateStatusBar()
{
  CString s;
  s.Format(IDS_COMPARERESULTS, m_numOnlyInCurrent, m_numOnlyInComp,
                               m_numConflicts, m_numIdentical);
  m_statusBar.SetPaneText(0, s, TRUE);
  m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
  m_statusBar.UpdateWindow();
}

bool
CCompareResultsDlg::ProcessFunction(const int ifunction)
{
  st_CompareInfo *st_info;
  st_info = new st_CompareInfo;

  st_CompareData *st_data;
  bool rc(false);

  st_data = (st_CompareData *)m_LCResults.GetItemData(m_row);
  int pos_column = st_data->column;
  if (m_column == pos_column || pos_column == -1) {
    st_info->pcore0 = m_pcore0;
    st_info->pcore1 = m_pcore1;
    st_info->pos0 = st_data->pos0;
    st_info->pos1 = st_data->pos1;
    st_info->column = m_column;

    LRESULT lres = ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                  WM_COMPARE_RESULT_FUNCTION, (WPARAM)st_info, (LPARAM)ifunction);
    if (lres  == TRUE)
      rc = true;
  }
  delete st_info;
  return rc;
}

void
CCompareResultsDlg::OnCompareViewEdit()
{
  bool bSourceRO = (m_column == 0) ? m_bOriginalDBReadOnly : m_bComparisonDBReadOnly;

  if (bSourceRO || m_column == 1)
    ProcessFunction(VIEW);
  else
    ProcessFunction(EDIT);
}

void
CCompareResultsDlg::OnCompareCopyToOriginalDB()
{
  if (m_bOriginalDBReadOnly)
    return;

  if (CopyLeftOrRight(true))
    m_OriginalDBChanged = true;
}

void
CCompareResultsDlg::OnCompareCopyToComparisonDB()
{
  if (m_bComparisonDBReadOnly)
    return;

  if (CopyLeftOrRight(false))
    m_ComparisonDBChanged = true;
}

struct equal_id
{
   equal_id(int const& id) : m_id(id) {}
   bool operator()(st_CompareData const& rdata) const
   {
     return (rdata.id == m_id);
   }

   int m_id;
};

bool
CCompareResultsDlg::CopyLeftOrRight(const bool bCopyLeft)
{
  // Check not already copied one way or another
  CString cs_text = m_LCResults.GetItemText(m_row, m_column);
  if (cs_text.Compare(_T("=")) == 0)
    return false;

  CString cs_msg;
  int ifunction;

  const CString cs_originaldb(MAKEINTRESOURCE(IDS_ORIGINALDB));
  const CString cs_comparisondb(MAKEINTRESOURCE(IDS_COMPARISONDB));
  if (bCopyLeft) {
    cs_msg.Format(IDS_COPYLEFTRIGHT, cs_comparisondb, cs_originaldb);
    ifunction = COPY_TO_ORIGINALDB;
  } else {
    cs_msg.Format(IDS_COPYLEFTRIGHT, cs_originaldb, cs_comparisondb);
    ifunction = COPY_TO_COMPARISONDB;
  }
  if (cs_text.Right(1) == _T("*"))
    cs_msg += CString(MAKEINTRESOURCE(IDS_COPYUNKNOWNFIELDS));
  if (AfxMessageBox(cs_msg, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES)
    return false;

  LRESULT lres(FALSE);
  st_CompareData *st_data;
  st_data = (st_CompareData *)m_LCResults.GetItemData(m_row);
  int pos_column = st_data->column;
  if (m_column == pos_column || pos_column == -1) {
    lres = ProcessFunction(ifunction);
  } else
    return false;

  if (lres != TRUE)
    return false;

  if (st_data->unknflds0)
    m_LCResults.SetItemText(m_row, 0, _T("=*"));
  else
    m_LCResults.SetItemText(m_row, 0, _T("="));
  if (st_data->unknflds1)
    m_LCResults.SetItemText(m_row, COMPARE, _T("=*"));
  else
    m_LCResults.SetItemText(m_row, COMPARE, _T("="));

  for (int i = 0; i < m_nCols - 5; i++)
    m_LCResults.SetItemText(m_row, USER + 1 + i, _T("-"));

  st_CompareData st_newdata;
  st_newdata = *st_data;
  st_newdata.bsDiffs.reset();

  int id = st_data->id;
  CompareData::iterator cd_iter;
  switch (pos_column) {
    case -1:
      m_numConflicts--;
      cd_iter = std::find_if(m_Conflicts.begin(), m_Conflicts.end(), equal_id(id));
      if (cd_iter != m_Conflicts.end())
        m_Conflicts.erase(cd_iter);
      break;
    case 0:
      m_numOnlyInCurrent--;
      cd_iter = std::find_if(m_OnlyInCurrent.begin(), m_OnlyInCurrent.end(), equal_id(id));
      if (cd_iter != m_OnlyInCurrent.end())
        m_OnlyInCurrent.erase(cd_iter);
      break;
    case 1:
      m_numOnlyInComp--;
      cd_iter = std::find_if(m_OnlyInComp.begin(), m_OnlyInComp.end(), equal_id(id));
      if (cd_iter != m_OnlyInComp.end())
        m_OnlyInComp.erase(cd_iter);
      break;
    default:
      ASSERT(0);
  }
  m_numIdentical++;
  st_newdata.id = m_numIdentical;
  m_Identical.push_back(st_newdata);
  UpdateStatusBar();

  st_data->column = -2;
  return true;
}

void
CCompareResultsDlg::OnItemDoubleClick( NMHDR* /* pNMHDR */, LRESULT *pResult)
{
  *pResult = 0;

  m_row = m_LCResults.GetNextItem(-1, LVNI_SELECTED);

  if (m_row == -1)
    return;

  CPoint pt = ::GetMessagePos();
  ScreenToClient(&pt);

  int colwidth0 = m_LCResults.GetColumnWidth(0);

  if (pt.x <= colwidth0) {
    m_column = 0;
  } else if  (pt.x <= (colwidth0 + m_LCResults.GetColumnWidth(1))) {
    m_column = 1;
  } else
    return;

  OnCompareViewEdit();
}

void
CCompareResultsDlg::OnItemRightClick( NMHDR* /* pNMHDR */, LRESULT *pResult)
{
  *pResult = 0;

  m_row = m_LCResults.GetNextItem(-1, LVNI_SELECTED);

  if (m_row == -1)
    return;

  CPoint msg_pt = ::GetMessagePos();
  CPoint client_pt(msg_pt);
  ScreenToClient(&client_pt);

  int ipopup, colwidth0;
  bool bTargetRO, bSourceRO;
  colwidth0 = m_LCResults.GetColumnWidth(0);

  if (client_pt.x <= colwidth0) {
    m_column = 0;
    ipopup = IDR_POPCOPYTOCOMPARISON;
    bTargetRO = m_bComparisonDBReadOnly;
    bSourceRO = m_bOriginalDBReadOnly;
  } else if  (client_pt.x <= (colwidth0 + m_LCResults.GetColumnWidth(1))) {
    m_column = 1;
    ipopup = IDR_POPCOPYTOORIGINAL;
    bTargetRO = m_bOriginalDBReadOnly;
    bSourceRO = m_bComparisonDBReadOnly;
  } else
    return;

  st_CompareData *st_data = (st_CompareData *)m_LCResults.GetItemData(m_row);
  int pos_column = st_data->column;
  if (m_column != pos_column && pos_column != -1)
    return;

  CMenu menu;
  if (menu.LoadMenu(ipopup)) {
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    // Disable copy if target is read-only
    if (bTargetRO)
      pPopup->EnableMenuItem(1, MF_BYPOSITION | MF_GRAYED);

    // Disable edit if source read-only OR if Comparison DB
    if (bSourceRO || m_column == 1) {
      const CString cs_View_Entry(MAKEINTRESOURCE(IDS_VIEWENTRY));
      pPopup->ModifyMenu(ID_MENUITEM_COMPVIEWEDIT, MF_BYCOMMAND,
                         ID_MENUITEM_COMPVIEWEDIT, cs_View_Entry);
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
  }
}

void
CCompareResultsDlg::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
  NMHEADER *pNMHeaderCtrl  = (NMHEADER *)pNMHDR;

  // Get column number to CItemData value
  int isortcolumn = pNMHeaderCtrl->iItem;

  if (m_iSortedColumn == isortcolumn) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    m_iSortedColumn = isortcolumn;
    m_bSortAscending = true;
  }

  m_LCResults.SortItems(CRCompareFunc, (LPARAM)this);

  // Reset item listindex
  for (int i = 0; i < m_LCResults.GetItemCount(); i++) {
    st_CompareData *st_data = (st_CompareData *)m_LCResults.GetItemData(i);
    st_data->listindex = i;
  }

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  CHeaderCtrl *pHDRCtrl;

  pHDRCtrl = m_LCResults.GetHeaderCtrl();
  pHDRCtrl->GetItem(isortcolumn, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  pHDRCtrl->SetItem(isortcolumn, &hdi);

  *pResult = TRUE;
}

  /*
   * Compare function used by m_LCResults.SortItems()
   * "The comparison function must return a negative value if the first item should precede
   * the second, a positive value if the first item should follow the second, or zero if
   * the two items are equivalent."
   */
int CALLBACK CCompareResultsDlg::CRCompareFunc(LPARAM lParam1, LPARAM lParam2,
           LPARAM lParamSort)
{

  // m_bSortAscending to determine the direction of the sort (duh)

  CCompareResultsDlg *self = (CCompareResultsDlg *)lParamSort;
  int nSortColumn = self->m_iSortedColumn;
  st_CompareData *LHS_st_data = (st_CompareData *)lParam1;
  st_CompareData *RHS_st_data = (st_CompareData *)lParam2;
  int LHS_Index = LHS_st_data->listindex;
  int RHS_Index = RHS_st_data->listindex;

  CString LHS_ItemText = self->m_LCResults.GetItemText(LHS_Index, nSortColumn);
  CString RHS_ItemText = self->m_LCResults.GetItemText(RHS_Index, nSortColumn);

  int iResult = LHS_ItemText.CompareNoCase(RHS_ItemText);

  if (!self->m_bSortAscending) {
    iResult *= -1;
  }
  return iResult;
}

void
CCompareResultsDlg::OnCopyToClipboard()
{
  CompareData::iterator cd_iter;
  CString resultStr(_T(""));
  CString buffer;

  if (m_OnlyInCurrent.size() > 0) {
    buffer.Format(IDS_COMPAREENTRIES1, m_cs_Filename1);
    resultStr += buffer;
    for (cd_iter = m_OnlyInCurrent.begin(); cd_iter != m_OnlyInCurrent.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS, st_data.group, st_data.title, st_data.user);
      resultStr += buffer;
    }
    resultStr += _T("\n");
  }

  if (m_OnlyInComp.size() > 0) {
    buffer.Format(IDS_COMPAREENTRIES2, m_cs_Filename2);
    resultStr += buffer;
    for (cd_iter = m_OnlyInComp.begin(); cd_iter != m_OnlyInComp.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS, st_data.group, st_data.title, st_data.user);
      resultStr += buffer;
    }
    resultStr += _T("\n");
  }

  if (m_Conflicts.size() > 0) {
    buffer.Format(IDS_COMPAREBOTHDIFF2, m_cs_Filename1, m_cs_Filename2);
    resultStr += buffer;

    const CString csx_password(MAKEINTRESOURCE(IDS_COMPPASSWORD));
    const CString csx_notes(MAKEINTRESOURCE(IDS_COMPNOTES));
    const CString csx_url(MAKEINTRESOURCE(IDS_COMPURL));
    const CString csx_autotype(MAKEINTRESOURCE(IDS_COMPAUTOTYPE));
    const CString csx_ctime(MAKEINTRESOURCE(IDS_COMPCTIME));
    const CString csx_pmtime(MAKEINTRESOURCE(IDS_COMPPMTIME));
    const CString csx_atime(MAKEINTRESOURCE(IDS_COMPATIME));
    const CString csx_ltime(MAKEINTRESOURCE(IDS_COMPLTIME));
    const CString csx_rmtime(MAKEINTRESOURCE(IDS_COMPRMTIME));
    const CString csx_pwhistory(MAKEINTRESOURCE(IDS_COMPPWHISTORY));

    for (cd_iter = m_Conflicts.begin(); cd_iter != m_Conflicts.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS2, st_data.group, st_data.title, st_data.user);
      resultStr += buffer;

      if (st_data.bsDiffs.test(CItemData::PASSWORD)) resultStr += csx_password;
      if (st_data.bsDiffs.test(CItemData::NOTES)) resultStr += csx_notes;
      if (st_data.bsDiffs.test(CItemData::URL)) resultStr += csx_url;
      if (st_data.bsDiffs.test(CItemData::AUTOTYPE)) resultStr += csx_autotype;
      if (st_data.bsDiffs.test(CItemData::CTIME)) resultStr += csx_ctime;
      if (st_data.bsDiffs.test(CItemData::PMTIME)) resultStr += csx_pmtime;
      if (st_data.bsDiffs.test(CItemData::ATIME)) resultStr += csx_atime;
      if (st_data.bsDiffs.test(CItemData::LTIME)) resultStr += csx_ltime;
      if (st_data.bsDiffs.test(CItemData::RMTIME)) resultStr += csx_rmtime;
      if (st_data.bsDiffs.test(CItemData::PWHIST)) resultStr += csx_pwhistory;
    }
  }

  app.SetClipboardData(resultStr);
}

void
CCompareResultsDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);

  CWnd *pwndListCtrl = GetDlgItem(IDC_RESULTLIST);
  CWnd *pwndODBText = GetDlgItem(IDC_COMPAREORIGINALDB);
  CWnd *pwndCDBText = GetDlgItem(IDC_COMPARECOMPARISONDB);
  CWnd *pwndCPY = GetDlgItem(IDC_COPYTOCLIPBOARD);
  CWnd *pwndOK = GetDlgItem(IDOK);

  if (!IsWindow(pwndListCtrl->GetSafeHwnd()))
    return;

  CRect ctrlRect, dlgRect;
  CPoint pt_top, pt;

  GetWindowRect(&dlgRect);

  // Allow the database names window width to grow/shrink (not height)
  pwndODBText->GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);
  pwndODBText->MoveWindow(pt_top.x, pt_top.y,
                    cx - pt_top.x - 5,
                    ctrlRect.Height(),
                    TRUE);

  GetDlgItem(IDC_COMPAREORIGINALDB)->SetWindowText(m_cs_Filename1);

  pwndCDBText->GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);
  pwndCDBText->MoveWindow(pt_top.x, pt_top.y,
                    cx - pt_top.x - 5,
                    ctrlRect.Height(),
                    TRUE);

  GetDlgItem(IDC_COMPARECOMPARISONDB)->SetWindowText(m_cs_Filename2);

  // Allow ListCtrl to grow/shrink but leave room for the buttons underneath!
  pwndListCtrl->GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  pwndListCtrl->MoveWindow(pt_top.x, pt_top.y,
                        cx - (2 * pt_top.x),
                        cy - m_cyBSpace,
                        TRUE);

  // Keep buttons in the bottom area
  int xleft, ytop;
  pwndCPY->GetWindowRect(&ctrlRect);
  xleft = (cx / 4) - (ctrlRect.Width() / 2);
  ytop = dlgRect.Height() - m_cyBSpace / 2 - m_cySBar;

  pwndCPY->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

  pwndOK->GetWindowRect(&ctrlRect);
  xleft = (3 * cx / 4) - (ctrlRect.Width() / 2);

  pwndOK->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

  m_statusBar.GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  m_statusBar.MoveWindow(pt_top.x, cy - ctrlRect.Height(),
                        cx - (2 * pt_top.x),
                        ctrlRect.Height(),
                        TRUE);
}

void CCompareResultsDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  CDialog::OnGetMinMaxInfo(lpMMI);

  if (this->GetSafeHwnd() != NULL) {
    lpMMI->ptMinTrackSize = CPoint(m_DialogMinWidth, m_DialogMinHeight);
    lpMMI->ptMaxTrackSize = CPoint(m_DialogMaxWidth, m_DialogMaxHeight);
  }
}
