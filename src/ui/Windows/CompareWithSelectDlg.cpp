/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file CompareWithSelectDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "CompareWithSelectDlg.h"

#include "core/PWScore.h"
#include "core/ItemData.h"

#include "os/UUID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCompareWithSelectDlg::CCompareWithSelectDlg(CWnd *pParent, 
  CItemData *pci, PWScore *pcore,
  CString &csProtect, CString &csAttachment)
  : CPWDialog(CCompareWithSelectDlg::IDD, pParent),
  m_pci(pci), m_pcore(pcore), m_csProtect(csProtect),
  m_csAttachment(csAttachment), m_pSelected(NULL), m_pImageList(NULL)
{
  ASSERT(pci != NULL && m_pcore != NULL);

  m_group = pci->GetGroup();
  m_title = pci->GetTitle();
  m_username = pci->GetUser();
}

CCompareWithSelectDlg::~CCompareWithSelectDlg()
{
}

void CCompareWithSelectDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ITEMTREE, m_cwItemTree);
}

BEGIN_MESSAGE_MAP(CCompareWithSelectDlg, CPWDialog)
  ON_WM_DESTROY()
  ON_NOTIFY(NM_CLICK, IDC_ITEMTREE, OnItemSelected)
  ON_NOTIFY(NM_DBLCLK, IDC_ITEMTREE, OnItemDblClick)
END_MESSAGE_MAP()

void CCompareWithSelectDlg::OnDestroy()
{
  // Remove image list
  m_cwItemTree.SetImageList(NULL, TVSIL_NORMAL);
  m_cwItemTree.SetImageList(NULL, TVSIL_STATE);

  m_pImageList->DeleteImageList();
  delete m_pImageList;
}

BOOL CCompareWithSelectDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_text;

  GetDlgItem(IDC_GROUP)->SetWindowText(m_group);
  GetDlgItem(IDC_TITLE)->SetWindowText(m_title);
  GetDlgItem(IDC_USERNAME)->SetWindowText(m_username);

  // Init stuff for tree view
  CBitmap bitmap;
  BITMAP bm;

  // Change all pixels in this 'grey' to transparent
  const COLORREF crTransparent = RGB(192, 192, 192);

  bitmap.LoadBitmap(IDB_GROUP);
  bitmap.GetBitmap(&bm);

  m_pImageList = new CImageList();

  VERIFY(m_pImageList->Create(bm.bmWidth, bm.bmHeight,
                              ILC_MASK | ILC_COLORDDB,
                              CCWTreeCtrl::NUM_IMAGES, 0) != 0);

  // Order of LoadBitmap() calls matches CCWTreeCtrl public enum
  //bitmap.LoadBitmap(IDB_GROUP); - already loaded above to get width
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  UINT bitmapResIDs[] = {IDB_GROUP,
    IDB_NORMAL, IDB_NORMAL_WARNEXPIRED, IDB_NORMAL_EXPIRED,
    IDB_ABASE, IDB_ABASE_WARNEXPIRED, IDB_ABASE_EXPIRED,
    IDB_ALIAS,
    IDB_SBASE, IDB_SBASE_WARNEXPIRED, IDB_SBASE_EXPIRED,
    IDB_SHORTCUT
  };

  for (int i = 1; i < sizeof(bitmapResIDs) / sizeof(bitmapResIDs[0]); i++) {
    bitmap.LoadBitmap(bitmapResIDs[i]);
    m_pImageList->Add(&bitmap, crTransparent);
    bitmap.DeleteObject();
  }

  m_cwItemTree.SetImageList(m_pImageList, TVSIL_NORMAL);
  m_cwItemTree.SetImageList(m_pImageList, TVSIL_STATE);

  // Populate Tree or List views
  ItemListIter listPos;
  for (listPos = m_pcore->GetEntryIter(); listPos != m_pcore->GetEntryEndIter();
       listPos++) {
    CItemData &ci = m_pcore->GetEntry(listPos);
    // Don't add shortcuts our ourselves
    if (ci.GetEntryType() != CItemData::ET_SHORTCUT &&
        m_pci->GetUUID() != ci.GetUUID())
      InsertItemIntoGUITree(ci);
  }

  m_cwItemTree.SortChildren(TVI_ROOT);
  
  // Disable OK button until an entry is selected
  GetDlgItem(IDOK)->EnableWindow(FALSE);
  
  return TRUE;  // return TRUE unless you set the focus to a control
}

void CCompareWithSelectDlg::OnItemDblClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  OnItemSelected(pNotifyStruct, pLResult);

  if (m_pSelected != NULL)
    EndDialog(IDOK);
}

void CCompareWithSelectDlg::OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0L;

  HTREEITEM hItem(NULL);

  // TreeView

  // Seems that under Vista with Windows Common Controls V6, it is ignoring
  // the single click on the button (+/-) of a node and only processing the
  // double click, which generates a copy of whatever the user selected
  // for a double click (except that it invalid for a node!) and then does
  // the expand/collapse as appropriate.
  // This codes attempts to fix this.
  switch (pNotifyStruct->code) {
    case NM_CLICK:
    {
      // Mouseclick - Need to find the item clicked via HitTest
      TVHITTESTINFO htinfo = {0};
      CPoint point = ::GetMessagePos();
      m_cwItemTree.ScreenToClient(&point);
      htinfo.pt = point;
      m_cwItemTree.HitTest(&htinfo);
      hItem = htinfo.hItem;

      // Ignore any clicks not on an item (group or entry)
      if (hItem == NULL ||
          htinfo.flags & (TVHT_NOWHERE | TVHT_ONITEMRIGHT |
                          TVHT_ABOVE   | TVHT_BELOW |
                          TVHT_TORIGHT | TVHT_TOLEFT)) {
          GetDlgItem(IDOK)->EnableWindow(FALSE);
          return;
      }

      // If a group
      if (!m_cwItemTree.IsLeaf(hItem)) {
        // If on indent or button
        if (htinfo.flags & (TVHT_ONITEMINDENT | TVHT_ONITEMBUTTON)) {
          m_cwItemTree.Expand(htinfo.hItem, TVE_TOGGLE);
          *pLResult = 1L; // We have toggled the group
          GetDlgItem(IDOK)->EnableWindow(FALSE);
          return;
        }
      }
      break;
    }
    case TVN_SELCHANGED:
      // Keyboard - We are given the new selected entry
      hItem = ((NMTREEVIEW *)pNotifyStruct)->itemNew.hItem;
      break;
    default:
      // No idea how we got here!
      GetDlgItem(IDOK)->EnableWindow(FALSE);
      return;
  }

  // Check it was on an item
  if (hItem != NULL && m_cwItemTree.IsLeaf(hItem)) {
    m_pSelected = (CItemData *)m_cwItemTree.GetItemData(hItem);
  }

  HTREEITEM hti = m_cwItemTree.GetDropHilightItem();
  if (hti != NULL)
    m_cwItemTree.SetItemState(hti, 0, TVIS_DROPHILITED);

  if (m_pSelected == NULL)
    return;

  // Can't select ourselves
  if (m_pSelected->GetGroup() == m_group && m_pSelected->GetTitle() == m_title &&
      m_pSelected->GetUser() == m_username) {
    // Unselect it
    m_cwItemTree.SetItemState(hItem, 0, TVIS_SELECTED);

    m_pSelected = NULL;
    GetDlgItem(IDOK)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CCompareWithSelectDlg::InsertItemIntoGUITree(CItemData &ci)
{
  HTREEITEM ti;
  StringX treeDispString = (LPCWSTR)m_cwItemTree.MakeTreeDisplayString(ci,
                                m_csProtect, m_csAttachment);

  // get path, create if necessary, add title as last node
  ti = m_cwItemTree.AddGroup(ci.GetGroup().c_str());
  ti = m_cwItemTree.InsertItem(treeDispString.c_str(), ti, TVI_SORT);

  m_cwItemTree.SetItemData(ti, (DWORD_PTR)&ci);
  
  int nImage = m_cwItemTree.GetEntryImage(ci);
  m_cwItemTree.SetItemImage(ti, nImage, nImage);
}

pws_os::CUUID CCompareWithSelectDlg::GetUUID()
{
  if (m_pSelected == NULL)
    return pws_os::CUUID::NullUUID();
  else
    return m_pSelected->GetUUID();
}
