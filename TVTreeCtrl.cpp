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

static const TCHAR GROUP_SEP = TCHAR('.');

CTVTreeCtrl::CTVTreeCtrl() : m_pimagelist(NULL), m_pDragImage(NULL), m_isRestoring(false),
m_bWithinThisInstance(false)
{
}

CTVTreeCtrl::~CTVTreeCtrl()
{
  delete m_pimagelist;
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
  CEdit* pEdit = GetEditControl();
  if (pEdit != NULL) {
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE; // DO NOT process further
  }

  //hitting the F2 key, being in-place editing of an item
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F2) {
    HTREEITEM hItem = GetSelectedItem();
    DboxMain *dbx = static_cast<DboxMain *>(m_parent); 
    if (hItem != NULL && !(dbx->IsMcoreReadOnly()))
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

  DROPEFFECT dropeffectRet = DROPEFFECT_MOVE;
  if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
    dropeffectRet = DROPEFFECT_COPY;

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

void CTVTreeCtrl::BeginLabelEdit(NMHDR *pNotifyStruct, LRESULT * &pLResult)
{
  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pNotifyStruct;

  *pLResult = TRUE; // TRUE cancels label editing

  /*
   Allowed formats:
     1.   title
   If preference ShowUsernameInTree is set:
     2.   title [username]
   If preferences ShowUsernameInTree and ShowPasswordInTree are set:
     3.   title [username] {password}

   Neither Title, Username or Password may contain square or curly brackes to be 
   edited in place and visible.
  */
  HTREEITEM ti = ptvinfo->item.hItem;
  PWSprefs *prefs = PWSprefs::GetInstance();

  if (IsLeafNode(ti)) {
    DWORD itemData = GetItemData(ti);
    ASSERT(itemData != NULL);
    CItemData *ci = (CItemData *)itemData;
    CMyString currentTitle, currentUser, currentPassword;

    // We cannot allow in-place edit if these fields contain braces!
    currentTitle = ci->GetTitle();
    if (currentTitle.FindOneOf(_T("[]{}")) != -1)
      return;

    currentUser = ci->GetUser();
    if (prefs->GetPref(PWSprefs::ShowUsernameInTree) &&
        currentUser.FindOneOf(_T("[]{}")) != -1)
      return;

    currentPassword = ci->GetPassword();
    if (prefs->GetPref(PWSprefs::ShowPasswordInTree) &&
        currentPassword.FindOneOf(_T("[]{}")) != -1)
      return;
  }  
  // Allow in-place editing
  *pLResult = FALSE;
  return;
}

static bool splitLeafText(const TCHAR *lt, CString &newTitle, 
                          CString &newUser, CString &newPassword)
{
  bool bPasswordSet(false);

  newTitle = _T("");
  newUser = _T("");
  newPassword = _T("");

  CString cs_leafText(lt);
  cs_leafText.Trim();
  if (cs_leafText.IsEmpty())
    return false;

  // Check no duplicate braces
  int OpenSquareBraceIndex = cs_leafText.Find(TCHAR('['));
  if (OpenSquareBraceIndex >= 0)
    if (cs_leafText.Find(TCHAR('['), OpenSquareBraceIndex + 1) != -1)
      return false;

  int CloseSquareBraceIndex = cs_leafText.Find(TCHAR(']'));
  if (CloseSquareBraceIndex >= 0)
    if (cs_leafText.Find(TCHAR(']'), CloseSquareBraceIndex + 1) != -1)
      return false;

  int OpenCurlyBraceIndex = cs_leafText.Find(TCHAR('{'));
  if (OpenCurlyBraceIndex >= 0)
    if (cs_leafText.Find(TCHAR('{'), OpenCurlyBraceIndex + 1) != -1)
      return false;

  int CloseCurlyBraceIndex = cs_leafText.Find(TCHAR('}'));
  if (CloseCurlyBraceIndex >= 0)
    if (cs_leafText.Find(TCHAR('}'), CloseCurlyBraceIndex + 1) != -1)
      return false;

  // Check we have both open and close brackets
  if (OpenSquareBraceIndex >= 0 && CloseSquareBraceIndex == -1)
    return false;
  if (OpenSquareBraceIndex == -1 && CloseSquareBraceIndex >= 0)
    return false;
  if (OpenCurlyBraceIndex >= 0 && CloseCurlyBraceIndex == -1)
    return false;
  if (OpenCurlyBraceIndex == -1 && CloseCurlyBraceIndex >= 0)
    return false;

  // Check we are in the right order - open before close
  if (OpenSquareBraceIndex >= 0 && CloseSquareBraceIndex < OpenSquareBraceIndex)
    return false;
  if (OpenCurlyBraceIndex >= 0 && CloseCurlyBraceIndex < OpenCurlyBraceIndex)
    return false;

  // Check we are in the right order - square before curly
  if (OpenSquareBraceIndex >= 0 && OpenCurlyBraceIndex >= 0 &&
      OpenCurlyBraceIndex < OpenSquareBraceIndex)
    return false;

  if (OpenSquareBraceIndex == -1 && OpenCurlyBraceIndex == -1) {
    // title
    newTitle = cs_leafText;
    return true;
  }

  if (OpenSquareBraceIndex >= 0 && OpenCurlyBraceIndex == -1) {
    // title [user]
    newTitle = cs_leafText.Left(OpenSquareBraceIndex);
    newTitle.Trim();
    newUser = cs_leafText.Mid(OpenSquareBraceIndex + 1, 
                  CloseSquareBraceIndex - OpenSquareBraceIndex - 1);
    newUser.Trim();
    goto final_check;
  }

  if (OpenSquareBraceIndex == -1 && OpenCurlyBraceIndex >= 0) {
    // title {password}
    newTitle = cs_leafText.Left(OpenCurlyBraceIndex);
    newTitle.Trim();
    newPassword = cs_leafText.Mid(OpenCurlyBraceIndex + 1, 
                  CloseCurlyBraceIndex - OpenCurlyBraceIndex - 1);
    newPassword.Trim();
    bPasswordSet = true;
    goto final_check;
  }

  if (OpenSquareBraceIndex >= 0 && OpenCurlyBraceIndex >= 0) {
    // title [user] {password}
    newTitle = cs_leafText.Left(OpenSquareBraceIndex);
    newTitle.Trim();
    newUser = cs_leafText.Mid(OpenSquareBraceIndex + 1, 
                  CloseSquareBraceIndex - OpenSquareBraceIndex - 1);
    newUser.Trim();
    newPassword = cs_leafText.Mid(OpenCurlyBraceIndex + 1, 
                  CloseCurlyBraceIndex - OpenCurlyBraceIndex - 1);
    newPassword.Trim();
    bPasswordSet = true;
    goto final_check;
  }

  return false; // Should never get here!

final_check:
  bool bRC(true);
  if (newTitle.IsEmpty())
    bRC = false;

  if (bPasswordSet && newPassword.IsEmpty())
    bRC = false;

  return bRC;
}

void CTVTreeCtrl::EndLabelEdit(NMHDR *pNotifyStruct, LRESULT * &pLResult)
{
  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 
  if (dbx->IsMcoreReadOnly())
    return; // don't drag in read-only mode

  // Initial verification performed in OnBeginLabelEdit - so some events may not get here!
  // Only items visible will be changed - e.g. if password is not shown and the user
  // puts in the new dispay text, it will be ignored.

  /* Allowed formats:
     1.   title
   If preference ShowUsernameInTree is set:
     2.   title [username]
   If preferences ShowUsernameInTree and ShowPasswordInTree are set:
     3.   title [username] {password}

   There can only be one of each:
     open square brace
     close square brace
     open curly brace
     close curly brace

   If pos_xtb = position of x = open/close, t = square/curly brackes, then

     pos_osb < pos_csb < pos_ocb < pos_ccb

   Title and Password are mandatory fields within the PWS database and so, if specified,
   these fields cannot be empty.
  */

  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pNotifyStruct;
  HTREEITEM ti = ptvinfo->item.hItem;
  if (ptvinfo->item.pszText != NULL && // NULL if edit cancelled,
      ptvinfo->item.pszText[0] != TCHAR('\0')) { // empty if text deleted - not allowed
    ptvinfo->item.mask = TVIF_TEXT;
    SetItem(&ptvinfo->item);
    if (IsLeafNode(ptvinfo->item.hItem)) {
      DWORD itemData = GetItemData(ti);
      ASSERT(itemData != NULL);
      CItemData *ci = (CItemData *)itemData;
      CString group, newTitle, newUser, newPassword;

      if (!splitLeafText(ptvinfo->item.pszText, newTitle, newUser, newPassword)) {
        // errors in user's input - restore text and refresh display
        goto bad_exit;
      }

      group = CString(ci->GetGroup());
      if (dbx->Find(group, newTitle, newUser) != dbx->End()) {
        CMyString temp;
        if (group.IsEmpty())
          temp.Format(IDS_ENTRYEXISTS2, newTitle, newUser);
        else
          temp.Format(IDS_ENTRYEXISTS, group, newTitle, newUser);
        AfxMessageBox(temp);
        goto bad_exit;
      }

      if (newUser.IsEmpty())
        newUser = CString(ci->GetUser());
      if (newPassword.IsEmpty())
        newPassword = CString(ci->GetPassword());

      PWSprefs *prefs = PWSprefs::GetInstance();
      CString treeDispString;
      bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
      bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);

      treeDispString = newTitle;
      if (bShowUsernameInTree) {
        treeDispString += _T(" [");
        treeDispString += newUser;
        treeDispString += _T("]");

        if (bShowPasswordInTree) {
          treeDispString += _T(" {");
          treeDispString += newPassword;
          treeDispString += _T("}");
	      }
      }

      // Update corresponding Tree mode text with the new display text (ie: in case 
      // the username was removed and had to be normalized back in).  Note that
      // we cannot do "SetItemText(ti, treeDispString)" here since Windows will
      // automatically overwrite and update the item text with the contents from 
      // the "ptvinfo->item.pszText" buffer.
      PWSUtil::strCopy(ptvinfo->item.pszText, ptvinfo->item.cchTextMax,
                      treeDispString, ptvinfo->item.cchTextMax);
      ptvinfo->item.pszText[ptvinfo->item.cchTextMax - 1] = TCHAR('\0');

      // update corresponding List mode text
      DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
      ASSERT(di != NULL);
      int lindex = di->list_index;

      // update the password database record - but only those items visible!!!
      ci->SetTitle(newTitle);
      dbx->UpdateListItemTitle(di->list_index, (CString)newTitle);
      if (bShowUsernameInTree) {
        ci->SetUser(newUser);
        dbx->UpdateListItemUser(lindex, newUser);
        if (bShowPasswordInTree) {
          ci->SetPassword(newPassword);
          dbx->UpdateListItemPassword(lindex, newPassword);
        }
      }
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
    dbx->SetChanged(DboxMain::Data);

    // Sort it as appropriate
     if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree))
      dbx->SortTree(ti);
    else
      SortChildren(GetParentItem(ti));

    // OK
    *pLResult = TRUE;
    return;
  }

bad_exit:
    // Refresh display to show old text - if we don't no one else will
    dbx->RefreshList();
    // restore text
    // (not that this is documented anywhere in MS's docs...)
    *pLResult = FALSE;
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
  DWORD itemData = GetItemData(hItem);
  ASSERT(itemData != NULL);
  CItemData *ci = (CItemData *)itemData;
  m_expandedItems.erase(ci);
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

  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 

  // avoid an infinite recursion
  tvstruct.item.hItem = hitemDrag;
  tvstruct.item.cchTextMax = sizeof(sztBuffer)/sizeof(TCHAR) - 1;
  tvstruct.item.pszText = sztBuffer;
  tvstruct.item.mask = (TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE
                        | TVIF_SELECTEDIMAGE | TVIF_TEXT);
  GetItem(&tvstruct.item);  // get information of the dragged element

  tvstruct.hParent = hitemDrop;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree)) 
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
    ci_title = dbx->GetUniqueTitle(path, ci_title0, ci_user, IDS_DRAGNUMBER);

    ci->SetGroup(path);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    if (ci_title.Compare(ci_title0) != 0) {
      ci->SetTitle(ci_title);
      CMyString treeDispString;
      treeDispString = ci_title;
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowUsernameInTree)) {
        treeDispString += _T(" [");
        treeDispString += ci_user;
        treeDispString += _T("]");
        if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPasswordInTree)) {
          CMyString ci_Password = ci->GetPassword();
          treeDispString += _T(" {");
          treeDispString += ci_Password;
          treeDispString += _T("}");
        }
      }
      // Update tree label
      SetItemText(hNewItem, treeDispString);
      // Update list field
      dbx->UpdateListItemTitle(di->list_index, (CString)ci_title);
    }
    // Mark database as modified!
    dbx->SetChanged(DboxMain::Data);
    // Update DisplayInfo record associated with ItemData
    di->tree_item = hNewItem;
  }
  SetItemData(hNewItem, itemData);
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree))
    dbx->SortTree(hitemDrop);
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

  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 

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
    ci_title = dbx->GetUniqueTitle(path, ci_title0, ci_user, IDS_DRAGNUMBER);

    temp.CreateUUID();
    temp.SetGroup(path);
    temp.SetTitle(ci_title);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    temp.SetDisplayInfo(ndi);

    dbx->AddEntry(temp);

    // Mark database as modified!
    dbx->SetChanged(DboxMain::Data);
  }

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree))
    dbx->SortTree(hitemDrop);
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
  // Note that we do not support the case where expanded node has only
  // tree subnodes, since there's nothing to get a CItemData from.
  // This borderline case is hereby deemed more trouble than it's
  // worth to handle correctly.

  *pLResult = FALSE;

  if (!m_isRestoring) {
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNotifyStruct;
    // now find a leaf son and add it's CitemData to set.
    HTREEITEM child = GetChildItem(pNMTreeView->itemNew.hItem);
    ASSERT(child != NULL); // can't expand something w/o children, right?
    do {
      if (IsLeafNode(child)) {
        DWORD itemData = GetItemData(child);
        ASSERT(itemData != NULL);
        CItemData *ci = (CItemData *)itemData;
        if (pNMTreeView->action == TVE_EXPAND) {
          m_expandedItems.insert(ci);
          return; // stop at first leaf child found
        } else if (pNMTreeView->action == TVE_COLLAPSE) {
          // order may change, so we need to check each leaf
          if (m_expandedItems.find(ci) != m_expandedItems.end())
            m_expandedItems.erase(ci);
        }
      } // IsLeafNode
      child = GetNextSiblingItem(child);
    } while (child != NULL);
  } // !m_isRestoring
}

void CTVTreeCtrl::RestoreExpanded()
{
  m_isRestoring = true;
  SetTreeItem_t::iterator it;

  for (it = m_expandedItems.begin(); it != m_expandedItems.end(); it++) {
    CItemData *ci = *it;
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    HTREEITEM parent = GetParentItem(di->tree_item);
    Expand(parent, TVE_EXPAND);
  }
  m_isRestoring = false;
}

void CTVTreeCtrl::ClearExpanded()
{
  m_expandedItems.clear();
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
  } while((hItem = this->GetNextSiblingItem(hItem)) != NULL);
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
    } while((hItem = this->GetNextSiblingItem(hItem)) != NULL);
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

void CTVTreeCtrl::BeginDrag(NMHDR * /* pNotifyStruct */, LRESULT * &/* pLResult */)
{
  TRACE("CTVTreeCtrl::BeginDrag()\n");

  // Can drag in read-only mode as it might be to someone else
  // Can't allow drop in read-only mode

  CPoint ptAction;
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
  mf.Write((LPCTSTR)cs_text, cs_text.GetLength() * sizeof(TCHAR));
  mf.Write(buffer, lBufLen);

  // Finished with (encrypted) buffer - free it
  free(buffer);

  DWORD dw_mflen = (DWORD)mf.GetLength();
  BYTE * mf_buffer = (BYTE *)(mf.Detach());

#ifdef _DEBUG
   CString cs_timestamp;
   cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: Drag data: length %d/0x%04x, value:\n"), cs_timestamp, dw_mflen, dw_mflen);
   PWSUtil::HexDump(mf_buffer, dw_mflen, cs_timestamp);
#endif /* DEBUG */

  ASSERT(m_pDragImage == NULL);
  m_pDragImage = CreateDragImage(m_hitemDrag);

  // Get client rectangle
  RECT rClient;
  GetClientRect(&rClient);

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

BOOL CTVTreeCtrl::OnDrop(CWnd* /* pWnd */, COleDataObject* pDataObject,
                              DROPEFFECT dropEffect, CPoint point)
{
  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 
  if (dbx->IsMcoreReadOnly())
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

  BOOL retval(FALSE);

  // On Drop of data from one tree to another
  HGLOBAL hGlobal;

  hGlobal = pDataObject->GetGlobalData(gbl_tcddCPFID);
  LPCTSTR pData = (LPCTSTR)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  SIZE_T memsize = GlobalSize(hGlobal);

  if (memsize < DD_MEMORY_MINSIZE)
    goto exit;

  memset(m_sending_classname, 0, DD_CLASSNAME_SIZE + sizeof(TCHAR));
  memcpy(m_sending_classname, pData, DD_CLASSNAME_SIZE);
  m_bWithinThisInstance = memcmp(gbl_classname, m_sending_classname, DD_CLASSNAME_SIZE) == 0;

  // iDDType = D&D type FROMTREE or for column D&D only FROMCC, FROMHDR
  // lBufLen = Length of D&D data appended to this data
  int iDDType;
  long lBufLen;

#if _MSC_VER >= 1400
  _stscanf_s(pData + DD_CLASSNAME_SIZE/sizeof(TCHAR), _T("%02x%08x"), &iDDType, &lBufLen);
#else
  _stscanf(pData + DD_CLASSNAME_SIZE/sizeof(TCHAR), _T("%02x%08x"), &iDDType, &lBufLen);
#endif

  // Check if it is from another TreeCtrl?
  // - we don't accept drop from anything else
  if (iDDType != FROMTREE || ((long)memsize < (long)(DD_MEMORY_MINSIZE + lBufLen)))
    goto exit;

  if (m_hitemDrop == NULL && GetCount() == 0) {
    // Dropping on to an empty database
    CMyString DropGroup (_T(""));
    ProcessData((BYTE *)(pData + DD_CLASSNAME_SIZE + DD_REQUIRED_DATA_SIZE),
                lBufLen, DropGroup);
    SelectItem(GetRootItem());
    retval = TRUE;
    goto exit;
  }

  if (IsLeafNode(m_hitemDrop) || bForceRoot)
    m_hitemDrop = GetParentItem(m_hitemDrop);

  if (m_bWithinThisInstance) {
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
    ProcessData((BYTE *)(pData + DD_CLASSNAME_SIZE + DD_REQUIRED_DATA_SIZE),
                lBufLen, DropGroup);
    SelectItem(m_hitemDrop);
    retval = TRUE;
  }

  GetParent()->SetFocus();

exit:
  GlobalUnlock(hGlobal);

  if (retval == TRUE)
    dbx->SetChanged(DboxMain::Data);

  return retval;
}

void CTVTreeCtrl::CompleteMove()
{
  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 
  // If drag within instance - we have already done ths
  if (m_bWithinThisInstance)
    return;

  // If drag to another instance, ignore in Read-only mode
  if (dbx->IsMcoreReadOnly())
    return;

  // After we have dragged successfully from our Tree to another Tree
  dbx->Delete();
  dbx->RefreshList();
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
  CArchive ar_out(&outDDmemfile, CArchive::store);
  out_oblist.Serialize(ar_out);
  ar_out.Flush();
  ar_out.Close();

  outLen = (long)outDDmemfile.GetLength();
  out_buffer = (BYTE *)outDDmemfile.Detach();

  while (!out_oblist.IsEmpty()) {
    delete (CDDObject *)out_oblist.RemoveHead();
  } 

  return (outLen > 0);
}

bool CTVTreeCtrl::ProcessData(BYTE *in_buffer, const long &inLen, const CMyString DropGroup)
{
  DboxMain *dbx = static_cast<DboxMain *>(m_parent); 

#ifdef _DEBUG
   CString cs_timestamp;
   cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: Drop data: length %d/0x%04x, value:\n"), cs_timestamp, inLen, inLen);
   PWSUtil::HexDump(in_buffer, inLen, cs_timestamp);
#endif /* DEBUG */

  if (inLen <= 0)
    return false;

  CDDObList in_oblist;

  CSMemFile inDDmemfile;

  inDDmemfile.Attach((BYTE *)in_buffer, inLen);

  CArchive ar_in (&inDDmemfile, CArchive::load);
  in_oblist.Serialize(ar_in);
  ar_in.Close();

  inDDmemfile.Detach();

  if (!in_oblist.IsEmpty()) {
    dbx->AddEntries(in_oblist, DropGroup);

    while (!in_oblist.IsEmpty()) {
      delete (CDDObject *)in_oblist.RemoveHead();
    }
  }

  return (inLen > 0);
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
