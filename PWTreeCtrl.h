/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/** \file
* Silly subclass of CTreeCtrl just to implement Drag&Drop.
*/

#include <Afxcmn.h>
#include "SecString.h"

class DboxMain;
class CItemData;
class CInfoDisplay;

// classes for implementing D&D
class CDDObList;
class CPWTDropTarget;
class CPWTDataSource;
class CPWTDropSource;

class CPWTreeCtrl : public CTreeCtrl
{
public:
  CPWTreeCtrl();
  ~CPWTreeCtrl();

  // indices of bitmaps in ImageList
  // NOTE for normal and base entries items, order MUST be: 
  //    Not-Expired, Warn-Expired, Expired
  // used by DboxMain::GetEntryImage & ExpPWListDlg
  enum {NODE = 0,
    NORMAL, WARNEXPIRED_NORMAL, EXPIRED_NORMAL,
    ALIASBASE, WARNEXPIRED_ALIASBASE, EXPIRED_ALIASBASE, ALIAS,
    SHORTCUTBASE, WARNEXPIRED_SHORTCUTBASE, EXPIRED_SHORTCUTBASE, SHORTCUT};

  void Initialize();
  void ActivateND(const bool bActivate);

  void DeleteWithParents(HTREEITEM hItem); // if a parent node becomes a leaf
  CString GetGroup(HTREEITEM hItem); // get group path to hItem
  HTREEITEM AddGroup(const CString &path);
  void SortTree(const HTREEITEM htreeitem);
  bool IsLeaf(HTREEITEM hItem);
  CSecString MakeTreeDisplayString(const CItemData &ci) const;
  void SetRestoreMode(bool flag) {m_isRestoring = flag;}
  void OnCollapseAll();
  void OnExpandAll();
  HTREEITEM GetNextTreeItem(HTREEITEM hItem);
  // Drag-n-Drop interface - called indirectly via src/tgt member functions
  // Source methods
  SCODE GiveFeedback(DROPEFFECT dropEffect );
  // target methods
  BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
    DROPEFFECT dropEffect, CPoint point);
  DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
    DWORD dwKeyState, CPoint point);
  DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
    DWORD dwKeyState, CPoint point);
  void OnDragLeave();
  bool IsDropOnMe() {return m_bWithinThisInstance;}
  int GetDDType() {return m_DDType;}
  void EndDrop() {m_bDropped = true;}
  void SetFilterState(bool bState);
  bool WasLabelEdited() {return m_bEditLabelCompleted;};

protected:
  //{{AFX_MSG(CPWTreeCtrl)
  afx_msg void OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *result);
  afx_msg void OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *result);
  afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnDestroy();
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  //
  //afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG

  BOOL PreTranslateMessage(MSG* pMsg);
  BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
  BOOL RenderTextData(CLIPFORMAT &cfFormat, HGLOBAL* phGlobal);
  BOOL RenderAllData(HGLOBAL* phGlobal);

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;
  HTREEITEM m_hitemDrag;
  HTREEITEM m_hitemDrop;

  // Hovering related items
  HTREEITEM m_hitemHover;
  DWORD m_TickCount;

  bool m_isRestoring; // don't update state vector when restoring state
  int m_nDragPathLen;
  bool m_bWithinThisInstance;
  // For dealing with distinguishing between left & right-mouse drag
  int m_DDType;

  // in an ideal world, following would be is-a, rather than has-a
  // (multiple inheritance) Microsoft doesn't really support this, however...
  CPWTDropTarget *m_DropTarget;
  CPWTDropSource *m_DropSource;
  CPWTDataSource *m_DataSource;
  friend class CPWTDataSource;
  // Clipboard format for our Drag & Drop
  CLIPFORMAT m_tcddCPFID;
  HGLOBAL m_hgDataALL, m_hgDataUTXT, m_hgDataTXT;
  CLIPFORMAT m_cfdropped;
  bool m_bDropped;

  CSecString m_eLabel; // label at start of edit, if we need to revert
  void SetNewStyle(long lStyleMask, BOOL bSetBits);
  bool MoveItem(HTREEITEM hitem, HTREEITEM hNewParent);
  bool CopyItem(HTREEITEM hitem, HTREEITEM hNewParent, const CSecString &prefix);
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
  void UpdateLeafsGroup(HTREEITEM hItem, CString prefix);
  void CollapseBranch(HTREEITEM hItem);
  CSecString GetPrefix(HTREEITEM hItem) const;
  bool CollectData(BYTE * &out_buffer, long &outLen);
  bool ProcessData(BYTE *in_buffer, const long &inLen, const CSecString &DropGroup);
  void GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem);
  void GetEntryData(CDDObList &out_oblist, CItemData *ci);

  // Notes Display
  UINT m_nHoverNDTimerID, m_nShowNDTimerID;
  CPoint m_HoverNDPoint;
  bool m_bShowNotes, m_bMouseInWindow;

  // Filter
  bool m_bFilterActive;
  bool m_bEditLabelCompleted;
};
