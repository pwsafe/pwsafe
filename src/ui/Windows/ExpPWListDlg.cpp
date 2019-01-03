/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "SecString.h"
#include "PWTreeCtrl.h"

#include "core/Util.h"
#include "core/PWSprefs.h"
#include "core/ItemData.h"

#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

// CExpPWListDlg dialog
CExpPWListDlg::CExpPWListDlg(CWnd *pParent,
  ExpiredList &expPWList, const CString &a_filespec,
  const CString &csProtect, const CString &csAttachment)
  : CPWDialog(CExpPWListDlg::IDD, pParent), m_expPWList(expPWList),
  m_csProtect(csProtect), m_csAttachment(csAttachment)
{
  m_Database = a_filespec; // Path Ellipsis=true, no length woes
  m_iSortedColumn = 4;
  m_bSortAscending = FALSE;
  m_idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);

  // Get all entries using core vector
  for (size_t i = 0; i < m_expPWList.size(); i++) {
    st_ExpLocalListEntry elle;
 
    // Find entry
    ItemListIter iter = GetMainDlg()->Find(m_expPWList[i].uuid);
    ASSERT(iter != GetMainDlg()->End());
    if (iter == GetMainDlg()->End())
      continue; // should not happen, but better than crashing as in Bug 1148
    const CItemData &ci = iter->second;
    
    // Get group/title/user values
    elle.sx_group = ci.GetGroup();
    elle.sx_title = ci.GetTitle();
    elle.sx_user  = ci.GetUser();
    elle.bIsProtected = ci.IsProtected();
    elle.bHasAttachment = ci.HasAttRef();

    // Get XTime and string versions
    elle.expirytttXTime = m_expPWList[i].expirytttXTime;
    elle.sx_expirylocdate = PWSUtil::ConvertToDateTimeString(elle.expirytttXTime, PWSUtil::TMC_LOCALE);
    
    // Get entrytype (used for selecting image)
    elle.et = ci.GetEntryType();
    
    elle.uuid = m_expPWList[i].uuid;
    
    // Save in local vector
    m_vExpLocalListEntries.push_back(elle);
  }
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
  DDX_Text(pDX, IDC_SELECTED_DATABASE, m_Database);
}

BEGIN_MESSAGE_MAP(CExpPWListDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(ID_HELP, OnIconHelp)
  ON_NOTIFY(HDN_ITEMCLICK, 0, OnHeaderClicked)
  ON_NOTIFY(NM_DBLCLK, IDC_EXPIRED_PASSWORD_LIST, OnItemDoubleClick)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

// CExpPWListDlg message handlers

BOOL CExpPWListDlg::PreTranslateMessage(MSG *pMsg)
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

  bitmap.LoadBitmap(IDB_GROUP);
  bitmap.GetBitmap(&bm);

  m_pImageList = new CImageList();
  // Number (12) corresponds to number in CPWTreeCtrl public enum
  VERIFY(m_pImageList->Create(bm.bmWidth, bm.bmHeight,
                              ILC_MASK | ILC_COLORDDB,
                              CPWTreeCtrl::NUM_IMAGES, 0));
  // Order of LoadBitmap() calls matches CPWTreeCtrl public enum
  // Also now used by CListCtrl!
  //bitmap.LoadBitmap(IDB_GROUP); - already loaded above to get width
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
  std::vector<st_ExpLocalListEntry>::const_iterator itempos;

  for (itempos = m_vExpLocalListEntries.begin();
       itempos != m_vExpLocalListEntries.end();
       itempos++) {
    const st_ExpLocalListEntry &elle = *itempos;

    // To get the correct bitmap image....
    int image = GetEntryImage(elle);

    // Add to ListCtrl
    cs_text.Empty();
    if (elle.bIsProtected)
      cs_text += m_csProtect;

    if (elle.bHasAttachment)
      cs_text += m_csAttachment;

    nPos = m_expPWListCtrl.InsertItem(++nPos, cs_text, image);
    m_expPWListCtrl.SetItemText(nPos, 1, elle.sx_group.c_str());
    m_expPWListCtrl.SetItemText(nPos, 2, elle.sx_title.c_str());
    m_expPWListCtrl.SetItemText(nPos, 3, elle.sx_user.c_str());
    m_expPWListCtrl.SetItemText(nPos, 4, elle.sx_expirylocdate.c_str());

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
    m_expPWListCtrl.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }

  // Redraw
  m_expPWListCtrl.SetRedraw(TRUE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CExpPWListDlg::OnOK()
{
  CPWDialog::OnOK();
}

void CExpPWListDlg::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_expPWListCtrl.SortItems(ExpPWCompareFunc, (LPARAM)this);

    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_expPWListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_expPWListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CExpPWListDlg::ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                             LPARAM closure)
{
  CExpPWListDlg *self = (CExpPWListDlg*)closure;
  int nSortColumn = self->m_iSortedColumn;
  const st_ExpLocalListEntry pLHS = self->m_vExpLocalListEntries[lParam1];
  const st_ExpLocalListEntry pRHS = self->m_vExpLocalListEntries[lParam2];
  CSecString group1, title1, username1;
  CSecString group2, title2, username2;
  int et1, et2;
  time_t t1, t2;

  int iResult(0);
  switch(nSortColumn) {
    case 0:
      et1 = (int)pLHS.et;
      et2 = (int)pRHS.et;
      if (et1 != et2)
        iResult = (et1 < et2) ? -1 : 1;
      break;
    case 1:
      group1 = pLHS.sx_group;
      group2 = pRHS.sx_group;
      iResult = ((CString)group1).CompareNoCase(group2);
      break;
    case 2:
      title1 = pLHS.sx_title;
      title2 = pRHS.sx_title;
      iResult = ((CString)title1).CompareNoCase(title2);
      break;
    case 3:
      username1 = pLHS.sx_user;
      username2 = pRHS.sx_user;
      iResult = ((CString)username1).CompareNoCase(username2);
      break;
    case 4:
      t1 = pLHS.expirytttXTime;
      t2 = pRHS.expirytttXTime;
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    default:
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}

void CExpPWListDlg::OnItemDoubleClick(NMHDR *, LRESULT *pLResult)
{
  *pLResult = 0;

  int irow = m_expPWListCtrl.GetNextItem(-1, LVNI_SELECTED);
  if (irow == -1)
    return;

  ASSERT(irow >= 0 && irow < (int)m_expPWList.size());
  size_t iv = (size_t)m_expPWListCtrl.GetItemData(irow);
  st_ExpLocalListEntry *pELLE = &m_vExpLocalListEntries[iv];

  ItemListIter iter = GetMainDlg()->Find(pELLE->uuid);
  ASSERT(iter != GetMainDlg()->End());
  if (iter == GetMainDlg()->End() || iter->second.IsProtected()) {
    return;
  }

  LRESULT lres = ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                                   PWS_MSG_EXPIRED_PASSWORD_EDIT,
                                   (WPARAM)pELLE, 0);

  if (lres == TRUE) {
    // Update row
    m_expPWListCtrl.SetItemText(irow, 1, pELLE->sx_group.c_str());
    m_expPWListCtrl.SetItemText(irow, 2, pELLE->sx_title.c_str());
    m_expPWListCtrl.SetItemText(irow, 3, pELLE->sx_user.c_str());

    // User may have changed the expiry date
    m_expPWListCtrl.SetItemText(irow, 4, pELLE->expirytttXTime != (time_t)0 ?
               pELLE->sx_expirylocdate.c_str() : L"");

    // Update image and unselect
    LVITEM lv= {0};
    lv.iItem = irow;
    lv.mask = LVIF_IMAGE | LVIF_STATE;
    lv.state = 0;
    lv.stateMask = LVIS_SELECTED;
    lv.iImage = GetEntryImage(*pELLE);
    m_expPWListCtrl.SetItem(&lv);

    // Refresh it
    m_expPWListCtrl.Update(irow);

    // Re-sort
    m_expPWListCtrl.SortItems(ExpPWCompareFunc, (LPARAM)this);
  }
}

int CExpPWListDlg::GetEntryImage(const st_ExpLocalListEntry &elle)
{
  if (elle.et == CItemData::ET_ALIAS)
    return CPWTreeCtrl::ALIAS;

  if (elle.et == CItemData::ET_SHORTCUT)
    return CPWTreeCtrl::SHORTCUT;

  int nImage;
  switch (elle.et) {
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
  if (elle.expirytttXTime != 0) {
    time_t now, warnexptime((time_t)0);
    time(&now);
    struct tm st;
    errno_t err;
    err = localtime_s(&st, &now);  // secure version
    ASSERT(err == 0);
    st.tm_mday += m_idays;
    warnexptime = mktime(&st);

    if (warnexptime == (time_t)-1)
      warnexptime = (time_t)0;

    if (elle.expirytttXTime <= now) {
      nImage += 2;  // Expired
    } else if (elle.expirytttXTime < warnexptime) {
      nImage += 1;  // Warn nearly expired
    }
  }

  return nImage;
}

void CExpPWListDlg::OnIconHelp()
{
  ShowHelp(L"::/html/images.html");
}
