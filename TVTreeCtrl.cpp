/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

// Need a set to keep track of what nodes are expanded, to re-expand
// after minimize
#include <set>

using namespace std ;

#include "stdafx.h"
#include "TVTreeCtrl.h"
#include "DboxMain.h"
#include "corelib/ItemData.h"
#include "corelib/MyString.h"
#include "corelib/Util.h"
#include "corelib/Pwsprefs.h"
#include "corelib/SMemFile.h"
#include "corelib/PWSrand.h"
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h >       // MFC OLE automation classes
#include "PasswordSafe.h"   // for access to external gbl_tcddCPFID
#include "DropSource.h"
#include "DropTarget.h"
#include "DDSupport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef set<CItemData *> SetTreeItem_t;
typedef SetTreeItem_t *SetTreeItemP_t;

static const TCHAR GROUP_SEP = TCHAR('.');

CTVTreeCtrl::CTVTreeCtrl() : m_pimagelist(NULL), m_pDragImage(NULL), m_isRestoring(false),
m_uiSendingSession(0), m_uiReceivingSession(0)
{
  m_expandedItems = new SetTreeItem_t;
}

CTVTreeCtrl::~CTVTreeCtrl()
{
  delete m_pimagelist;
  delete (SetTreeItem_t *)m_expandedItems;
}

BEGIN_MESSAGE_MAP(CTVTreeCtrl, CTreeCtrl)
  //{{AFX_MSG_MAP(CTVTreeCtrl)
  ON_WM_DESTROY()
  ON_WM_LBUTTONDBLCLK()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTVTreeCtrl::OnDestroy()
{
  CImageList  *pimagelist;

  pimagelist = GetImageList(TVSIL_NORMAL);
  if (pimagelist != NULL) {
    pimagelist->DeleteImageList();
    delete pimagelist;
  }

  m_TCDropTarget.Revoke();
}

BOOL CTVTreeCtrl::PreTranslateMessage(MSG* pMsg)
{
  // When an item is being edited make sure the edit control
  // receives certain important key strokes
  if (GetEditControl()) {
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE; // DO NOT process further
  }

  //hitting the F2 key, being in-place editing of an item
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F2) {
    HTREEITEM hItem = GetSelectedItem();
    if (hItem != NULL && !((DboxMain *)GetParent())->IsReadOnly())
      EditLabel(hItem);
    return TRUE;
  }

  //Let the parent class do its thing
  return CTreeCtrl::PreTranslateMessage(pMsg);
}

DROPEFFECT CTVTreeCtrl::OnDragEnter(CWnd* pWnd , COleDataObject* pDataObject,
                  DWORD dwKeyState, CPoint point)
{
  return CDropTarget::OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}

#define SCROLL_BORDER 25
#define SCROLL_SPEED_ZONE_WIDTH 20

DROPEFFECT CTVTreeCtrl::OnDragOver(CWnd* pWnd , COleDataObject* /* pDataObject */,
                  DWORD dwKeyState, CPoint point)
{
  CTVTreeCtrl *pDestTreeCtrl;

  DROPEFFECT dropeffectRet = DROPEFFECT_COPY;
  if ((dwKeyState & MK_SHIFT) == MK_SHIFT)
    dropeffectRet = DROPEFFECT_MOVE;

  // Doesn't matter that we didn't initialize m_calls
  m_calls++;

  // Expand and highlight the item under the mouse and 
  pDestTreeCtrl = (CTVTreeCtrl *)pWnd;
  HTREEITEM hTItem = pDestTreeCtrl->HitTest(point);
  // Use m_calls to slow down expanding nodes
  if (hTItem != NULL && (m_calls % 32 == 0)) {
    pDestTreeCtrl->Expand(hTItem, TVE_EXPAND);
    pDestTreeCtrl->SelectDropTarget(hTItem);
  }  

  CRect rectClient;
  pWnd->GetClientRect(&rectClient);
  pWnd->ClientToScreen(rectClient);
  pWnd->ClientToScreen(&point);

  int slowscroll_up = 6 - (rectClient.top + SCROLL_BORDER - point.y) / SCROLL_SPEED_ZONE_WIDTH;
  int slowscroll_down = 6 - (point.y - rectClient.bottom + SCROLL_BORDER ) / SCROLL_SPEED_ZONE_WIDTH;

  // Scroll Tree control depending on mouse position
  int iMaxV = GetScrollLimit(SB_VERT);
  int iPosV = GetScrollPos(SB_VERT);

  int nScrollDir = -1;
  if ((point.y > rectClient.bottom - SCROLL_BORDER) && (iPosV != iMaxV) &&
      (m_calls % (slowscroll_down > 0 ? slowscroll_down : 1)) == 0)
    nScrollDir = SB_LINEDOWN;
  else
  if ((point.y < rectClient.top + SCROLL_BORDER) && (iPosV != 0) &&
      (m_calls % (slowscroll_up > 0 ? slowscroll_up : 1)) == 0)
    nScrollDir = SB_LINEUP;

  if (nScrollDir != -1) {
    int nScrollPos = pWnd->GetScrollPos(SB_VERT);
    WPARAM wParam = MAKELONG(nScrollDir, nScrollPos);
    pWnd->SendMessage(WM_VSCROLL, wParam);
  }
  
  int iPosH = GetScrollPos(SB_HORZ);
  int iMaxH = GetScrollLimit(SB_HORZ);

  nScrollDir = -1;
  if ((point.x < rectClient.left + SCROLL_BORDER) && (iPosH != 0))
    nScrollDir = SB_LINELEFT;
  else
  if ((point.x > rectClient.right - SCROLL_BORDER) && (iPosH != iMaxH))
    nScrollDir = SB_LINERIGHT;
  
  if (nScrollDir != -1) {
    int nScrollPos = pWnd->GetScrollPos(SB_VERT);
    WPARAM wParam = MAKELONG(nScrollDir, nScrollPos);
    pWnd->SendMessage(WM_HSCROLL, wParam);
  }
  
  return dropeffectRet;  
}

void CTVTreeCtrl::SetNewStyle(long lStyleMask, BOOL bSetBits)
{
  long        lStyleOld;

  lStyleOld = GetWindowLong(CWnd::m_hWnd, GWL_STYLE);
  lStyleOld &= ~lStyleMask;
  if (bSetBits)
    lStyleOld |= lStyleMask;

  SetWindowLong(CWnd::m_hWnd, GWL_STYLE, lStyleOld);
  SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CTVTreeCtrl::UpdateLeafsGroup(HTREEITEM hItem, CString prefix)
{
  // Starting with hItem, update the Group field of all of hItem's
  // children. Called after a label has been edited.
  if (IsLeafNode(hItem)) {
    DWORD itemData = GetItemData(hItem);
    ASSERT(itemData != NULL);
    CItemData *ci = (CItemData *)itemData;
    ci->SetGroup(CMyString(prefix));
  } else { // update prefix with current group name and recurse
    if (!prefix.IsEmpty())
      prefix += GROUP_SEP;
    prefix += GetItemText(hItem);
    HTREEITEM child;
    for(child = GetChildItem(hItem); child != NULL; child = GetNextSiblingItem(child)) {
      UpdateLeafsGroup(child, prefix);
    }
  }
}


void CTVTreeCtrl::BeginDrag(NMHDR * /* pNotifyStruct */, LRESULT * &/* pLResult */)
{
    TRACE("CTVTreeCtrl::BeginDrag()\n");
    // Can drag in read-only mode as it might be to someone else
    // Can't allow drop in read-only mode

    CPoint      ptAction;
    GetCursorPos(&ptAction);
    ScreenToClient(&ptAction);

    UINT uFlags;
    m_hitemDrag = HitTest(ptAction, &uFlags);
    if ((m_hitemDrag == NULL) || !(TVHT_ONITEM & uFlags)) {
        return;
    }

    SelectItem(m_hitemDrag);

    long lBufLen;
    BYTE * buffer(NULL);
    CString cs_text;

    // Start of Drag of entries.....
    // CollectData allocates buffer - need to free later
    if (!CollectData(buffer, lBufLen))
        return;

    cs_text.Format(_T("%s%02x%08x"), gbl_classname, FROMTREE, lBufLen);

    CMemFile mf;
    mf.Write(cs_text, sizeof(gbl_classname) - 1 + 10);
    mf.Write(buffer, lBufLen);

    // Finished with (encrypted) buffer - free it
    free(buffer);

    DWORD dw_mflen = (DWORD)mf.GetLength();
    LPCTSTR mf_buffer = LPCTSTR(mf.Detach());

    ASSERT(m_pDragImage == NULL);
    m_pDragImage = CreateDragImage(m_hitemDrag);

    // Get client rectangle
    RECT rClient;
    GetClientRect(&rClient);

    // Set our session numbers (should be different!)
    m_uiSendingSession = PWSrand::GetInstance()->RandUInt();
    m_uiReceivingSession = PWSrand::GetInstance()->RandUInt();

    // Start dragging
    StartDragging(mf_buffer, dw_mflen, gbl_tcddCPFID, &rClient, &ptAction);

    // Cleanup
    free((void *)mf_buffer);

// End dragging image
    m_pDragImage->DragLeave(GetDesktopWindow());
    m_pDragImage->EndDrag();

    delete m_pDragImage;
    m_pDragImage = NULL;
}

void CTVTreeCtrl::BeginLabelEdit(NMHDR * /* pNotifyStruct */, LRESULT * &pLResult)
{
  //TV_DISPINFO *ptvinfo = (TV_DISPINFO *)pNotifyStruct;
  *pLResult = FALSE; // TRUE cancels label editing
}

static void splitLeafText(const TCHAR *lt, CString &newTitle, CString &newUser)
{
  CString leafText(lt);
  int leftBraceIndex = leafText.Find(TCHAR('['));

  if (leftBraceIndex == -1) {
    newTitle = leafText;
    newUser = _T("");
  } else {
    newTitle = leafText.Left(leftBraceIndex - 1);
    int rightBraceIndex = leafText.Find(TCHAR(']'));
    if (rightBraceIndex == -1) {
      newUser = leafText.Mid(leftBraceIndex + 1);
    } else {
      newUser = leafText.Mid(leftBraceIndex + 1, rightBraceIndex - leftBraceIndex - 1);
    }
  }
}

void CTVTreeCtrl::EndLabelEdit(NMHDR *pNotifyStruct, LRESULT * &pLResult)
{
  if (((DboxMain *)GetParent())->IsReadOnly())
    return; // don't drag in read-only mode

  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pNotifyStruct;
  HTREEITEM ti = ptvinfo->item.hItem;
  if (ptvinfo->item.pszText != NULL && // NULL if edit cancelled,
      ptvinfo->item.pszText[0] != TCHAR('\0')) { // empty if text deleted - not allowed
    ptvinfo->item.mask = TVIF_TEXT;
    SetItem(&ptvinfo->item);
    if (IsLeafNode(ptvinfo->item.hItem)) {
      /*
       * Leaf text is of the form: title [user]
       * If edited text contains '[', then the user is updated
       * If not, then the user is retrieved and the leaf is updated
       */
      // Update leaf's title
      DWORD itemData = GetItemData(ti);
      ASSERT(itemData != NULL);
      CItemData *ci = (CItemData *)itemData;
      CString newTitle, newUser;
      splitLeafText(ptvinfo->item.pszText, newTitle, newUser);
      if (newUser.IsEmpty())
        newUser = CString(ci->GetUser());
      CString treeDispString;
    treeDispString = newTitle;
    treeDispString += _T(" [");
    treeDispString += newUser;
    treeDispString += _T("]");

    PWSprefs *prefs = PWSprefs::GetInstance();
    bool bShowPasswordInList = prefs->GetPref(PWSprefs::ShowPWInList);

    if (bShowPasswordInList) {
      CString newPassword = CString(ci->GetPassword());
      treeDispString += _T(" [");
      treeDispString += newPassword;
      treeDispString += _T("]");
    }

      // Update corresponding Tree mode text with the new display text (ie: in case
      // the username was removed and had to be normalized back in).  Note that
      // we cannot do "SetItemText(ti, treeDispString)" here since Windows will
      // automatically overwrite and update the item text with the contents from
      // the "ptvinfo->item.pszText" buffer.
      PWSUtil::strCopy(ptvinfo->item.pszText, ptvinfo->item.cchTextMax,
                      treeDispString, ptvinfo->item.cchTextMax);

      // update the password database record.
      ci->SetTitle(newTitle); ci->SetUser(newUser);

      // update corresponding List mode text
      DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
      ASSERT(di != NULL);
      int lindex = di->list_index;
      ((DboxMain *)m_parent)->UpdateListItemTitle(lindex, newTitle);
      ((DboxMain *)m_parent)->UpdateListItemUser(lindex, newUser);
    } else {
      // Update all leaf children with new path element
      // prefix is path up to and NOT including renamed node
      CString prefix;
      HTREEITEM parent, current = ti;
      do {
        parent = GetParentItem(current);
        if (parent == NULL) {
          break;
        }
        current = parent;
        if (!prefix.IsEmpty())
          prefix = GROUP_SEP + prefix;
        prefix = GetItemText(current) + prefix;
      } while (1);
      UpdateLeafsGroup(ti, prefix);
    }
    // Mark database as modified
    ((DboxMain *)m_parent)->SetChanged(DboxMain::Data);
    SortChildren(GetParentItem(ti));
    *pLResult = TRUE;
  } else {
    // restore text
    // (not that this is documented anywhere in MS's docs...)
    *pLResult = FALSE;
  }
}

bool CTVTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
  do {
    if (hitemChild == hitemSuspectedParent)
      break;
  } while ((hitemChild = GetParentItem(hitemChild)) != NULL);

  return (hitemChild != NULL);
}

bool CTVTreeCtrl::IsLeafNode(HTREEITEM hItem)
{
  // ItemHasChildren() won't work in the general case
  BOOL status;
  int i, dummy;
  status = GetItemImage(hItem, i, dummy);
  ASSERT(status);
  return (i != NODE);
}

void CTVTreeCtrl::DeleteWithParents(HTREEITEM hItem)
{
  // We don't want nodes that have no children to remain
  HTREEITEM p;
  do {
    p = GetParentItem(hItem);
    DeleteItem(hItem);
    if (ItemHasChildren(p))
      break;
    hItem = p;
  } while (p != TVI_ROOT && p != NULL);
}

void CTVTreeCtrl::DeleteFromSet(HTREEITEM hItem)
{
  SetTreeItemP_t pSet = SetTreeItemP_t(m_expandedItems);
  DWORD itemData = GetItemData(hItem);
  ASSERT(itemData != NULL);
  CItemData *ci = (CItemData *)itemData;
  pSet->erase(ci);
}

// Return the full path leading up to a given item, but
// not including the name of the item itself.
CString CTVTreeCtrl::GetGroup(HTREEITEM hItem)
{
  CString retval;
  CString nodeText;
  while (hItem != NULL) {
    nodeText = GetItemText(hItem);
    if (!retval.IsEmpty())
      nodeText += GROUP_SEP;
    retval = nodeText + retval;
    hItem = GetParentItem(hItem);
  }
  return retval;
}

static CMyString GetPathElem(CMyString &path)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // (assuming GROUP_SEP is '.')

  CMyString retval;
  int N = path.Find(GROUP_SEP);
  if (N == -1) {
    retval = path;
    path = _T("");
  } else {
    const int Len = path.GetLength();
    retval = CMyString(path.Left(N));
    path = CMyString(path.Right(Len - N - 1));
  }
  return retval;
}

static bool ExistsInTree(CTreeCtrl &Tree, HTREEITEM node,
       const CMyString &s, HTREEITEM &si)
{
  // returns true iff s is a direct descendant of node
  HTREEITEM ti = Tree.GetChildItem(node);

  while (ti != NULL) {
    const CMyString itemText = Tree.GetItemText(ti);
    if (itemText == s) {
      si = ti;
      return true;
    }
    ti = Tree.GetNextItem(ti, TVGN_NEXT);
  }
  return false;
}

HTREEITEM CTVTreeCtrl::AddGroup(const CString &group)
{
  // Add a group at the end of path
  HTREEITEM ti = TVI_ROOT;
  HTREEITEM si;
  if (!group.IsEmpty()) {
    CMyString path = group;
    CMyString s;
    do {
      s = GetPathElem(path);
      if (!ExistsInTree(*this, ti, s, si)) {
        ti = InsertItem(s, ti, TVI_SORT);
        SetItemImage(ti, CTVTreeCtrl::NODE, CTVTreeCtrl::NODE);
      } else
        ti = si;
    } while (!path.IsEmpty());
  }
  return ti;
}

bool CTVTreeCtrl::TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
  TV_INSERTSTRUCT     tvstruct;
  TCHAR               sztBuffer[260];  // max visible
  HTREEITEM           hNewItem, hFirstChild;
  DWORD itemData = GetItemData(hitemDrag);

  // avoid an infinite recursion
  tvstruct.item.hItem = hitemDrag;
  tvstruct.item.cchTextMax = sizeof(sztBuffer)/sizeof(TCHAR) - 1;
  tvstruct.item.pszText = sztBuffer;
  tvstruct.item.mask = (TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE
                        | TVIF_SELECTEDIMAGE | TVIF_TEXT);
  GetItem(&tvstruct.item);  // get information of the dragged element

  tvstruct.hParent = hitemDrop;
  if (((DboxMain *)GetParent())->IsExplorerTree())
    tvstruct.hInsertAfter = TVI_LAST;
  else
    tvstruct.hInsertAfter = TVI_SORT;

  tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  hNewItem = InsertItem(&tvstruct);
  if (itemData != 0) { // Non-NULL itemData implies Leaf
    CItemData *ci = (CItemData *)itemData;
    // Update Group
    CMyString path, elem;
    HTREEITEM p, q = hNewItem;
    do {
      p = GetParentItem(q);
      if (p != NULL) {
        elem = CMyString(GetItemText(p));
        if (!path.IsEmpty())
          elem += GROUP_SEP;
        path = elem + path;
        q = p;
      } else
        break;
    } while (1);
    // Get information from current selected entry
    CMyString ci_user = ci->GetUser();
    CMyString ci_title0 = ci->GetTitle();
    CMyString ci_title;
    ci_title = ((DboxMain *)m_parent)->GetUniqueTitle(path, 
                           ci_title0, ci_user, IDS_DRAGNUMBER);

    ci->SetGroup(path);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    if (ci_title.Compare(ci_title0) != 0) {
      ci->SetTitle(ci_title);
      CMyString treeDispString;
      treeDispString = ci_title;
      treeDispString += _T(" [");
      treeDispString += ci_user;
      treeDispString += _T("]");
      if( PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWInList)) {
        CMyString ci_Password = ci->GetPassword();
        treeDispString += _T(" [");
        treeDispString += ci_Password;
        treeDispString += _T("]");
      }
      // Update tree label
      SetItemText(hNewItem, treeDispString);
      // Update list field
      ((DboxMain *)GetParent())->UpdateListItemTitle(di->list_index, (CString)ci_title);
    }
    // Mark database as modified!
    ((DboxMain *)GetParent())->SetChanged(DboxMain::Data);
    // Update DisplayInfo record associated with ItemData
    di->tree_item = hNewItem;
  }
  SetItemData(hNewItem, itemData);
  if (((DboxMain *)GetParent())->IsExplorerTree())
    ((DboxMain *)GetParent())->SortTree(hitemDrop);
  else
    SortChildren(hitemDrop);

  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    TransferItem(hFirstChild, hNewItem);  // recursively transfer all the items
    DeleteItem(hFirstChild);
  }
  return true;
}

bool CTVTreeCtrl::CopyItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
  HTREEITEM hFirstChild;
  DWORD itemData = GetItemData(hitemDrag);

  if (itemData != 0) { // Non-NULL itemData implies Leaf
    CItemData *ci = (CItemData *)itemData;
    CItemData temp(*ci);

    // Update Group
    CMyString path, elem;
    path = CMyString(GetItemText(hitemDrop));
    HTREEITEM p, q = hitemDrop;
    do {
      p = GetParentItem(q);
      if (p != NULL) {
        elem = CMyString(GetItemText(p));
        if (!path.IsEmpty())
          elem += GROUP_SEP;
        path = elem + path;
        q = p;
      } else
        break;
    } while (1);

    // Get information from current selected entry
    CMyString ci_user = ci->GetUser();
    CMyString ci_title0 = ci->GetTitle();
    CMyString ci_title;
    ci_title = ((DboxMain *)m_parent)->GetUniqueTitle(path, ci_title0,
                            ci_user, IDS_DRAGNUMBER);

    temp.CreateUUID();
    temp.SetGroup(path);
    temp.SetTitle(ci_title);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    temp.SetDisplayInfo(ndi);

    ((DboxMain *)m_parent)->AddEntry(temp);

    // Mark database as modified!
    ((DboxMain *)GetParent())->SetChanged(DboxMain::Data);
  }

  if (((DboxMain *)GetParent())->IsExplorerTree())
    ((DboxMain *)GetParent())->SortTree(hitemDrop);
  else
    SortChildren(hitemDrop);

  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    CopyItem(hFirstChild, hitemDrop);  // recursively copy all the items
  }

  return true;
}

void CTVTreeCtrl::ExpandCollapse(NMHDR *pNotifyStruct, LRESULT * &pLResult)
{
  // The hItem that is expanded isn't the one that will be restored,
  // since the tree is rebuilt in DboxMain::RefreshList. Therefore, we
  // need to store the corresponding elements. But groups have none, so
  // we store the first (or any) child element, and upon restore, expand
  // the parent. Ugh++.

  *pLResult = FALSE;

  if (!m_isRestoring) {
    SetTreeItemP_t pSet = SetTreeItemP_t(m_expandedItems);
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNotifyStruct;
    // now find a leaf son and add it's CitemData to set.
    HTREEITEM child = GetChildItem(pNMTreeView->itemNew.hItem);
    ASSERT(child != NULL); // can't expand something w/o children, right?
    do {
      if (IsLeafNode(child)) {
        break; // stop at first leaf child found
      }
      child = GetNextSiblingItem(child);
    } while (child != NULL);

    if (child == NULL) {
      // case where expanded node has only tree subnodes,
      // nothing to get a CItemData from. This borderline
      // case is hereby deemed more trouble than it's worth to
      // handle correctly.
      return;
    }
    DWORD itemData = GetItemData(child);
    ASSERT(itemData != NULL);
    CItemData *ci = (CItemData *)itemData;
    if (pNMTreeView->action == TVE_EXPAND)
      pSet->insert(ci);
    else if (pNMTreeView->action == TVE_COLLAPSE) {
      ASSERT(pSet->find(ci) != pSet->end());
      pSet->erase(ci);
    }
  }
}

void CTVTreeCtrl::RestoreExpanded()
{
  m_isRestoring = true;
  SetTreeItemP_t pSet = SetTreeItemP_t(m_expandedItems);
  SetTreeItem_t::iterator it;

  for (it = pSet->begin(); it != pSet->end(); it++) {
    CItemData *ci = *it;
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    HTREEITEM parent = GetParentItem(di->tree_item);
    Expand(parent, TVE_EXPAND);
  }
  m_isRestoring = false;
}

void CTVTreeCtrl::ClearExpanded()
{
  SetTreeItemP_t pSet = SetTreeItemP_t(m_expandedItems);
  pSet->clear();
}

void CTVTreeCtrl::OnExpandAll()
{
  // Updated to test for zero entries!
  HTREEITEM hItem = this->GetRootItem();
  if (hItem == NULL)
    return;
  SetRedraw(FALSE);
  do {
    this->Expand(hItem,TVE_EXPAND);
    hItem = this->GetNextItem(hItem,TVGN_NEXTVISIBLE);
  }
  while (hItem);
  this->EnsureVisible(this->GetSelectedItem());
  SetRedraw();
}

void CTVTreeCtrl::OnCollapseAll()
{
  // Courtesy of Zafir Anjum from www.codeguru.com
  // Updated to test for zero entries!
  HTREEITEM hItem = this->GetRootItem();
  if (hItem == NULL)
    return;
  SetRedraw(FALSE);
  do {
    CollapseBranch(hItem);
  }
  while((hItem = this->GetNextSiblingItem(hItem)) != NULL);
  SetRedraw();
}

void CTVTreeCtrl::CollapseBranch(HTREEITEM hItem)
{
  // Courtesy of Zafir Anjumfrom www.codeguru.com
  if(this->ItemHasChildren(hItem)) {
    this->Expand(hItem, TVE_COLLAPSE);
    hItem = this->GetChildItem(hItem);
    do {
      CollapseBranch(hItem);
    }
    while((hItem = this->GetNextSiblingItem(hItem)) != NULL);
  }
}

HTREEITEM
CTVTreeCtrl::GetNextTreeItem(HTREEITEM hItem)
{
  if (NULL == hItem)
    return this->GetRootItem();

    // First, try to go to this item's 1st child
    HTREEITEM hReturn = this->GetChildItem(hItem);

    // If no more child items...
    while (hItem && !hReturn) {
        // Get this item's next sibling
        hReturn = this->GetNextSiblingItem(hItem);

        // If hReturn is NULL, then there are no
        // sibling items, and we're on a leaf node.
        // Backtrack up the tree one level, and
        // we'll look for a sibling on the next
        // iteration (or we'll reach the root and
        // quit).
        hItem = this->GetParentItem(hItem);
    }
    return hReturn;
}

void CTVTreeCtrl::OnLButtonDblClk(UINT /* nFlags */, CPoint /* point */)
{
  ((DboxMain *)m_parent)->DoItemDoubleClick();
}

BOOL CTVTreeCtrl::OnDrop(CWnd* /* pWnd */, COleDataObject* pDataObject,
                              DROPEFFECT dropEffect, CPoint point)
{
  if (((DboxMain *)GetParent())->IsReadOnly())
    return FALSE; // don't drop in read-only mode

  if (!pDataObject->IsDataAvailable(gbl_tcddCPFID, NULL))
    return FALSE;

  UINT uFlags;
  m_hitemDrop = HitTest(point, &uFlags);

  bool bForceRoot(false);
  switch (uFlags) {
    case TVHT_ABOVE:
    case TVHT_BELOW:
    case TVHT_TOLEFT:
    case TVHT_TORIGHT:
      return FALSE;
    case TVHT_NOWHERE:
      if (m_hitemDrop == NULL) {
        // Treat as drop in root
        m_hitemDrop = GetRootItem();
        bForceRoot = true;
      } else
        return FALSE;
      break;
    case TVHT_ONITEM:
    case TVHT_ONITEMBUTTON:
    case TVHT_ONITEMICON:
    case TVHT_ONITEMINDENT:
    case TVHT_ONITEMLABEL:
    case TVHT_ONITEMRIGHT:
    case TVHT_ONITEMSTATEICON:
      if (m_hitemDrop == NULL)
        return FALSE;
      break;
    default :
      return FALSE;
  }

  m_uiReceivingSession = m_uiSendingSession;

  BOOL retval(FALSE);

  // On Drop of data from one tree to another
  HGLOBAL hGlobal;

  hGlobal = pDataObject->GetGlobalData(gbl_tcddCPFID);
  LPCTSTR pData = (LPCTSTR)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  SIZE_T memsize = GlobalSize(hGlobal);

  if (memsize < DD_MEMORY_MINSIZE)
    goto exit;

  memset(m_sending_classname, 0, sizeof(gbl_classname));
  memcpy(m_sending_classname, pData, sizeof(gbl_classname) - 1);
  bool our_data = memcmp(gbl_classname, m_sending_classname, sizeof(gbl_classname) - 1) == 0;

  int iDDType;
  long lBufLen;

#if _MSC_VER >= 1400
  _stscanf_s(pData + sizeof(gbl_classname) - 1, _T("%02x%08x"), &iDDType, &lBufLen);
#else
  _stscanf(pData + sizeof(gbl_classname) - 1, _T("%02x%08x"), &iDDType, &lBufLen);
#endif

  // Check if it is from another TreeCtrl?
  // - we don't accept drop from anything else
  if (iDDType != FROMTREE || ((long)memsize < (DD_MEMORY_MINSIZE + lBufLen)))
    goto exit;

  if (m_hitemDrop == NULL && GetCount() == 0) {
    // Dropping on to an empty database
    CMyString DropGroup (_T(""));
    ProcessData((BYTE *)(pData + sizeof(gbl_classname) - 1 + 10), lBufLen, DropGroup);
    SelectItem(GetRootItem());
    retval = TRUE;
    goto exit;
  }

  if (IsLeafNode(m_hitemDrop) || bForceRoot)
    m_hitemDrop = GetParentItem(m_hitemDrop);

  if (our_data) {
    // from me! - easy
    HTREEITEM parent = GetParentItem(m_hitemDrag);
    if (m_hitemDrag != m_hitemDrop &&
      !IsChildNodeOf(m_hitemDrop, m_hitemDrag) &&
      parent != m_hitemDrop) {
      // drag operation completed successfully.
      if (dropEffect == DROPEFFECT_MOVE) {
        // Transfer them & delete
        TransferItem(m_hitemDrag, m_hitemDrop);
        DeleteItem(m_hitemDrag);
      } else if (dropEffect == DROPEFFECT_COPY) {
        CopyItem(m_hitemDrag, m_hitemDrop);
      }
      SelectItem(m_hitemDrop);
      retval = TRUE;
    } else {
      // drag failed or cancelled, revert to last selected
      SelectItem(m_hitemDrag);
    }
  } else {
    // from someone else!
    // Now add it
    CMyString DropGroup = CMyString(GetGroup(m_hitemDrop));
    ProcessData((BYTE *)(pData + sizeof(gbl_classname) - 1 + 10), lBufLen, DropGroup);
    SelectItem(m_hitemDrop);
    retval = TRUE;
  }

  GetParent()->SetFocus();

exit:
  GlobalUnlock(hGlobal);

  if (retval == TRUE)
    ((DboxMain *)m_parent)->SetChanged(DboxMain::Data);

  return retval;
}

void CTVTreeCtrl::CompleteMove()
{
  // If drag within instance - we have already done ths
  if (m_uiReceivingSession == m_uiSendingSession)
    return;

  // If drag to another instance, ignore in Read-only mode
  if (((DboxMain *)GetParent())->IsReadOnly())
    return;

  // After we have dragged successfully from our Tree to another Tree
  ((DboxMain *)m_parent)->Delete();
  ((DboxMain *)m_parent)->RefreshList();
}

bool CTVTreeCtrl::CollectData(BYTE * &out_buffer, long &outLen)
{
  DWORD itemData = GetItemData(m_hitemDrag);
  CItemData *ci = (CItemData *)itemData;

  CDDObList out_oblist;

  if (IsLeafNode(m_hitemDrag)) {
    ASSERT(itemData != NULL);
    m_nDragPathLen = 0;
    GetEntryData(out_oblist, ci);
    out_oblist.m_bDragNode = false;
  } else {
    m_nDragPathLen = GetGroup(GetParentItem(m_hitemDrag)).GetLength();
    GetGroupEntriesData(out_oblist, m_hitemDrag);
    out_oblist.m_bDragNode = true;
  }

  CSMemFile outDDmemfile;
  CArchive ar_out (&outDDmemfile, CArchive::store);
  out_oblist.Serialize(ar_out);
  ar_out.Flush();
  ar_out.Close();

  DWORD dw_outmflen = (DWORD)outDDmemfile.GetLength();
  unsigned char * outddmemfile_buffer = (unsigned char *)outDDmemfile.Detach();
  EncryptSendingData(outddmemfile_buffer, dw_outmflen, out_buffer, outLen);

  while (!out_oblist.IsEmpty()) {
    delete (CDDObject *)out_oblist.RemoveHead();
  } 

  trashMemory(outddmemfile_buffer, dw_outmflen);
  free(outddmemfile_buffer);

  return (outLen > 0);
}

bool CTVTreeCtrl::ProcessData(BYTE *in_buffer, const long &inLen, const CMyString DropGroup)
{
  unsigned char * clear_buffer(NULL);
  long clearLen(0);

  DecryptReceivedData(in_buffer, inLen, clear_buffer, clearLen);

  if (clearLen <= 0)
    return false;

  CDDObList in_oblist;

  CSMemFile inDDmemfile;

  inDDmemfile.Attach((BYTE *)clear_buffer, clearLen);

  CArchive ar_in (&inDDmemfile, CArchive::load);
  in_oblist.Serialize(ar_in);
  ar_in.Close();

  inDDmemfile.Detach();
  trashMemory(clear_buffer, clearLen);
  free(clear_buffer);

  if (!in_oblist.IsEmpty()) {
    ((DboxMain *)m_parent)->AddEntries(in_oblist, DropGroup);

    while (!in_oblist.IsEmpty()) {
      delete (CDDObject *)in_oblist.RemoveHead();
    }
  }

  return (clearLen > 0);
}

void CTVTreeCtrl::DecryptReceivedData(BYTE * &in_buffer, const long &inLen,
                                      unsigned char * &out_buffer, long &outLen)
{
  CSMemFile inMemFile;
  CMyString passwd;

  out_buffer = NULL;
  outLen = 0;

  // Point to data
  inMemFile.Attach((BYTE *)in_buffer, inLen, 0);

  // Generate password from their classname and the CLIPFORMAT
  passwd.Format(_T("%s%04x"), m_sending_classname, gbl_tcddCPFID);

  // Decrypt it
  DecryptMemory(out_buffer, outLen, passwd, &inMemFile);

  inMemFile.Detach();
}

void CTVTreeCtrl::EncryptSendingData(unsigned char * &in_buffer, const long &inLen,
                                     BYTE * &out_buffer, long &outLen)
{
  CSMemFile outMemFile;
  CMyString passwd;

  // Generate password from our classname and the CLIPFORMAT
  passwd.Format(_T("%s%04x"), gbl_classname, gbl_tcddCPFID);

  // Encrypt it
  EncryptMemory(in_buffer, inLen, passwd, &outMemFile);

  outLen = (long)outMemFile.GetLength();
  if (outLen > 0) {
    // Create buffer for clipboard
    out_buffer = (BYTE *)outMemFile.Detach();
  }
}

void
CTVTreeCtrl::GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem)
{
  if (IsLeafNode(hItem)) {
    DWORD itemData = GetItemData(hItem);
    ASSERT(itemData != NULL);
    CItemData *ci = (CItemData *)itemData;
    GetEntryData(out_oblist, ci);
  } else {
    HTREEITEM child;
    for (child = GetChildItem(hItem);
      child != NULL;
      child = GetNextSiblingItem(child)) {
      GetGroupEntriesData(out_oblist, child);
    }
  }
}

void
CTVTreeCtrl::GetEntryData(CDDObList &out_oblist, CItemData *ci)
{
  CDDObject *pDDObject = new CDDObject;

  uuid_array_t uuid_array;
  ci->GetUUID(uuid_array);

  memcpy(pDDObject->m_DD_UUID, uuid_array, sizeof(uuid_array));

  const CMyString cs_Group = ci->GetGroup();
  if (out_oblist.m_bDragNode && m_nDragPathLen > 0)
    pDDObject->m_DD_Group = cs_Group.Right(cs_Group.GetLength() - m_nDragPathLen - 1);
  else
    pDDObject->m_DD_Group = cs_Group;

  pDDObject->m_DD_Title= ci->GetTitle();
  pDDObject->m_DD_User= ci->GetUser();
  pDDObject->m_DD_Notes = ci->GetNotes();
  pDDObject->m_DD_Password = ci->GetPassword();
  pDDObject->m_DD_URL = ci->GetURL();
  pDDObject->m_DD_AutoType= ci->GetAutoType();
  pDDObject->m_DD_PWHistory = ci->GetPWHistory();

  ci->GetCTime(pDDObject->m_DD_CTime);
  ci->GetPMTime(pDDObject->m_DD_PMTime);
  ci->GetATime(pDDObject->m_DD_ATime);
  ci->GetLTime(pDDObject->m_DD_LTime);
  ci->GetRMTime(pDDObject->m_DD_RMTime);

  out_oblist.AddTail(pDDObject);
}
