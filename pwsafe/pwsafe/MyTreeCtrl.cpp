/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/*
 * Silly subclass of CTreeCtrl just to implement Drag&Drop.
 *
 */


#include "stdafx.h"
#include "MyTreeCtrl.h"
#include "DboxMain.h"
#include "DDSupport.h"
#include "corelib/ItemData.h"
#include "corelib/MyString.h"
#include "corelib/Util.h"
#include "corelib/Pwsprefs.h"
#include "corelib/SMemFile.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR GROUP_SEP = TCHAR('.');

/**
 * Following classes are used to "Fake" multiple inheritance:
 * Ideally, CMyTreeCtrl should derive from CTreeCtrl, COleDropTarget
 * and COleDropSource. However, since m'soft, in their infinite
 * wisdom, couldn't get this common use-case straight,
 * we use the following classes as proxies: CMyTreeCtrl
 * has a member var for each, registers said member appropriately
 * for D&D, and member calls parent's method to do the grunt work.
 */

class CPWTDropTarget : public COleDropTarget
{
 public:
 CPWTDropTarget(CMyTreeCtrl *parent) : m_tree(*parent) {}
  DROPEFFECT OnDragEnter(CWnd* pWnd , COleDataObject* pDataObject,
                         DWORD dwKeyState, CPoint point)
  {return m_tree.OnDragEnter(pWnd, pDataObject, dwKeyState, point);}
  DROPEFFECT OnDragOver(CWnd* pWnd , COleDataObject* pDataObject,
                        DWORD dwKeyState, CPoint point)
  {return m_tree.OnDragOver(pWnd, pDataObject, dwKeyState, point);}
  void OnDragLeave(CWnd*)
  {m_tree.OnDragLeave();}
  BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
              DROPEFFECT dropEffect, CPoint point)
  {return m_tree.OnDrop(pWnd, pDataObject, dropEffect, point);}
 private:
  CMyTreeCtrl &m_tree;
};

class CPWTDropSource : public COleDropSource
{
 public:
 CPWTDropSource(CMyTreeCtrl *parent) : m_tree(*parent) {}
  virtual SCODE GiveFeedback(DROPEFFECT dropEffect )
  {return m_tree.GiveFeedback(dropEffect);}
 private:
  CMyTreeCtrl &m_tree;
};

class CPWTDataSource : public COleDataSource
{
 public:
  CPWTDataSource(CMyTreeCtrl *parent, COleDropSource *ds)
    : m_tree(*parent), m_DropSource(ds) {}
  DROPEFFECT StartDragging(BYTE *szData, DWORD dwLength, CLIPFORMAT cpfmt,
                           RECT *rClient, CPoint *ptMousePos)
{
  HGLOBAL hgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLength);
  ASSERT(hgData != NULL);

  LPCSTR lpData = (LPCSTR)GlobalLock(hgData);
  ASSERT(lpData != NULL);

  memcpy((void *)lpData, szData, dwLength);
  CacheGlobalData(cpfmt, hgData);

  DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE,
                                     (LPCRECT)rClient, m_DropSource);
  return dropEffect;
}
 private:
  CMyTreeCtrl &m_tree;
  COleDropSource *m_DropSource;
};

/**
 * Impleemntat5ion of CMyTreeCtrl begins here
 */

CMyTreeCtrl::CMyTreeCtrl() : m_isRestoring(false)
{
  // Register a clipboard format for column drag & drop. 
  // Note that it's OK to register same format more than once:
  // "If a registered format with the specified name already exists,
  // a new format is not registered and the return value identifies the existing format."

  CString cs_CPF(MAKEINTRESOURCE(IDS_CPF_TVDD));
  m_tcddCPFID = (CLIPFORMAT)RegisterClipboardFormat(cs_CPF);
  ASSERT(m_tcddCPFID != 0);
  m_DropTarget = new CPWTDropTarget(this);
  m_DropSource = new CPWTDropSource(this);
  m_DataSource = new CPWTDataSource(this, m_DropSource);
}

CMyTreeCtrl::~CMyTreeCtrl()
{
  delete m_DropTarget;
  delete m_DropSource;
  delete m_DataSource;
}


BEGIN_MESSAGE_MAP(CMyTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CMyTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnExpandCollapse)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTreeItemSelected)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMyTreeCtrl::Initialize()
{
  m_DropTarget->Register(this);
}

void CMyTreeCtrl::OnDestroy()
{
  CImageList  *pimagelist;

  pimagelist = GetImageList(TVSIL_NORMAL);
  if (pimagelist != NULL) {
    pimagelist->DeleteImageList();
    delete pimagelist;
  }
  m_DropTarget->Revoke();
}

BOOL CMyTreeCtrl::PreTranslateMessage(MSG* pMsg) 
{
  // When an item is being edited make sure the edit control
  // receives certain important key strokes
  if (GetEditControl()) {
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE; // DO NOT process further
  }

  // F2 key -> begin in-place editing of an item
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F2) {
    HTREEITEM hItem = GetSelectedItem();
    if (hItem != NULL && !((DboxMain *)GetParent())->IsMcoreReadOnly())
      EditLabel(hItem);
    return TRUE;
  }

  // Let the parent class do its thing
  return CTreeCtrl::PreTranslateMessage(pMsg);
}

SCODE CMyTreeCtrl::GiveFeedback(DROPEFFECT de )
{
  // If user chooses copy, show d&d cursor with '+'
  // For move, ghost cursor's enough
  if ((de & DROPEFFECT_COPY) == DROPEFFECT_COPY) {
    return DRAGDROP_S_USEDEFAULTCURSORS;
  } else {
    return S_OK;
  }
}

DROPEFFECT CMyTreeCtrl::OnDragEnter(CWnd* , COleDataObject* ,
                                    DWORD dwKeyState, CPoint )
{
  return ((dwKeyState & MK_CONTROL) == MK_CONTROL) ?
    DROPEFFECT_COPY : DROPEFFECT_MOVE;
}

DROPEFFECT CMyTreeCtrl::OnDragOver(CWnd* pWnd , COleDataObject* /* pDataObject */,
                                   DWORD dwKeyState, CPoint point)
{
  if (this != pWnd) {
    TRACE(_T("Inter-process Drag&Drop"));
  }
  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);
  ASSERT(pil != NULL);

  pil->DragMove(point);
  // Expand and highlight the item under the mouse and 
  CMyTreeCtrl *pDestTreeCtrl = (CMyTreeCtrl *)pWnd;
  HTREEITEM hTItem = pDestTreeCtrl->HitTest(point);
  if (hTItem != NULL) {
    pil->DragLeave(this);
    pDestTreeCtrl->Expand(hTItem, TVE_EXPAND);
    pDestTreeCtrl->SelectDropTarget(hTItem);
    m_hitemDrop = hTItem;
    pil->DragEnter(this, point);
  }

  CRect rectClient;
  pWnd->GetClientRect(&rectClient);
  pWnd->ClientToScreen(rectClient);
  pWnd->ClientToScreen(&point);

  // Scroll Tree control depending on mouse position
  int iMaxV = GetScrollLimit(SB_VERT);
  int iPosV = GetScrollPos(SB_VERT);

  const int SCROLL_BORDER = 10;
  int nScrollDir = -1;
  if ((point.y > rectClient.bottom - SCROLL_BORDER) && (iPosV != iMaxV))
    nScrollDir = SB_LINEDOWN;
  else if ((point.y < rectClient.top + SCROLL_BORDER) && (iPosV != 0))
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
  else if ((point.x > rectClient.right - SCROLL_BORDER) && (iPosH != iMaxH))
    nScrollDir = SB_LINERIGHT;
  
  if (nScrollDir != -1) {
    int nScrollPos = pWnd->GetScrollPos(SB_VERT);
    WPARAM wParam = MAKELONG(nScrollDir, nScrollPos);
    pWnd->SendMessage(WM_HSCROLL, wParam);
  }
  
  DROPEFFECT dropeffectRet = ((dwKeyState & MK_CONTROL) == MK_CONTROL) ?
    DROPEFFECT_COPY : DROPEFFECT_MOVE;
  
  return dropeffectRet;
}

void CMyTreeCtrl::OnDragLeave()
{
  ShowCursor(TRUE);
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

void CMyTreeCtrl::OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pnmhdr;

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

void CMyTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  DboxMain *dbx = static_cast<DboxMain *>(GetParent());
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

  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pnmhdr;
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
      DboxMain *dbx = static_cast<DboxMain *>(GetParent());
      dbx->UpdateListItemTitle(lindex, newTitle);
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
  return (i != NODE);
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

void CMyTreeCtrl::DeleteFromSet(HTREEITEM hItem)
{
  DWORD itemData = GetItemData(hItem);
  ASSERT(itemData != NULL);
  CItemData *ci = (CItemData *)itemData;
  m_expandedItems.erase(ci);
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

  DboxMain *dbx = static_cast<DboxMain *>(GetParent());
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
    CMyString ci_title = ci_title0;
    if (dbx->Find(path, ci_title0, ci_user) != dbx->End()) {
      // Find a unique "Title"
      ItemListConstIter listpos;
      int i = 0;
      CString s_copy;
      do {
        i++;
        s_copy.Format(IDS_DRAGNUMBER, i);
        ci_title = ci_title0 + CMyString(s_copy);
        listpos = dbx->Find(path, ci_title, ci_user);
      } while (listpos != dbx->End());
    }

    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);

    // Update group and in the ListView
    ci->SetGroup(path);
    dbx->UpdateListItemGroup(di->list_index, (CString)path);

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

BOOL CMyTreeCtrl::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
                         DROPEFFECT dropEffect, CPoint point)
{
  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);
  ASSERT(pil != NULL);

  pil->DragLeave(this);
  pil->EndDrag();
  pil->DeleteImageList();

  HTREEITEM parent = GetParentItem(m_hitemDrag);
  if (IsLeafNode(m_hitemDrop))
    m_hitemDrop = GetParentItem(m_hitemDrop);

  if (m_hitemDrag != m_hitemDrop &&
      !IsChildNodeOf(m_hitemDrop, m_hitemDrag) &&
      parent != m_hitemDrop) {
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
  SelectDropTarget(NULL);
  return TRUE;
}


void CMyTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
  // This method is called when a drag action is detected.
  // It set the whole D&D mechanism in motion...
  CPoint      ptAction;

  NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  *pResult = 0;

  GetCursorPos(&ptAction);
  ScreenToClient(&ptAction);
  m_hitemDrag = pNMTreeView->itemNew.hItem;
  m_hitemDrop = NULL;
  SelectItem(m_hitemDrag);

  CImageList *pil = CreateDragImage(m_hitemDrag);
  pil->SetDragCursorImage(0, CPoint(0, 0));
  pil->BeginDrag(0, CPoint(0,0));
  pil->DragMove(ptAction);
  pil->DragEnter(this, ptAction);
  ShowCursor(FALSE);
  SetCapture();

  long lBufLen;
  BYTE *buffer = NULL;

  // Start of Drag of entries.....
  // CollectData allocates buffer - need to free later
  if (!CollectData(buffer, lBufLen))
    return;

  // If you want to encrypt the data - do it here and write out the
  // new encrypted buffer length here instead of lBufLen
  CString cs_text;
  cs_text.Format(_T("%08x%02x%02x"), GetCurrentProcessId(), FROMTREE, lBufLen);

  CMemFile mf;
  mf.Write(cs_text, cs_text.GetLength() * sizeof(TCHAR));

  // If the data has been encrypted - append it here to the "header"
  // instead of the clear text buffer
  mf.Write(buffer, lBufLen);

  // Finished with buffer - trash it and free it
  // If using an encrypted buffer - only need to free it
  trashMemory((void *)buffer, lBufLen);
  free(buffer);

  DWORD dw_mflen = (DWORD)mf.GetLength();
  BYTE *mf_buffer = (BYTE *)(mf.Detach());

#ifdef DUMP_DATA
   CString cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: Drag data: length %d/0x%04x, value:\n"), cs_timestamp, dw_mflen, dw_mflen);
   PWSUtil::HexDump(mf_buffer, dw_mflen, cs_timestamp);
#endif /* DUMP_DATA */

  // Get client rectangle
  RECT rClient;
  GetClientRect(&rClient);

  // Start dragging
  DROPEFFECT de = m_DataSource->StartDragging(mf_buffer, dw_mflen, m_tcddCPFID,
                                              &rClient, &ptAction);

  // Cleanup
  // Finished with buffer - trash it and free it
  // If using an encrypted buffer - only need to free it
  trashMemory((void *)mf_buffer, dw_mflen);
  free((void *)mf_buffer);

  if (SUCCEEDED(de)) { // If inter-process Move, we need to delete original
    // wrong place to clean up imagelist?
    pil->DragLeave(GetDesktopWindow());
    pil->EndDrag();
    pil->DeleteImageList();
    delete pil;
    ShowCursor(TRUE);
  } else {
    TRACE(_T("m_DataSource->StartDragging() failed"));
  }
}

void CMyTreeCtrl::OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0L;
  NMTREEVIEW *ptv = (NMTREEVIEW *)pNotifyStruct;
  HTREEITEM hti = ptv->itemNew.hItem;
  if (hti != NULL) {
    CItemData *ci = (CItemData *)GetItemData(hti);
    static_cast<DboxMain *>(GetParent())->UpdateToolBarForSelectedItem(ci);
  }
}

void CMyTreeCtrl::OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *)
{
  // The hItem that is expanded isn't the one that will be restored,
  // since the tree is rebuilt in DboxMain::RefreshList. Therefore, we
  // need to store the corresponding elements. But groups have none, so
  // we store the first (or any) child element, and upon restore, expand
  // the parent. Ugh++.
  // Note that we do not support thecase where expanded node has only
  // tree subnodes, since there's nothing to get a CItemData from.
  // This borderline case is hereby deemed more trouble than it's
  // worth to handle correctly.

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

void CMyTreeCtrl::RestoreExpanded()
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

void CMyTreeCtrl::ClearExpanded()
{
  m_expandedItems.clear();
}

void CMyTreeCtrl::OnExpandAll() 
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

void CMyTreeCtrl::OnCollapseAll() 
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

void CMyTreeCtrl::CollapseBranch(HTREEITEM hItem)
{
	// Courtesy of Zafir Anjumfrom www.codeguru.com
	if(ItemHasChildren(hItem)) {
		Expand(hItem, TVE_COLLAPSE);
		hItem = GetChildItem(hItem);
		do {
			CollapseBranch(hItem);
		} while((hItem = GetNextSiblingItem(hItem)) != NULL);
	}
}

HTREEITEM
CMyTreeCtrl::GetNextTreeItem(HTREEITEM hItem) 
{
	if (NULL == hItem)
		return GetRootItem(); 

  // First, try to go to this item's 1st child 
  HTREEITEM hReturn = GetChildItem(hItem); 

  // If no more child items... 
  while (hItem && !hReturn) { 
    // Get this item's next sibling 
    hReturn = GetNextSiblingItem(hItem); 

    // If hReturn is NULL, then there are no 
    // sibling items, and we're on a leaf node. 
    // Backtrack up the tree one level, and 
    // we'll look for a sibling on the next 
    // iteration (or we'll reach the root and 
    // quit). 
    hItem = GetParentItem(hItem); 
  }
  return hReturn;
} 

bool CMyTreeCtrl::CollectData(BYTE * &out_buffer, long &outLen)
{
  DWORD itemData = GetItemData(m_hitemDrag);
  CItemData *ci = (CItemData *)itemData;

  CDDObList out_oblist;

  if (IsLeafNode(m_hitemDrag)) {
    ASSERT(itemData != NULL);
    m_nDragPathLen = 0;
    out_oblist.m_bDragNode = false;
    GetEntryData(out_oblist, ci);
  } else {
    m_nDragPathLen = GetGroup(GetParentItem(m_hitemDrag)).GetLength();
    out_oblist.m_bDragNode = true;
    GetGroupEntriesData(out_oblist, m_hitemDrag);
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

bool CMyTreeCtrl::ProcessData(BYTE *in_buffer, const long &inLen, const CMyString &DropGroup)
{
  DboxMain *pDbx = static_cast<DboxMain *>(GetParent()); 

#ifdef DUMP_DATA
   CString cs_timestamp;
   cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: Drop data: length %d/0x%04x, value:\n"), cs_timestamp, inLen, inLen);
   PWSUtil::HexDump(in_buffer, inLen, cs_timestamp);
#endif /* DUMP_DATA */

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
    pDbx->AddEntries(in_oblist, DropGroup);

    while (!in_oblist.IsEmpty()) {
      delete (CDDObject *)in_oblist.RemoveHead();
    }
  }

  return (inLen > 0);
}

void
CMyTreeCtrl::GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem)
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
CMyTreeCtrl::GetEntryData(CDDObList &out_oblist, CItemData *ci)
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
