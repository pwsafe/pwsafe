/*
 * Silly subclass of CTreeCtrl just to implement Drag&Drop.
 *
 * Based on MFC sample code from CMNCTRL1
 */

// Need a set to keep track of what nodes are expanded, to re-expand
// after minimize
#pragma warning(disable:4786)
#pragma warning(push,3) // sad that VC6 cannot cleanly compile standard headers
#include <set>
#pragma warning(pop)

using namespace std ;


#include "stdafx.h"
#include "MyTreeCtrl.h"
#include "DboxMain.h"
#include "corelib/ItemData.h"
#include "corelib/MyString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef set<CItemData *> SetTreeItem_t;
typedef SetTreeItem_t *SetTreeItemP_t;

static const TCHAR GROUP_SEP = TCHAR('.');

CMyTreeCtrl::CMyTreeCtrl() : m_bDragging(false), m_pimagelist(NULL)
{
  m_expandedItems = new SetTreeItem_t;
  m_isRestoring = false;
}

CMyTreeCtrl::~CMyTreeCtrl()
{
  delete m_pimagelist;
  delete m_expandedItems;
}


BEGIN_MESSAGE_MAP(CMyTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CMyTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
        ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnExpandCollapse)
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMyTreeCtrl::OnDestroy()
{
  CImageList  *pimagelist;

  pimagelist = GetImageList(TVSIL_NORMAL);
  if (pimagelist != NULL) {
    pimagelist->DeleteImageList();
    delete pimagelist;
  }
}

BOOL CMyTreeCtrl::PreTranslateMessage(MSG* pMsg) 
{
  // When an item is being edited make sure the edit control
  // receives certain important key strokes
  if (GetEditControl())
  {
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE; // DO NOT process further
  }

  //Hitting the Escape key, Cancelling drag & drop
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && m_bDragging)
  {
    EndDragging(TRUE);
    return TRUE;
  }
  //hitting the F2 key, being in-place editing of an item
  else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F2)
  {
    HTREEITEM hItem = GetSelectedItem();
    if (hItem != NULL) 
      EditLabel(hItem);
    return TRUE;
  }

  //Let the parent class do its thing
  return CTreeCtrl::PreTranslateMessage(pMsg);
}


void CMyTreeCtrl::SetNewStyle(long lStyleMask, BOOL bSetBits)
{
  long        lStyleOld;

  lStyleOld = GetWindowLong(m_hWnd, GWL_STYLE);
  lStyleOld &= ~lStyleMask;
  if (bSetBits)
    lStyleOld |= lStyleMask;

  SetWindowLong(m_hWnd, GWL_STYLE, lStyleOld);
  SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CMyTreeCtrl::UpdateLeafsGroup(HTREEITEM hItem, CString prefix)
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

void CMyTreeCtrl::OnBeginLabelEdit(LPNMHDR , LRESULT *pLResult)
{
  //TV_DISPINFO *ptvinfo = (TV_DISPINFO *)pnmhdr;
  *pLResult = FALSE; // TRUE cancels label editing
}

static void splitLeafText(const char *lt, CString &newTitle, CString &newUser)
{
  CString leafText(lt);
  int leftBraceIndex = leafText.Find('[');

  if (leftBraceIndex == -1) {
    newTitle = leafText;
    newUser = _T("");
  } else {
    newTitle = leafText.Left(leftBraceIndex - 1);
    int rightBraceIndex = leafText.Find(']');
    if (rightBraceIndex == -1) {
      newUser = leafText.Mid(leftBraceIndex + 1);
    } else {
      newUser = leafText.Mid(leftBraceIndex + 1, rightBraceIndex - leftBraceIndex - 1);
    }
  }
}

static void makeLeafText(CString &treeDispString, const CString &title, const CString &user)
{
  treeDispString = title;
  if (!user.IsEmpty()) {
    treeDispString += _T(" [");
    treeDispString += user;
    treeDispString += _T("]");
  }
}

void CMyTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  TV_DISPINFO *ptvinfo = (TV_DISPINFO *)pnmhdr;
  HTREEITEM ti = ptvinfo->item.hItem;
  DboxMain *parent = (DboxMain *)GetParent();
  if (ptvinfo->item.pszText != NULL && // NULL if edit cancelled,
      ptvinfo->item.pszText[0] != '\0') // empty if text deleted - not allowed
  {
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
      makeLeafText(treeDispString, newTitle, newUser);

      // Update corresponding Tree mode text with the new display text (ie: in case 
      // the username was removed and had to be normalized back in).  Note that
      // we cannot do "SetItemText(ti, treeDispString)" here since Windows will
      // automatically overwrite and update the item text with the contents from 
      // the "ptvinfo->item.pszText" buffer.
      strncpy(ptvinfo->item.pszText, treeDispString, ptvinfo->item.cchTextMax);
      ptvinfo->item.pszText[ptvinfo->item.cchTextMax - 1] = '\0';

      // update the password database record.
      ci->SetTitle(newTitle); ci->SetUser(newUser);
      // update corresponding List mode text
      DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
      ASSERT(di != NULL);
      int lindex = di->list_index;
      parent->UpdateListItemTitle(lindex, newTitle);
      parent->UpdateListItemUser(lindex, newUser);
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
    parent->SetChanged(true);
    SortChildren(GetParentItem(ti));
    *pLResult = TRUE;
  } else {
    // restore text
    // (not that this is documented anywhere in MS's docs...)
    *pLResult = FALSE;
  }
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
  if (m_bDragging) {
    UINT                flags;

    ASSERT(m_pimagelist != NULL);
    m_pimagelist->DragMove(point);
    HTREEITEM hitem = HitTest(point, &flags);
    if (hitem != NULL) {
      m_pimagelist->DragLeave(this);
      SelectDropTarget(hitem);
      m_hitemDrop = hitem;
      m_pimagelist->DragEnter(this, point);
    }
  }
  CTreeCtrl::OnMouseMove(nFlags, point);
}

bool CMyTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
  do {
    if (hitemChild == hitemSuspectedParent)
      break;
  } while ((hitemChild = GetParentItem(hitemChild)) != NULL);

  return (hitemChild != NULL);
}

bool CMyTreeCtrl::IsLeafNode(HTREEITEM hItem)
{
  // ItemHasChildren() won't work in the general case
  BOOL status;
  int i, dummy;
  status = GetItemImage(hItem, i, dummy);
  ASSERT(status);
  return (i == LEAF);
}

void CMyTreeCtrl::DeleteWithParents(HTREEITEM hItem)
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

// Return the full path leading up to a given item, but
// not including the name of the item itself.
CString CMyTreeCtrl::GetGroup(HTREEITEM hItem)
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

HTREEITEM CMyTreeCtrl::AddGroup(const CString &group)
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
	SetItemImage(ti, CMyTreeCtrl::NODE, CMyTreeCtrl::NODE);
      } else
	ti = si;
    } while (!path.IsEmpty());
  }
  return ti;
}

bool CMyTreeCtrl::TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
  TV_INSERTSTRUCT     tvstruct;
  TCHAR               sztBuffer[128];
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
    ci->SetGroup(path);
    // Mark database as modified!
    ((DboxMain *)GetParent())->SetChanged(true);
    // Update DisplayInfo record associated with ItemData
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    di->tree_item = hNewItem;
  }
  SetItemData(hNewItem, itemData);

  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    TransferItem(hFirstChild, hNewItem);  // recursively transfer all the items
    DeleteItem(hFirstChild);
  }
  return true;
}

void CMyTreeCtrl::EndDragging(BOOL bCancel)
{
  if (m_bDragging) {
    ASSERT(m_pimagelist != NULL);
    m_pimagelist->DragLeave(this);
    m_pimagelist->EndDrag();
    delete m_pimagelist;
    m_pimagelist = NULL;
    HTREEITEM parent = GetParentItem(m_hitemDrag);

    if (!bCancel &&
        m_hitemDrag != m_hitemDrop &&
	!IsLeafNode(m_hitemDrop) &&
	!IsChildNodeOf(m_hitemDrop, m_hitemDrag) &&
	parent != m_hitemDrop)
    {
      // drag operation completed successfully.
      TransferItem(m_hitemDrag, m_hitemDrop);
      DeleteItem(m_hitemDrag);
      while (parent != NULL && !ItemHasChildren(parent)) {
	HTREEITEM grandParent = GetParentItem(parent);
	DeleteItem(parent);
	parent = grandParent;
      }
      SelectItem(m_hitemDrop);
    } else {
      // drag failed or cancelled, revert to last selected
      SelectItem(m_hitemDrag);
    }
    ReleaseCapture();
    m_bDragging = FALSE;
    SelectDropTarget(NULL);
  }
}


void CMyTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (m_bDragging) {
    // Dragging operation should be ended, but first check to see
    // if the mouse is outside the window to decide if the 
    // drag operation should be aborted.
    CRect clientRect;
    GetClientRect(&clientRect);
    if (clientRect.PtInRect(point))
      EndDragging(FALSE);
    else
      EndDragging(TRUE);
  }
  CTreeCtrl::OnLButtonUp(nFlags, point);
}


void CMyTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CPoint      ptAction;

  NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  *pResult = 0;

  GetCursorPos(&ptAction);
  ScreenToClient(&ptAction);
  ASSERT(!m_bDragging);
  m_bDragging = TRUE;
  m_hitemDrag = pNMTreeView->itemNew.hItem;
  m_hitemDrop = NULL;
  SelectItem(m_hitemDrag);

  ASSERT(m_pimagelist == NULL);
  m_pimagelist = CreateDragImage(m_hitemDrag);
  m_pimagelist->DragShowNolock(TRUE);
  m_pimagelist->SetDragCursorImage(0, CPoint(0, 0));
  m_pimagelist->BeginDrag(0, CPoint(0,0));
  m_pimagelist->DragMove(ptAction);
  m_pimagelist->DragEnter(this, ptAction);
  SetCapture();
}


void CMyTreeCtrl::OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *)
{
  // The hItem that is expanded isn't the one that will be restored,
  // since the tree is rebuilt in DboxMain::RefreshList. Therefore, we
  // need to store the corresponding elements. But groups have none, so
  // we store the first (or any) child element, and upon restore, expand
  // the parent. Ugh++.

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
    TRACE(_T("CMyTreeCtrl::OnExpandCollapse(hitem = %x, citem = %x, action = %d)\n"),
	  pNMTreeView->itemNew.hItem, ci, pNMTreeView->action);
    if (pNMTreeView->action == TVE_EXPAND)
      pSet->insert(ci);
    else if (pNMTreeView->action == TVE_COLLAPSE) {
      ASSERT(pSet->find(ci) != pSet->end());
      pSet->erase(ci);
    }
  }
}

void CMyTreeCtrl::RestoreExpanded()
{
  m_isRestoring = true;
  SetTreeItemP_t pSet = SetTreeItemP_t(m_expandedItems);
  SetTreeItem_t::iterator it;

  for (it = pSet->begin(); it != pSet->end(); it++) {
    TRACE(_T("CMyTreeCtrl::RestoreExpanded() iterating %x\n"), *it);
    CItemData *ci = *it;
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    HTREEITEM parent = GetParentItem(di->tree_item);
    TRACE(_T("di->tree_item = %x\n"), di->tree_item);
    Expand(parent, TVE_EXPAND);
  }
  m_isRestoring = false;
}
