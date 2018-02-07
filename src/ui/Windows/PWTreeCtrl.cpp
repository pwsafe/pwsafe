/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PWTreeCtrl.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"
#include "DDSupport.h"
#include "SecString.h"
#include "SMemFile.h"
#include "GeneralMsgBox.h"

#include "core/ItemData.h"
#include "core/Util.h"
#include "core/Pwsprefs.h"

#include "os/debug.h"

#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Hover time of 1.5 seconds before expanding a group during D&D
#define HOVERTIME 1500

extern const wchar_t GROUP_SEP = L'.';
extern const wchar_t *GROUP_SEP2 = L".";

// following header for D&D data passed over OLE:
// Process ID of sender (to determine if src == tgt)
// Type of data
// Length of actual payload, in bytes.
static const char *OLE_HDR_FMT = "%08x%02x%08x";
static const int OLE_HDR_LEN = 18;

/*
* Following classes are used to "Fake" multiple inheritance:
* Ideally, CPWTreeCtrlX should derive from CTreeCtrl, COleDropTarget
* and COleDropSource. However, since m'soft, in their infinite
* wisdom, couldn't get this common use-case straight,
* we use the following classes as proxies: CPWTreeCtrlX
* has a member var for each, registers said member appropriately
* for D&D, and member calls parent's method to do the grunt work.
*/

class CPWTDropTarget : public COleDropTarget
{
public:
  CPWTDropTarget(CPWTreeCtrlX *parent) : m_tree(*parent) {}

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
  CPWTreeCtrlX &m_tree;
};

class CPWTDropSource : public COleDropSource
{
public:
  CPWTDropSource(CPWTreeCtrlX *parent) : m_tree(*parent) {}

  virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
  {
    // To prevent processing in multiple calls to CStaticDataSource::OnRenderGlobalData
    //  Only process the request if data has been dropped.
    SCODE sCode = COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
    if (sCode == DRAGDROP_S_DROP) {
      m_tree.EndDrop();
    }
    return sCode;
  }

  virtual SCODE GiveFeedback(DROPEFFECT dropEffect)
  {return m_tree.GiveFeedback(dropEffect);}

private:
  CPWTreeCtrlX &m_tree;
};

class CPWTDataSource : public COleDataSource
{
public:
  CPWTDataSource(CPWTreeCtrlX *parent, COleDropSource *ds)
    : m_tree(*parent), m_pDropSource(ds) {}

  DROPEFFECT StartDragging(CLIPFORMAT cpfmt, LPCRECT rClient)
  {
    DelayRenderData(cpfmt);
    DelayRenderData(CF_UNICODETEXT);
    DelayRenderData(CF_TEXT);

    m_tree.m_cfdropped = 0;
    DROPEFFECT de = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE,
                               rClient, m_pDropSource);
    // Cleanup:
    // Standard processing is for the recipient to do this!!!
    if (de == DROPEFFECT_NONE) {
      if (m_tree.m_hgDataALL != NULL) {
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
  CPWTreeCtrlX &m_tree;
  COleDropSource *m_pDropSource;
};

/**
* Implementation of CPWTreeCtrlX begins here
*/

CPWTreeCtrlX::CPWTreeCtrlX()
  : m_isRestoring(false), m_bWithinThisInstance(true),
  m_bMouseInWindow(false), m_nHoverNDTimerID(0), m_nShowNDTimerID(0),
  m_hgDataALL(NULL), m_hgDataTXT(NULL), m_hgDataUTXT(NULL),
  m_bTreeFilterActive(false), m_bUseHighLighting(false)
{
  // Register a clipboard format for column drag & drop.
  // Note that it's OK to register same format more than once:
  // "If a registered format with the specified name already exists,
  // a new format is not registered and the return value identifies the existing format."

  CString cs_CPF(MAKEINTRESOURCE(IDS_CPF_TVDD));
  m_tcddCPFID = (CLIPFORMAT)RegisterClipboardFormat(cs_CPF);
  ASSERT(m_tcddCPFID != 0);

  // Instantiate "proxy" objects for D&D.

  // The members are currently pointers mainly to hide
  // their implementation from the header file. If this changes,
  // e.g., if we make them nested classes, then they should
  // be non-pointers.
  m_pDropTarget = new CPWTDropTarget(this);
  m_pDropSource = new CPWTDropSource(this);
  m_pDataSource = new CPWTDataSource(this, m_pDropSource);
}

CPWTreeCtrlX::~CPWTreeCtrlX()
{
  // See comment in constructor re these member variables

  // Don't delete m_pDataSource but first release all references and
  // this routine will delete it when the references get to 0.
  m_pDataSource->InternalRelease();

  // delete the Drop Target & Source
  delete m_pDropTarget;
  delete m_pDropSource;
}

BEGIN_MESSAGE_MAP(CPWTreeCtrlX, CTreeCtrl)
  //{{AFX_MSG_MAP(CPWTreeCtrlX)
  ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
  ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginDrag)
  ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnExpandCollapse)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelectionChanged)
  ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_MOUSEMOVE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_VSCROLL()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWTreeCtrlX::Initialize()
{
  // This should really be in OnCreate(), but for some reason,
  // it was never called.
  m_pDropTarget->Register(this);
}

void CPWTreeCtrlX::ActivateND(const bool bActivate)
{
  m_bShowNotes = bActivate;
  if (!m_bShowNotes) {
    m_bMouseInWindow = false;
  }
}

void CPWTreeCtrlX::OnDestroy()
{
  CImageList *pimagelist = GetImageList(TVSIL_NORMAL);
  if (pimagelist != NULL) {
    pimagelist->DeleteImageList();
    delete pimagelist;
  }
  m_pDropTarget->Revoke();
}

void CPWTreeCtrlX::OnPaint()
{
  CTreeCtrl::OnPaint();

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::TREEONLY);
}

void CPWTreeCtrlX::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::TREEONLY);
}

BOOL CPWTreeCtrlX::PreTranslateMessage(MSG *pMsg)
{
  // When an item is being edited make sure the edit control
  // receives certain important key strokes
  if (GetEditControl()) {
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE; // DO NOT process further
  }

  // Process User's AutoType shortcut
  if (app.GetMainDlg()->CheckPreTranslateAutoType(pMsg))
    return TRUE;

  // Process User's Delete shortcut
  if (app.GetMainDlg()->CheckPreTranslateDelete(pMsg))
    return TRUE;
  
  // Process user's Rename shortcut
  if (app.GetMainDlg()->CheckPreTranslateRename(pMsg)) {
    //  Send via main window to ensure it isn't an Edit in place
    HTREEITEM hItem = GetSelectedItem();
    CItemData *pci = (CItemData *)GetItemData(hItem);
    app.GetMainDlg()->SendMessage(WM_COMMAND,
          pci == NULL ? ID_MENUITEM_RENAMEENTRY : ID_MENUITEM_RENAMEGROUP);
    return TRUE;
  }

  // Let the parent class do its thing
  return CTreeCtrl::PreTranslateMessage(pMsg);
}

SCODE CPWTreeCtrlX::GiveFeedback(DROPEFFECT )
{
  app.GetMainDlg()->ResetIdleLockCounter();
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

DROPEFFECT CPWTreeCtrlX::OnDragEnter(CWnd *, COleDataObject *pDataObject,
                                    DWORD dwKeyState, CPoint )
{
  if (pDataObject->IsDataAvailable(CF_HDROP, NULL)) {
    return DROPEFFECT_MOVE;
  }

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

DROPEFFECT CPWTreeCtrlX::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject,
                                   DWORD dwKeyState, CPoint point)
{
  if (pDataObject->IsDataAvailable(CF_HDROP, NULL)) {
    return DROPEFFECT_MOVE;
  }

  // Is it ours?
  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) 
    return DROPEFFECT_NONE;

  CPWTreeCtrlX *pDestTreeCtrl = (CPWTreeCtrlX *)pWnd;
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

void CPWTreeCtrlX::OnDragLeave()
{
  m_TickCount = 0;
  m_bWithinThisInstance = false;
  //We leave or window, so we need to clean DropTarget selection
  SelectDropTarget(NULL);

  // ShowCursor's semantics are VERY odd - RTFM
  while (ShowCursor(TRUE) < 0)
    ;
}

void CPWTreeCtrlX::SetUpFont()
{
  Fonts::GetInstance()->SetUpFont(this, Fonts::GetInstance()->GetTreeListFont());
}

void CPWTreeCtrlX::OnBeginLabelEdit(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pNotifyStruct;

  *pLResult = TRUE; // TRUE cancels label editing

  // Check IsInRename to prevent unintentional Edit in place
  if (app.GetMainDlg()->IsDBReadOnly() || (!app.GetMainDlg()->IsInRename() &&
      !app.GetMainDlg()->IsInAddGroup()))
    return;

  m_bEditLabelCompleted = false;

  /*
    Allowed formats for an entry are:
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
    CItemData *pci = (CItemData *)itemData;

    // Don't allow edit if a protected entry
    if (pci->IsProtected())
      return;

    CSecString currentTitle, currentUser, currentPassword;

    // We cannot allow in-place edit if these fields contain braces!
    currentTitle = pci->GetTitle();
    if (currentTitle.FindOneOf(L"[]{}") != -1)
      return;

    currentUser = pci->GetUser();
    if (prefs->GetPref(PWSprefs::ShowUsernameInTree) &&
      currentUser.FindOneOf(L"[]{}") != -1)
      return;

    currentPassword = pci->GetPassword();
    if (prefs->GetPref(PWSprefs::ShowPasswordInTree) &&
      currentPassword.FindOneOf(L"[]{}") != -1)
      return;
  }

  // In case we have to revert:
  m_eLabel = CSecString(GetItemText(ti));
  // Allow in-place editing
  *pLResult = FALSE;
}

static bool splitLeafText(const wchar_t *lt, StringX &newTitle, 
                          StringX &newUser, StringX &newPassword,
                          bool &bUserSet)
{
  bool bPasswordSet(false);
  bUserSet = false;

  newTitle = newUser = newPassword = L"";

  CString cs_leafText(lt);
  cs_leafText.Trim();
  if (cs_leafText.IsEmpty())
    return false;

  // Check no duplicate braces
  int OpenSquareBraceIndex = cs_leafText.Find(L'[');
  if (OpenSquareBraceIndex >= 0)
    if (cs_leafText.Find(L'[', OpenSquareBraceIndex + 1) != -1)
      return false;

  int CloseSquareBraceIndex = cs_leafText.Find(L']');
  if (CloseSquareBraceIndex >= 0)
    if (cs_leafText.Find(L']', CloseSquareBraceIndex + 1) != -1)
      return false;

  int OpenCurlyBraceIndex = cs_leafText.Find(L'{');
  if (OpenCurlyBraceIndex >= 0)
    if (cs_leafText.Find(L'{', OpenCurlyBraceIndex + 1) != -1)
      return false;

  int CloseCurlyBraceIndex = cs_leafText.Find(L'}');
  if (CloseCurlyBraceIndex >= 0)
    if (cs_leafText.Find(L'}', CloseCurlyBraceIndex + 1) != -1)
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
    bUserSet = true;
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
    bUserSet = true;
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

void CPWTreeCtrlX::OnSelectionChanged(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0;
  
  NMTREEVIEW *pNMTreeView = (NMTREEVIEW *)pNotifyStruct;
  
  // Don't bother if no entries or not via the keyboard/mouse (check this first
  // as more likely than no entries).
   if (pNMTreeView->action == TVC_UNKNOWN || GetCount() == 0)
     return;

  app.GetMainDlg()->OnItemSelected(pNotifyStruct, pLResult, true);
}

void CPWTreeCtrlX::OnDeleteItem(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0;
  // Clear pointer to CItemData of item being deleted so no other
  // routine has a problem with an invalid value.
  HTREEITEM hItem = ((NMTREEVIEW *)pNotifyStruct)->itemOld.hItem;
  if (hItem != NULL)
    SetItemData(hItem, 0);
}

void CPWTreeCtrlX::OnEndLabelEdit(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  if (app.GetMainDlg()->IsDBReadOnly())
    return; // don't edit in read-only mode

  // Initial verification performed in OnBeginLabelEdit - so some events may not get here!
  // Only items visible will be changed - e.g. if password is not shown and the user
  // puts a new password in the new display text, it will be ignored.

  /* Allowed formats for an entry:
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

  CommandInterface *pcore = (CommandInterface *)app.GetCore();

  MultiCommands *pmulticmds = MultiCommands::Create(pcore);

  NMTVDISPINFO *ptvinfo = (NMTVDISPINFO *)pNotifyStruct;
  HTREEITEM ti = ptvinfo->item.hItem;
  if (ptvinfo->item.pszText == NULL ||     // NULL if edit cancelled,
      ptvinfo->item.pszText[0] == L'\0') { // empty if text deleted - not allowed
    // If called from AddGroup, user cancels EditLabel - save it
    // (Still called "New Group" or the changed name if that already existed)
    if (app.GetMainDlg()->IsInAddGroup()) {
      // m_eLabel is the old name but need to get the path
      StringX sxPath = GetGroup(ti);
      pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxPath,
                             DBEmptyGroupsCommand::EG_ADD));

      // But if parent was empty - it isn't now!
      size_t iLast_Group_Separator = sxPath.find_last_of(GROUP_SEP2);
      if (iLast_Group_Separator != StringX::npos) {
        StringX sxParent = sxPath.substr(0, iLast_Group_Separator);
        if (app.GetMainDlg()->IsEmptyGroup(sxParent)) {
          pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxParent,
            DBEmptyGroupsCommand::EG_DELETE));
        }
      }
      // Do it
      app.GetMainDlg()->Execute(pmulticmds);

      app.GetMainDlg()->ChangeOkUpdate();

      app.GetMainDlg()->ResetInAddGroup();
      *pLResult = TRUE;
    }
    return;
  }

  // Now do the rename
  ptvinfo->item.mask = TVIF_TEXT;
  SetItem(&ptvinfo->item);
  const StringX sxNewText = ptvinfo->item.pszText;

  // Set up what we need
  StringX sxNewTitle, sxNewUser, sxNewPassword;  // For Leaf
  StringX sxOldPath, sxNewPath;                  // For Node
  CString prefix;                                // For Node
  
  PWSprefs *prefs = PWSprefs::GetInstance();
  bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);
  bool bIsLeaf = IsLeaf(ptvinfo->item.hItem);
  CItemData *pci(NULL);

  if (bIsLeaf) { // Leaf
    pci = (CItemData *)GetItemData(ti);
    ASSERT(pci != NULL);

    bool bUserSet;
    if (!splitLeafText(sxNewText.c_str(), sxNewTitle, sxNewUser, sxNewPassword, bUserSet)) {
      // errors in user's input - restore text and refresh display
      goto bad_exit;
    }

    // The user can change any of Title, User & Password via rename
    // but ONLY if that field is being displayed.
    if (sxNewTitle.empty()) {
      // Title empty - change ignored as mandatory field
      sxNewTitle = pci->GetTitle();
    }

    if (!bShowUsernameInTree || !bUserSet) {
      // Username not visible - changed ignored - OR
      // Username visible but  user hasn't explicitly changed it
      sxNewUser = pci->GetUser();
    }

    if (!bShowPasswordInTree || sxNewPassword.empty()) {
      // Password change ignored if not visible or empty as mandatory field
      sxNewPassword = pci->GetPassword();
    }

    StringX sxGroup = pci->GetGroup();
    if ((sxNewTitle != pci->GetTitle() || sxNewUser != pci->GetUser()) &&
        app.GetMainDlg()->Find(sxGroup, sxNewTitle, sxNewUser) != app.GetMainDlg()->End()) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
      goto bad_exit;
    }

    StringX treeDispString = sxNewTitle;
    if (bShowUsernameInTree) {
      treeDispString += L" [";
      treeDispString += sxNewUser;
      treeDispString += L"]";

      if (bShowPasswordInTree) {
        treeDispString += L" {";
        treeDispString += sxNewPassword;
        treeDispString += L"}";
      }
    }

    // Update corresponding Tree mode text with the new display text (ie: in case 
    // the username was removed and had to be normalized back in).  Note that
    // we cannot do "SetItemText(ti, treeDispString)" here since Windows will
    // automatically overwrite and update the item text with the contents from 
    // the "ptvinfo->item.pszText" buffer.
    PWSUtil::strCopy(ptvinfo->item.pszText, ptvinfo->item.cchTextMax,
                     treeDispString.c_str(), ptvinfo->item.cchTextMax);
    ptvinfo->item.pszText[ptvinfo->item.cchTextMax - 1] = L'\0';

    // update corresponding List mode text - but  only those visible in Tree
    DisplayInfo *pdi = app.GetMainDlg()->GetEntryGUIInfo(*pci);
    int lindex = pdi->list_index;

    if (sxNewTitle != pci->GetTitle()) {
      app.GetMainDlg()->UpdateListItemTitle(lindex, sxNewTitle);
    }

    if (bShowUsernameInTree) {
      if(sxNewUser != pci->GetUser()) {
        app.GetMainDlg()->UpdateListItemUser(lindex, sxNewUser);
      }
      if (bShowPasswordInTree && sxNewPassword != pci->GetPassword()) {
        app.GetMainDlg()->UpdateListItemPassword(lindex, sxNewPassword);
      }
    }
  } else { // Node
    /* TO DO - Deal with username in display when group renamed! */

    // Check that pathname with node name doesn't already exist.
    // (We could do fancy merging if so, but for now we'll reject the rename)
    // Check is simple - just see if there's a sibling GROUP name sxNewText

    HTREEITEM hParent = GetParentItem(ti);
    HTREEITEM hSibling = GetChildItem(hParent);
    do {
      if (hSibling != ti && !IsLeaf(hSibling)) {
        CString cs_SiblingText = GetItemText(hSibling);
        if (cs_SiblingText == sxNewText.c_str())
          goto bad_exit;
      }
      hSibling = GetNextSiblingItem(hSibling);
    } while (hSibling != NULL);
    // If we made it here, then name's unique.

    // PR2407325: If the user edits a group name so that it has
    // a GROUP_SEP, all hell breaks loose.
    // Right Thing (tm) would be to parse and create subgroups as
    // needed, but this is too hard (for now), so we'll just reject
    // any group name that has one or more GROUP_SEP.
    if (sxNewText.find(GROUP_SEP) != StringX::npos) {
      SetItemText(ti, m_eLabel);
      goto bad_exit;
    } else {
      // Update all leaf children with new path element
      // prefix is path up to and NOT including renamed node
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

      if (prefix.IsEmpty()) {
        sxOldPath = (LPCWSTR)m_eLabel;
        sxNewPath = sxNewText;
      } else {
        sxOldPath = StringX(prefix) + StringX(GROUP_SEP2) + (LPCWSTR)m_eLabel;
        sxNewPath = StringX(prefix) + StringX(GROUP_SEP2) + sxNewText;
      }

      if (app.GetMainDlg()->IsEmptyGroup(sxOldPath)) {
        // Rename single empty group
        pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxOldPath, sxNewPath,
                        DBEmptyGroupsCommand::EG_RENAME));
      } else {
        if (!app.GetMainDlg()->IsInAddGroup()) {
          // Rename any empty groups within this group
          // Get current empty groups
          pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxOldPath, sxNewPath,
            DBEmptyGroupsCommand::EG_RENAMEPATH));

          // Update map of groups
          app.GetMainDlg()->UpdateGroupNamesInMap(sxOldPath, sxNewPath);
        }
      }

    } // good group name (no GROUP_SEP)
  } // !IsLeaf

  if (bIsLeaf) {
    // Update Leaf
    if (sxNewTitle != pci->GetTitle()) {
      pmulticmds->Add(UpdateEntryCommand::Create(pcore, *pci,
                                                 CItemData::TITLE, sxNewTitle));
    }

    if (bShowUsernameInTree) {
      if (sxNewUser != pci->GetUser()) {
        pmulticmds->Add(UpdateEntryCommand::Create(pcore, *pci,
                                                   CItemData::USER, sxNewUser));
      }
      if (bShowPasswordInTree && sxNewPassword != pci->GetPassword()) {
        pmulticmds->Add(UpdateEntryCommand::Create(pcore, *pci,
                                                   CItemData::PASSWORD, sxNewPassword));
      }
    }

    if (!pmulticmds->IsEmpty()) {
      // We refresh the Tree view as rename may alter entry's posiition
      Command *pcmd_undo = UpdateGUICommand::Create(pcore,
                                          UpdateGUICommand::WN_UNDO,
                                          UpdateGUICommand::GUI_REFRESH_TREE);
      pmulticmds->Insert(pcmd_undo, 0);

      Command *pcmd_redo = UpdateGUICommand::Create(pcore,
                                          UpdateGUICommand::WN_EXECUTE_REDO,
                                          UpdateGUICommand::GUI_REFRESH_TREE);
      pmulticmds->Add(pcmd_redo);
    }
  } else {
    // Group
    if (app.GetMainDlg()->IsInAddGroup()) {
      // Add the group
      pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxNewPath,
                      DBEmptyGroupsCommand::EG_ADD));

      // But if parent was empty - it isn't now!
      size_t iLast_Group_Separator = sxNewPath.find_last_of(GROUP_SEP2);
      if (iLast_Group_Separator != StringX::npos) {
        StringX sxParent = sxNewPath.substr(0, iLast_Group_Separator);
        if (app.GetMainDlg()->IsEmptyGroup(sxParent)) {
          pmulticmds->Add(DBEmptyGroupsCommand::Create(pcore, sxParent,
                          DBEmptyGroupsCommand::EG_DELETE));
        }
      }

      app.GetMainDlg()->ResetInAddGroup();
      *pLResult = TRUE;
    } else {
      // We refresh the view
      Command *pcmd_undo = UpdateGUICommand::Create(pcore,
                                                UpdateGUICommand::WN_UNDO,
                                                UpdateGUICommand::GUI_REFRESH_BOTHVIEWS);
      pmulticmds->Add(pcmd_undo);

      // Update Group
      pmulticmds->Add(RenameGroupCommand::Create(pcore, sxOldPath, sxNewPath));

      // We refresh the view
      Command *pcmd_redo = UpdateGUICommand::Create(pcore,
                                              UpdateGUICommand::WN_EXECUTE_REDO,
                                              UpdateGUICommand::GUI_REFRESH_BOTHVIEWS);
      pmulticmds->Add(pcmd_redo);
    }
  }

  if (!pmulticmds->IsEmpty())
    app.GetMainDlg()->Execute(pmulticmds);
  else
    delete pmulticmds;

  app.GetMainDlg()->ChangeOkUpdate();

  // put edited text in right order by sorting
  SortTree(GetParentItem(ti));

  // OK
  *pLResult = TRUE;
  m_bEditLabelCompleted = true;

  return;

bad_exit:
  // Refresh display to show old text - if we don't no one else will
  app.GetMainDlg()->RefreshViews();

  // restore text
  *pLResult = FALSE;
}

bool CPWTreeCtrlX::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent) const
{
  do {
    if (hitemChild == hitemSuspectedParent)
      break;
  } while ((hitemChild = GetParentItem(hitemChild)) != NULL);

  return (hitemChild != NULL);
}

bool CPWTreeCtrlX::IsLeaf(HTREEITEM hItem) const
{
  // ItemHasChildren() won't work in the general case
  int i, dummy;
  BOOL status = GetItemImage(hItem, i, dummy);
  // if on empty space, hItem's NULL, status is false
  return (status && i != GROUP && i != EMPTY_GROUP);
}

// Returns the number of children of this group
// If bRecurse is true - also count grandchildren, great-grandchildren etc. etc.
// Otherwise only immediate children
int CPWTreeCtrlX::CountChildren(HTREEITEM hStartItem, bool bRecurse) const
{
  // Walk the Tree!
  int num = 0;
  if (hStartItem != NULL && ItemHasChildren(hStartItem)) {
    HTREEITEM hChildItem = GetChildItem(hStartItem);
    while (hChildItem != NULL) {
      if (ItemHasChildren(hChildItem) && bRecurse) {
        num += CountChildren(hChildItem);
      } else {
        num++;
      }
      hChildItem = GetNextSiblingItem(hChildItem);
    }
  }
  return num;
}

// Returns the number of non-node children of this group
int CPWTreeCtrlX::CountLeafChildren(HTREEITEM hStartItem) const
{
  // Walk the Tree!
  int num = 0;
  if (hStartItem != NULL && ItemHasChildren(hStartItem)) {
    HTREEITEM hChildItem = GetChildItem(hStartItem);
    while (hChildItem != NULL) {
      if (ItemHasChildren(hChildItem)) {
        num += CountLeafChildren(hChildItem);
      } else {
        // Only add if this is a leaf
        if (GetItemData(hChildItem) != NULL)
          num++;
      }
      hChildItem = GetNextSiblingItem(hChildItem);
    }
  }
  return num;
}

void CPWTreeCtrlX::DeleteWithParents(HTREEITEM hItem)
{
  // We don't want nodes that have no children to remain
  HTREEITEM parent;
  do {
    StringX sxPath = (LPCWSTR)GetGroup(hItem);
    parent = GetParentItem(hItem);
    DeleteItem(hItem);
    if (ItemHasChildren(parent))
      break;
    app.GetMainDlg()->m_mapGroupToTreeItem.erase(sxPath);
    app.GetMainDlg()->m_mapTreeItemToGroup.erase(hItem);
    hItem = parent;
  } while (parent != TVI_ROOT && parent != NULL);
}

// If passed an entry, return the full path leading up to a given item, but
// not including the name of the item itself.
// If passed a Group, return full path including this group.
CString CPWTreeCtrlX::GetGroup(HTREEITEM hItem)
{
  CString retval(L""), nodeText;
  if (hItem == TVI_ROOT)
    return retval;

  HTREEITEM hi(hItem);

  // Determine if an entry - if so - ignore first item text which will
  // be the entry's title and potentially its username and password
  if (IsLeaf(hi)) {
    hi = GetParentItem(hi);
  }

  while (hi != NULL) {
    nodeText = GetItemText(hi);
    if (!retval.IsEmpty())
      nodeText += GROUP_SEP;

    retval = nodeText + retval;
    hi = GetParentItem(hi);
  }
  return retval;
}

static StringX GetFirstPathElem(StringX &sxPath)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // path = "a..b.c.d"
  // will return "a." and path will be "b.c.d"
   // (assuming GROUP_SEP is '.')

  StringX sxElement;
  size_t dotPos = sxPath.find_first_of(GROUP_SEP);
  size_t len=sxPath.length();
  if (dotPos == StringX::npos){
    sxElement = sxPath;
    sxPath = L"";
  } else {
    while ((dotPos < len) && (sxPath[dotPos] == GROUP_SEP)) {// look for consecutive dots
      dotPos++;
    }
    if (dotPos < len) {
      sxElement = sxPath.substr(0, dotPos-1);
      sxPath = sxPath.substr(dotPos);
    }
    else { // trailing dots
      sxElement = sxPath;
      sxPath = L"";
    }
  }
  return sxElement;
}

bool CPWTreeCtrlX::ExistsInTree(HTREEITEM &node, const CSecString &s, HTREEITEM &si) const
{
  // returns true iff s is a direct descendant of node
  HTREEITEM ti = GetChildItem(node);

  while (ti != NULL) {
    const CSecString itemText = GetItemText(ti);
    if (itemText == s)
      if (!IsLeaf(ti)) { // A non-node doesn't count
        si = ti;
        return true;
      }
    ti = GetNextItem(ti, TVGN_NEXT);
  }
  return false;
}

HTREEITEM CPWTreeCtrlX::AddGroup(const CString &group, bool &bAlreadyExists)
{
  // Add a group at the end of path
  HTREEITEM ti = TVI_ROOT;
  HTREEITEM si;
  bAlreadyExists = true;

  if (!group.IsEmpty()) {
    StringX sxPath = group;
    StringX sxTemp, sxPath2Root(L"");
    do {
      sxTemp = GetFirstPathElem(sxPath);
      if (sxPath2Root.empty())
        sxPath2Root = sxTemp;
      else
        sxPath2Root += GROUP_SEP2 + sxTemp;

      if (!ExistsInTree(ti, sxTemp, si)) {
        ti = InsertItem(sxTemp.c_str(), ti, TVI_SORT);
        SetItemImage(ti, CPWTreeCtrlX::GROUP, CPWTreeCtrlX::GROUP);
        bAlreadyExists = false;
      } else
        ti = si;

      app.GetMainDlg()->m_mapGroupToTreeItem[sxPath2Root] = ti;
      app.GetMainDlg()->m_mapTreeItemToGroup[ti] = sxPath2Root;
    } while (!sxPath.empty());

    if (app.GetMainDlg()->IsEmptyGroup(StringX(group)))
      SetItemImage(ti, CPWTreeCtrlX::EMPTY_GROUP, CPWTreeCtrlX::EMPTY_GROUP);
  }
  return ti;
}

bool CPWTreeCtrlX::MoveItem(MultiCommands *pmulticmds, HTREEITEM hitemDrag, HTREEITEM hitemDrop,
                           const CSecString &sPrefix)
{
  TV_INSERTSTRUCT  tvstruct;
  wchar_t sztBuffer[260];  // max visible

  tvstruct.item.hItem = hitemDrag;
  tvstruct.item.cchTextMax = _countof(sztBuffer) - 1;
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
    CItemData *pci = (CItemData *)itemData;

    DisplayInfo *pdi = app.GetMainDlg()->GetEntryGUIInfo(*pci);
    ASSERT(pdi->list_index >= 0);

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
    CSecString ci_user = pci->GetUser();
    CSecString ci_title0 = pci->GetTitle();
    CSecString ci_title = app.GetMainDlg()->GetUniqueTitle(path, ci_title0, ci_user, IDS_DRAGNUMBER);

    // Update list field with new group
    pmulticmds->Add(UpdateEntryCommand::Create(app.GetCore(), *pci,
                                               CItemData::GROUP, path));
    app.GetMainDlg()->UpdateListItemGroup(pdi->list_index, (LPCWSTR)path);

    if (ci_title.Compare(ci_title0) != 0) {
      pmulticmds->Add(UpdateEntryCommand::Create(app.GetCore(), *pci,
                                                 CItemData::TITLE, ci_title));
    }
    // Update tree label
    SetItemText(hNewItem, MakeTreeDisplayString(*pci));
    // Update list field with new title
    app.GetMainDlg()->UpdateListItemTitle(pdi->list_index, (LPCWSTR)ci_title);

    // Update DisplayInfo record associated with ItemData
    pdi->tree_item = hNewItem;
  } else {
    // Group processing
    // If original group was empty, need to update the vector of empty groups
    // UNLESS that group already exists as a non-empty group
    CSecString sOldGroup(GetGroup(hitemDrag));
    if (app.GetMainDlg()->IsEmptyGroup(sOldGroup)) {
      CSecString sNewGroup, DropPrefix;
      if (IsLeaf(hitemDrop))
        DropPrefix = GetPrefix(hitemDrop);
      else
        DropPrefix = CSecString(GetGroup(hitemDrop));

      if (DropPrefix.IsEmpty())
        sNewGroup = CSecString(GetItemText(m_hitemDrag));
      else
        sNewGroup = DropPrefix + CSecString(GROUP_SEP2) + CSecString(GetItemText(hitemDrag));

      // We mustn't rename an empty group if it already exists as a non-empty group
      auto iter = app.GetMainDlg()->m_mapGroupToTreeItem.find(sNewGroup);
      if (iter == app.GetMainDlg()->m_mapGroupToTreeItem.end()) {
        pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(),
          sOldGroup, sNewGroup, DBEmptyGroupsCommand::EG_RENAME));
      }
    }
  }

  HTREEITEM hFirstChild;
  while ((hFirstChild = GetChildItem(hitemDrag)) != NULL) {
    MoveItem(pmulticmds, hFirstChild, hNewItem, sPrefix);  // recursively move all the items
  }

  // We are moving it - so now delete original from TreeCtrl
  // Why can't MoveItem & CopyItem be the same except for the following line?!?
  // For one, if we don't delete, we'll get into an infinite recursion.
  // For two, entry type may change on Copy or bases will get extra dependents
  DeleteItem(hitemDrag);
  return true;
}

bool CPWTreeCtrlX::CopyItem(MultiCommands *pmulticmds, HTREEITEM hitemDrag, HTREEITEM hitemDrop,
                           const CSecString &Prefix)
{
  DWORD_PTR itemData = GetItemData(hitemDrag);

  if (itemData == 0) { // we're dragging a group
    // Are we copying an empty group?
    if (CountChildren(hitemDrag) == 0) {
      // Yes - add new empty group in the right place
      CSecString sxPath, DropPrefix;
      if (IsLeaf(hitemDrop))
        DropPrefix = GetPrefix(hitemDrop);
      else
        DropPrefix = CSecString(GetGroup(hitemDrop));

      if (DropPrefix.IsEmpty())
        sxPath = CSecString(GetItemText(hitemDrag));
      else
        sxPath = DropPrefix + CSecString(GROUP_SEP2) + CSecString(GetItemText(hitemDrag));

      pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(), sxPath,
        DBEmptyGroupsCommand::EG_ADD));
    } else {
      // No - copy items
      HTREEITEM hChild = GetChildItem(hitemDrag);

      while (hChild != NULL) {
        CopyItem(pmulticmds, hChild, hitemDrop, Prefix);
        hChild = GetNextItem(hChild, TVGN_NEXT);
      }
    }
  } else { // we're dragging a leaf
    CItemData *pci = (CItemData *)itemData;
    CItemData ci_temp(*pci); // copy construct a duplicate

    // Update Group: chop away prefix, replace
    CSecString sOldPath(ci_temp.GetGroup());
    if (!Prefix.IsEmpty()) {
      sOldPath = sOldPath.Right(sOldPath.GetLength() - Prefix.GetLength() - 1);
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

    CSecString sNewPath;
    if (path.IsEmpty())
      sNewPath = sOldPath;
    else {
      sNewPath = path;
      if (!sOldPath.IsEmpty())
        sNewPath += GROUP_SEP + sOldPath;
    }
    // Get information from current selected entry
    CSecString ci_user = pci->GetUser();
    CSecString ci_title0 = pci->GetTitle();
    CSecString ci_title = app.GetMainDlg()->GetUniqueTitle(sNewPath, ci_title0,
                                                 ci_user, IDS_DRAGNUMBER);

    ci_temp.CreateUUID(); // Copy needs its own UUID
    ci_temp.SetGroup(sNewPath);
    ci_temp.SetTitle(ci_title);

    CItemData::EntryType temp_et = ci_temp.GetEntryType();
    switch (temp_et) {
    case CItemData::ET_ALIASBASE:
    case CItemData::ET_SHORTCUTBASE:
      // An alias or shortcut can only have one base
      ci_temp.SetNormal();
      // Deliberate fall-thru
    case CItemData::ET_NORMAL:
      pmulticmds->Add(AddEntryCommand::Create(app.GetCore(), ci_temp));
      break;
    case CItemData::ET_ALIAS:
      ci_temp.SetPassword(CSecString(L"[Alias]"));
      // Get base of original alias and make this copy point to it
      pmulticmds->Add(AddEntryCommand::Create(app.GetCore(), ci_temp,
                                     app.GetMainDlg()->GetBaseEntry(pci)->GetUUID()));
      break;
    case CItemData::ET_SHORTCUT:
      ci_temp.SetPassword(CSecString(L"[Shortcut]"));
      // Get base of original shortcut and make this copy point to it
      pmulticmds->Add(AddEntryCommand::Create(app.GetCore(), ci_temp,
                                     app.GetMainDlg()->GetBaseEntry(pci)->GetUUID()));
      break;
    default:
      ASSERT(0);
    }
  } // leaf handling
  return true;
}

BOOL CPWTreeCtrlX::OnDrop(CWnd *, COleDataObject *pDataObject,
                         DROPEFFECT dropEffect, CPoint point)
{
  // We need to cancel DropTarget (SelectDropTarget(NULL)) selection 
  // before every return, otherwise next mouse/keyboard selection 
  // will be treated as drop target selections

  if (pDataObject->IsDataAvailable(CF_HDROP, NULL)) {
    HGLOBAL hg;
    HDROP hdrop;
    UINT nFiles;
    wchar_t szDraggedFile[MAX_PATH];

    hg = pDataObject->GetGlobalData(CF_HDROP);
    if (hg == NULL) {
      pws_os::Trace(L"CPWTreeCtrlX::OnDrop() No global data\n");
      goto done;
    }

    hdrop = HDROP(GlobalLock(hg));
    if (hdrop == NULL) {
      pws_os::Trace(L"CPWTreeCtrlX::OnDrop() Could not lock global data\n");
      GlobalUnlock(hg);
      goto done;
    }

    nFiles = DragQueryFile(hdrop, UINT(-1), NULL, 0);
    // Support exactly one file being dropped
    if (nFiles != 1) {
      pws_os::Trace(L"CPWTreeCtrlX::OnDrop(): %d files dropped\n", nFiles);
      GlobalUnlock(hg);
      goto done;
    }

    DragQueryFile(hdrop, 0, szDraggedFile, MAX_PATH);
    GlobalUnlock(hg);
    m_droppedFile = szDraggedFile;
    app.GetMainDlg()->PostMessage(PWS_MSG_DROPPED_FILE);

  done:
    SelectDropTarget(NULL);
    return FALSE;
  }

  // Is it ours?
  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) {
    SelectDropTarget(NULL);
    return FALSE;
  }

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

  if (app.GetMainDlg()->IsDBReadOnly()) {
    SelectDropTarget(NULL);
    return FALSE; // don't drop in read-only mode
  }

  if (!pDataObject->IsDataAvailable(m_tcddCPFID, NULL)) {
    SelectDropTarget(NULL);
    return FALSE; // don't drop if not ours
  }

  UINT uFlags;
  HTREEITEM hitemDrop = HitTest(point, &uFlags);

  bool bForceRoot(false);
  switch (uFlags) {
  case TVHT_ABOVE: case TVHT_BELOW: case TVHT_TOLEFT: case TVHT_TORIGHT:
    SelectDropTarget(NULL);
    return FALSE;
  case TVHT_NOWHERE:
    if (hitemDrop == NULL) {
      // Treat as drop in root
      hitemDrop = GetRootItem();
      bForceRoot = true;
    }
    else {
      SelectDropTarget(NULL); 
      return FALSE;
    }
    break;
  case TVHT_ONITEM: case TVHT_ONITEMBUTTON: case TVHT_ONITEMICON:
  case TVHT_ONITEMINDENT: case TVHT_ONITEMLABEL: case TVHT_ONITEMRIGHT:
  case TVHT_ONITEMSTATEICON:
    if (hitemDrop == NULL){
      SelectDropTarget(NULL);
      return FALSE;
    }
    break;
  default:
    SelectDropTarget(NULL);
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

  sscanf_s((char *)pData, OLE_HDR_FMT, &lPid, &iDDType, &lBufLen);
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
          CItemData *pci;
          DWORD_PTR itemData;

          itemData = GetItemData(m_hitemDrop);
          if (itemData == NULL) {
            // Dropping on a group
            cs_group = CSecString(GetGroup(m_hitemDrop));
          } else {
            // Dropping on an entry
            pci = (CItemData *)itemData;
            cs_group = pci->GetGroup();
          }

          itemData = GetItemData(m_hitemDrag);
          ASSERT(itemData != NULL);
          pci = (CItemData *)itemData;
          cs_title.Format(IDS_SCTARGET, static_cast<LPCWSTR>(pci->GetTitle().c_str()));
          cs_user = pci->GetUser();

          // If there is a matching entry in our list, generate unique one
          if (app.GetMainDlg()->Find(cs_group, cs_title, cs_user) != app.GetMainDlg()->End()) {
            cs_title = app.GetMainDlg()->GetUniqueTitle(cs_group, cs_title, cs_user, IDS_DRAGNUMBER);
          }
          StringX sxNewDBPrefsString(L"");
          app.GetMainDlg()->CreateShortcutEntry(pci, cs_group, cs_title, cs_user, sxNewDBPrefsString);
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
    CSecString DropGroup (L"");
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
    
    // Only interested in children of the parent - not grand-children etc.
    const int numchildren = CountChildren(parent, false);

    if (m_hitemDrag != hitemDrop &&
        !IsChildNodeOf(hitemDrop, m_hitemDrag) &&
        parent != hitemDrop) {
      // Drag operation allowed
      if (dropEffect == DROPEFFECT_MOVE || dropEffect == DROPEFFECT_COPY) {

        MultiCommands *pmulticmds = MultiCommands::Create(app.GetCore());
        pmulticmds->Add(UpdateGUICommand::Create(app.GetCore(),
          UpdateGUICommand::WN_UNDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));

        // Make sure that the folder to which drag is performed will 
        // be removed from the vector of empty groups
        StringX sxGroup(GetGroup(hitemDrop));
        if (app.GetMainDlg()->IsEmptyGroup(sxGroup)) {
          pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(), sxGroup,
            DBEmptyGroupsCommand::EG_DELETE));
        }

        StringX sxDropGroup(L"");
        bool bEmptyGroup(false);

        switch (dropEffect) {
        case DROPEFFECT_MOVE:
          // If item was last entry in the parent group, be it a leaf or a node,
          // make it an empty group
          if (numchildren == 1) {
            const StringX sxPath = GetGroup(parent);
            pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(), sxPath,
              DBEmptyGroupsCommand::EG_ADD));
          }

          if (IsLeaf(m_hitemDrop)) {
            CItemData *pci = (CItemData *)GetItemData(m_hitemDrop);
            ASSERT(pci != NULL);
            sxDropGroup = pci->GetGroup();
          } else {
            sxDropGroup = GetGroup(m_hitemDrag);
            if (app.GetCore()->IsEmptyGroup(sxDropGroup)) {
              bEmptyGroup = true;
            }
          }

          if (bEmptyGroup) {
            // Move empty group
            // If it is, then the user has selected the last entry (groups
            // before it are, by definition, not empty)

            // Delete it from list of empty groups
            pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(), sxDropGroup,
              DBEmptyGroupsCommand::EG_DELETE));

            // Add it in its new location
            CSecString sxPath, DropPrefix;
            if (IsLeaf(hitemDrop))
              DropPrefix = GetPrefix(hitemDrop);
            else
              DropPrefix = CSecString(GetGroup(hitemDrop));

            if (DropPrefix.IsEmpty())
              sxPath = CSecString(GetItemText(m_hitemDrag));
            else
              sxPath = DropPrefix + CSecString(GROUP_SEP2) + CSecString(GetItemText(m_hitemDrag));

            pmulticmds->Add(DBEmptyGroupsCommand::Create(app.GetCore(), sxPath,
              DBEmptyGroupsCommand::EG_ADD));
          } else {
            // Move tree item (entry or non-empty group)
            MoveItem(pmulticmds, m_hitemDrag, hitemDrop, sxDropGroup);
          }
          break;
        case DROPEFFECT_COPY:
          CopyItem(pmulticmds, m_hitemDrag, hitemDrop, GetPrefix(m_hitemDrag));
          SortTree(hitemDrop);
          break;
        }

        pmulticmds->Add(UpdateGUICommand::Create(app.GetCore(),
          UpdateGUICommand::WN_EXECUTE_REDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));

        // Do it
        app.GetMainDlg()->Execute(pmulticmds);

        SelectItem(hitemDrop);
        retval = TRUE;
      }
    } else {
      // Drag failed or cancelled, revert to last selected
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
  app.GetMainDlg()->FixListIndexes();
  GetParent()->SetFocus();

exit:
  SelectDropTarget(NULL);
  GlobalUnlock(hGlobal);

  if (retval == TRUE) {
    app.GetMainDlg()->ChangeOkUpdate();
    if (app.GetMainDlg()->IsFilterActive())
      app.GetMainDlg()->RefreshViews();
  }
  return retval;
}

void CPWTreeCtrlX::OnBeginDrag(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // This sets the whole D&D mechanism in motion...
  if (pNotifyStruct->code == TVN_BEGINDRAG)
    m_DDType = FROMTREE_L; // Left  mouse D&D
  else
    m_DDType = FROMTREE_R; // Right mouse D&D

  CPoint ptAction;

  NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNotifyStruct;
  *pLResult = 0L;

  GetCursorPos(&ptAction);
  ScreenToClient(&ptAction);
  m_hitemDrag = pNMTreeView->itemNew.hItem;
  m_hitemDrop = NULL;
  SelectItem(m_hitemDrag);

  // Bug in MS TreeCtrl and CreateDragImage.  During Drag, it doesn't show
  // the entry's text as well as the drag image if the font is not MS Sans Serif !!!!
  SetFont(Fonts::GetInstance()->GetDragFixFont(), false);
  CImageList *pil = CreateDragImage(m_hitemDrag);
  SetFont(Fonts::GetInstance()->GetTreeListFont(), false);

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
    CItemData *pci = (CItemData *)itemData;
    if (pci->IsNormal() || pci->IsShortcutBase())
      m_DDType = FROMTREE_RSC;  // Shortcut creation allowed (if within this instance)
  }

  // Start dragging
  m_bDropped = false;
  DROPEFFECT de = m_pDataSource->StartDragging(m_tcddCPFID, &rClient);

  // If inter-process Move, we need to delete original
  if (m_cfdropped == m_tcddCPFID &&
      (de & DROPEFFECT_MOVE) == DROPEFFECT_MOVE &&
      !m_bWithinThisInstance && !app.GetMainDlg()->IsDBReadOnly()) {
    MultiCommands *pmcmd = MultiCommands::Create(app.GetMainDlg()->GetCore());
    app.GetMainDlg()->Delete(pmcmd); // XXX assume we've a selected item here!
   
    // Now do it
    if (!pmcmd->IsEmpty()) {
      app.GetMainDlg()->Execute(pmcmd);
    }
  }

  // wrong place to clean up imagelist?
  pil->DragLeave(GetDesktopWindow());
  pil->EndDrag();
  pil->DeleteImageList();
  delete pil;

  if (de == DROPEFFECT_NONE) {
    pws_os::Trace(L"m_pDataSource->StartDragging() failed\n");
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
  }

  while (ShowCursor(TRUE) < 0)
    ;

  // We did call SetCapture - do we release it here?  If not, where else?
  ReleaseCapture();
}

void CPWTreeCtrlX::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_ND_HOVER:
      KillTimer(m_nHoverNDTimerID);
      m_nHoverNDTimerID = 0;
      if (app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint)) {
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
      app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint, false);
      break;
    default:
      CTreeCtrl::OnTimer(nIDEvent);
      break;
  }
}

void CPWTreeCtrlX::OnMouseMove(UINT nFlags, CPoint point)
{
  app.GetMainDlg()->ResetIdleLockCounter();
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
    app.GetMainDlg()->SetNotesWindow(CPoint(0, 0), false);
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

LRESULT CPWTreeCtrlX::OnMouseLeave(WPARAM, LPARAM)
{
  KillTimer(m_nHoverNDTimerID);
  KillTimer(m_nShowNDTimerID);
  m_nHoverNDTimerID = m_nShowNDTimerID = 0;
  m_HoverNDPoint = CPoint(0, 0);
  app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint, false);
  m_bMouseInWindow = false;
  return 0L;
}

void CPWTreeCtrlX::OnExpandCollapse(NMHDR *, LRESULT *)
{
  // We need to update the parent's state vector of expanded nodes
  // so that it will be persistent across minimize, lock, save, etc.
  // (unless we're in the middle of restoring the state!)

  if (!m_isRestoring) {
    app.GetMainDlg()->SaveGUIStatusEx(DboxMain::TREEONLY);
  }
}

void CPWTreeCtrlX::OnExpandAll() 
{
  // Updated to test for zero entries!
  HTREEITEM hItem = this->GetRootItem();
  if (hItem == NULL)
    return;

  CWaitCursor wait;

  //SetRedraw(FALSE);
  do {
    Expand(hItem, TVE_EXPAND);
  } while ((hItem = GetNextItem(hItem, TVGN_NEXTVISIBLE)) != NULL);

  EnsureVisible(GetSelectedItem());
  //SetRedraw(TRUE);

  wait.Restore();

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::TREEONLY);
}

void CPWTreeCtrlX::OnCollapseAll() 
{
  // Courtesy of Zafir Anjum from www.codeguru.com
  // Updated to test for zero entries!
  HTREEITEM hItem = GetRootItem();
  if (hItem == NULL)
    return;

  //SetRedraw(FALSE);
  do {
    CollapseBranch(hItem);
  } while((hItem = GetNextSiblingItem(hItem)) != NULL);
  //SetRedraw(TRUE);

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::TREEONLY);
}

void CPWTreeCtrlX::CollapseBranch(HTREEITEM hItem)
{
  // Courtesy of Zafir Anjumfrom www.codeguru.com
  if (ItemHasChildren(hItem)) {
    Expand(hItem, TVE_COLLAPSE);
    hItem = GetChildItem(hItem);
    do {
      CollapseBranch(hItem);
    } while((hItem = GetNextSiblingItem(hItem)) != NULL);
  }
}

HTREEITEM CPWTreeCtrlX::GetNextTreeItem(HTREEITEM hItem) 
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

void CPWTreeCtrlX::Iterate(HTREEITEM hItem, TreeItemFunctor &functor)
{
  if (hItem) {
    // Start at supplied entry
    functor(hItem); // apply whatever needs to be done
    hItem = GetNextItem(hItem, TVGN_CHILD);
    while (hItem) {
      Iterate(hItem, functor);
      hItem = GetNextItem(hItem, TVGN_NEXT);
    }
  } else {
    // Start at root
    HTREEITEM hItem2 = GetNextItem(NULL, TVGN_ROOT);
    while (hItem2) {
      Iterate(hItem2, functor);
      hItem2 = GetNextItem(hItem2, TVGN_NEXT);
    }
  }
}

bool CPWTreeCtrlX::CollectData(BYTE * &out_buffer, long &outLen)
{
  DWORD_PTR itemData = GetItemData(m_hitemDrag);
  CItemData *pci = (CItemData *)itemData;
  std::vector<StringX> vEmptyGroups;

  CDDObList out_oblist;

  if (IsLeaf(m_hitemDrag)) {
    ASSERT(itemData != NULL);
    m_nDragPathLen = 0;
    out_oblist.m_bDragNode = false;
    GetEntryData(out_oblist, pci);
  } else {
    const StringX DragPathParent = GetGroup(GetParentItem(m_hitemDrag));
    m_nDragPathLen = (int)DragPathParent.length();

    StringX DragPath = GetGroup(m_hitemDrag);
    // Check if this is an empty group
    if (app.GetMainDlg()->IsEmptyGroup(DragPath)) {
      // Don't bother looking for children
      vEmptyGroups.push_back(DragPath.substr(m_nDragPathLen == 0 ? m_nDragPathLen : m_nDragPathLen + 1));
    } else {
      out_oblist.m_bDragNode = true;
      GetGroupEntriesData(out_oblist, m_hitemDrag);

      // Get all empty groups within original drag group
      std::vector<StringX> vAllEmptyGroups;
      vAllEmptyGroups = app.GetCore()->GetEmptyGroups();

      DragPath += StringX(L".");
      const size_t draglen = DragPath.length();

      for (size_t i = 0; i < vAllEmptyGroups.size(); i++) {
        if (CompareNoCase(vAllEmptyGroups[i].substr(0, draglen), DragPath) == 0) {
          StringX veg = vAllEmptyGroups[i].substr(m_nDragPathLen == 0 ? 0 : m_nDragPathLen + 1,
                 StringX::npos);
          vEmptyGroups.push_back(veg);
        }
      }
    }
  }

  CSMemFile outDDmemfile;
  out_oblist.DDSerialize(outDDmemfile);

  while (!out_oblist.IsEmpty()) {
    delete (CDDObject *)out_oblist.RemoveHead();
  }

  // Now process empty groups
  if (!vEmptyGroups.empty()) {
    if (outDDmemfile.GetLength() == 0) {
      // No entries written but as we are going to write empty groups
      // we need to put a zero count
      int nCount(0);
      outDDmemfile.Write((void *)&nCount, sizeof(nCount));
      outDDmemfile.Write((void *)&out_oblist.m_bDragNode, sizeof(bool));
    }

    // Add special field to ensure we recognise the extra data correctly
    // when dropping
    outDDmemfile.Write("egrp", 4);

    size_t nemptygroups = vEmptyGroups.size();
    outDDmemfile.Write((void *)&nemptygroups, sizeof(nemptygroups));
    for (size_t i = 0; i < nemptygroups; i++) {
      CUTF8Conv conv;
      const unsigned char *utf8;
      size_t utf8Len;

      if (conv.ToUTF8(vEmptyGroups[i].c_str(), utf8, utf8Len)) {
        outDDmemfile.Write((void *)&utf8Len, sizeof(utf8Len));
        outDDmemfile.Write(reinterpret_cast<const char *>(utf8), (UINT)utf8Len);
      } else {
        ASSERT(0);
      }
    }

    outDDmemfile.Flush();
    long len = (long)outDDmemfile.GetLength();
    unsigned char *buffer = new unsigned char[len];
    outDDmemfile.SeekToBegin();
    outDDmemfile.Read(buffer, len);
    //pws_os::HexDump(buffer, len, L"", 32);
    delete[] buffer;
  }

  outLen = (long)outDDmemfile.GetLength();
  out_buffer = (BYTE *)outDDmemfile.Detach();

  return (outLen > 0);
}

bool CPWTreeCtrlX::ProcessData(BYTE *in_buffer, const long &inLen,
                              const CSecString &DropGroup)
{
#ifdef DUMP_DATA
  std:wstring stimestamp;
  PWSUtil::GetTimeStamp(stimestamp);
  pws_os::Trace(L"Drop data: length %d/0x%04x, value:\n", inLen, inLen);
  pws_os::HexDump(in_buffer, inLen, stimestamp);
#endif /* DUMP_DATA */

  if (inLen <= 0)
    return false;

  CDDObList in_oblist;
  CSMemFile inDDmemfile;

  inDDmemfile.Attach((BYTE *)in_buffer, inLen);

  // Get all the entries
  in_oblist.DDUnSerialize(inDDmemfile);

  // Now check if empty group list is appended to the item data
  // Empty groups have a dummy header of 'egrp' to check it is ours
  // Note: No EOF indication in CFile and hence CMemfile only to check
  // we have read in is what we wanted!
  std::vector<StringX> vsxEmptyGroups;
  StringX sxDropGroup(L"");
  if (!DropGroup.IsEmpty())
    sxDropGroup = StringX(LPCWSTR(DropGroup)) + StringX(L".");

  char chdr[5] = { 0 };
  UINT num_chars = inDDmemfile.Read(chdr, 4);
  if (num_chars == 4 && strcmp(chdr, "egrp") == 0) {
    // It is ours - now process the empty groups being dropped
    size_t nemptygroups, utf8Len, buffer_size = 512;
    unsigned char *utf8 = new unsigned char[buffer_size];
    inDDmemfile.Read((void *)&nemptygroups, sizeof(nemptygroups));
    for (size_t i = 0; i < nemptygroups; i++) {
      StringX sxEmptyGroup;
      CUTF8Conv conv;
      UINT num_read;

      num_read = inDDmemfile.Read((void *)&utf8Len, sizeof(utf8Len));
      ASSERT(num_read == sizeof(utf8Len));
      if (utf8Len > buffer_size) {
        delete[] utf8;
        utf8 = new unsigned char[utf8Len * 2];
        buffer_size = utf8Len * 2;
      }

      // Clear buffer
      memset(utf8, 0, buffer_size);
      num_read = inDDmemfile.Read(utf8, (UINT)utf8Len);
      ASSERT(num_read == utf8Len);        
        
      conv.FromUTF8(utf8, utf8Len, sxEmptyGroup);
      vsxEmptyGroups.push_back(sxDropGroup + sxEmptyGroup);
    }
    delete[] utf8;
  }

  if (!in_oblist.IsEmpty() || !vsxEmptyGroups.empty()) {
    app.GetMainDlg()->AddDDEntries(in_oblist, DropGroup, vsxEmptyGroups);

    while (!in_oblist.IsEmpty()) {
      delete (CDDObject *)in_oblist.RemoveHead();
    }
  }

  // All finished
  inDDmemfile.Detach();
  return (inLen > 0);
}

void CPWTreeCtrlX::GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem)
{
  if (IsLeaf(hItem)) {
    DWORD_PTR itemData = GetItemData(hItem);
    ASSERT(itemData != NULL);
    CItemData *pci = (CItemData *)itemData;
    GetEntryData(out_oblist, pci);
  } else {
    HTREEITEM child;
    for (child = GetChildItem(hItem);
      child != NULL;
      child = GetNextSiblingItem(child)) {
        GetGroupEntriesData(out_oblist, child);
    }
  }
}

void CPWTreeCtrlX::GetEntryData(CDDObList &out_oblist, CItemData *pci)
{
  ASSERT(pci != NULL);
  CDDObject *pDDObject = new CDDObject;

  if (out_oblist.m_bDragNode && m_nDragPathLen > 0) {
    CItemData ci2(*pci); // we need a copy since to modify the group
    const CSecString cs_Group = pci->GetGroup();
    ci2.SetGroup(cs_Group.Right(cs_Group.GetLength() - m_nDragPathLen - 1));
    pDDObject->FromItem(ci2);
  } else {
    pDDObject->FromItem(*pci);
  }

  if (pci->IsDependent()) {
    // I'm an alias or shortcut; pass on ptr to my base item
    // to retrieve its group/title/user
    const CItemData *pbci = app.GetMainDlg()->GetBaseEntry(pci);
    ASSERT(pbci != NULL);
    pDDObject->SetBaseItem(pbci);
  }

  out_oblist.AddTail(pDDObject);
}

CSecString CPWTreeCtrlX::GetPrefix(HTREEITEM hItem) const
{
  // return all path components between hItem and root.
  // e.g., if hItem is X in a.b.c.X.y.z, then return a.b.c
  CSecString retval;
  HTREEITEM p = GetParentItem(hItem);
  while (p != NULL) {
    retval = CSecString(GetItemText(p)) + retval;
    p = GetParentItem(p);
    if (p != NULL)
      retval = CSecString(GROUP_SEP2) + retval;
  }
  return retval;
}

CSecString CPWTreeCtrlX::MakeTreeDisplayString(const CItemData &ci) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);

  CSecString treeDispString = ci.GetTitle();
  if (bShowUsernameInTree) {
    treeDispString += L" [";
    treeDispString += ci.GetUser();
    treeDispString += L"]";

    if (bShowPasswordInTree) {
      treeDispString += L" {";
      treeDispString += ci.GetPassword();
      treeDispString += L"}";
    }
  }

  if (ci.IsProtected() || ci.HasAttRef()) {
    treeDispString += L" ";
  }

  if (ci.IsProtected())
    treeDispString += Fonts::GetInstance()->GetProtectedSymbol();

   if (ci.HasAttRef())
    treeDispString += Fonts::GetInstance()->GetAttachmentSymbol();

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

void CPWTreeCtrlX::SortTree(const HTREEITEM htreeitem)
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

void CPWTreeCtrlX::SetFilterState(bool bState)
{
  m_bTreeFilterActive = bState;

  // Red if filter active, black if not
  SetTextColor(m_bTreeFilterActive ? RGB(168, 0, 0) : RGB(0, 0, 0));
}

BOOL CPWTreeCtrlX::OnEraseBkgnd(CDC* pDC)
{
  if (m_bTreeFilterActive && app.GetMainDlg()->GetNumPassedFiltering() == 0) {
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

BOOL CPWTreeCtrlX::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
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
    
BOOL CPWTreeCtrlX::RenderTextData(CLIPFORMAT &cfFormat, HGLOBAL* phGlobal)
{
  if (!IsLeaf(m_hitemDrag)) {
    pws_os::Trace(L"CPWTreeCtrlX::RenderTextData - not a leaf!\n");
    return FALSE;
  }

  // Save format for use by D&D to ensure we don't 'move' when dropping text!
  m_cfdropped = cfFormat;

  DWORD_PTR itemData = GetItemData(m_hitemDrag);
  const CItemData *pci = (const CItemData *)itemData;

  if (pci->IsDependent()) {
    const CItemData *pbci = app.GetMainDlg()->GetBaseEntry(pci);
    ASSERT(pbci != NULL);
    pci = pbci;
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

  if (cfFormat == CF_UNICODETEXT) {
    // So is requested data!
    dwBufLen = (ilen + 1) * sizeof(wchar_t);
    lpszW = new WCHAR[ilen + 1];
    (void) wcsncpy_s(lpszW, ilen + 1, cs_dragdata, ilen);
  } else {
    // They want it in ASCII - use lpszW temporarily
    lpszW = cs_dragdata.GetBuffer(ilen + 1);
    dwBufLen = WideCharToMultiByte(CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL);
    ASSERT(dwBufLen != 0);
    lpszA = new char[dwBufLen];
    WideCharToMultiByte(CP_ACP, 0, lpszW, -1, lpszA, dwBufLen, NULL, NULL);
    cs_dragdata.ReleaseBuffer();
    lpszW = NULL;
  }

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
    pws_os::Trace(L"CPWTreeCtrlX::OnRenderTextData - *phGlobal NOT NULL!\n");
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(*phgData);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      pws_os::Trace(L"CPWTreeCtrlX::OnRenderTextData - NOT enough room - FAIL\n");
    } else {
      // Enough room - copy our data into supplied area
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
  delete[] lpszA;
  delete[] lpszW;
  // Since lpDataBuffer pointed to one of the above - just zero the pointer
  lpDataBuffer = NULL;

  // If retval == TRUE, recipient is responsible for freeing the global memory
  // if D&D succeeds (see after StartDragging in OnMouseMove)
  if (retval == FALSE) {
    pws_os::Trace(L"CPWTreeCtrlX::RenderTextData - returning FALSE!\n");
    if (*phgData != NULL) {
      lpData = GlobalLock(*phgData);
      SIZE_T memsize = GlobalSize(*phgData);
      if (lpData != NULL && memsize > 0) {
        trashMemory(lpData, memsize);
      }
      GlobalUnlock(*phgData);
      GlobalFree(*phgData);
      *phgData = NULL;
    }
  } else {
    /*
    pws_os::Trace(L"CPWTreeCtrlX::RenderTextData - D&D Data:\n");
    if (cfFormat == CF_UNICODETEXT) {
      pws_os::Trace(L"\t\"%ls\"\n", (LPWSTR)lpData);  // data is Unicode
    } else {
      pws_os::Trace(L"\t\"%hs\"\n", (LPSTR)lpData);  // data is NOT Unicode
    }
    */
  }

  if (lpData != NULL)
    GlobalUnlock(*phgData);

    return retval;
}

BOOL CPWTreeCtrlX::RenderAllData(HGLOBAL* phGlobal)
{
  long lBufLen;
  BYTE *buffer = NULL;

  ASSERT(m_hgDataALL == NULL);

  // CollectData allocates buffer - need to free later
  if (!CollectData(buffer, lBufLen))
    return FALSE;

  char header[OLE_HDR_LEN + 1];
  // Note: GetDDType will return either FROMTREE or FROMTREE_R
  sprintf_s(header, sizeof(header),
            OLE_HDR_FMT, GetCurrentProcessId(), GetDDType(), lBufLen);
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
  std::wstring stimestamp;
  PWSUtil::GetTimeStamp(stimestamp);
  pws_os::Trace(L"Drag data: length %d/0x%04x, value:\n", dwBufLen, dwBufLen);
  pws_os::HexDump(lpDataBuffer, dwBufLen, stimestamp);
#endif /* DUMP_DATA */

  BOOL retval(FALSE);
  if (*phGlobal == NULL) {
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
    pws_os::Trace(L"CPWTreeCtrlX::OnRenderAllData - *phGlobal NOT NULL!\n");
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(m_hgDataALL);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      pws_os::Trace(L"CPWTreeCtrlX::OnRenderAllData - NOT enough room - FAIL\n");
    } else {
      // Enough room - copy our data into supplied area
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
    pws_os::Trace(L"CPWTreeCtrlX::RenderAllData - returning FALSE!\n");
    if (m_hgDataALL != NULL) {
      lpData = GlobalLock(m_hgDataALL);
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

CFont *CPWTreeCtrlX::GetFontBasedOnStatus(HTREEITEM &hItem, CItemData *pci, COLORREF &cf)
{
  Fonts *pFonts = Fonts::GetInstance();
  if (pci == NULL) {
    StringX path = GetGroup(hItem);
    if (app.GetMainDlg()->IsNodeModified(path)) {
      cf = pFonts->GetModified_Color();
      return pFonts->GetItalicTreeListFont();
    } else
      return NULL;
  }

  switch (pci->GetStatus()) {
    case CItemData::ES_ADDED:
    case CItemData::ES_MODIFIED:
      cf = pFonts->GetModified_Color();
      return pFonts->GetItalicTreeListFont();
    default:
      break;
  }
  return NULL;
}

void CPWTreeCtrlX::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMTVCUSTOMDRAW *pTVCD = reinterpret_cast<NMTVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;

  static bool bchanged_item_font(false);
  static bool bitem_selected(false);
  static CFont *pTreeListFont;
  static CDC *pDC = NULL;
  
  HTREEITEM hItem = (HTREEITEM)pTVCD->nmcd.dwItemSpec;
  CItemData *pci = (CItemData *)pTVCD->nmcd.lItemlParam;

  switch (pTVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      pDC = CDC::FromHandle(pTVCD->nmcd.hdc);
      bchanged_item_font = false;
      pTreeListFont = Fonts::GetInstance()->GetTreeListFont();
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      bitem_selected = (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED) != 0;
      if (m_bUseHighLighting) {
        COLORREF cf;
        CFont *uFont = GetFontBasedOnStatus(hItem, pci, cf);
        if (uFont != NULL) {
          bchanged_item_font = true;
          pDC->SelectObject(uFont);
          // Set text color only when current node isn't selected
          if (!bitem_selected)
            pTVCD->clrText = cf;
          *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
        }
      }
      break;

    case CDDS_ITEMPOSTPAINT:
      // Item PostPaint - restore old font if any
      if (bchanged_item_font) {
        bchanged_item_font = bitem_selected = false;
        SelectObject(pTVCD->nmcd.hdc, (HFONT)pTreeListFont);
        *pLResult |= CDRF_NEWFONT;
      }
      break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_POSTPAINT:
    */
    default:
      break;
  }
}

HTREEITEM CPWTreeCtrlX::FindItem(const CString &path, HTREEITEM hRoot)
{
  // check whether the current item is the searched one
  CString cs_thispath = GetGroup(hRoot);// + GROUP_SEP2 + GetItemText(hRoot);
  if (cs_thispath.Compare(path) == 0)
    return hRoot; 

  // get a handle to the first child item
  HTREEITEM hSub = GetChildItem(hRoot);
  // iterate as long a new item is found
  while (hSub) {
    // check the children of the current item
    HTREEITEM hFound = FindItem(path, hSub);
    if (hFound)
      return hFound; 

    // get the next sibling of the current item
    hSub = GetNextSiblingItem(hSub);
  } 

  // return NULL if nothing was found
  return NULL;
}
