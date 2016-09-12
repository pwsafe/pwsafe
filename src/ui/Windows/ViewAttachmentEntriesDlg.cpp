/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewAttachmentEntriesDlg.cpp : implementation file
//

#include "stdafx.h"

#include "ViewAttachmentEntriesDlg.h"
#include "PWTreeCtrl.h"

#include "afxdialogex.h"

// CViewAttachmentEntriesDlg dialog

IMPLEMENT_DYNAMIC(CViewAttachmentEntriesDlg, CPWDialog)

CViewAttachmentEntriesDlg::CViewAttachmentEntriesDlg(CWnd *pParent, std::vector<st_gtui> *pvst_gtui)
	: CPWDialog(IDD_VIEWATTACHMENTENTRIES, pParent), m_pvst_gtui(pvst_gtui)
{
  m_iSortedColumn = 1;
  m_bSortAscending = FALSE;
}

CViewAttachmentEntriesDlg::~CViewAttachmentEntriesDlg()
{
}

void CViewAttachmentEntriesDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ENTRYLIST, m_lcEntryList);
}

BEGIN_MESSAGE_MAP(CViewAttachmentEntriesDlg, CPWDialog)
  ON_NOTIFY(HDN_ITEMCLICK, 0, OnHeaderClicked)
END_MESSAGE_MAP()

// CViewAttachmentEntriesDlg message handlers

BOOL CViewAttachmentEntriesDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // No data - don't display
  if (m_pvst_gtui == NULL) {
    EndDialog(IDOK);
    return TRUE;
  }

  DWORD dwStyleEx = m_lcEntryList.GetExtendedStyle();
  dwStyleEx |= LVS_EX_FULLROWSELECT;
  m_lcEntryList.SetExtendedStyle(dwStyleEx);

  CString cs_text;
  m_lcEntryList.InsertColumn(0, L"");
  cs_text.LoadString(IDS_GROUP);
  m_lcEntryList.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_TITLE);
  m_lcEntryList.InsertColumn(2, cs_text);
  cs_text.LoadString(IDS_USERNAME);
  m_lcEntryList.InsertColumn(3, cs_text);

  CBitmap bitmap;
  BITMAP bm;

  // Change all pixels in this 'grey' to transparent
  const COLORREF crTransparent = RGB(192, 192, 192);

  bitmap.LoadBitmap(IDB_GROUP);
  bitmap.GetBitmap(&bm);

  m_pImageList = new CImageList();
  // Number (12) corresponds to number in CPWTreeCtrl public enum
  BOOL status = m_pImageList->Create(bm.bmWidth, bm.bmHeight,
    ILC_MASK | ILC_COLORDDB,
    CPWTreeCtrl::NUM_IMAGES, 0);
  ASSERT(status != 0);

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

  m_lcEntryList.SetImageList(m_pImageList, LVSIL_SMALL);
  m_lcEntryList.SetImageList(m_pImageList, LVSIL_NORMAL);

  int nPos = 0;
  std::vector<st_gtui>::const_iterator itempos;

  for (itempos = m_pvst_gtui->begin(); itempos != m_pvst_gtui->end(); itempos++) {
    const st_gtui &stgtui = *itempos;

    // Add to ListCtrl
    nPos = m_lcEntryList.InsertItem(++nPos, NULL, stgtui.image);
    m_lcEntryList.SetItemText(nPos, 1, stgtui.sxGroup.c_str());
    m_lcEntryList.SetItemText(nPos, 2, stgtui.sxTitle.c_str());
    m_lcEntryList.SetItemText(nPos, 3, stgtui.sxUser.c_str());

    // original nPos == index in vector: need to know even if user sorts rows
    m_lcEntryList.SetItemData(nPos, static_cast<DWORD>(nPos));
  }

  int nCols = m_lcEntryList.GetHeaderCtrl()->GetItemCount();

  // First is an image
  m_lcEntryList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  // Resize columns
  for (int i = 1; i < nCols; i++) {
    m_lcEntryList.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_lcEntryList.GetColumnWidth(i);
    m_lcEntryList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_lcEntryList.GetColumnWidth(i);
    m_lcEntryList.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }
  m_lcEntryList.SetColumnWidth(nCols - 1, LVSCW_AUTOSIZE_USEHEADER);

  return TRUE;
}

void CViewAttachmentEntriesDlg::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *)pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_lcEntryList.SortItems(CompareFunc, (LPARAM)this);

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
    m_lcEntryList.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_lcEntryList.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CViewAttachmentEntriesDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                                    LPARAM closure)
{
  CViewAttachmentEntriesDlg *self = (CViewAttachmentEntriesDlg *)closure;

  int nSortColumn = self->m_iSortedColumn;
  const st_gtui pLHS = self->m_pvst_gtui->at(lParam1);
  const st_gtui pRHS = self->m_pvst_gtui->at(lParam2);
  CSecString group1, title1, username1;
  CSecString group2, title2, username2;
  int image1, image2;

  int iResult(0);
  switch (nSortColumn) {
  case 0:
    image1 = (int)pLHS.image;
    image2 = (int)pRHS.image;
    if (image1 != image2)
      iResult = (image1 < image2) ? -1 : 1;
    break;
  case 1:
    group1 = pLHS.sxGroup;
    group2 = pRHS.sxGroup;
    iResult = ((CString)group1).CompareNoCase(group2);
    break;
  case 2:
    title1 = pLHS.sxTitle;
    title2 = pRHS.sxTitle;
    iResult = ((CString)title1).CompareNoCase(title2);
    break;
  case 3:
    username1 = pLHS.sxUser;
    username2 = pRHS.sxUser;
    iResult = ((CString)username1).CompareNoCase(username2);
    break;
  default:
    ASSERT(FALSE);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}
