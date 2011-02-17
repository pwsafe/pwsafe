/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExpPWListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExpPWListDlg.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"
#include "SecString.h"
#include "PWTreeCtrl.h"

#include "core/Util.h"
#include "core/PWSprefs.h"
#include "core/ItemData.h"

#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

// CExpPWListDlg dialog
CExpPWListDlg::CExpPWListDlg(CWnd* pParent,
                             ExpiredList &expPWList,
                             const CString& a_filespec)
  : CPWDialog(CExpPWListDlg::IDD, pParent), m_expPWList(expPWList)
{
  m_pDbx = reinterpret_cast<DboxMain *>(pParent);
  m_message = a_filespec; // Path Ellipsis=true, no length woes
  m_iSortedColumn = 4;
  m_bSortAscending = FALSE;
  m_idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
}

CExpPWListDlg::~CExpPWListDlg()
{
}

void CExpPWListDlg::OnDestroy()
{
  m_pImageList->DeleteImageList();
  delete m_pImageList;
}

void CExpPWListDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EXPIRED_PASSWORD_LIST, m_expPWListCtrl);
  DDX_Text(pDX, IDC_MESSAGE, m_message);
}

BEGIN_MESSAGE_MAP(CExpPWListDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(ID_HELP, OnIconHelp)
  ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
  ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
  ON_NOTIFY(NM_DBLCLK, IDC_EXPIRED_PASSWORD_LIST, OnItemDoubleClick)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

// CExpPWListDlg message handlers

BOOL CExpPWListDlg::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

BOOL CExpPWListDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  DWORD dwStyleEx = m_expPWListCtrl.GetExtendedStyle();
  dwStyleEx |= LVS_EX_FULLROWSELECT;
  m_expPWListCtrl.SetExtendedStyle(dwStyleEx);

  //m_expPWListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

  CString cs_text;
  m_expPWListCtrl.InsertColumn(0, L"");
  cs_text.LoadString(IDS_GROUP);
  m_expPWListCtrl.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_TITLE);
  m_expPWListCtrl.InsertColumn(2, cs_text);
  cs_text.LoadString(IDS_USERNAME);
  m_expPWListCtrl.InsertColumn(3, cs_text);
  cs_text.LoadString(IDS_PASSWORDEXPIRYDATE);
  m_expPWListCtrl.InsertColumn(4, cs_text);

  CBitmap bitmap;
  BITMAP bm;

  // Change all pixels in this 'grey' to transparent
  const COLORREF crTransparent = RGB(192, 192, 192);

  bitmap.LoadBitmap(IDB_NODE);
  bitmap.GetBitmap(&bm);

  m_pImageList = new CImageList();
  // Number (12) corresponds to number in CPWTreeCtrl public enum
  BOOL status = m_pImageList->Create(bm.bmWidth, bm.bmHeight,
                                     ILC_MASK | ILC_COLOR, 12, 0);
  ASSERT(status != 0);

  // Order of LoadBitmap() calls matches CPWTreeCtrl public enum
  // Also now used by CListCtrl!
  //bitmap.LoadBitmap(IDB_NODE); - already loaded above to get width
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  UINT bitmapResIDs[] = {
    IDB_NORMAL, IDB_NORMAL_WARNEXPIRED, IDB_NORMAL_EXPIRED,
    IDB_ABASE, IDB_ABASE_WARNEXPIRED, IDB_ABASE_EXPIRED,
    IDB_ALIAS,
    IDB_SBASE, IDB_SBASE_WARNEXPIRED, IDB_SBASE_EXPIRED,
    IDB_SHORTCUT,
  };

  for (int i = 0; i < sizeof(bitmapResIDs) / sizeof(bitmapResIDs[0]); i++) {
    bitmap.LoadBitmap(bitmapResIDs[i]);
    m_pImageList->Add(&bitmap, crTransparent);
    bitmap.DeleteObject();
  }

  m_expPWListCtrl.SetImageList(m_pImageList, LVSIL_SMALL);
  m_expPWListCtrl.SetImageList(m_pImageList, LVSIL_NORMAL);

  int nPos = 0;
  ExpiredList::const_iterator itempos;

  for (itempos = m_expPWList.begin();
       itempos != m_expPWList.end();
       itempos++) {
    const ExpPWEntry exppwentry = *itempos;

    // To get the correct bitmap image....
    int image = GetEntryImage(exppwentry);

    // Add to ListCtrl
    nPos = m_expPWListCtrl.InsertItem(++nPos, NULL, image);
    m_expPWListCtrl.SetItemText(nPos, 1, exppwentry.group.c_str());
    m_expPWListCtrl.SetItemText(nPos, 2, exppwentry.title.c_str());
    m_expPWListCtrl.SetItemText(nPos, 3, exppwentry.user.c_str());
    m_expPWListCtrl.SetItemText(nPos, 4, exppwentry.expirylocdate.c_str());

    // original nPos == index in vector: need to know even if user sorts rows
    m_expPWListCtrl.SetItemData(nPos, static_cast<DWORD>(nPos));
  }

  // Stop flickers
  m_expPWListCtrl.SetRedraw(FALSE);

  // Sort by expiry date - soonest first
  m_expPWListCtrl.SortItems(ExpPWCompareFunc, (LPARAM)this);

  // Set column widths
  for (int i = 0; i < 5; i++) {
    m_expPWListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_expPWListCtrl.GetColumnWidth(i);
    m_expPWListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_expPWListCtrl.GetColumnWidth(i);
    m_expPWListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }

  // Redraw
  m_expPWListCtrl.SetRedraw(TRUE);

  return TRUE;
}

void CExpPWListDlg::OnOK()
{
  CPWDialog::OnOK();
}

void CExpPWListDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_expPWListCtrl.SortItems(ExpPWCompareFunc, (LPARAM)this);

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
    m_expPWListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_expPWListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pResult = 0;
}

int CALLBACK CExpPWListDlg::ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                             LPARAM closure)
{
  CExpPWListDlg *self = (CExpPWListDlg*)closure;
  int nSortColumn = self->m_iSortedColumn;
  const ExpPWEntry pLHS = self->m_expPWList[lParam1];
  const ExpPWEntry pRHS = self->m_expPWList[lParam2];
  CSecString group1, title1, username1;
  CSecString group2, title2, username2;
  int et1, et2;
  time_t t1, t2;

  int iResult;
  switch(nSortColumn) {
    case 0:
      et1 = (int)pLHS.et;
      et2 = (int)pRHS.et;
      iResult = (et1 < et2) ? -1 : 1;
      break;
    case 1:
      group1 = pLHS.group;
      group2 = pRHS.group;
      iResult = ((CString)group1).CompareNoCase(group2);
      break;
    case 2:
      title1 = pLHS.title;
      title2 = pRHS.title;
      iResult = ((CString)title1).CompareNoCase(title2);
      break;
    case 3:
      username1 = pLHS.user;
      username2 = pRHS.user;
      iResult = ((CString)username1).CompareNoCase(username2);
      break;
    case 4:
      t1 = pLHS.expirytttXTime;
      t2 = pRHS.expirytttXTime;
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    default:
      iResult = 0; // should never happen - just keep compiler happy
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending)
    iResult *= -1;

  return iResult;
}

void CExpPWListDlg::OnItemDoubleClick(NMHDR* /* pNMHDR */, LRESULT *pResult)
{
  *pResult = 0;

  int irow = m_expPWListCtrl.GetNextItem(-1, LVNI_SELECTED);
  if (irow == -1)
    return;

  ASSERT(irow >= 0 && irow < (int)m_expPWList.size());
  size_t iv = (size_t)m_expPWListCtrl.GetItemData(irow);
  ExpPWEntry *pEE = &m_expPWList[iv];

  LRESULT lres = ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                                   PWS_MSG_EXPIRED_PASSWORD_EDIT,
                                   (WPARAM)pEE, 0);
  if (lres == TRUE) {
    // Update row
    // Update image and unselect
    LVITEM lv= {0};
    lv.iItem = irow;
    lv.mask = LVIF_IMAGE | LVIF_STATE;
    lv.state = 0;
    lv.stateMask = LVIS_SELECTED;
    lv.iImage = GetEntryImage(*pEE);
    m_expPWListCtrl.SetItem(&lv);

    // User may have edited the group/title/user fields!!!
    m_expPWListCtrl.SetItemText(irow, 1, pEE->group.c_str());
    m_expPWListCtrl.SetItemText(irow, 2, pEE->title.c_str());
    m_expPWListCtrl.SetItemText(irow, 3, pEE->user.c_str());

    // User may have changed the expiry date
    m_expPWListCtrl.SetItemText(irow, 4, pEE->expirytttXTime != (time_t)0 ?
               pEE->expirylocdate.c_str() : L"");

    // Refresh it
    m_expPWListCtrl.Update(irow);

    // Re-sort
    m_expPWListCtrl.SortItems(ExpPWCompareFunc, (LPARAM)this);
  }
}

int CExpPWListDlg::GetEntryImage(const ExpPWEntry &ee)
{
  if (ee.et == CItemData::ET_ALIAS)
    return CPWTreeCtrl::ALIAS;

  if (ee.et == CItemData::ET_SHORTCUT)
    return CPWTreeCtrl::SHORTCUT;

  int nImage;
  switch (ee.et) {
    case CItemData::ET_NORMAL:
      nImage = CPWTreeCtrl::NORMAL;
      break;
    case CItemData::ET_ALIASBASE:
      nImage = CPWTreeCtrl::ALIASBASE;
      break;
    case CItemData::ET_SHORTCUTBASE:
      nImage = CPWTreeCtrl::SHORTCUTBASE;
      break;
    default:
      nImage = CPWTreeCtrl::NORMAL;
      break;
  }

  // Entry has been updated - need to check further as it might be OK now
  if (ee.expirytttXTime != 0) {
    time_t now, warnexptime((time_t)0);
    time(&now);
    struct tm st;
#if (_MSC_VER >= 1400)
    errno_t err;
    err = localtime_s(&st, &now);  // secure version
    ASSERT(err == 0);
#else
    st = *localtime(&now);
    ASSERT(st != NULL); // null means invalid time
#endif
    st.tm_mday += m_idays;
    warnexptime = mktime(&st);

    if (warnexptime == (time_t)-1)
      warnexptime = (time_t)0;

    if (ee.expirytttXTime <= now) {
      nImage += 2;  // Expired
    } else if (ee.expirytttXTime < warnexptime) {
      nImage += 1;  // Warn nearly expired
    }
  }

  return nImage;
}

void CExpPWListDlg::OnIconHelp()
{
  CString cs_HelpTopic = app.GetHelpFileName() + L"::/html/images.html";
  ::HtmlHelp(this->GetSafeHwnd(), (LPCWSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
}
