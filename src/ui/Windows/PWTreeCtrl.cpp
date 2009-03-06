/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
*  Use of CInfoDisplay to replace MS's broken ToolTips support.
*  Based on CInfoDisplay class taken from Asynch Explorer by
*  Joseph M. Newcomer [MVP]; http://www.flounder.com
*  Additional enhancements to the use of this code have been made to 
*  allow for delayed showing of the display and for a limited period.
*/

#include "stdafx.h"
#include "PWTreeCtrl.h"
#include "DboxMain.h"
#include "DDSupport.h"
#include "InfoDisplay.h"
#include "SecString.h"
#include "SMemFile.h"

#include "corelib/ItemData.h"
#include "corelib/Util.h"
#include "corelib/Pwsprefs.h"

#include "os/debug.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Hover time of 1.5 seconds before expanding a group during D&D
#define HOVERTIME 1500

static const TCHAR GROUP_SEP = TCHAR('.');

// following header for D&D data passed over OLE:
// Process ID of sender (to determine if src == tgt)
// Type of data
// Length of actual payload, in bytes.
static const char *OLE_HDR_FMT = "%08x%02x%08x";
static const int OLE_HDR_LEN = 18;

/*
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

  virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
  {
    // To prevent processing in multiple calls to CStaticDataSource::OnRenderGlobalData
    //  Only process the request if data has been dropped.
    SCODE sCode = COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
    if (sCode == DRAGDROP_S_DROP) {
      TRACE(_T("CStaticDropSource::QueryContinueDrag - dropped\n"));
      m_tree.EndDrop();
    }
    return sCode;
  }

  virtual SCODE GiveFeedback(DROPEFFECT dropEffect )
  {return m_tree.GiveFeedback(dropEffect);}

private:
  CPWTreeCtrl &m_tree;
};

class CPWTDataSource : public COleDataSource
{
public:
  CPWTDataSource(CPWTreeCtrl *parent, COleDropSource *ds)
    : m_tree(*parent), m_DropSource(ds) {}

  DROPEFFECT StartDragging(CLIPFORMAT cpfmt, LPCRECT rClient)
  {
    DelayRenderData(cpfmt);
    DelayRenderData(CF_UNICODETEXT);
    DelayRenderData(CF_TEXT);

    m_tree.m_cfdropped = 0;
    TRACE(_T("CPWTDataSource::StartDragging - calling DoDragDrop\n"));
    DROPEFFECT de = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE,
                               rClient, m_DropSource);
    // Cleanup:
    // Standard processing is for the recipient to do this!!!
    if (de == DROPEFFECT_NONE) {
      if (m_tree.m_hgDataALL != NULL) {
        TRACE(_T("CPWTDataSource::StartDragging - Unlock/Free m_hgDataALL\n"));
        LPVOID lpData = GlobalLock(m_tree.m_hgDataALL);
        SIZE_T memsize = GlobalSize(m_tree.m_hgDataALL);
        if (lpData != NULL && memsize > 0) {
          trashMemory(lpData, memsize);
        }
        GlobalUnlock(m_tree.m_hgDataALL);
        GlobalFree(m_tree.m_hgDataALL);
        m_tree.m_hgDataALL = NULL;
      }
      if (m_tree.m_hgDataTXT != NULL) {
        TRACE(_T("CPWTDataSource::StartDragging - Unlock/Free m_hgDataTXT\n"));
        LPVOID lpData = GlobalLock(m_tree.m_hgDataTXT);
        SIZE_T memsize = GlobalSize(m_tree.m_hgDataTXT);
        if (lpData != NULL && memsize > 0) {
          trashMemory(lpData, memsize);
        }
        GlobalUnlock(m_tree.m_hgDataTXT);
        GlobalFree(m_tree.m_hgDataTXT);
        m_tree.m_hgDataTXT = NULL;
      }
      if (m_tree.m_hgDataUTXT != NULL) {
        TRACE(_T("CPWTDataSource::StartDragging - Unlock/Free m_hgDataUTXT\n"));
        LPVOID lpData = GlobalLock(m_tree.m_hgDataUTXT);
        SIZE_T memsize = GlobalSize(m_tree.m_hgDataUTXT);
        if (lpData != NULL && memsize > 0) {
          trashMemory(lpData, memsize);
        }
        GlobalUnlock(m_tree.m_hgDataUTXT);
        GlobalFree(m_tree.m_hgDataUTXT);
        m_tree.m_hgDataUTXT = NULL;
      }
    }
    return de;
  }

  virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
  {return m_tree.OnRenderGlobalData(lpFormatEtc, phGlobal);}

private:
  CPWTreeCtrl &m_tree;
  COleDropSource *m_DropSource;
};

/**
* Implementation of CPWTreeCtrl begins here
*/

CPWTreeCtrl::CPWTreeCtrl()
  : m_isRestoring(false), m_bWithinThisInstance(true),
  m_bMouseInWindow(false), m_nHoverNDTimerID(0), m_nShowNDTimerID(0),
  m_hgDataALL(NULL), m_hgDataTXT(NULL), m_hgDataUTXT(NULL),
  m_bFilterActive(false)
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
  ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginDrag)
  ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnExpandCollapse)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTreeItemSelected)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_MOUSEMOVE()
  ON_WM_ERASEBKGND()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWTreeCtrl::Initialize()
{
  m_pDbx = static_cast<DboxMain *>(GetParent());

  // This should really be in OnCreate(), but for some reason,
  // it was never called.
  m_DropTarget->Register(this);
}

void CPWTreeCtrl::ActivateND(const bool bActivate)
{
  m_bShowNotes = bActivate;
  if (!m_bShowNotes) {
    m_bMouseInWindow = false;
  }
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
    if (hItem != NULL && !m_pDbx->IsMcoreReadOnly())
      EditLabel(hItem);
    return TRUE;
  }

  // Let the parent class do its thing
  return CTreeCtrl::PreTranslateMessage(pMsg);
}

SCODE CPWTreeCtrl::GiveFeedback(DROPEFFECT )
{
  m_pDbx->ResetIdleLockCounter();
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

DROPEFFECT CPWTreeCtrl::OnDragEnter(CWnd* , COleDataObject* pDataObject,
                                    DWORD dwKeyState, CPoint )
{
  // Is it ours?
  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) 
    return DROPEFFECT_NONE;

  m_TickCount = GetTickCount();
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

DROPEFFECT CPWTreeCtrl::OnDragOver(CWnd* pWnd , COleDataObject* pDataObject,
                                   DWORD dwKeyState, CPoint point)
{
  // Is it ours?
  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) 
    return DROPEFFECT_NONE;

  CPWTreeCtrl *pDestTreeCtrl = (CPWTreeCtrl *)pWnd;
  HTREEITEM hHitItem(NULL);

  POINT p, hs;
  CImageList* pil = CImageList::GetDragImage(&p, &hs);

  if (pil != NULL) pil->DragMove(point);

  // Can't use a timer, as the WM_TIMER msg is very low priority and
  // would not get processed during the drag processing.
  // Implement expand if hovering over a group
  if (m_TickCount != 0 && (GetTickCount() - m_TickCount >= HOVERTIME)) {
    m_TickCount = GetTickCount();

    if (m_hitemHover != NULL && !pDestTreeCtrl->IsLeaf(m_hitemHover)) {
      pDestTreeCtrl->SelectItem(m_hitemHover);
      pDestTreeCtrl->Expand(m_hitemHover, TVE_EXPAND);
    }
  }

  hHitItem = pDestTreeCtrl->HitTest(point);

  // Are we hovering over the same entry?
  if (hHitItem == NULL || hHitItem != m_hitemHover) {
    // No - reset hover item and ticking
    m_hitemHover = hHitItem;
    m_TickCount = GetTickCount();
  }

  if (hHitItem != NULL) {
    // Highlight the item under the mouse anyway
    if (pil != NULL) pil->DragLeave(this);
    pDestTreeCtrl->SelectDropTarget(hHitItem);
    m_hitemDrop = hHitItem;
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
  m_TickCount = 0;
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
    ci->SetGroup(CSecString(prefix));
  } else { // update prefix with current group name and recurse
    if (!prefix.IsEmpty())
      prefix += GROUP_SEP;
    prefix += GetItemText(hItem);
    HTREEITEM child;
    for (child = GetChildItem(hItem); child != NULL; child = GetNextSiblingItem(child)) {
      UpdateLeafsGroup(child, prefix);
    }
  }
}

void CPWTreeCtrl::OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pnmhdr;

  *pLResult = TRUE; // TRUE cancels label editing
  m_bEditLabelCompleted = false;

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
    CSecString currentTitle, currentUser, currentPassword;

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
  // In case we have to revert:
  m_eLabel = CSecString(GetItemText(ti));
  // Allow in-place editing
  *pLResult = FALSE;
}

static bool splitLeafText(const TCHAR *lt, StringX &newTitle, 
                          StringX &newUser, StringX &newPassword)
{
  bool bPasswordSet(false);

  newTitle = newUser = newPassword = _T("");

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
    Trim(newTitle);
    newUser = cs_leafText.Mid(OpenSquareBraceIndex + 1, 
                              CloseSquareBraceIndex - OpenSquareBraceIndex - 1);
    Trim(newUser);
    goto final_check;
  }

  if (OpenSquareBraceIndex == -1 && OpenCurlyBraceIndex >= 0) {
    // title {password}
    newTitle = cs_leafText.Left(OpenCurlyBraceIndex);
    Trim(newTitle);
    newPassword = cs_leafText.Mid(OpenCurlyBraceIndex + 1, 
                                  CloseCurlyBraceIndex - OpenCurlyBraceIndex - 1);
    Trim(newPassword);
    bPasswordSet = true;
    goto final_check;
  }

  if (OpenSquareBraceIndex >= 0 && OpenCurlyBraceIndex >= 0) {
    // title [user] {password}
    newTitle = cs_leafText.Left(OpenSquareBraceIndex);
    Trim(newTitle);
    newUser = cs_leafText.Mid(OpenSquareBraceIndex + 1, 
                              CloseSquareBraceIndex - OpenSquareBraceIndex - 1);
    Trim(newUser);
    newPassword = cs_leafText.Mid(OpenCurlyBraceIndex + 1, 
                                  CloseCurlyBraceIndex - OpenCurlyBraceIndex - 1);
    Trim(newPassword);
    bPasswordSet = true;
    goto final_check;
  }

  return false; // Should never get here!

final_check:
  bool bRC(true);
  if (newTitle.empty())
    bRC = false;

  if (bPasswordSet && newPassword.empty())
    bRC = false;

  return bRC;
}

void CPWTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
  if (m_pDbx->IsMcoreReadOnly())
    return; // don't edit in read-only mode

  // Initial verification performed in OnBeginLabelEdit - so some events may not get here!
  // Only items visible will be changed - e.g. if password is not shown and the user
  // puts a new password in the new dispay text, it will be ignored.

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
      StringX group, newTitle, newUser, newPassword;

      if (!splitLeafText(ptvinfo->item.pszText, newTitle, newUser, newPassword)) {
        // errors in user's input - restore text and refresh display
        goto bad_exit;
      }

      group = ci->GetGroup();
      if (m_pDbx->Find(group, newTitle, newUser) != m_pDbx->End()) {
        CSecString temp;
        if (group.empty())
          temp.Format(IDS_ENTRYEXISTS2, newTitle, newUser);
        else
          temp.Format(IDS_ENTRYEXISTS, group, newTitle, newUser);
        AfxMessageBox(temp);
        goto bad_exit;
      }

      if (newUser.empty())
        newUser = ci->GetUser();
      if (newPassword.empty())
        newPassword = ci->GetPassword();

      PWSprefs *prefs = PWSprefs::GetInstance();
      StringX treeDispString;
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
                       treeDispString.c_str(), ptvinfo->item.cchTextMax);
      ptvinfo->item.pszText[ptvinfo->item.cchTextMax - 1] = TCHAR('\0');

      // update corresponding List mode text
      DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
      ASSERT(di != NULL);
      int lindex = di->list_index;

      // update the password database record - but only those items visible!!!
      ci->SetTitle(newTitle);
      m_pDbx->UpdateListItemTitle(lindex, newTitle.c_str());
      if (bShowUsernameInTree) {
        ci->SetUser(newUser);
        m_pDbx->UpdateListItemUser(lindex, newUser.c_str());
        if (bShowPasswordInTree) {
          ci->SetPassword(newPassword);
          m_pDbx->UpdateListItemPassword(lindex, newPassword.c_str());
        }
      }
    } else { // !IsLeaf
      // PR2407325: If the user edits a group name so that it has
      // a GROUP_SEP, all hell breaks loose.
      // Right Thing (tm) would be to parse and create subgroups as
      // needed, but this is too hard (for now), so we'll just reject
      // any group name that has one or more GROUP_SEP.
      StringX hasSep(ptvinfo->item.pszText);
      if (hasSep.find(GROUP_SEP) != StringX::npos) {
        SetItemText(ti, m_eLabel);
        goto bad_exit;
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
      } // good group name (no GROUP_SEP)
    } // !IsLeaf
    // Mark database as modified
    m_pDbx->SetChanged(DboxMain::Data);
    m_pDbx->ChangeOkUpdate();

    // put edited text in right order by sorting
    SortTree(GetParentItem(ti));

    // OK
    *pLResult = TRUE;
    m_bEditLabelCompleted = true;
    if (m_pDbx->IsFilterActive())
      m_pDbx->RefreshViews();

    return;
  }

bad_exit:
  // Refresh display to show old text - if we don't no one else will
  RECT rect;
  if (GetItemRect(ti, &rect, FALSE) != FALSE) {
    InvalidateRect(&rect);
  }
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

static CSecString GetPathElem(CSecString &path)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // (assuming GROUP_SEP is '.')

  CSecString retval;
  int N = path.Find(GROUP_SEP);
  if (N == -1) {
    retval = path;
    path = _T("");
  } else {
    const int Len = path.GetLength();
    retval = CSecString(path.Left(N));
    path = CSecString(path.Right(Len - N - 1));
  }
  return retval;
}

static bool ExistsInTree(CTreeCtrl &Tree, HTREEITEM node,
                         const CSecString &s, HTREEITEM &si)
{
  // returns true iff s is a direct descendant of node
  HTREEITEM ti = Tree.GetChildItem(node);

  while (ti != NULL) {
    const CSecString itemText = Tree.GetItemText(ti);
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
    CSecString path = group;
    CSecString s;
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
    CSecString path, elem;
    HTREEITEM p, q = hNewItem;
    do {
      p = GetParentItem(q);
      if (p != NULL) {
        elem = CSecString(GetItemText(p));
        if (!path.IsEmpty())
          elem += GROUP_SEP;
        path = elem + path;
        q = p;
      } else
        break;
    } while (1);

    // Get information from current selected entry
    CSecString ci_user = ci->GetUser();
    CSecString ci_title0 = ci->GetTitle();
    CSecString ci_title = m_pDbx->GetUniqueTitle(path, ci_title0, ci_user, IDS_DRAGNUMBER);

    // Update list field with new group
    ci->SetGroup(path);
    m_pDbx->UpdateListItemGroup(di->list_index, (CString)path);

    if (ci_title.Compare(ci_title0) != 0) {
      ci->SetTitle(ci_title);
    }
    // Update tree label
    SetItemText(hNewItem, MakeTreeDisplayString(*ci));
    // Update list field with new title
    m_pDbx->UpdateListItemTitle(di->list_index, (CString)ci_title);

    // Mark database as modified!
    m_pDbx->SetChanged(DboxMain::Data);
    // Update DisplayInfo record associated with ItemData
    di->tree_item = hNewItem;
  } // leaf processing

  HTREEITEM hFirstChild;
  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    MoveItem(hFirstChild, hNewItem);  // recursively move all the items
  }

  // We are moving it - so now delete original from TreeCtrl
  // Why can't MoveItem & CopyItem be the same except for the following line?!?
  // For one, if we don't delete, we'll get into an infinite recursion.
  // For two, entry type may change on Copy or bases will get extra dependents
  DeleteItem(hitemDrag);
  return true;
}

bool CPWTreeCtrl::CopyItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop,
                           const CSecString &prefix)
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
    CSecString oldPath(temp.GetGroup());
    if (!prefix.IsEmpty()) {
      oldPath = oldPath.Right(oldPath.GetLength() - prefix.GetLength() - 1);
    }
    // with new path
    CSecString path, elem;
    path = CSecString(GetItemText(hitemDrop));
    HTREEITEM p, q = hitemDrop;
    do {
      p = GetParentItem(q);
      if (p != NULL) {
        elem = CSecString(GetItemText(p));
        if (!path.IsEmpty())
          elem += GROUP_SEP;
        path = elem + path;
        q = p;
      } else
        break;
    } while (1);

    CSecString newPath;
    if (path.IsEmpty())
      newPath = oldPath;
    else {
      newPath = path;
      if (!oldPath.IsEmpty())
        newPath += GROUP_SEP + oldPath;
    }
    // Get information from current selected entry
    CSecString ci_user = ci->GetUser();
    CSecString ci_title0 = ci->GetTitle();
    CSecString ci_title = m_pDbx->GetUniqueTitle(newPath, ci_title0, ci_user, IDS_DRAGNUMBER);

    // Needs new UUID as they must be unique and this is a copy operation
    // but before we do, save the original
    uuid_array_t original_uuid, temp_uuid, base_uuid;
    temp.GetUUID(original_uuid);
    temp.CreateUUID();
    // May need it later...
    temp.GetUUID(temp_uuid);

    temp.SetGroup(newPath);
    temp.SetTitle(ci_title);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    temp.SetDisplayInfo(ndi);

    CItemData::EntryType temp_et = temp.GetEntryType();

    switch (temp_et) {
      case CItemData::ET_NORMAL:
        break;
      case CItemData::ET_ALIASBASE:
      case CItemData::ET_SHORTCUTBASE:
        // An alias or shortcut can only have one base
        temp.SetNormal();
        break;
      case CItemData::ET_ALIAS:
        // Get base of original alias and make this copy point to it
        m_pDbx->GetAliasBaseUUID(original_uuid, base_uuid);
        m_pDbx->AddDependentEntry(base_uuid, temp_uuid, CItemData::ET_ALIAS);
        temp.SetPassword(CSecString(_T("[Alias]")));
        break;
      case CItemData::ET_SHORTCUT:
        // Get base of original shortcut and make this copy point to it
        m_pDbx->GetShortcutBaseUUID(original_uuid, base_uuid);
        m_pDbx->AddDependentEntry(base_uuid, temp_uuid, CItemData::ET_SHORTCUT);
        temp.SetPassword(CSecString(_T("[Shortcut]")));
        break;
      default:
        ASSERT(0);
    }

    m_pDbx->AddEntry(temp);

    // Mark database as modified!
    m_pDbx->SetChanged(DboxMain::Data);
  } // leaf handling
  return true;
}

BOOL CPWTreeCtrl::OnDrop(CWnd* , COleDataObject* pDataObject,
                         DROPEFFECT dropEffect, CPoint point)
{
  // Is it ours?
  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) 
    return FALSE;

  m_TickCount = 0;
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

  if (m_pDbx->IsMcoreReadOnly())
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

  // iDDType = D&D type FROMTREE/FROMTREE_R or 
  //    for column D&D only FROMCC, FROMHDR
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

  // Check if it is from another TreeCtrl (left or right mouse drag)?
  // - we don't accept drop from anything else
  if (iDDType != FROMTREE_L && iDDType != FROMTREE_R && iDDType != FROMTREE_RSC)
    goto exit;

  if (iDDType == FROMTREE_R || iDDType == FROMTREE_RSC) {
    CMenu menu;
    if (menu.LoadMenu(IDR_POPRIGHTDRAG)) {
      CMenu* pPopup = menu.GetSubMenu(0);
      ASSERT(pPopup != NULL);
      ClientToScreen(&point);
      pPopup->SetDefaultItem(GetKeyState(VK_CONTROL) < 0 ? 
                             ID_MENUITEM_COPYHERE : ID_MENUITEM_MOVEHERE);
      if (!m_bWithinThisInstance || iDDType != FROMTREE_RSC)
        pPopup->EnableMenuItem(ID_MENUITEM_RCREATESHORTCUT, MF_BYCOMMAND | MF_GRAYED);

      DWORD dwcode = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | 
                                            TPM_NONOTIFY | TPM_RETURNCMD,
                                            point.x, point.y, this);
      pPopup->DestroyMenu();
      switch (dwcode) {
        case ID_MENUITEM_COPYHERE:
          dropEffect = DROPEFFECT_COPY;
          break;
        case ID_MENUITEM_MOVEHERE:
          dropEffect = DROPEFFECT_MOVE;
          break;
        case ID_MENUITEM_RCREATESHORTCUT:
        {
          // Shortcut group from drop point, title & user from drag entry
          CSecString cs_group, cs_title, cs_user;
          CItemData *ci;
          DWORD_PTR itemData;

          itemData = GetItemData(m_hitemDrop);
          if (itemData == NULL) {
            // Dropping on a group
            cs_group = CSecString(GetGroup(m_hitemDrop));
          } else {
            // Dropping on an entry
            ci = (CItemData *)itemData;
            cs_group = ci->GetGroup();
          }

          itemData = GetItemData(m_hitemDrag);
          ASSERT(itemData != NULL);
          ci = (CItemData *)itemData;
          cs_title.Format(IDS_SCTARGET, ci->GetTitle().c_str());
          cs_user = ci->GetUser();

          // If there is a matching entry in our list, generate unique one
          if (m_pDbx->Find(cs_group, cs_title, cs_user) != m_pDbx->End()) {
            cs_title = m_pDbx->GetUniqueTitle(cs_group, cs_title, cs_user, IDS_DRAGNUMBER);
          }
          m_pDbx->CreateShortcutEntry(ci, cs_group, cs_title, cs_user);
          retval = TRUE;
          SelectItem(NULL);  // Deselect
          goto exit;
        }
        case ID_MENUITEM_CANCEL:
        default:
          SelectItem(NULL);  // Deselect
          goto exit;
      }
    }
  }

  if (hitemDrop == NULL && GetCount() == 0) {
    // Dropping on to an empty database
    CSecString DropGroup (_T(""));
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
      } else
      if (dropEffect == DROPEFFECT_COPY) {
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
    CSecString DropGroup = CSecString(GetGroup(hitemDrop));
    ProcessData((BYTE *)pData, lBufLen, DropGroup);
    SelectItem(hitemDrop);
    retval = TRUE;
  }

  SortTree(TVI_ROOT);
  m_pDbx->FixListIndexes();
  GetParent()->SetFocus();

exit:
  GlobalUnlock(hGlobal);
  if (retval == TRUE) {
    m_pDbx->SetChanged(DboxMain::Data);
    m_pDbx->ChangeOkUpdate();
    if (m_pDbx->IsFilterActive())
      m_pDbx->RefreshViews();
  }
  return retval;
}

void CPWTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
  // This sets the whole D&D mechanism in motion...
  if (pNMHDR->code == TVN_BEGINDRAG)
    m_DDType = FROMTREE_L; // Left  mouse D&D
  else
    m_DDType = FROMTREE_R; // Right mouse D&D

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

  if (m_DDType == FROMTREE_R && IsLeaf(m_hitemDrag)) {
    DWORD_PTR itemData = GetItemData(m_hitemDrag);
    CItemData *ci = (CItemData *)itemData;
    if (ci->IsNormal() || ci->IsShortcutBase())
      m_DDType = FROMTREE_RSC;  // Shortcut creation allowed (if within this instance)
  }

  // Start dragging
  m_bDropped = false;
  DROPEFFECT de = m_DataSource->StartDragging(m_tcddCPFID, &rClient);

  // If inter-process Move, we need to delete original
  if (m_cfdropped == m_tcddCPFID &&
      (de & DROPEFFECT_MOVE) == DROPEFFECT_MOVE &&
      !m_bWithinThisInstance && !m_pDbx->IsMcoreReadOnly()) {
    m_pDbx->Delete();
  }

  // wrong place to clean up imagelist?
  pil->DragLeave(GetDesktopWindow());
  pil->EndDrag();
  pil->DeleteImageList();
  delete pil;

  if (de == DROPEFFECT_NONE) {
    TRACE(_T("m_DataSource->StartDragging() failed"));
    // Do cleanup - otherwise this is the responsibility of the recipient!
    if (m_hgDataALL != NULL) {
      LPVOID lpData = GlobalLock(m_hgDataALL);
      SIZE_T memsize = GlobalSize(m_hgDataALL);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(m_hgDataALL);
      GlobalFree(m_hgDataALL);
      m_hgDataALL = NULL;
    }
    if (m_hgDataTXT != NULL) {
      LPVOID lpData = GlobalLock(m_hgDataTXT);
      SIZE_T memsize = GlobalSize(m_hgDataTXT);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(m_hgDataTXT);
      GlobalFree(m_hgDataTXT);
      m_hgDataTXT = NULL;
    }
    if (m_hgDataUTXT != NULL) {
      LPVOID lpData = GlobalLock(m_hgDataUTXT);
      SIZE_T memsize = GlobalSize(m_hgDataUTXT);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(m_hgDataUTXT);
      GlobalFree(m_hgDataUTXT);
      m_hgDataUTXT = NULL;
    }
  } else {
    while (ShowCursor(TRUE) < 0)
      ;
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
    m_pDbx->UpdateToolBarForSelectedItem(ci);
  }
}

void CPWTreeCtrl::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_ND_HOVER:
      KillTimer(m_nHoverNDTimerID);
      m_nHoverNDTimerID = 0;
      if (m_pDbx->SetNotesWindow(m_HoverNDPoint)) {
        if (m_nShowNDTimerID) {
          KillTimer(m_nShowNDTimerID);
          m_nShowNDTimerID = 0;
        }
        m_nShowNDTimerID = SetTimer(TIMER_ND_SHOWING, TIMEINT_ND_SHOWING, NULL);
      }
      break;
    case TIMER_ND_SHOWING:
      KillTimer(m_nShowNDTimerID);
      m_nShowNDTimerID = 0;
      m_HoverNDPoint = CPoint(0, 0);
      m_pDbx->SetNotesWindow(m_HoverNDPoint, false);
      break;
    default:
      CTreeCtrl::OnTimer(nIDEvent);
      break;
  }
}

void CPWTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bShowNotes)
    return;

  if (m_nHoverNDTimerID) {
    if (HitTest(m_HoverNDPoint) == HitTest(point))
      return;
    KillTimer(m_nHoverNDTimerID);
    m_nHoverNDTimerID = 0;
  }

  if (m_nShowNDTimerID) {
    if (HitTest(m_HoverNDPoint) == HitTest(point))
      return;
    KillTimer(m_nShowNDTimerID);
    m_nShowNDTimerID = 0;
    m_pDbx->SetNotesWindow(CPoint(0, 0), false);
  }

  if (!m_bMouseInWindow) {
    m_bMouseInWindow = true;
    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0};
    VERIFY(TrackMouseEvent(&tme));
  }

  m_nHoverNDTimerID = SetTimer(TIMER_ND_HOVER, HOVER_TIME_ND, NULL);
  m_HoverNDPoint = point;

  CTreeCtrl::OnMouseMove(nFlags, point);
}

LRESULT CPWTreeCtrl::OnMouseLeave(WPARAM, LPARAM)
{
  KillTimer(m_nHoverNDTimerID);
  KillTimer(m_nShowNDTimerID);
  m_nHoverNDTimerID = m_nShowNDTimerID = 0;
  m_HoverNDPoint = CPoint(0, 0);
  m_pDbx->SetNotesWindow(m_HoverNDPoint, false);
  m_bMouseInWindow = false;
  return 0L;
}

void CPWTreeCtrl::OnExpandCollapse(NMHDR *, LRESULT *)
{
  // We need to update the parent's state vector of expanded nodes
  // so that it will be persistent across miminize, lock, save, etc.
  // (unless we're in the middle of restoring the state!)

  if (!m_isRestoring) {
    m_pDbx->SaveDisplayStatus();
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

bool CPWTreeCtrl::ProcessData(BYTE *in_buffer, const long &inLen, const CSecString &DropGroup)
{
#ifdef DUMP_DATA
  CString cs_timestamp;
  cs_timestamp = PWSUtil::GetTimeStamp();
  TRACE(_T("%s: Drop data: length %d/0x%04x, value:\n"), cs_timestamp, inLen, inLen);
  pws_os::HexDump(in_buffer, inLen, cs_timestamp);
#endif /* DUMP_DATA */

  if (inLen <= 0)
    return false;

  CDDObList in_oblist;
  CSMemFile inDDmemfile;

  inDDmemfile.Attach((BYTE *)in_buffer, inLen);

  in_oblist.DDUnSerialize(inDDmemfile);

  inDDmemfile.Detach();

  if (!in_oblist.IsEmpty()) {
    m_pDbx->AddEntries(in_oblist, DropGroup);

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
    const CSecString cs_Group = ci->GetGroup();
    ci2.SetGroup(cs_Group.Right(cs_Group.GetLength() - m_nDragPathLen - 1));
    pDDObject->FromItem(ci2);
  } else {
    pDDObject->FromItem(*ci);
  }

  if (ci->IsAlias() || ci->IsShortcut()) {
    // I'm an alias or shortcut; pass on ptr to my base item
    // to retrieve its group/title/user
    CItemData *cibase(NULL);
    uuid_array_t base_uuid, entry_uuid;
    ci->GetUUID(entry_uuid);
    if (ci->IsAlias())
      m_pDbx->GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_pDbx->GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_pDbx->Find(base_uuid);
    ASSERT(iter != m_pDbx->End());
    cibase = &(iter->second);
    pDDObject->SetBaseItem(cibase);
  }

    out_oblist.AddTail(pDDObject);
}

CSecString CPWTreeCtrl::GetPrefix(HTREEITEM hItem) const
{
  // return all path components beween hItem and root.
  // e.g., if hItem is X in a.b.c.X.y.z, then return a.b.c
  CSecString retval;
  HTREEITEM p = GetParentItem(hItem);
  while (p != NULL) {
    retval = CSecString(GetItemText(p)) + retval;
    p = GetParentItem(p);
    if (p != NULL)
      retval = CSecString(GROUP_SEP) + retval;
  }
  return retval;
}

CSecString CPWTreeCtrl::MakeTreeDisplayString(const CItemData &ci) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);

  CSecString treeDispString = ci.GetTitle();
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

    iResult = CompareNoCase(pLHS->GetGroup(), pRHS->GetGroup());
    if (iResult == 0) {
      iResult = CompareNoCase(pLHS->GetTitle(), pRHS->GetTitle());
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS->GetUser(), pRHS->GetUser());
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

void CPWTreeCtrl::SetFilterState(bool bState)
{
  m_bFilterActive = bState;

  // Red if filter active, black if not
  SetTextColor(m_bFilterActive ? RGB(168, 0, 0) : RGB(0, 0, 0));
}

BOOL CPWTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
  if (m_bFilterActive && m_pDbx->GetNumPassedFiltering() == 0) {
    int nSavedDC = pDC->SaveDC(); //save the current DC state

    // Set up variables
    COLORREF clrText = RGB(168, 0, 0);
    COLORREF clrBack = ::GetSysColor(COLOR_WINDOW);    //system background color
    CBrush cbBack(clrBack);

    CRect rc;
    GetClientRect(&rc);  //get client area of the ListCtrl

    // Here is the string we want to display (or you can use a StringTable entry)
    const CString cs_emptytext(MAKEINTRESOURCE(IDS_NOITEMSPASSEDFILTERING));

    // Now we actually display the text
    // set the text color
    pDC->SetTextColor(clrText);
    // set the background color
    pDC->SetBkColor(clrBack);
    // fill the client area rect
    pDC->FillRect(&rc, &cbBack);
    // select a font
    pDC->SelectStockObject(ANSI_VAR_FONT);
    // and draw the text
    pDC->DrawText(cs_emptytext, -1, rc,
                  DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);

    // Restore dc
    pDC->RestoreDC(nSavedDC);
    ReleaseDC(pDC);
  } else {
    //  If there are items in the TreeCtrl, we need to call the base class function
    CTreeCtrl::OnEraseBkgnd(pDC);
  }

  return TRUE;
}

BOOL CPWTreeCtrl::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
  if (m_hgDataALL != NULL) {
    TRACE(_T("CPWTreeCtrl::OnRenderGlobalData - Unlock/Free m_hgDataALL\n"));
    LPVOID lpData = GlobalLock(m_hgDataALL);
    SIZE_T memsize = GlobalSize(m_hgDataALL);
    if (lpData != NULL && memsize > 0) {
      trashMemory(lpData, memsize);
    }
    GlobalUnlock(m_hgDataALL);
    GlobalFree(m_hgDataALL);
    m_hgDataALL = NULL;
  }
  if (m_hgDataTXT != NULL) {
    TRACE(_T("CPWTreeCtrl::OnRenderGlobalData - Unlock/Free m_hgDataTXT\n"));
    LPVOID lpData = GlobalLock(m_hgDataTXT);
    SIZE_T memsize = GlobalSize(m_hgDataTXT);
    if (lpData != NULL && memsize > 0) {
      trashMemory(lpData, memsize);
    }
    GlobalUnlock(m_hgDataTXT);
    GlobalFree(m_hgDataTXT);
    m_hgDataTXT = NULL;
  }
  if (m_hgDataUTXT != NULL) {
    TRACE(_T("CPWTreeCtrl::OnRenderGlobalData - Unlock/Free m_hgDataUTXT\n"));
    LPVOID lpData = GlobalLock(m_hgDataUTXT);
    SIZE_T memsize = GlobalSize(m_hgDataUTXT);
    if (lpData != NULL && memsize > 0) {
      trashMemory(lpData, memsize);
    }
    GlobalUnlock(m_hgDataUTXT);
    GlobalFree(m_hgDataUTXT);
    m_hgDataUTXT = NULL;
  }

  BOOL retval;
  if (lpFormatEtc->cfFormat == CF_UNICODETEXT ||
      lpFormatEtc->cfFormat == CF_TEXT) {
    retval = RenderTextData(lpFormatEtc->cfFormat, phGlobal);
  } else if (lpFormatEtc->cfFormat == m_tcddCPFID) {
    m_cfdropped = m_tcddCPFID;
    retval = RenderAllData(phGlobal);
  } else
    return FALSE;

  return retval;
}
    
BOOL CPWTreeCtrl::RenderTextData(CLIPFORMAT &cfFormat, HGLOBAL* phGlobal)
{
  if (!IsLeaf(m_hitemDrag)) {
    TRACE(_T("CPWTreeCtrl::RenderTextData - not a leaf!\n"));
    return FALSE;
  }

  // Save format for use by D&D to ensure we don't 'move' when dropping text!
  m_cfdropped = cfFormat;

  DWORD_PTR itemData = GetItemData(m_hitemDrag);
  CItemData *pci = (CItemData *)itemData;

  const CItemData::EntryType entrytype = pci->GetEntryType();
  if (entrytype == CItemData::ET_ALIAS) {
    // This is an alias
    uuid_array_t entry_uuid, base_uuid;
    pci->GetUUID(entry_uuid);
    m_pDbx->GetAliasBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_pDbx->Find(base_uuid);
    if (iter != m_pDbx->End()) {
      pci = &(iter->second);
    }
  }

  if (entrytype == CItemData::ET_SHORTCUT) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    pci->GetUUID(entry_uuid);
    m_pDbx->GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_pDbx->Find(base_uuid);
    if (iter != m_pDbx->End()) {
      pci = &(iter->second);
    }
  }
 
  CSecString cs_dragdata;
  cs_dragdata = pci->GetPassword();

  const int ilen = cs_dragdata.GetLength();
  if (ilen == 0) {
    // Password shouldn't ever be empty!
    return FALSE;
  }

  DWORD dwBufLen;
  LPSTR lpszA(NULL);
  LPWSTR lpszW(NULL);

#ifdef UNICODE
  // We are Unicode!
  if (cfFormat == CF_UNICODETEXT) {
    // So is requested data!
    dwBufLen = (ilen + 1) * sizeof(wchar_t);
    lpszW = new WCHAR[ilen + 1];
    TRACE(_T("lpszW allocated %p, size %d\n"), lpszW, dwBufLen);
#if (_MSC_VER >= 1400)
    (void) wcsncpy_s(lpszW, ilen + 1, cs_dragdata, ilen);
#else
    (void)wcsncpy(lpszW, cs_dragdata, ilen);
    lpszW[ilen] = L'\0';
#endif
  } else {
    // They want it in ASCII - use lpszW temporarily
    lpszW = cs_dragdata.GetBuffer(ilen + 1);
    dwBufLen = WideCharToMultiByte(CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL);
    ASSERT(dwBufLen != 0);
    lpszA = new char[dwBufLen];
    TRACE(_T("lpszA allocated %p, size %d\n"), lpszA, dwBufLen);
    WideCharToMultiByte(CP_ACP, 0, lpszW, -1, lpszA, dwBufLen, NULL, NULL);
    cs_dragdata.ReleaseBuffer();
    lpszW = NULL;
  }
#else
  // We are Ascii!
  if (cfFormat == CF_TEXT) {
    // So is requested data!
    dwBufLen = ilen + 1;
    lpszA = new char[ilen + 1];
    TRACE(_T("lpszA allocated %p, size %d\n"), lpszA, dwBufLen);
#if (_MSC_VER >= 1400)
    (void) strncpy_s(lpszA, ilen + 1, cs_dragdata, ilen);
#else
    (void)strncpy(lpszA, cs_dragdata, ilen);
    lpszA[ilen] = '\0';
#endif
  } else {
    // They want it in UNICODE - use lpszA temporarily
    lpszA = cs_dragdata.GetBuffer(ilen + 1);
    dwBufLen = MultiByteToWideChar(CP_ACP, 0, lpszA, -1, NULL, NULL);
    lpszW = new WCHAR[dwBufLen];
    TRACE(_T("lpszW allocated %p, size %d\n"), lpszW, dwBufLen);
    MultiByteToWideChar(CP_ACP, 0, lpszA, -1, lpszW, dwBufLen);
    cs_dragdata.ReleaseBuffer();
    lpszA = NULL;
  }
#endif

  LPVOID lpData(NULL);
  LPVOID lpDataBuffer;
  HGLOBAL *phgData;
  if (cfFormat == CF_UNICODETEXT) {
    lpDataBuffer = (LPVOID)lpszW;
    phgData = &m_hgDataUTXT;
  } else {
    lpDataBuffer = (LPVOID)lpszA;
    phgData = &m_hgDataTXT;
  }

  BOOL retval(FALSE);
  if (*phGlobal == NULL) {
    TRACE(_T("CPWTreeCtrl::OnRenderTextData - Alloc global memory\n"));
    *phgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBufLen);
    ASSERT(*phgData != NULL);
    if (*phgData == NULL)
      goto bad_return;

    lpData = GlobalLock(*phgData);
    ASSERT(lpData != NULL);
    if (lpData == NULL)
      goto bad_return;

    // Copy data
    memcpy(lpData, lpDataBuffer, dwBufLen);
    *phGlobal = *phgData;
    GlobalUnlock(*phgData);
    retval = TRUE;
  } else {
    TRACE(_T("CPWTreeCtrl::OnRenderTextData - *phGlobal NOT NULL!\n"));
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(*phgData);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      TRACE(_T("CPWTreeCtrl::OnRenderTextData - NOT enough room - FAIL\n"));
    } else {
      // Enough room - copy our data into supplied area
      TRACE(_T("CPWTreeCtrl::OnRenderTextData - enough room - copy our data\n"));
      LPVOID pInGlobalLock = GlobalLock(*phGlobal);
      ASSERT(pInGlobalLock != NULL);
      if (pInGlobalLock == NULL)
        goto bad_return;

      memcpy(pInGlobalLock, lpDataBuffer, ourSize);
      GlobalUnlock(*phGlobal);
      retval = TRUE;
    }
  }

bad_return:
  // Finished with buffer - trash it
  trashMemory(lpDataBuffer, dwBufLen);
  // Free the strings (only one is actually in use)
  TRACE(_T("lpszA freed %p\n"), lpszA);
  delete[] lpszA;
  TRACE(_T("lpszW freed %p\n"), lpszW);
  delete[] lpszW;
  // Since lpDataBuffer pointed to one of the above - just zero the pointer
  lpDataBuffer = NULL;

  // If retval == TRUE, recipient is responsible for freeing the global memory
  // if D&D succeeds (see after StartDragging in OnMouseMove)
  if (retval == FALSE) {
    TRACE(_T("CPWTreeCtrl::RenderTextData - returning FALSE!\n"));
    if (*phgData != NULL) {
      LPVOID lpData = GlobalLock(*phgData);
      SIZE_T memsize = GlobalSize(*phgData);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(*phgData);
      GlobalFree(*phgData);
      *phgData = NULL;
    }
  } else {
    TRACE(_T("CPWTreeCtrl::RenderTextData - D&D Data:"));
    if (cfFormat == CF_UNICODETEXT) {
#ifdef UNICODE
      TRACE(_T("\"%s\"\n"), (LPWSTR)lpData);  // we are Unicode, data is Unicode
#else
      TRACE(_T("\"%S\"\n"), (LPSTR)lpData);   // we are NOT Unicode, data is Unicode
#endif
    } else {
#ifdef UNICODE
      TRACE(_T("\"%S\"\n"), (LPSTR)lpData);  // we are Unicode, data is NOT Unicode
#else
      TRACE(_T("\"%s\"\n"), (LPWSTR)lpData);  // we are NOT Unicode, data is NOT Unicode
#endif
    }
  }

  if (lpData != NULL)
    GlobalUnlock(*phgData);

    return retval;
}

BOOL CPWTreeCtrl::RenderAllData(HGLOBAL* phGlobal)
{
  long lBufLen;
  BYTE *buffer = NULL;

  ASSERT(m_hgDataALL == NULL);

  // CollectData allocates buffer - need to free later
  if (!CollectData(buffer, lBufLen))
    return FALSE;

  char header[OLE_HDR_LEN+1];
  // Note: GetDDType will return either FROMTREE or FROMTREE_R
#if _MSC_VER >= 1400
  sprintf_s(header, sizeof(header),
            OLE_HDR_FMT, GetCurrentProcessId(), GetDDType(), lBufLen);
#else
  sprintf(header, OLE_HDR_FMT, GetCurrentProcessId(), GetDDType(), lBufLen);
#endif
  CMemFile mf;
  mf.Write(header, OLE_HDR_LEN);
  mf.Write(buffer, lBufLen);

  // Finished with buffer - trash it and free it
  trashMemory((void *)buffer, lBufLen);
  free(buffer);

  LPVOID lpData(NULL);
  LPVOID lpDataBuffer;
  DWORD dwBufLen = (DWORD)mf.GetLength();
  lpDataBuffer = (LPVOID)(mf.Detach());

#ifdef DUMP_DATA
  CString cs_timestamp = PWSUtil::GetTimeStamp();
  TRACE(_T("%s: Drag data: length %d/0x%04x, value:\n"), cs_timestamp,
        dwBufLen, dwBufLen);
  pws_os::HexDump(lpDataBuffer, dwBufLen, cs_timestamp);
#endif /* DUMP_DATA */

  BOOL retval(FALSE);
  if (*phGlobal == NULL) {
    TRACE(_T("CPWTreeCtrl::OnRenderAllData - Alloc global memory\n"));
    m_hgDataALL = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBufLen);
    ASSERT(m_hgDataALL != NULL);
    if (m_hgDataALL == NULL)
      goto bad_return;

    lpData = GlobalLock(m_hgDataALL);
    ASSERT(lpData != NULL);
    if (lpData == NULL)
      goto bad_return;

    // Copy data
    memcpy(lpData, lpDataBuffer, dwBufLen);
    *phGlobal = m_hgDataALL;
    retval = TRUE;
  } else {
    TRACE(_T("CPWTreeCtrl::OnRenderAllData - *phGlobal NOT NULL!\n"));
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(m_hgDataALL);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      TRACE(_T("CPWTreeCtrl::OnRenderAllData - NOT enough room - FAIL\n"));
    } else {
      // Enough room - copy our data into supplied area
      TRACE(_T("CPWTreeCtrl::OnRenderAllData - enough room - copy our data\n"));
      LPVOID pInGlobalLock = GlobalLock(*phGlobal);
      ASSERT(pInGlobalLock != NULL);
      if (pInGlobalLock == NULL)
        goto bad_return;

      memcpy(pInGlobalLock, lpDataBuffer, ourSize);
      GlobalUnlock(*phGlobal);
      retval = TRUE;
    }
  }

bad_return:
  if (dwBufLen != 0 && lpDataBuffer != NULL) {
    trashMemory(lpDataBuffer, dwBufLen);
    free(lpDataBuffer);
    lpDataBuffer = NULL;
  }

  // If retval == TRUE, recipient is responsible for freeing the global memory
  // if D&D succeeds
  if (retval == FALSE) {
    TRACE(_T("CPWTreeCtrl::RenderAllData - returning FALSE!\n"));
    if (m_hgDataALL != NULL) {
      LPVOID lpData = GlobalLock(m_hgDataALL);
      SIZE_T memsize = GlobalSize(m_hgDataALL);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(m_hgDataALL);
      GlobalFree(m_hgDataALL);
      m_hgDataALL = NULL;
    }
  }

  if (lpData != NULL)
    GlobalUnlock(m_hgDataALL);

  return retval;
}
