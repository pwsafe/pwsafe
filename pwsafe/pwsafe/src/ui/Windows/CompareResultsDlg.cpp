/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// CompareResultsDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "CompareResultsDlg.h"

#include "core/PWScore.h"
#include "core/Report.h"

#include "resource.h"
#include "resource2.h"
#include "resource3.h"

#include <vector>
#include <algorithm>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCompareResultsDlg, CPWResizeDialog)

//-----------------------------------------------------------------------------
CCompareResultsDlg::CCompareResultsDlg(CWnd* pParent,
                                       CompareData &OnlyInCurrent, CompareData &OnlyInComp,
                                       CompareData &Conflicts, CompareData &Identical,
                                       CItemData::FieldBits &bsFields,
                                       PWScore *pcore0, PWScore *pcore1,
                                       CReport *pRpt)
  : CPWResizeDialog(CCompareResultsDlg::IDD, pParent),
  m_OnlyInCurrent(OnlyInCurrent), m_OnlyInComp(OnlyInComp),
  m_Conflicts(Conflicts), m_Identical(Identical),
  m_bsFields(bsFields), m_pcore0(pcore0), m_pcore1(pcore1),
  m_pRpt(pRpt), m_bSortAscending(true), m_iSortedColumn(0),
  m_OriginalDBChanged(false), m_ComparisonDBChanged(false),
  m_bTreatWhiteSpaceasEmpty(false),
  m_ShowIdenticalEntries(BST_UNCHECKED)
{
  m_pDbx = static_cast<DboxMain *>(pParent);
}

CCompareResultsDlg::~CCompareResultsDlg()
{
}

// Return whether first [g:t:u] is greater than the second [g:t:u]
// used in std::sort in OnInitDialog below.
bool GTUCompare2(st_CompareData elem1, st_CompareData elem2)
{
  if (elem1.group != elem2.group)
    return _wcsicmp(elem1.group.c_str(), elem2.group.c_str()) < 0;

  if (elem1.title != elem2.title)
    return _wcsicmp(elem1.title.c_str(), elem2.title.c_str()) < 0;

  return _wcsicmp(elem1.user.c_str(), elem2.user.c_str()) < 0;
}

void CCompareResultsDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_RESULTLIST, m_LCResults);
}

BEGIN_MESSAGE_MAP(CCompareResultsDlg, CPWResizeDialog)
  ON_WM_SIZE()
  ON_WM_INITMENUPOPUP()
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDC_SHOW_IDENTICAL_ENTRIES, OnShowIdenticalEntries)
  ON_NOTIFY(NM_RCLICK, IDC_RESULTLIST, OnItemRightClick)
  ON_NOTIFY(NM_DBLCLK, IDC_RESULTLIST, OnItemDoubleClick)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_RESULTLIST, OnItemChanging)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_RESULTLISTHDR, OnColumnClick)
  ON_COMMAND(ID_MENUITEM_COMPVIEWEDIT, OnCompareViewEdit)
  ON_COMMAND(ID_MENUITEM_SYNCHRONIZE, OnCompareSynchronize)
  ON_COMMAND(ID_MENUITEM_COPY_TO_ORIGINAL, OnCompareCopyToOriginalDB)
  ON_COMMAND(ID_MENUITEM_COPYALL_TO_ORIGINAL, OnCompareCopyAllToOriginalDB)
  ON_COMMAND(ID_MENUITEM_SYNCHRONIZEALL, OnCompareSynchronizeAll)
END_MESSAGE_MAP()

BOOL CCompareResultsDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);

  AddMainCtrlID(IDC_RESULTLIST);
  AddBtnsCtrlIDs(vibottombtns);

  UINT statustext[1] = {IDS_STATCOMPANY};
  SetStatusBar(&statustext[0], 1);

  CPWResizeDialog::OnInitDialog();

  m_menuManager.Install(this);
  m_menuManager.SetImageList(&m_pDbx->m_MainToolBar);
  m_menuManager.SetMapping(&m_pDbx->m_MainToolBar);

  m_LCResults.GetHeaderCtrl()->SetDlgCtrlID(IDC_RESULTLISTHDR);

  DWORD dwExtendedStyle = m_LCResults.GetExtendedStyle();
  dwExtendedStyle |= (LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
  m_LCResults.SetExtendedStyle(dwExtendedStyle);

  CString cs_header;
  int i;
  const struct {
    UINT ids; int ncol;
  } FixedCols[] = {{IDS_ORIGINALDB, CURRENT}, {IDS_COMPARISONDB, COMPARE},
                   {IDS_GROUP, GROUP}, {IDS_TITLE, TITLE}, {IDS_USERNAME, USER},
  };

  for (i = 0; i < sizeof(FixedCols) / sizeof(FixedCols[0]); i++) {
    cs_header.LoadString(FixedCols[i].ids);
    m_LCResults.InsertColumn(FixedCols[i].ncol, cs_header);
  }

  const struct {
    CItemData::FieldType ft; UINT ids; int ncol;
  } OptCols[] = {{CItemData::PASSWORD, IDS_PASSWORD, PASSWORD},
                 {CItemData::NOTES, IDS_NOTES, NOTES},
                 {CItemData::URL, IDS_URL, URL},
                 {CItemData::AUTOTYPE, IDS_AUTOTYPE, AUTOTYPE},
                 {CItemData::PWHIST, IDS_PWHISTORY, PWHIST},
                 {CItemData::CTIME, IDS_CREATED, CTIME},
                 {CItemData::ATIME, IDS_LASTACCESSED, ATIME},
                 {CItemData::XTIME, IDS_PASSWORDEXPIRYDATE, XTIME},
                 {CItemData::XTIME_INT, IDS_PASSWORDEXPIRYDATEINT, XTIME_INT},
                 {CItemData::PMTIME, IDS_PASSWORDMODIFIED, PMTIME},
                 {CItemData::RMTIME, IDS_LASTMODIFIED, RMTIME},
                 {CItemData::POLICY, IDS_PWPOLICY, POLICY},
                 {CItemData::SYMBOLS, IDS_SYMBOLS, SYMBOLS},
                 {CItemData::POLICYNAME, IDS_POLICYNAME, POLICYNAME},
                 {CItemData::RUNCMD, IDS_RUNCOMMAND, RUNCMD},
                 {CItemData::EMAIL, IDS_EMAIL, EMAIL},
                 {CItemData::DCA, IDS_DCA, DCA},
                 {CItemData::SHIFTDCA, IDS_SHIFTDCA, SHIFTDCA},
                 {CItemData::PROTECTED, IDS_PROTECTED, PROTECTED},
  };

  for (i = 0; i < sizeof(OptCols) / sizeof(OptCols[0]); i++)
    if (m_bsFields.test(OptCols[i].ft)) {
      cs_header.LoadString(OptCols[i].ids);
      m_LCResults.InsertColumn(OptCols[i].ncol, cs_header, LVCFMT_CENTER);
    }
  m_nCols = m_LCResults.GetHeaderCtrl()->GetItemCount();

  m_numOnlyInCurrent = m_OnlyInCurrent.size();
  m_numOnlyInComp = m_OnlyInComp.size();
  m_numConflicts = m_Conflicts.size();
  m_numIdentical = m_Identical.size();
  m_LCResults.SetItemCount((int)(m_numOnlyInCurrent + m_numOnlyInComp +
                           m_numConflicts + m_numIdentical));

  // Sort the entries first by group, title, user (not case sensitive)
  if (m_numOnlyInCurrent > 0)
    std::sort(m_OnlyInCurrent.begin(), m_OnlyInCurrent.end(), GTUCompare2);
  if (m_numOnlyInComp > 0)
    std::sort(m_OnlyInComp.begin(), m_OnlyInComp.end(), GTUCompare2);
  if (m_numConflicts > 0)
    std::sort(m_Conflicts.begin(), m_Conflicts.end(), GTUCompare2);
  if (m_numIdentical > 0)
    std::sort(m_Identical.begin(), m_Identical.end(), GTUCompare2);

  AddCompareEntries(false);
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

  int itotalwidth = 0;
  for (i = 0; i < m_nCols; i++) {
    itotalwidth += m_LCResults.GetColumnWidth(i);
  }

  int iMaxWidth = itotalwidth + 16;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);

  GetDlgItem(IDC_COMPAREORIGINALDB)->SetWindowText(m_scFilename1);
  GetDlgItem(IDC_COMPARECOMPARISONDB)->SetWindowText(m_scFilename2);

  WriteReportData();
  UpdateStatusBar();
  return FALSE;
}

void CCompareResultsDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
  // Add pretty pictures to our menu
  m_pDbx->CPRInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CCompareResultsDlg::AddCompareEntries(const bool bAddIdentical)
{
  int i, iItem = 0;
  CompareData::iterator cd_iter;

  if (bAddIdentical && m_numIdentical > 0) {
    for (cd_iter = m_Identical.begin(); cd_iter != m_Identical.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      if (st_data.unknflds0)
        m_LCResults.InsertItem(iItem, L"=*");
      else
        m_LCResults.InsertItem(iItem, L"=");
      if (st_data.unknflds1)
        m_LCResults.SetItemText(iItem, COMPARE, L"=*");
      else
        m_LCResults.SetItemText(iItem, COMPARE, L"=");

      m_LCResults.SetItemText(iItem, GROUP, st_data.group.c_str());
      if (st_data.bIsProtected0)
        m_LCResults.SetItemText(iItem, TITLE, (st_data.title + StringX(L" #")).c_str());
      else
        m_LCResults.SetItemText(iItem, TITLE, st_data.title.c_str());
      m_LCResults.SetItemText(iItem, USER, st_data.user.c_str());
      for (i = USER + 1; i < m_nCols; i++)
        m_LCResults.SetItemText(iItem, i, L"-");

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, MAKELONG(IDENTICAL, st_data.id));
      iItem++;
    }
  }

  if (m_numOnlyInCurrent > 0) {
    for (cd_iter = m_OnlyInCurrent.begin(); cd_iter != m_OnlyInCurrent.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      if (st_data.unknflds0)
        m_LCResults.InsertItem(iItem, L"Y*");
      else
        m_LCResults.InsertItem(iItem, L"Y");

      m_LCResults.SetItemText(iItem, COMPARE, L"-");
      m_LCResults.SetItemText(iItem, GROUP, st_data.group.c_str());
      if (st_data.bIsProtected0)
        m_LCResults.SetItemText(iItem, TITLE, (st_data.title + StringX(L" #")).c_str());
      else
        m_LCResults.SetItemText(iItem, TITLE, st_data.title.c_str());
      m_LCResults.SetItemText(iItem, USER, st_data.user.c_str());
      for (i = USER + 1; i < m_nCols; i++)
        m_LCResults.SetItemText(iItem, i, L"-");

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, MAKELONG(CURRENT, st_data.id));
      iItem++;
    }
  }

  if (m_numOnlyInComp > 0) {
    for (cd_iter = m_OnlyInComp.begin(); cd_iter != m_OnlyInComp.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      m_LCResults.InsertItem(iItem, L"-");
      if (st_data.unknflds1)
        m_LCResults.SetItemText(iItem, COMPARE, L"Y*");
      else
        m_LCResults.SetItemText(iItem, COMPARE, L"Y");

      m_LCResults.SetItemText(iItem, GROUP, st_data.group.c_str());
      m_LCResults.SetItemText(iItem, TITLE, st_data.title.c_str());
      m_LCResults.SetItemText(iItem, USER, st_data.user.c_str());
      for (i = USER + 1; i < m_nCols; i++)
        m_LCResults.SetItemText(iItem, i, L"-");

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, MAKELONG(COMPARE, st_data.id));
      iItem++;
    }
  }

  if (m_numConflicts > 0) {
    int icol;
    for (cd_iter = m_Conflicts.begin(); cd_iter != m_Conflicts.end(); cd_iter++) {
      st_CompareData &st_data = *cd_iter;

      if (st_data.unknflds0)
        m_LCResults.InsertItem(iItem, L"Y*");
      else
        m_LCResults.InsertItem(iItem, L"Y");

      if (st_data.unknflds1)
        m_LCResults.SetItemText(iItem, COMPARE, L"Y*");
      else
        m_LCResults.SetItemText(iItem, COMPARE, L"Y");

      m_LCResults.SetItemText(iItem, GROUP, st_data.group.c_str());
      if (st_data.bIsProtected0)
        m_LCResults.SetItemText(iItem, TITLE, (st_data.title + StringX(L" #")).c_str());
      else
        m_LCResults.SetItemText(iItem, TITLE, st_data.title.c_str());
      m_LCResults.SetItemText(iItem, USER, st_data.user.c_str());

      // Start of the 'data' columns (if present)
      icol = PASSWORD;
      if (m_bsFields.test(CItemData::PASSWORD))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PASSWORD) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::NOTES))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::NOTES) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::URL))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::URL) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::AUTOTYPE))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::AUTOTYPE) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::PWHIST))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PWHIST) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::CTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::CTIME) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::ATIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::ATIME) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::XTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::XTIME) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::XTIME_INT))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::XTIME_INT) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::PMTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PMTIME) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::RMTIME))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::RMTIME) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::POLICY))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::POLICY) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::RUNCMD))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::RUNCMD) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::DCA))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::DCA) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::EMAIL))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::EMAIL) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::PROTECTED))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::PROTECTED) ? L"X" : L"-");
      if (m_bsFields.test(CItemData::SYMBOLS))
        m_LCResults.SetItemText(iItem, icol++, st_data.bsDiffs.test(CItemData::SYMBOLS) ? L"X" : L"-");

      st_data.listindex = iItem;
      m_LCResults.SetItemData(iItem, MAKELONG(BOTH, st_data.id));
      iItem++;
    }
  }
}

void CCompareResultsDlg::OnShowIdenticalEntries()
{
  m_ShowIdenticalEntries = ((CButton *)GetDlgItem(IDC_SHOW_IDENTICAL_ENTRIES))->GetCheck();

  m_LCResults.SetRedraw(FALSE);
  m_LCResults.DeleteAllItems();

  AddCompareEntries(m_ShowIdenticalEntries == BST_CHECKED);

  int i;

  for (i = 0; i < m_nCols - 1; i++) {
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int header_width = m_LCResults.GetColumnWidth(i);
    m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int data_width = m_LCResults.GetColumnWidth(i);
    if (header_width > data_width)
      m_LCResults.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }
  m_LCResults.SetColumnWidth(m_nCols - 1, LVSCW_AUTOSIZE_USEHEADER);

  // Reset item listindex
  for (i = 0; i < m_LCResults.GetItemCount(); i++) {
    DWORD_PTR dwItemData = m_LCResults.GetItemData(i);
    st_CompareData *pst_data = GetCompareData(dwItemData);
    ASSERT(pst_data != NULL);
    pst_data->listindex = i;
  }

  m_LCResults.SortItems(CRCompareFunc, (LPARAM)this);
  m_LCResults.SetRedraw(TRUE);
  m_LCResults.Invalidate();
}

void CCompareResultsDlg::OnCancel()
{
  m_menuManager.Cleanup();

  CPWResizeDialog::OnCancel();
}

void CCompareResultsDlg::OnOK()
{
  m_menuManager.Cleanup();

  CPWResizeDialog::OnOK();
}

void CCompareResultsDlg::OnHelp()
{
  CString cs_HelpTopic = app.GetHelpFileName() + L"::/html/compare_results.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CCompareResultsDlg::UpdateStatusBar()
{
  m_results.Format(IDS_COMPARERESULTS, m_numOnlyInCurrent, m_numOnlyInComp,
                                       m_numConflicts, m_numIdentical);
  m_statusBar.SetPaneText(0, m_results, TRUE);
  m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
  m_statusBar.UpdateWindow();
}

LRESULT CCompareResultsDlg::ProcessAllFunction(const int ifunction,
                                               std::vector<st_CompareData *> vpst_data)
{
  std::vector<st_CompareInfo *> vpst_info;
  for (size_t index = 0; index < vpst_data.size(); index++) {
    st_CompareData *pst_data = vpst_data[index];
    st_CompareInfo *pst_info;
    pst_info = new st_CompareInfo;

    pst_info->pcore0 = m_pcore0;
    pst_info->pcore1 = m_pcore1;
    pst_info->uuid0 = pst_data->uuid0;
    pst_info->uuid1 = pst_data->uuid1;
    pst_info->clicked_column = 1;
    vpst_info.push_back(pst_info);
  }

  LRESULT lres = ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                               PWS_MSG_COMPARE_RESULT_ALLFNCTN,
                               (WPARAM)&vpst_info, (LPARAM)ifunction);
  if (lres == TRUE) {
    CSecString group, title, user, buffer, cs_tmp;
    ItemListIter pos;

    for (size_t index = 0; index < vpst_info.size(); index++) {
      st_CompareData *pst_data = vpst_data[index];
      st_CompareInfo *pst_info = vpst_info[index];
      switch (ifunction) {
        case COPYALL_TO_ORIGINALDB:
        case SYNCHALL:
          // UUID of copied entry returned - now update data
          pst_data->uuid0 = pst_info->uuid0;

          pos = m_pcore1->Find(pst_info->uuid1);
          ASSERT(pos != m_pcore1->GetEntryEndIter());

          group = pos->second.GetGroup();
          title = pos->second.GetTitle();
          user = pos->second.GetUser();
          cs_tmp.LoadString(IDS_ORIGINALDB);
          buffer.Format(ifunction == SYNCHALL ? IDS_SYNCENTRY : IDS_COPYENTRY,
                        cs_tmp, group, title, user);
          m_pRpt->WriteLine((LPCWSTR)buffer);
          break;
        case EDIT:
        case VIEW:
        case SYNCH:
          break;
        default:
          ASSERT(0);
      }
    }
  }

  // Delete structures - vector will be deleted at end of scope automatically
  for (size_t index = 0; index < vpst_info.size(); index++) {
    st_CompareInfo *pst_info = vpst_info[index];
    delete pst_info;
  }

  return lres;
}

bool CCompareResultsDlg::ProcessFunction(const int ifunction,
                                         st_CompareData *pst_data)
{
  st_CompareInfo *pst_info;
  pst_info = new st_CompareInfo;

  bool rc(false);

  int indatabase = pst_data->indatabase;
  if (m_LCResults.GetColumn() == indatabase || indatabase == BOTH) {
    pst_info->pcore0 = m_pcore0;
    pst_info->pcore1 = m_pcore1;
    pst_info->uuid0 = pst_data->uuid0;
    pst_info->uuid1 = pst_data->uuid1;
    pst_info->clicked_column = m_LCResults.GetColumn();

    LRESULT lres = ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                                 PWS_MSG_COMPARE_RESULT_FUNCTION,
                                 (WPARAM)pst_info, (LPARAM)ifunction);
    if (lres == TRUE) {
      CSecString group, title, user, buffer, cs_tmp;
      ItemListIter pos;

      switch (ifunction) {
        case COPY_TO_ORIGINALDB:
        case SYNCH:
          // UUID of copied entry returned - now update data
          pst_data->uuid0 = pst_info->uuid0;

          pos = m_pcore1->Find(pst_info->uuid1);
          ASSERT(pos != m_pcore1->GetEntryEndIter());

          group = pos->second.GetGroup();
          title = pos->second.GetTitle();
          user = pos->second.GetUser();
          cs_tmp.LoadString(IDS_ORIGINALDB);
          buffer.Format(ifunction == SYNCH ? IDS_SYNCENTRY : IDS_COPYENTRY,
                        cs_tmp, group, title, user);
          m_pRpt->WriteLine((LPCWSTR)buffer);
          break;
        case EDIT:
        case VIEW:
          break;
        default:
          ASSERT(0);
      }
      rc = true;
    }
  }

  delete pst_info;
  return rc;
}

st_CompareData * CCompareResultsDlg::GetCompareData(const LONG_PTR dwItemData)
{
  return GetCompareData(dwItemData, this);
}

st_CompareData * CCompareResultsDlg::GetCompareData(const LONG_PTR dwItemData,
                                                    CCompareResultsDlg *self)
{
  const int iList = (short int)LOWORD(dwItemData);
  const int id = HIWORD(dwItemData);
  CompareData::iterator cd_iter;
  st_CompareData *retval(NULL);

  switch (iList) {
    case IDENTICAL:
      cd_iter = std::find_if(self->m_Identical.begin(), self->m_Identical.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != self->m_Identical.end())
        retval = &*cd_iter;
      break;
    case BOTH:
      cd_iter = std::find_if(self->m_Conflicts.begin(), self->m_Conflicts.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != self->m_Conflicts.end())
        retval = &*cd_iter;
      break;
    case CURRENT:
      cd_iter = std::find_if(self->m_OnlyInCurrent.begin(), self->m_OnlyInCurrent.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != self->m_OnlyInCurrent.end())
        retval = &*cd_iter;
      break;
    case COMPARE:
      cd_iter = std::find_if(self->m_OnlyInComp.begin(), self->m_OnlyInComp.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != self->m_OnlyInComp.end())
        retval = &*cd_iter;
      break;
    default:
      ASSERT(0);
  }
  return retval;
}

void CCompareResultsDlg::OnCompareViewEdit()
{
  bool bDatabaseRO = (m_LCResults.GetColumn() == CURRENT) ? m_bOriginalDBReadOnly : m_bComparisonDBReadOnly;

  DWORD_PTR dwItemData = m_LCResults.GetItemData(m_LCResults.GetRow());
  st_CompareData *pst_data = GetCompareData(dwItemData);
  ASSERT(pst_data != NULL);

  if (bDatabaseRO || m_LCResults.GetColumn() == COMPARE)
    ProcessFunction(VIEW, pst_data);
  else
    ProcessFunction(EDIT, pst_data);
}

void CCompareResultsDlg::OnCompareSynchronize()
{
  if (m_bOriginalDBReadOnly)
    return;

  CGeneralMsgBox gmb;
  CString cs_temp, cs_title;
  // Initialize set
  GTUSet setGTU;

  // First check database
  if (!m_pcore0->GetUniqueGTUValidated() && !m_pcore0->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_SYNCHFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, m_pcore0->GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }
  setGTU.clear();  // Don't need it anymore - so clear it now

  DWORD_PTR dwItemData = m_LCResults.GetItemData(m_LCResults.GetRow());
  st_CompareData *pst_data = GetCompareData(dwItemData);
  ASSERT(pst_data != NULL);

  ProcessFunction(SYNCH, pst_data);
}

void CCompareResultsDlg::OnCompareCopyToOriginalDB()
{
  if (m_bOriginalDBReadOnly)
    return;

  // Check not already equal
  CString cs_text = m_LCResults.GetItemText(m_LCResults.GetRow(), m_LCResults.GetColumn());
  if (cs_text.Compare(L"=") == 0)
    return;

  CGeneralMsgBox gmb;
  CString cs_msg;
  int ifunction;

  const CString cs_originaldb(MAKEINTRESOURCE(IDS_ORIGINALDB));
  const CString cs_comparisondb(MAKEINTRESOURCE(IDS_COMPARISONDB));

  cs_msg.Format(IDS_COPYLEFTRIGHT, cs_comparisondb, cs_originaldb);
  ifunction = COPY_TO_ORIGINALDB;

  if (cs_text.Right(1) == L"*")
    cs_msg += CString(MAKEINTRESOURCE(IDS_COPYUNKNOWNFIELDS));

  if (gmb.AfxMessageBox(cs_msg, NULL,
                        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
    return;

  LRESULT lres(FALSE);
  DWORD_PTR dwItemData = m_LCResults.GetItemData(m_LCResults.GetRow());
  st_CompareData *pst_data = GetCompareData(dwItemData);
  ASSERT(pst_data != NULL);

  const int indatabase = pst_data->indatabase;
  if (m_LCResults.GetColumn() == indatabase || indatabase == BOTH) {
    lres = ProcessFunction(ifunction, pst_data);
  } else
    return;

  if (lres != TRUE)
    return;

  if (pst_data->unknflds0)
    m_LCResults.SetItemText(m_LCResults.GetRow(), CURRENT, L"=*");
  else
    m_LCResults.SetItemText(m_LCResults.GetRow(), CURRENT, L"=");

  if (pst_data->unknflds1)
    m_LCResults.SetItemText(m_LCResults.GetRow(), COMPARE, L"=*");
  else
    m_LCResults.SetItemText(m_LCResults.GetRow(), COMPARE, L"=");

  for (int i = 0; i < m_nCols - 5; i++)
    m_LCResults.SetItemText(m_LCResults.GetRow(), USER + 1 + i, L"-");

  st_CompareData st_newdata;
  st_newdata = *pst_data;
  st_newdata.bsDiffs.reset();

  const int id = pst_data->id;
  CompareData::iterator cd_iter;
  switch (indatabase) {
    case BOTH:
      m_numConflicts--;
      cd_iter = std::find_if(m_Conflicts.begin(), m_Conflicts.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != m_Conflicts.end())
        m_Conflicts.erase(cd_iter);
      break;
    case CURRENT:
      m_numOnlyInCurrent--;
      cd_iter = std::find_if(m_OnlyInCurrent.begin(), m_OnlyInCurrent.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != m_OnlyInCurrent.end())
        m_OnlyInCurrent.erase(cd_iter);
      break;
    case COMPARE:
      m_numOnlyInComp--;
      cd_iter = std::find_if(m_OnlyInComp.begin(), m_OnlyInComp.end(),
                             std::bind2nd(std::equal_to<int>(), id));
      if (cd_iter != m_OnlyInComp.end())
        m_OnlyInComp.erase(cd_iter);
      break;
    case IDENTICAL:
    default:
      ASSERT(0);
  }

  m_numIdentical++;
  st_newdata.id = static_cast<int>(m_numIdentical);
  st_newdata.indatabase = IDENTICAL;
  m_Identical.push_back(st_newdata);
  m_LCResults.SetItemData(m_LCResults.GetRow(), MAKELONG(IDENTICAL, st_newdata.id));
  UpdateStatusBar();

  m_OriginalDBChanged = true;
}

void CCompareResultsDlg::OnCompareCopyAllToOriginalDB()
{
  DoAllFunctions(COPYALL_TO_ORIGINALDB);
}

void CCompareResultsDlg::OnCompareSynchronizeAll()
{
  DoAllFunctions(SYNCHALL);
}

void CCompareResultsDlg::DoAllFunctions(const int ifunction)
{
  // Shouldn't ever get here if original is R-O
  if (m_bOriginalDBReadOnly)
    return;

  CGeneralMsgBox gmb;
  CString cs_msg;

  const CString cs_originaldb(MAKEINTRESOURCE(IDS_ORIGINALDB));
  const CString cs_comparisondb(MAKEINTRESOURCE(IDS_COMPARISONDB));

  cs_msg.Format(ifunction == COPYALL_TO_ORIGINALDB ? IDS_COPYALL : IDS_SYNCHRONIZEALL,
                cs_comparisondb, cs_originaldb);

  // Check if any records have unknown fields
  POSITION pos = m_LCResults.GetFirstSelectedItemPosition();

  while (pos) {
    int irow = m_LCResults.GetNextSelectedItem(pos);
    CString cs_text = m_LCResults.GetItemText(irow, 1);
    if (cs_text.Right(1) == L"*") {
      cs_msg += CString(MAKEINTRESOURCE(IDS_COPYUNKNOWNFIELDS));
      break;
    }
  }

  if (gmb.AfxMessageBox(cs_msg, NULL,
                        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
    return;

  // Get vector of all the data
  pos = m_LCResults.GetFirstSelectedItemPosition();
  std::vector<st_CompareData *> vpst_data;
  while (pos) {
    int irow = m_LCResults.GetNextSelectedItem(pos);
    DWORD_PTR dwItemData = m_LCResults.GetItemData(irow);
    st_CompareData *pst_data = GetCompareData(dwItemData);
    ASSERT(pst_data != NULL);
    vpst_data.push_back(pst_data);
  }

  // Do it
  LRESULT lres = ProcessAllFunction(ifunction, vpst_data);

  if (lres != TRUE)
    return;

  size_t index = 0;
  pos = m_LCResults.GetFirstSelectedItemPosition();

  while (pos) {
    const int irow = m_LCResults.GetNextSelectedItem(pos);

    ASSERT(index < vpst_data.size());

    st_CompareData *pst_data = vpst_data[index];
    bool bNowEqual = true; // Must be if copied, now check if synch'd
    if (ifunction == SYNCHALL)
      bNowEqual = CompareEntries(pst_data);
   
    if (bNowEqual) {
      // Update Compare results
      if (pst_data->unknflds0)
        m_LCResults.SetItemText(irow, CURRENT, L"=*");
      else
        m_LCResults.SetItemText(irow, CURRENT, L"=");

      if (pst_data->unknflds1)
        m_LCResults.SetItemText(irow, COMPARE, L"=*");
      else
        m_LCResults.SetItemText(irow, COMPARE, L"=");

      for (int i = 0; i < m_nCols - 5; i++)
        m_LCResults.SetItemText(irow, USER + 1 + i, L"-");

      st_CompareData st_newdata;
      st_newdata = *pst_data;
      st_newdata.bsDiffs.reset();

      const int id = pst_data->id;
      CompareData::iterator cd_iter;
      switch (pst_data->indatabase) {
        case BOTH:
          m_numConflicts--;
          cd_iter = std::find_if(m_Conflicts.begin(), m_Conflicts.end(),
                                 std::bind2nd(std::equal_to<int>(), id));
          if (cd_iter != m_Conflicts.end())
            m_Conflicts.erase(cd_iter);
          break;
        case COMPARE:
          m_numOnlyInComp--;
          cd_iter = std::find_if(m_OnlyInComp.begin(), m_OnlyInComp.end(),
                                 std::bind2nd(std::equal_to<int>(), id));
          if (cd_iter != m_OnlyInComp.end())
            m_OnlyInComp.erase(cd_iter);
          break;
        case CURRENT:
        case IDENTICAL:
        default:
          ASSERT(0);
      }

      m_numIdentical++;
      st_newdata.id = static_cast<int>(m_numIdentical);
      st_newdata.indatabase = IDENTICAL;
      m_Identical.push_back(st_newdata);
      m_LCResults.SetItemData(m_LCResults.GetRow(), MAKELONG(IDENTICAL, st_newdata.id));
    }
    index++;
  }

  UpdateStatusBar();

  m_OriginalDBChanged = true;
}

void CCompareResultsDlg::OnItemDoubleClick(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMLISTVIEW *pNMLV = (NMLISTVIEW *)pNMHDR;

  if (m_LCResults.GetSelectedCount() > 1)
    return;

  m_LCResults.SetRow(pNMLV->iItem);

  switch (pNMLV->iSubItem) {
    case 0:
      m_LCResults.SetColumn(CURRENT);
      break;
    case 1:
      m_LCResults.SetColumn(COMPARE);
      break;
    default:
      return;
  }

  OnCompareViewEdit();
}

void CCompareResultsDlg::OnItemRightClick(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMLISTVIEW *pNMLV = (NMLISTVIEW *)pNMHDR;

  m_LCResults.SetRow(pNMLV->iItem);

  CPoint msg_pt = ::GetMessagePos();
  CMenu menu;
  int ipopup;
  bool bTargetRO, bSourceRO;

  if (m_LCResults.GetSelectedCount() != 1) {
    // Special processing - only allow "Copy All" items to original or 
    // "Synchronise All" items to original
    // Do not allow "Synchronise All" is any selected entry does not
    // have a corresponding entry in the original DB
    // No point if original is R-O - is checked in OnItemChanging()
    m_LCResults.SetColumn(COMPARE);
    ipopup = IDR_POPCOPYALLTOORIGINAL;
    bTargetRO = m_bOriginalDBReadOnly;
    bSourceRO = m_bComparisonDBReadOnly;
    
    bool bNoSyncAll(false);
    POSITION pos = m_LCResults.GetFirstSelectedItemPosition();

    while (pos) {
      const int irow = m_LCResults.GetNextSelectedItem(pos);
      DWORD_PTR dwItemData = m_LCResults.GetItemData(irow);
      st_CompareData *pst_data = GetCompareData(dwItemData);
      ASSERT(pst_data != NULL);
      if (pst_data->uuid0 == pws_os::CUUID::NullUUID() || pst_data->indatabase != BOTH) {
        bNoSyncAll = true;
        break;
      }
    }

    if (menu.LoadMenu(ipopup)) {
      MENUINFO minfo ={0};
      minfo.cbSize = sizeof(MENUINFO);
      minfo.fMask = MIM_MENUDATA;
      minfo.dwMenuData = ipopup;
      BOOL brc = menu.SetMenuInfo(&minfo);
      ASSERT(brc != 0);

      CMenu *pPopup = menu.GetSubMenu(0);
      ASSERT(pPopup != NULL);
      
      if (bNoSyncAll)
        pPopup->RemoveMenu(ID_MENUITEM_SYNCHRONIZEALL, MF_BYCOMMAND);

      pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
    }
    return;
  }

  switch (pNMLV->iSubItem) {
    case 0:
      // Column is the current database
      // Therefore: Source = Current DB, Target = Comparison DB
      m_LCResults.SetColumn(CURRENT);
      ipopup = IDR_POPEDITVIEWORIGINAL;
      bTargetRO = m_bComparisonDBReadOnly;
      bSourceRO = m_bOriginalDBReadOnly;
      break;
    case 1:
      // Column is the comparison database
      // Therefore: Source = Comparison DB, Target = Current DB
      m_LCResults.SetColumn(COMPARE);
      ipopup = IDR_POPCOPYTOORIGINAL;
      bTargetRO = m_bOriginalDBReadOnly;
      bSourceRO = m_bComparisonDBReadOnly;
      break;
    default:
      return;
  }

  DWORD_PTR dwItemData = m_LCResults.GetItemData(m_LCResults.GetRow());
  st_CompareData *pst_data = GetCompareData(dwItemData);
  ASSERT(pst_data != NULL);

  // Get where this entry is:
  // IDENTICAL means CURRENT + COMPARE but identical
  // BOTH      means CURRENT + COMPARE but with differences
  int indatabase = pst_data->indatabase;

  // If entry isn't in this database that the user click or the entry is only
  // in the other column - do nothing
  if (m_LCResults.GetColumn() != indatabase && 
      (indatabase != BOTH && indatabase != IDENTICAL))
    return;

  if (menu.LoadMenu(ipopup)) {
    MENUINFO minfo ={0};
    minfo.cbSize = sizeof(MENUINFO);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = ipopup;
    BOOL brc = menu.SetMenuInfo(&minfo);
    ASSERT(brc != 0);

    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    // Disable copy/sychnronize if target is read-only or entry is protected
    // Delete synchronize if not in both databases (and not already identical)
    if (m_LCResults.GetColumn() == COMPARE) {
      // User clicked on Comparison DB
      if (bTargetRO) {
        // Can't modify RO DB
        pPopup->RemoveMenu(ID_MENUITEM_COPY_TO_ORIGINAL, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_SYNCHRONIZE, MF_BYCOMMAND);
      } else {
        // If it is in the current DB (i.e. BOTH as we know it is in the compare
        // column as the user has clicked on it) and is protected - don't allow copy
        if (indatabase == BOTH || pst_data->bIsProtected0)
          pPopup->RemoveMenu(ID_MENUITEM_COPY_TO_ORIGINAL, MF_BYCOMMAND);
      }
      // Can't synchonize if not in both databases!
      if (indatabase != BOTH)
        pPopup->RemoveMenu(ID_MENUITEM_SYNCHRONIZE, MF_BYCOMMAND);
    }

    // Change Edit to View if source read-only OR entry is protected OR if Comparison DB
    if (bSourceRO || pst_data->bIsProtected0) {
      const CString cs_View_Entry(MAKEINTRESOURCE(IDS_VIEWENTRY2));
      pPopup->ModifyMenu(ID_MENUITEM_COMPVIEWEDIT, MF_BYCOMMAND,
                         ID_MENUITEM_COMPVIEWEDIT, cs_View_Entry);
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
  }
}

void CCompareResultsDlg::OnItemChanging(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = FALSE;  // Allow change

  NMLISTVIEW *pNMLV = reinterpret_cast<NMLISTVIEW *>(pNMHDR);
  int irow = pNMLV->iItem;
  
  // Check if state unchanged - unchanged == not interested
  if ((pNMLV->uChanged & LVIF_STATE) != LVIF_STATE)
    return;

  // Has the selected state changed?
  // Was selected and now not or Wasn't selected and still isn't - ignore
  if (((pNMLV->uOldState & LVIS_SELECTED)!= 0 && (pNMLV->uNewState & LVIS_SELECTED) == 0) ||
      ((pNMLV->uOldState & LVIS_SELECTED) == 0 && (pNMLV->uNewState & LVIS_SELECTED) == 0)) {
    return;
  }

  DWORD_PTR dwItemData = m_LCResults.GetItemData(irow);
  st_CompareData *pst_data = GetCompareData(dwItemData);
  ASSERT(pst_data != NULL);

  CPoint pt(GetMessagePos());
  m_LCResults.ScreenToClient(&pt);

  LVHITTESTINFO hti = {0};
  hti.pt = pt;
  int iItem = m_LCResults.SubItemHitTest(&hti);
  
  // Ignore any clicks not on an item
  if (iItem == -1 ||
      hti.flags & (LVHT_NOWHERE |
                   LVHT_ABOVE   | LVHT_BELOW |
                   LVHT_TORIGHT | LVHT_TOLEFT)) {
      return;
  }

  
  if ((GetKeyState(VK_CONTROL) & 0x8000)) {
    // Control key pressed - multi-select
    /*
       Since we only allow multi-select for Copy or Synchronise from Compare DB
       to the original Db, we won't allow if row entry is not in the Compare DB
       ...  pst_data->indatabase == CURRENT

       We also won't allow if entry is in the original DB and protected.
       ...  pst_data->bIsProtected0

       or if the first of the multiple selection was not in the Compare DB
       ... m_bFirstInCompare

       or if user changes column
       ... m_LCResults.GetColumn() == pNMLV->iSubItem

       No point if original is R-O or if already equal to original
       ... !m_bOriginalDBReadOnly || pst_data->indatabase == IDENTICAL
    */

    if (pst_data->indatabase == CURRENT || pst_data->indatabase == IDENTICAL ||
        pst_data->bIsProtected0 ||
        !m_bFirstInCompare || m_bOriginalDBReadOnly ||
        m_LCResults.GetColumn() != hti.iSubItem) {
      *pLResult = TRUE; // Deny change
      return;
    }
  } else {
    // Single-select or first selection
    m_bFirstInCompare = pst_data->indatabase != CURRENT;  // Not in Current DB only
  }
}

void CCompareResultsDlg::OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMHEADER *pNMHeaderCtrl  = (NMHEADER *)pNotifyStruct;

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
    DWORD_PTR dwItemData = m_LCResults.GetItemData(i);
    st_CompareData *pst_data = GetCompareData(dwItemData);
    ASSERT(pst_data != NULL);
    pst_data->listindex = i;
  }

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP   0x0400
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

  *pLResult = TRUE; // Say we have done all processing on return
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
  const int nSortColumn = self->m_iSortedColumn;
  st_CompareData *LHS_st_data = GetCompareData(lParam1, self);
  st_CompareData *RHS_st_data = GetCompareData(lParam2, self);
  const int LHS_Index = LHS_st_data->listindex;
  const int RHS_Index = RHS_st_data->listindex;

  CString LHS_ItemText = self->m_LCResults.GetItemText(LHS_Index, nSortColumn);
  CString RHS_ItemText = self->m_LCResults.GetItemText(RHS_Index, nSortColumn);

  int iResult = LHS_ItemText.CompareNoCase(RHS_ItemText);

  if (!self->m_bSortAscending) {
    iResult *= -1;
  }
  return iResult;
}

void CCompareResultsDlg::WriteReportData()
{
  CompareData::iterator cd_iter;
  CString buffer;

  if (!m_OnlyInCurrent.empty()) {
    buffer.Format(IDS_COMPAREENTRIES1, m_scFilename1);
    m_pRpt->WriteLine((LPCWSTR)buffer);
    for (cd_iter = m_OnlyInCurrent.begin(); cd_iter != m_OnlyInCurrent.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_pRpt->WriteLine((LPCWSTR)buffer);
    }
    m_pRpt->WriteLine();
  }

  if (!m_OnlyInComp.empty()) {
    buffer.Format(IDS_COMPAREENTRIES2, m_scFilename2);
    m_pRpt->WriteLine((LPCWSTR)buffer);
    for (cd_iter = m_OnlyInComp.begin(); cd_iter != m_OnlyInComp.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_pRpt->WriteLine((LPCWSTR)buffer);
    }
    m_pRpt->WriteLine();
  }

  if (!m_Conflicts.empty()) {
    buffer.Format(IDS_COMPAREBOTHDIFF);
    m_pRpt->WriteLine((LPCWSTR)buffer);

    const CString csx_password(MAKEINTRESOURCE(IDS_COMPPASSWORD));
    const CString csx_notes(MAKEINTRESOURCE(IDS_COMPNOTES));
    const CString csx_url(MAKEINTRESOURCE(IDS_COMPURL));
    const CString csx_autotype(MAKEINTRESOURCE(IDS_COMPAUTOTYPE));
    const CString csx_ctime(MAKEINTRESOURCE(IDS_COMPCTIME));
    const CString csx_pmtime(MAKEINTRESOURCE(IDS_COMPPMTIME));
    const CString csx_atime(MAKEINTRESOURCE(IDS_COMPATIME));
    const CString csx_xtime(MAKEINTRESOURCE(IDS_COMPXTIME));
    const CString csx_xtimeint(MAKEINTRESOURCE(IDS_COMPXTIME_INT));
    const CString csx_rmtime(MAKEINTRESOURCE(IDS_COMPRMTIME));
    const CString csx_pwhistory(MAKEINTRESOURCE(IDS_COMPPWHISTORY));
    const CString csx_policy(MAKEINTRESOURCE(IDS_COMPPWPOLICY));
    const CString csx_runcmd(MAKEINTRESOURCE(IDS_COMPRUNCOMMAND));
    const CString csx_dca(MAKEINTRESOURCE(IDS_COMPDCA));
    const CString csx_shiftdca(MAKEINTRESOURCE(IDS_COMPSHIFTDCA));
    const CString csx_email(MAKEINTRESOURCE(IDS_COMPEMAIL));
    const CString csx_protected(MAKEINTRESOURCE(IDS_COMPPROTECTED));
    const CString csx_symbols(MAKEINTRESOURCE(IDS_COMPSYMBOLS));
    const CString csx_policyname(MAKEINTRESOURCE(IDS_COMPPOLICYNAME));

    for (cd_iter = m_Conflicts.begin(); cd_iter != m_Conflicts.end();
         cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      buffer.Format(IDS_COMPARESTATS2, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_pRpt->WriteLine(std::wstring(buffer));
      buffer.Empty();

      // Non-time fields
      if (st_data.bsDiffs.test(CItemData::PASSWORD)) buffer += csx_password;
      if (st_data.bsDiffs.test(CItemData::NOTES)) buffer += csx_notes;
      if (st_data.bsDiffs.test(CItemData::URL)) buffer += csx_url;
      if (st_data.bsDiffs.test(CItemData::AUTOTYPE)) buffer += csx_autotype;
      if (st_data.bsDiffs.test(CItemData::PWHIST)) buffer += csx_pwhistory;
      if (st_data.bsDiffs.test(CItemData::POLICY)) buffer += csx_policy;
      if (st_data.bsDiffs.test(CItemData::RUNCMD)) buffer += csx_runcmd;
      if (st_data.bsDiffs.test(CItemData::DCA)) buffer += csx_dca;
      if (st_data.bsDiffs.test(CItemData::SHIFTDCA)) buffer += csx_shiftdca;
      if (st_data.bsDiffs.test(CItemData::EMAIL)) buffer += csx_email;
      if (st_data.bsDiffs.test(CItemData::PROTECTED)) buffer += csx_protected;
      if (st_data.bsDiffs.test(CItemData::SYMBOLS)) buffer += csx_symbols;
      if (st_data.bsDiffs.test(CItemData::POLICYNAME)) buffer += csx_policyname;

      // Time fields
      if (st_data.bsDiffs.test(CItemData::CTIME)) buffer += csx_ctime;
      if (st_data.bsDiffs.test(CItemData::PMTIME)) buffer += csx_pmtime;
      if (st_data.bsDiffs.test(CItemData::ATIME)) buffer += csx_atime;
      if (st_data.bsDiffs.test(CItemData::XTIME)) buffer += csx_xtime;
      if (st_data.bsDiffs.test(CItemData::RMTIME)) buffer += csx_rmtime;
      if (st_data.bsDiffs.test(CItemData::XTIME_INT)) buffer += csx_xtimeint;

      m_pRpt->WriteLine((LPCWSTR)buffer);
    }
    m_pRpt->WriteLine();
  }
}

void CCompareResultsDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (!IsWindow(m_LCResults.GetSafeHwnd()))
    return;

  // As main control is a CListCtrl, need to do this on the last column
  m_LCResults.SetColumnWidth(m_nCols - 1, LVSCW_AUTOSIZE_USEHEADER);

  // CPWResizeDialog only handles main control, bottom buttons
  // and status bar - we need to do the ones above the main control
  CWnd *pwndODBText = GetDlgItem(IDC_COMPAREORIGINALDB);
  CWnd *pwndCDBText = GetDlgItem(IDC_COMPARECOMPARISONDB);

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
                          ctrlRect.Height(), TRUE);

  GetDlgItem(IDC_COMPAREORIGINALDB)->SetWindowText(m_scFilename1);

  pwndCDBText->GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);
  pwndCDBText->MoveWindow(pt_top.x, pt_top.y,
                          cx - pt_top.x - 5,
                          ctrlRect.Height(), TRUE);

  GetDlgItem(IDC_COMPARECOMPARISONDB)->SetWindowText(m_scFilename2);
}

inline bool FieldsNotEqual(StringX a, StringX b, const bool bTreatWhiteSpaceasEmpty)
{
  if (bTreatWhiteSpaceasEmpty) { // m_treatwhitespaceasempty
    EmptyIfOnlyWhiteSpace(a);
    EmptyIfOnlyWhiteSpace(b);
  }
  return a != b;
}

bool CCompareResultsDlg::CompareEntries(st_CompareData *pst_data)
{
  CItemData::FieldBits bsConflicts;
  
  bsConflicts.reset();

  ItemListIter iter;
  iter = m_pcore0->Find(pst_data->uuid0);
  CItemData currentItem = iter->second;
  iter = m_pcore1->Find(pst_data->uuid1);
  CItemData compItem = iter->second;
  
  StringX sxCurrentPassword, sxComparisonPassword;

  if (currentItem.GetEntryType() == CItemData::ET_ALIAS ||
      currentItem.GetEntryType() == CItemData::ET_SHORTCUT) {
     CItemData *pci_base = m_pcore0->GetBaseEntry(&currentItem);
     sxCurrentPassword == pci_base->GetPassword();
  } else
    sxCurrentPassword == currentItem.GetPassword();

  if (compItem.GetEntryType() == CItemData::ET_ALIAS ||
      compItem.GetEntryType() == CItemData::ET_SHORTCUT) {
    CItemData *pci_base = m_pcore1->GetBaseEntry(&compItem);
    sxComparisonPassword == pci_base->GetPassword();
  } else
    sxComparisonPassword == compItem.GetPassword();

  if (m_bsFields.test(CItemData::PASSWORD) &&
      sxCurrentPassword != sxComparisonPassword)
    bsConflicts.flip(CItemData::PASSWORD);

  if (m_bsFields.test(CItemData::NOTES) &&
      FieldsNotEqual(currentItem.GetNotes(), compItem.GetNotes(), m_bTreatWhiteSpaceasEmpty))
    bsConflicts.flip(CItemData::NOTES);
  if (m_bsFields.test(CItemData::CTIME) &&
      currentItem.GetCTime() != compItem.GetCTime())
    bsConflicts.flip(CItemData::CTIME);
  if (m_bsFields.test(CItemData::PMTIME) &&
      currentItem.GetPMTime() != compItem.GetPMTime())
    bsConflicts.flip(CItemData::PMTIME);
  if (m_bsFields.test(CItemData::ATIME) &&
      currentItem.GetATime() != compItem.GetATime())
    bsConflicts.flip(CItemData::ATIME);
  if (m_bsFields.test(CItemData::XTIME) &&
      currentItem.GetXTime() != compItem.GetXTime())
    bsConflicts.flip(CItemData::XTIME);
  if (m_bsFields.test(CItemData::RMTIME) &&
      currentItem.GetRMTime() != compItem.GetRMTime())
    bsConflicts.flip(CItemData::RMTIME);
  if (m_bsFields.test(CItemData::XTIME_INT)) {
    int current_xint, comp_xint;
    currentItem.GetXTimeInt(current_xint);
    compItem.GetXTimeInt(comp_xint);
    if (current_xint != comp_xint)
      bsConflicts.flip(CItemData::XTIME_INT);
    }
  if (m_bsFields.test(CItemData::URL) &&
      FieldsNotEqual(currentItem.GetURL(), compItem.GetURL(),
                     m_bTreatWhiteSpaceasEmpty))
    bsConflicts.flip(CItemData::URL);
  if (m_bsFields.test(CItemData::AUTOTYPE) &&
      FieldsNotEqual(currentItem.GetAutoType(), compItem.GetAutoType(),
                     m_bTreatWhiteSpaceasEmpty))
    bsConflicts.flip(CItemData::AUTOTYPE);
  if (m_bsFields.test(CItemData::PWHIST) &&
    currentItem.GetPWHistory() != compItem.GetPWHistory())
    bsConflicts.flip(CItemData::PWHIST);
  if (m_bsFields.test(CItemData::POLICY) &&
      currentItem.GetPWPolicy() != compItem.GetPWPolicy())
    bsConflicts.flip(CItemData::POLICY);
  if (m_bsFields.test(CItemData::POLICYNAME) &&
      currentItem.GetPolicyName() != compItem.GetPolicyName())
    bsConflicts.flip(CItemData::POLICYNAME);
  if (m_bsFields.test(CItemData::RUNCMD) &&
      currentItem.GetRunCommand() != compItem.GetRunCommand())
    bsConflicts.flip(CItemData::RUNCMD);
  if (m_bsFields.test(CItemData::DCA) &&
      currentItem.GetDCA() != compItem.GetDCA())
    bsConflicts.flip(CItemData::DCA);
  if (m_bsFields.test(CItemData::SHIFTDCA) &&
      currentItem.GetShiftDCA() != compItem.GetShiftDCA())
    bsConflicts.flip(CItemData::SHIFTDCA);
  if (m_bsFields.test(CItemData::EMAIL) &&
      currentItem.GetEmail() != compItem.GetEmail())
    bsConflicts.flip(CItemData::EMAIL);
  if (m_bsFields.test(CItemData::PROTECTED) &&
      currentItem.GetProtected() != compItem.GetProtected())
    bsConflicts.flip(CItemData::PROTECTED);
  if (m_bsFields.test(CItemData::SYMBOLS) &&
      currentItem.GetSymbols() != compItem.GetSymbols())
    bsConflicts.flip(CItemData::SYMBOLS);

  return bsConflicts.none();
}

// Compare CListCtrl

CCPListCtrl::CCPListCtrl()
  : m_row(-1), m_column(-1)
{
}

CCPListCtrl::~CCPListCtrl()
{
}

BEGIN_MESSAGE_MAP(CCPListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CCPListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCPListCtrl::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_LBUTTONDOWN) {
    // Get cell position for CustomDraw
    CPoint pt(pMsg->pt);
    ScreenToClient(&pt);
    LVHITTESTINFO htinfo = {0};
    htinfo.pt = pt;
    int index = SubItemHitTest(&htinfo);
    // Ignore any clicks not on an item or not on cloumn 0 or 1
    if (index != -1 &&
        (htinfo.iSubItem == 0 || htinfo.iSubItem == 1) &&
        !(htinfo.flags & (LVHT_NOWHERE |
                          LVHT_ABOVE   | LVHT_BELOW |
                          LVHT_TORIGHT | LVHT_TOLEFT))) {
      if ((GetKeyState(VK_CONTROL) & 0x8000) && m_column != htinfo.iSubItem) {
        //  Multi-select - can't change column
        return TRUE;
      }
      m_row = htinfo.iItem;
      m_column = htinfo.iSubItem;
    }
  }
  return CListCtrl::PreTranslateMessage(pMsg);
} 

bool CCPListCtrl::IsSelected(const int iRow)
{
  POSITION pos = GetFirstSelectedItemPosition();

  while (pos) {
    if (GetNextSelectedItem(pos) == iRow)
      return true;
  }

  return false;
}

void CCPListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = (NMLVCUSTOMDRAW *)pNotifyStruct;

  *pLResult = CDRF_DODEFAULT;

  static COLORREF crWindowText, cfNormalTextBkgrd, crSelectedText, crSelectedBkgrd;
  bool bIsSelected = IsSelected(pLVCD->nmcd.dwItemSpec);
  
  switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      crWindowText = GetTextColor();
      cfNormalTextBkgrd = GetTextBkColor();
      crSelectedText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
      crSelectedBkgrd = ::GetSysColor(COLOR_HIGHLIGHT);
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      if ((pLVCD->nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED) {
        pLVCD->clrText = crWindowText;
        pLVCD->clrTextBk = cfNormalTextBkgrd;
        pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
        *pLResult |= CDRF_NEWFONT;
      }
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      if (bIsSelected && m_column != -1 && pLVCD->iSubItem == m_column) {
        pLVCD->clrText = crSelectedText;
        pLVCD->clrTextBk = crSelectedBkgrd;
        pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
      } else {
        pLVCD->clrText = crWindowText;
        pLVCD->clrTextBk = cfNormalTextBkgrd;
      }
      *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
      break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_POSTPAINT:
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    case CDDS_ITEMPOSTPAINT:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    */
    default:
      break;
  }
}
