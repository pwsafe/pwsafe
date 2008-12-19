/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWHistDlg.cpp : implementation file
//

#include "stdafx.h"

#include "PasswordSafe.h"
#include "resource.h"
#include "resource3.h"  // String resources
#include "DboxMain.h"
#include "PWHistDlg.h"
#include "corelib/ItemData.h"
#include "corelib/PWSprefs.h"


// CPWHistDlg dialog

IMPLEMENT_DYNAMIC(CPWHistDlg, CDialog)
CPWHistDlg::CPWHistDlg(CWnd* pParent, bool IsReadOnly,
                       CSecString &HistStr, PWHistList &PWHistList,
                       size_t NumPWHistory, size_t &MaxPWHistory,
                       BOOL &SavePWHistory)
  : CPWDialog(CPWHistDlg::IDD, pParent),
  m_PWH_IsReadOnly(IsReadOnly),
  m_HistStr(HistStr), m_PWHistList(PWHistList),
  m_NumPWHistory(NumPWHistory), m_MaxPWHistory(MaxPWHistory),
  m_SavePWHistory(SavePWHistory),
  m_ClearPWHistory(false),
  m_iSortedColumn(-1), m_bSortAscending(true)
{
  m_oldMaxPWHistory = m_MaxPWHistory;
}

CPWHistDlg::~CPWHistDlg()
{
}

void CPWHistDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PWHISTORY_LIST, m_PWHistListCtrl);
  DDX_Check(pDX, IDC_SAVE_PWHIST, m_SavePWHistory);
  DDX_Text(pDX, IDC_MAXPWHISTORY, m_MaxPWHistory);
  DDV_MinMaxInt(pDX, int(m_MaxPWHistory), 1, 255);
}

BEGIN_MESSAGE_MAP(CPWHistDlg, CPWDialog)
  ON_BN_CLICKED(IDC_CLEAR_PWHIST, OnBnClickedClearPWHist)
  ON_BN_CLICKED(IDC_SAVE_PWHIST, OnCheckedSavePasswordHistory)
  ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
  ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
  ON_NOTIFY(NM_CLICK, IDC_PWHISTORY_LIST, OnHistListClick)
  ON_BN_CLICKED(IDC_PWH_COPY_ALL, OnBnClickedPwhCopyAll)
END_MESSAGE_MAP()

BOOL CPWHistDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory ? TRUE : FALSE);

  BOOL bpwh_count = m_PWHistList.empty() ? FALSE : TRUE;
  GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(bpwh_count);
  GetDlgItem(IDC_PWHISTORY_LIST)->EnableWindow(bpwh_count);

  if (m_PWH_IsReadOnly) {
    GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);  // overrides count
  }

  m_PWHistListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  CString cs_text;
  cs_text.LoadString(IDS_SETDATETIME);
  m_PWHistListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_PASSWORD);
  m_PWHistListCtrl.InsertColumn(1, cs_text);

  PWHistList::iterator iter;
  DWORD nIdx;
  for (iter = m_PWHistList.begin(), nIdx = 0;
       iter != m_PWHistList.end(); iter++, nIdx++) {
    int nPos = 0;
    const PWHistEntry pwhentry = *iter;
    if (pwhentry.changedate != _T("1970-01-01 00:00:00"))
      nPos = m_PWHistListCtrl.InsertItem(nPos, pwhentry.changedate.c_str());
    else {
      cs_text.LoadString(IDS_UNKNOWN);
      cs_text.Trim();
      nPos = m_PWHistListCtrl.InsertItem(nPos, cs_text);
    }
    m_PWHistListCtrl.SetItemText(nPos, 1, pwhentry.password.c_str());
    m_PWHistListCtrl.SetItemData(nPos, nIdx);
  }

  m_PWHistListCtrl.SetRedraw(FALSE);
  for (int i = 0; i < 2; i++) {
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }
  m_PWHistListCtrl.SetRedraw(TRUE);

  TCHAR buffer[10];
#if _MSC_VER >= 1400
  _stprintf_s(buffer, 10, _T("%d"), m_NumPWHistory);
#else
  _stprintf(buffer, _T("%d"), m_NumPWHistory);
#endif

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  if (m_MaxPWHistory == 0)
    m_MaxPWHistory = PWSprefs::GetInstance()->
                     GetPref(PWSprefs::NumPWHistoryDefault);

  pspin->SetBuddy(GetDlgItem(IDC_MAXPWHISTORY));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos((int)m_MaxPWHistory);

  return TRUE;
}

// CPWHistDlg message handlers
void CPWHistDlg::OnBnClickedClearPWHist()
{
  m_ClearPWHistory = true;
  m_PWHistListCtrl.DeleteAllItems();
}

void CPWHistDlg::OnOK() 
{
  if (UpdateData(TRUE) == FALSE)
    return;

  /* Handle history header.
  * Header is in the form fmmnn, where:
  * f = {0,1} if password history is on/off
  * mm = 2 digits max size of history list
  * nn = 2 digits current size of history list
  *
  * Special case: history empty and password history off - do nothing
  */

  if (m_ClearPWHistory == TRUE) {
    m_PWHistList.erase(m_PWHistList.begin(), m_PWHistList.end());
    m_HistStr = m_HistStr.Left(5);
  }

  if (!(m_HistStr.IsEmpty() && m_SavePWHistory == FALSE)) {
    TCHAR buffer[6];
#if _MSC_VER >= 1400
    _stprintf_s
#else
    _stprintf
#endif
      (buffer,
#if _MSC_VER >= 1400
      6,
#endif
      _T("%1x%02x%02x"),
      (m_SavePWHistory == FALSE) ? 0 : 1,
      m_MaxPWHistory,
      m_PWHistList.size()
      );
    if (m_HistStr.GetLength() >= 5) {
      for (int i = 0; i < 5; i++) m_HistStr.SetAt(i, buffer[i]);
    } else {
      m_HistStr = buffer;
    }
  }
  CPWDialog::OnOK();
}

void CPWHistDlg::OnCheckedSavePasswordHistory()
{
  m_SavePWHistory = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck();
  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory ? TRUE : FALSE);
}

void CPWHistDlg::OnHistListClick(NMHDR* pNMHDR, LRESULT*)
{
  LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pNMHDR;
  ASSERT(lpnmitem != NULL);
  int item = lpnmitem->iItem;
  if (item == -1)
    return;

  size_t itempos = size_t(m_PWHistListCtrl.GetItemData(item));
  const PWHistEntry pwhentry = m_PWHistList[itempos];
  DboxMain *pDbx = static_cast<DboxMain *>(GetParent()->GetParent());
  pDbx->SetClipboardData(pwhentry.password);
}

void CPWHistDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

  if(phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if(phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = true;

    m_iSortedColumn = phdn->iItem;
    m_PWHistListCtrl.SortItems(PWHistCompareFunc, (LPARAM)this);

    // Note: WINVER defines the minimum system level for which this is program compiled and
    // NOT the level of system it is running on!
    // In this case, these values are defined in Windows XP and later and supported
    // by V6 of comctl32.dll (supplied with Windows XP) and later.
    // They should be ignored by earlier levels of this dll or .....
    //     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_PWHistListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= (m_bSortAscending ? HDF_SORTUP : HDF_SORTDOWN);
    m_PWHistListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pResult = 0;
}

int CALLBACK CPWHistDlg::PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                           LPARAM closure)
{
  CPWHistDlg *self = (CPWHistDlg*)closure;
  int nSortColumn = self->m_iSortedColumn;
  size_t Lpos = (size_t)lParam1;
  size_t Rpos = (size_t)lParam2;
  const PWHistEntry pLHS = self->m_PWHistList[Lpos];
  const PWHistEntry pRHS = self->m_PWHistList[Rpos];
  CSecString password1, changedate1;
  CSecString password2, changedate2;
  time_t t1, t2;

  int iResult;
  switch(nSortColumn) {
    case 0:
      t1 = pLHS.changetttdate;
      t2 = pRHS.changetttdate;
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case 1:
      password1 = pLHS.password;
      password2 = pRHS.password;
      iResult = ((CString)password1).Compare(password2);
      break;
    default:
      iResult = 0; // should never happen - just keep compiler happy
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending)
    iResult *= -1;

  return iResult;
}

void CPWHistDlg::OnBnClickedPwhCopyAll()
{
  CSecString HistStr;
  PWHistList::iterator iter;

  for (iter = m_PWHistList.begin(); iter != m_PWHistList.end(); iter++) {
    const PWHistEntry &ent = *iter;
    HistStr += ent.changedate;
    HistStr += _T("\t");
    HistStr += ent.password;
    HistStr += _T("\r\n");
  }

  DboxMain *pDbx = static_cast<DboxMain *>(GetParent()->GetParent());
  pDbx->SetClipboardData(HistStr);
}
