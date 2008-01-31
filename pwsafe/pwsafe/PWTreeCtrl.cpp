/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "PWTreeCtrl.h"
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

// following header for D&D data passed over OLE:
// Process ID of sender (to determine if src == tgt)
// Type of data
// Length of actual payload, in bytes.
static const char *OLE_HDR_FMT = "%08x%02x%08x";
static const int OLE_HDR_LEN = 18;

/**
* Following classes are used to "Fake" multiple inheritance:
* Ideally, CPWTreeCtrl should derive from CTreeCtrl, COleDropTarget
* and COleDropSource. However, since m'soft, in their infinite
* wisdom, couldn't get this common use-case straight,
* we use the following classes as proxies: CPWTreeCtrl
* has a member var for each, registers said member appropriately
* for D&D, and member calls parent's method to do the grunt work.
*/

class CPWTDropTarget : public COleDropTarget
{
public:
  CPWTDropTarget(CPWTreeCtrl *parent) : m_tree(*parent) {}

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
  CPWTreeCtrl &m_tree;
};

class CPWTDropSource : public COleDropSource
{
public:
  CPWTDropSource(CPWTreeCtrl *parent) : m_tree(*parent) {}

  virtual SCODE GiveFeedback(DROPEFFECT dropEffect )
  {return m_tree.GiveFeedback(dropEffect);}

private:
  CPWTreeCtrl &m_tree;
};

class CPWTDataSource : public COleDataSource
{
public:
  CPWTDataSource(CPWTreeCtrl *parent, COleDropSource *ds)
    : m_tree(*parent), m_DropSource(ds),
    m_mfBuffer(NULL), m_mfBufLen(0), m_hgData(NULL) {}
  DROPEFFECT StartDragging(CLIPFORMAT cpfmt, LPCRECT rClient)
  {
    DelayRenderData(cpfmt);
    DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE,
                                       rClient, m_DropSource);
    // Cleanup:
    // It is not clear if we can cleanup m_mfBuffer
    // in OnRenderGlobalData, so better to err
    // on the side of safety, and cleanup after
    // DoDragDrop has completed.

    if (m_mfBufLen != 0) {
      trashMemory((void *)m_mfBuffer, m_mfBufLen);
      free((void *)m_mfBuffer);
      m_mfBuffer = NULL; m_mfBufLen = 0;
    }
    if (m_hgData != NULL) {
      GlobalUnlock(m_hgData);
      GlobalFree(m_hgData); m_hgData = NULL;
    }
    return dropEffect;
  }
  virtual BOOL OnRenderGlobalData(LPFORMATETC , HGLOBAL* phGlobal)
  {
    long lBufLen;
    BYTE *buffer = NULL;

    ASSERT(m_mfBuffer == NULL && m_mfBufLen == 0);
    ASSERT(m_hgData == NULL);
    // CollectData allocates buffer - need to free later
    if (!m_tree.CollectData(buffer, lBufLen))
      return FALSE;

    char header[OLE_HDR_LEN+1];
#if _MSC_VER >= 1400
    sprintf_s(header, sizeof(header),
              OLE_HDR_FMT, GetCurrentProcessId(), FROMTREE, lBufLen);
#else
    sprintf(header, OLE_HDR_FMT, GetCurrentProcessId(), FROMTREE, lBufLen);
#endif
    CMemFile mf;
    mf.Write(header, OLE_HDR_LEN);
    mf.Write(buffer, lBufLen);

    // Finished with buffer - trash it and free it
    trashMemory((void *)buffer, lBufLen);
    free(buffer);

    m_mfBufLen = (DWORD)mf.GetLength();
    m_mfBuffer = (BYTE *)(mf.Detach());

#ifdef DUMP_DATA
    CString cs_timestamp = PWSUtil::GetTimeStamp();
    TRACE(_T("%s: Drag data: length %d/0x%04x, value:\n"), cs_timestamp,
          m_mfBufLen, m_mfBufLen);
    PWSUtil::HexDump(m_mfBuffer, m_mfBufLen, cs_timestamp);
#endif /* DUMP_DATA */


    m_hgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, m_mfBufLen);
    ASSERT(m_hgData != NULL);

    LPCSTR lpData = (LPCSTR)GlobalLock(m_hgData);
    ASSERT(lpData != NULL);

    memcpy((void *)lpData, m_mfBuffer, m_mfBufLen);

    if (*phGlobal == NULL) {
      *phGlobal = m_hgData;
    } else {
      SIZE_T inSize = GlobalSize(*phGlobal);
      SIZE_T ourSize = GlobalSize(m_hgData);
      if (inSize < ourSize)
        return FALSE;

      ASSERT(0); // not ready to handle this yet. Is it possible in our case?
    }
    return TRUE;
  }

private:
  CPWTreeCtrl &m_tree;
  COleDropSource *m_DropSource;
  BYTE *m_mfBuffer;
  DWORD m_mfBufLen;
  HGLOBAL m_hgData;
};

/**
* Implementation of CPWTreeCtrl begins here
*/

CPWTreeCtrl::CPWTreeCtrl()
  : m_isRestoring(false), m_bWithinThisInstance(true)
{
  // Register a clipboard format for column drag & drop. 
  // Note that it's OK to register same format more than once:
  // "If a registered format with the specified name already exists,
  // a new format is not registered and the return value identifies the existing format."

  CString cs_CPF(MAKEINTRESOURCE(IDS_CPF_TVDD));
  m_tcddCPFID = (CLIPFORMAT)RegisterClipboardFormat(cs_CPF);
  ASSERT(m_tcddCPFID != 0);

  // instantiate "proxy" objects for D&D.
  // The members are currently pointers mainly to hide
  // their implementation from the header file. If this changes,
  // e.g., if we make them nested classes, then they should
  // be non-pointers.
  m_DropTarget = new CPWTDropTarget(this);
  m_DropSource = new CPWTDropSource(this);
  m_DataSource = new CPWTDataSource(this, m_DropSource);
}

CPWTreeCtrl::~CPWTreeCtrl()
{
  // see comment in constructor re these member variables
  delete m_DropTarget;
  delete m_DropSource;
  delete m_DataSource;
}

BEGIN_MESSAGE_MAP(CPWTreeCtrl, CTreeCtrl)
  //{{AFX_MSG_MAP(CPWTreeCtrl)
  ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
  ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnExpandCollapse)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTreeItemSelected)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWTreeCtrl::Initialize()
{
  // This should really be in OnCreate(), but for some reason,
  // was never called.
  m_DropTarget->Register(this);
}

void CPWTreeCtrl::OnDestroy()
{
  CImageList *pimagelist = GetImageList(TVSIL_NORMAL);
  if (pimagelist != NULL) {
    pimagelist->DeleteImageList();
    delete pimagelist;
  }
  m_DropTarget->Revoke();
}

BOOL CPWTreeCtrl::PreTranslateMessage(MSG* pMsg) 
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

SCODE CPWTreeCtrl::GiveFeedback(DROPEFFECT )
{
  DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
  pDbx->ResetIdleLockCounter();
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

DROPEFFECT CPWTreeCtrl::OnDragEnter(CWnd* , COleDataObject* ,
                                    DWORD dwKeyState, CPoint )
{
  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);
  if (pil != NULL) {
    while (ShowCursor(FALSE) >= 0)
      ;
  }
  m_bWithinThisInstance = true;
  return ((dwKeyState & MK_CONTROL) == MK_CONTROL) ? 
         DROPEFFECT_COPY : DROPEFFECT_MOVE;
}

DROPEFFECT CPWTreeCtrl::OnDragOver(CWnd* pWnd , COleDataObject* /* pDataObject */,
                                   DWORD dwKeyState, CPoint point)
{
  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);

  if (pil != NULL) pil->DragMove(point);

  // Expand and highlight the item under the mouse and 
  CPWTreeCtrl *pDestTreeCtrl = (CPWTreeCtrl *)pWnd;
  HTREEITEM hTItem = pDestTreeCtrl->HitTest(point);
  if (hTItem != NULL) {
    if (pil != NULL) pil->DragLeave(this);
    pDestTreeCtrl->Expand(hTItem, TVE_EXPAND);
    pDestTreeCtrl->SelectDropTarget(hTItem);
    m_hitemDrop = hTItem;
    if (pil != NULL) pil->DragEnter(this, point);
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

  DROPEFFECT dropeffectRet;
  // If we're dragging between processes, default is to COPY, Ctrl key
  // changes this to MOVE.
  // If we're dragging in the same process, default is to MOVE, Ctrl
  // key changes this to COPY
  if (pil == NULL)
    dropeffectRet = ((dwKeyState & MK_CONTROL) == MK_CONTROL) ? 
                    DROPEFFECT_MOVE : DROPEFFECT_COPY;
  else
    dropeffectRet = ((dwKeyState & MK_CONTROL) == MK_CONTROL) ? 
                    DROPEFFECT_COPY : DROPEFFECT_MOVE;

  return dropeffectRet;
}

void CPWTreeCtrl::OnDragLeave()
{
  m_bWithinThisInstance = false;
  // ShowCursor's semantics are VERY odd - RTFM
  while (ShowCursor(TRUE) < 0)
    ;
}

void CPWTreeCtrl::SetNewStyle(long lStyleMask, BOOL bSetBits)
{
  long lStyleOld = GetWindowLong(m_hWnd, GWL_STYLE);

  lStyleOld &= ~lStyleMask;
  if (bSetBits)
    lStyleOld |= lStyleMask;

  SetWindowLong(m_hWnd, GWL_STYLE, lStyleOld);
  SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CPWTreeCtrl::UpdateLeafsGroup(HTREEITEM hItem, CString prefix)
{
  // Starting with hItem, update the Group field of all of hItem's
  // children. Called after a label has been edited.
  if (IsLeaf(hItem)) {
    DWORD_PTR itemData = GetItemData(hItem);
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

void CPWTreeCtrl::OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
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

  if (IsLeaf(ti)) {
    DWORD_PTR itemData = GetItemData(ti);
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

void CPWTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
  if (pDbx->IsMcoreReadOnly())
    return; // don't edit in read-only mode

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
    if (IsLeaf(ptvinfo->item.hItem)) {
      DWORD_PTR itemData = GetItemData(ti);
      ASSERT(itemData != NULL);
      CItemData *ci = (CItemData *)itemData;
      CString group, newTitle, newUser, newPassword;

      if (!splitLeafText(ptvinfo->item.pszText, newTitle, newUser, newPassword)) {
        // errors in user's input - restore text and refresh display
        goto bad_exit;
      }

      group = CString(ci->GetGroup());
      if (pDbx->Find(group, newTitle, newUser) != pDbx->End()) {
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
      DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
      pDbx->UpdateListItemTitle(lindex, newTitle);
      if (bShowUsernameInTree) {
        ci->SetUser(newUser);
        pDbx->UpdateListItemUser(lindex, newUser);
        if (bShowPasswordInTree) {
          ci->SetPassword(newPassword);
          pDbx->UpdateListItemPassword(lindex, newPassword);
        }
      }
    } else {
      // Update all leaf children with new path element
      // prefix is path up to and NOT including renamed node
      CString prefix;
      HTREEITEM parent, current = ti;
      do {
        parent = GetParentItem(current);
        if (parent == NULL)
          break;

        current = parent;
        if (!prefix.IsEmpty())
          prefix = GROUP_SEP + prefix;
        prefix = GetItemText(current) + prefix;
      } while (1);
      UpdateLeafsGroup(ti, prefix);
    }
    // Mark database as modified
    pDbx->SetChanged(DboxMain::Data);

    // put edited text in right order by sorting
    SortTree(GetParentItem(ti));

    // OK
    *pLResult = TRUE;
    return;
  }

bad_exit:
  // Refresh display to show old text - if we don't no one else will
  pDbx->RefreshViews();
  // restore text
  // (not that this is documented anywhere in MS's docs...)
  *pLResult = FALSE;
}

bool CPWTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
  do {
    if (hitemChild == hitemSuspectedParent)
      break;
  } while ((hitemChild = GetParentItem(hitemChild)) != NULL);

  return (hitemChild != NULL);
}

bool CPWTreeCtrl::IsLeaf(HTREEITEM hItem)
{
  // ItemHasChildren() won't work in the general case
  int i, dummy;
  BOOL status = GetItemImage(hItem, i, dummy);
  ASSERT(status);
  return (i != NODE);
}

void CPWTreeCtrl::DeleteWithParents(HTREEITEM hItem)
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
CString CPWTreeCtrl::GetGroup(HTREEITEM hItem)
{
  CString retval, nodeText;
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

HTREEITEM CPWTreeCtrl::AddGroup(const CString &group)
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
        SetItemImage(ti, CPWTreeCtrl::NODE, CPWTreeCtrl::NODE);
      } else
        ti = si;
    } while (!path.IsEmpty());
  }
  return ti;
}

bool CPWTreeCtrl::MoveItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
  TV_INSERTSTRUCT  tvstruct;
  TCHAR sztBuffer[260];  // max visible

  DboxMain *pDbx = static_cast<DboxMain *>(GetParent()); 

  tvstruct.item.hItem = hitemDrag;
  tvstruct.item.cchTextMax = sizeof(sztBuffer)/sizeof(TCHAR) - 1;
  tvstruct.item.pszText = sztBuffer;
  tvstruct.item.mask = (TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE |
                        TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT);
  GetItem(&tvstruct.item);  // get information of the dragged element

  LPARAM itemData = tvstruct.item.lParam;
  tvstruct.hParent = hitemDrop;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree)) 
    tvstruct.hInsertAfter = TVI_LAST;
  else
    tvstruct.hInsertAfter = TVI_SORT;

  tvstruct.item.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  HTREEITEM hNewItem = InsertItem(&tvstruct);
  if (itemData != 0) { // Non-NULL itemData implies Leaf
    CItemData *ci = (CItemData *)itemData;

    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    ASSERT(di->list_index >= 0);

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
    CMyString ci_title = pDbx->GetUniqueTitle(path, ci_title0, ci_user, IDS_DRAGNUMBER);

    // Update list field with new group
    ci->SetGroup(path);
    pDbx->UpdateListItemGroup(di->list_index, (CString)path);

    if (ci_title.Compare(ci_title0) != 0) {
      ci->SetTitle(ci_title);
    }
    // Update tree label
    SetItemText(hNewItem, MakeTreeDisplayString(*ci));
    // Update list field with new title
    pDbx->UpdateListItemTitle(di->list_index, (CString)ci_title);

    // Mark database as modified!
    pDbx->SetChanged(DboxMain::Data);
    // Update DisplayInfo record associated with ItemData
    di->tree_item = hNewItem;
  } // leaf processing

  HTREEITEM hFirstChild;
  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    MoveItem(hFirstChild, hNewItem);  // recursively move all the items
  }

  // We are moving it - so now delete original from TreeCtrl
  // Why can't MoveItem & CopyItem be the same except for the following line?!?
  // For one, if we don't delete, we'l get into an infinite recursion.
  DeleteItem(hitemDrag);
  return true;
}

bool CPWTreeCtrl::CopyItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop,
                           const CMyString &prefix)
{
  DWORD_PTR itemData = GetItemData(hitemDrag);

  if (itemData == 0) { // we're dragging a group
    HTREEITEM hChild = GetChildItem(hitemDrag);

    while (hChild != NULL) {
      CopyItem(hChild, hitemDrop, prefix);
      hChild = GetNextItem(hChild, TVGN_NEXT);
    }
  } else { // we're dragging a leaf
    CItemData *ci = (CItemData *)itemData;
    CItemData temp(*ci); // copy construct a duplicate

    // Update Group: chop away prefix, replace
    CMyString oldPath(temp.GetGroup());
    if (!prefix.IsEmpty()) {
      oldPath = oldPath.Right(oldPath.GetLength() - prefix.GetLength() - 1);
    }
    // with new path
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

    CMyString newPath;
    if (path.IsEmpty())
      newPath = oldPath;
    else {
      newPath = path;
      if (!oldPath.IsEmpty())
        newPath += GROUP_SEP + oldPath;
    }
    // Get information from current selected entry
    DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
    CMyString ci_user = ci->GetUser();
    CMyString ci_title0 = ci->GetTitle();
    CMyString ci_title = pDbx->GetUniqueTitle(newPath, ci_title0, ci_user, IDS_DRAGNUMBER);

    // Needs new UUID as they must be unique and this is a copy operation
    temp.CreateUUID();
    temp.SetGroup(newPath);
    temp.SetTitle(ci_title);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    temp.SetDisplayInfo(ndi);

    pDbx->AddEntry(temp);

    // Mark database as modified!
    pDbx->SetChanged(DboxMain::Data);
  } // leaf handling
  return true;
}

BOOL CPWTreeCtrl::OnDrop(CWnd* , COleDataObject* pDataObject,
                         DROPEFFECT dropEffect, CPoint point)
{
  while (ShowCursor(TRUE) < 0)
    ;
  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);
  // pil will be NULL if we're the target of inter-process D&D

  if (pil != NULL) {
    pil->DragLeave(this);
    pil->EndDrag();
    pil->DeleteImageList();
  }

  DboxMain *pDbx = static_cast<DboxMain *>(GetParent()); 
  if (pDbx->IsMcoreReadOnly())
    return FALSE; // don't drop in read-only mode

  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL))
    return FALSE;

  UINT uFlags;
  HTREEITEM hitemDrop = HitTest(point, &uFlags);

  bool bForceRoot(false);
  switch (uFlags) {
    case TVHT_ABOVE: case TVHT_BELOW: case TVHT_TOLEFT: case TVHT_TORIGHT:
      return FALSE;
    case TVHT_NOWHERE:
      if (hitemDrop == NULL) {
        // Treat as drop in root
        hitemDrop = GetRootItem();
        bForceRoot = true;
      } else
        return FALSE;
      break;
    case TVHT_ONITEM: case TVHT_ONITEMBUTTON: case TVHT_ONITEMICON:
    case TVHT_ONITEMINDENT: case TVHT_ONITEMLABEL: case TVHT_ONITEMRIGHT:
    case TVHT_ONITEMSTATEICON:
      if (hitemDrop == NULL)
        return FALSE;
      break;
    default:
      return FALSE;
  }

  BOOL retval(FALSE);

  // On Drop of data from one tree to another
  HGLOBAL hGlobal = pDataObject->GetGlobalData(m_tcddCPFID);
  BYTE *pData = (BYTE *)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  SIZE_T memsize = GlobalSize(hGlobal);

  if (memsize < OLE_HDR_LEN) // OLE_HDR_FMT 
    goto exit;

  // iDDType = D&D type FROMTREE or for column D&D only FROMCC, FROMHDR
  // lBufLen = Length of D&D data appended to this data
  unsigned long lPid;
  int iDDType;
  long lBufLen;

#if _MSC_VER >= 1400
  sscanf_s((char *)pData, OLE_HDR_FMT, &lPid, &iDDType, &lBufLen);
#else
  sscanf((char *)pData, OLE_HDR_FMT, &lPid, &iDDType, &lBufLen);
#endif
  pData += OLE_HDR_LEN; // so ProcessData won't sweat

  // NULL-ness of pil is also a good indicator of intra/inter-ness
  // alternately, we can also raise an m_flag in OnBeginDrag.
  // However, plugging the process ID in the header
  // is the most direct and straightforward way to do this,
  // and probably the most robust...
  m_bWithinThisInstance = (lPid == GetCurrentProcessId());

  // Check if it is from another TreeCtrl?
  // - we don't accept drop from anything else
  if (iDDType != FROMTREE)
    goto exit;

  if (hitemDrop == NULL && GetCount() == 0) {
    // Dropping on to an empty database
    CMyString DropGroup (_T(""));
    ProcessData(pData, lBufLen, DropGroup);
    SelectItem(GetRootItem());
    retval = TRUE;
    goto exit;
  }

  if (IsLeaf(hitemDrop) || bForceRoot)
    hitemDrop = GetParentItem(hitemDrop);

  if (m_bWithinThisInstance) {
    // from me! - easy
    HTREEITEM parent = GetParentItem(m_hitemDrag);
    if (m_hitemDrag != hitemDrop &&
      !IsChildNodeOf(hitemDrop, m_hitemDrag) &&
      parent != hitemDrop) {
        // drag operation allowed
        if (dropEffect == DROPEFFECT_MOVE) {
          MoveItem(m_hitemDrag, hitemDrop);
        } else if (dropEffect == DROPEFFECT_COPY) {
          CopyItem(m_hitemDrag, hitemDrop, GetPrefix(m_hitemDrag));
          SortTree(hitemDrop);
        }
        SelectItem(hitemDrop);
        retval = TRUE;
    } else {
      // drag failed or cancelled, revert to last selected
      SelectItem(m_hitemDrag);
      goto exit;
    }
  } else { // from someone else!
    // Now add it
    CMyString DropGroup = CMyString(GetGroup(hitemDrop));
    ProcessData((BYTE *)pData, lBufLen, DropGroup);
    SelectItem(hitemDrop);
    retval = TRUE;
  }

  SortTree(TVI_ROOT);
  pDbx->FixListIndexes();
  GetParent()->SetFocus();

exit:
  GlobalUnlock(hGlobal);
  if (retval == TRUE)
    pDbx->SetChanged(DboxMain::Data);
  return retval;
}

void CPWTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
  // This method is called when a drag action is detected.
  // It set the whole D&D mechanism in motion...
  CPoint ptAction;

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
  while (ShowCursor(FALSE) >= 0)
    ;
  SetCapture();

  RECT rClient;
  GetClientRect(&rClient);

  // Start dragging
  DROPEFFECT de = m_DataSource->StartDragging(m_tcddCPFID, &rClient);

  if (SUCCEEDED(de)) {
    DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
    // If inter-process Move, we need to delete original
    if ((de & DROPEFFECT_MOVE) == DROPEFFECT_MOVE &&
        !m_bWithinThisInstance && !pDbx->IsMcoreReadOnly()) {
      pDbx->Delete();
    }
    // wrong place to clean up imagelist?
    pil->DragLeave(GetDesktopWindow());
    pil->EndDrag();
    pil->DeleteImageList();
    delete pil;
    while (ShowCursor(TRUE) < 0)
      ;
  } else {
    TRACE(_T("m_DataSource->StartDragging() failed"));
  }
}

void CPWTreeCtrl::OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *)
{
  HTREEITEM hti = GetDropHilightItem();
  if (hti != NULL)
    SetItemState(hti, 0, TVIS_DROPHILITED);

  NMTREEVIEW *ptv = (NMTREEVIEW *)pNotifyStruct;
  hti = ptv->itemNew.hItem;
  if (hti != NULL) {
    CItemData *ci = (CItemData *)GetItemData(hti);
    static_cast<DboxMain *>(GetParent())->UpdateToolBarForSelectedItem(ci);
  }
}

void CPWTreeCtrl::OnExpandCollapse(NMHDR *, LRESULT *)
{
  // We need to update the parent's state vector of expanded nodes
  // so that it will be persistent across miminize, lock, save, etc.
  // (unless we're in the middle of restoring the state!)

  if (!m_isRestoring) {
    DboxMain *pDbx = static_cast<DboxMain *>(GetParent()); 
    pDbx->SaveDisplayStatus();
  }
}

void CPWTreeCtrl::OnExpandAll() 
{
  // Updated to test for zero entries!
  HTREEITEM hItem = this->GetRootItem();
  if (hItem == NULL)
    return;
  SetRedraw(FALSE);
  do {
    Expand(hItem,TVE_EXPAND);
    hItem = GetNextItem(hItem,TVGN_NEXTVISIBLE);
  } while (hItem);
  EnsureVisible(GetSelectedItem());
  SetRedraw(TRUE);
}

void CPWTreeCtrl::OnCollapseAll() 
{
  // Courtesy of Zafir Anjum from www.codeguru.com
  // Updated to test for zero entries!
  HTREEITEM hItem = GetRootItem();
  if (hItem == NULL)
    return;
  SetRedraw(FALSE);
  do {
    CollapseBranch(hItem);
  } while((hItem = GetNextSiblingItem(hItem)) != NULL);
  SetRedraw(TRUE);
}

void CPWTreeCtrl::CollapseBranch(HTREEITEM hItem)
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

HTREEITEM CPWTreeCtrl::GetNextTreeItem(HTREEITEM hItem) 
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

bool CPWTreeCtrl::CollectData(BYTE * &out_buffer, long &outLen)
{
  DWORD_PTR itemData = GetItemData(m_hitemDrag);
  CItemData *ci = (CItemData *)itemData;

  CDDObList out_oblist;

  if (IsLeaf(m_hitemDrag)) {
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
  out_oblist.DDSerialize(outDDmemfile);

  outLen = (long)outDDmemfile.GetLength();
  out_buffer = (BYTE *)outDDmemfile.Detach();

  while (!out_oblist.IsEmpty()) {
    delete (CDDObject *)out_oblist.RemoveHead();
  } 

  return (outLen > 0);
}

bool CPWTreeCtrl::ProcessData(BYTE *in_buffer, const long &inLen, const CMyString &DropGroup)
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

  in_oblist.DDUnSerialize(inDDmemfile);

  inDDmemfile.Detach();

  if (!in_oblist.IsEmpty()) {
    pDbx->AddEntries(in_oblist, DropGroup);

    while (!in_oblist.IsEmpty()) {
      delete (CDDObject *)in_oblist.RemoveHead();
    }
  }
  return (inLen > 0);
}

void CPWTreeCtrl::GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem)
{
  if (IsLeaf(hItem)) {
    DWORD_PTR itemData = GetItemData(hItem);
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

void CPWTreeCtrl::GetEntryData(CDDObList &out_oblist, CItemData *ci)
{
  ASSERT(ci != NULL);
  CDDObject *pDDObject = new CDDObject;

  if (out_oblist.m_bDragNode && m_nDragPathLen > 0) {
    CItemData ci2(*ci); // we need a copy since to modify the group
    const CMyString cs_Group = ci->GetGroup();
    ci2.SetGroup(cs_Group.Right(cs_Group.GetLength() - m_nDragPathLen - 1));
    pDDObject->FromItem(ci2);
  } else {
    pDDObject->FromItem(*ci);
  }

  if (ci->IsAlias()) {
    // I'm an alias; pass on ptr to my base item to retrieve its group/title/user
    CItemData *cibase(NULL);
    uuid_array_t base_uuid, entry_uuid;
    ci->GetUUID(entry_uuid);
    DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
    pDbx->GetAliasBaseUUID(entry_uuid, base_uuid);
    ItemListIter iter = pDbx->Find(base_uuid);
    ASSERT(iter != pDbx->End());
    cibase = &(iter->second);
    pDDObject->SetBaseItem(cibase);
  } else
    if (ci->IsShortcut()) {
      // I'm a shortcut; pass on ptr to my base item to retrieve its group/title/user
      CItemData *cibase(NULL);
      uuid_array_t base_uuid, entry_uuid;
      ci->GetUUID(entry_uuid);
      DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
      pDbx->GetShortcutBaseUUID(entry_uuid, base_uuid);
      ItemListIter iter = pDbx->Find(base_uuid);
      ASSERT(iter != pDbx->End());
      cibase = &(iter->second);
      pDDObject->SetBaseItem(cibase);
    }

    out_oblist.AddTail(pDDObject);
}

CMyString CPWTreeCtrl::GetPrefix(HTREEITEM hItem) const
{
  // return all path components beween hItem and root.
  // e.g., if hItem is X in a.b.c.X.y.z, then return a.b.c
  CMyString retval;
  HTREEITEM p = GetParentItem(hItem);
  while ( p != NULL) {
    retval = CMyString(GetItemText(p)) + retval;
    p = GetParentItem(p);
    if (p != NULL)
      retval = CMyString(GROUP_SEP) + retval;
  }
  return retval;
}

CMyString CPWTreeCtrl::MakeTreeDisplayString(const CItemData &ci) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);

  CMyString treeDispString = ci.GetTitle();
  if (bShowUsernameInTree) {
    treeDispString += _T(" [");
    treeDispString += ci.GetUser();
    treeDispString += _T("]");

    if (bShowPasswordInTree) {
      treeDispString += _T(" {");
      treeDispString += ci.GetPassword();
      treeDispString += _T("}");
    }
  }
  return treeDispString;
}

static int CALLBACK ExplorerCompareProc(LPARAM lParam1, LPARAM lParam2,
                                        LPARAM /* closure */)
{
  int iResult;
  CItemData* pLHS = (CItemData *)lParam1;
  CItemData* pRHS = (CItemData *)lParam2;
  if (pLHS == pRHS) { // probably both null, in any case, equal
    iResult = 0;
  } else if (pLHS == NULL) {
    iResult = -1;
  } else if (pRHS == NULL) {
    iResult = 1;
  } else {

    iResult = (pLHS->GetGroup()).CompareNoCase(pRHS->GetGroup());
    if (iResult == 0) {
      iResult = (pLHS->GetTitle()).CompareNoCase(pRHS->GetTitle());
      if (iResult == 0) {
        iResult = (pLHS->GetUser()).CompareNoCase(pRHS->GetUser());
      }
    }
  }
  return iResult;
}

void CPWTreeCtrl::SortTree(const HTREEITEM htreeitem)
{
  TVSORTCB tvs;
  HTREEITEM hti(htreeitem);

  if (hti == NULL)
    hti = TVI_ROOT;

  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree)) {
    SortChildren(hti);
    return;
  }

  // here iff user prefers "explorer type view", that is,
  // groups first.


  // unbelievable, but we have to recurse ourselves!
  // foreach child of hti
  //  if !IsLeaf
  //   SortTree(child)
  if (hti == TVI_ROOT || !IsLeaf(hti)) {
    HTREEITEM hChildItem = GetChildItem(hti);

    while (hChildItem != NULL) {
      if (ItemHasChildren(hChildItem))
        SortTree(hChildItem);
      hChildItem = GetNextItem(hChildItem, TVGN_NEXT);
    }
  }

  tvs.hParent = hti;
  tvs.lpfnCompare = ExplorerCompareProc;
  tvs.lParam = (LPARAM)this;

  SortChildrenCB(&tvs);
}
