/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "PWTouch.h"
#include "SecString.h"
#include "Fonts.h"

#include <vector>

class CItemData;
class MultiCommands;

// classes for implementing D&D
class CDDObList;
class CPWTDropTarget;
class CPWTDataSource;
class CPWTDropSource;

class CPWTreeCtrlX : public CTreeCtrl
{
public:
  CPWTreeCtrlX();
  ~CPWTreeCtrlX();

  // indices of bitmaps in ImageList
  // NOTE for normal and base entries items, order MUST be: 
  //    Not-Expired, Warn-Expired, Expired
  // used by DboxMain::GetEntryImage & ExpPWListDlg
  enum {GROUP = 0,
    NORMAL, WARNEXPIRED_NORMAL, EXPIRED_NORMAL,
    ALIASBASE, WARNEXPIRED_ALIASBASE, EXPIRED_ALIASBASE, ALIAS,
    SHORTCUTBASE, WARNEXPIRED_SHORTCUTBASE, EXPIRED_SHORTCUTBASE, SHORTCUT,
    EMPTY_GROUP,
    NUM_IMAGES};

  struct TreeItemFunctor { // For use by Iterate()
    virtual void operator()(HTREEITEM) = 0;
    virtual ~TreeItemFunctor() {}                                   
  };
  
  void Initialize();
  void ActivateND(const bool bActivate);

  void DeleteWithParents(HTREEITEM hItem); // if a parent node becomes a leaf
  CString GetGroup(HTREEITEM hItem); // get group path to hItem
  HTREEITEM AddGroup(const CString &path, bool &bAlreadyExists);
  void SortTree(const HTREEITEM htreeitem);
  bool IsLeaf(HTREEITEM hItem) const;
  int CountChildren(HTREEITEM hStartItem, bool bRecurse = true) const;
  int CountLeafChildren(HTREEITEM hStartItem) const;
  CSecString MakeTreeDisplayString(const CItemData &ci) const;
  void SetRestoreMode(bool flag) {m_isRestoring = flag;}
  void OnCollapseAll();
  void OnExpandAll();
  HTREEITEM GetNextTreeItem(HTREEITEM hItem);
  // Iterate() applies functor to all items in subtree rooted at hItem,
  // including hItem
  void Iterate(HTREEITEM hItem, TreeItemFunctor &functor);
  // Drag-n-Drop interface - called indirectly via src/tgt member functions
  // Source methods
  SCODE GiveFeedback(DROPEFFECT dropEffect);
  // target methods
  BOOL OnDrop(CWnd *pWnd, COleDataObject *pDataObject,
    DROPEFFECT dropEffect, CPoint point);
  DROPEFFECT OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject,
    DWORD dwKeyState, CPoint point);
  DROPEFFECT OnDragOver(CWnd *pWnd, COleDataObject *pDataObject,
    DWORD dwKeyState, CPoint point);
  void OnDragLeave();
  bool IsDropOnMe() {return m_bWithinThisInstance;}
  int GetDDType() {return m_DDType;}
  void EndDrop() {m_bDropped = true;}
  void SetFilterState(bool bState);
  bool WasLabelEdited() {return m_bEditLabelCompleted;};
  void SetUpFont();
  void SetHighlightChanges(bool bvalue)
  {m_bUseHighLighting = bvalue;}
  HTREEITEM FindItem(const CString &path, HTREEITEM hRoot);
  const StringX &GetDroppedFile() const {return m_droppedFile;}

 protected:
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  //{{AFX_MSG(CPWTreeCtrlX)
  afx_msg void OnBeginLabelEdit(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnEndLabelEdit(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnBeginDrag(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnSelectionChanged(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnDeleteItem(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnDestroy();
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnPaint();
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  //}}AFX_MSG

  BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
  BOOL RenderTextData(CLIPFORMAT &cfFormat, HGLOBAL* phGlobal);
  BOOL RenderAllData(HGLOBAL* phGlobal);

  DECLARE_MESSAGE_MAP()

private:
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
  CPWTDropTarget *m_pDropTarget;
  CPWTDropSource *m_pDropSource;
  CPWTDataSource *m_pDataSource;
  friend class CPWTDataSource;

  // Clipboard format for our Drag & Drop
  CLIPFORMAT m_tcddCPFID;
  HGLOBAL m_hgDataALL, m_hgDataUTXT, m_hgDataTXT;
  CLIPFORMAT m_cfdropped;
  bool m_bDropped;
  StringX m_droppedFile;
  
  CSecString m_eLabel; // label at start of edit, if we need to revert

  bool MoveItem(MultiCommands *pmulticmds, HTREEITEM hitem, HTREEITEM hNewParent,
    const CSecString &sxPrefix);
  bool CopyItem(MultiCommands *pmulticmds, HTREEITEM hitem, HTREEITEM hNewParent,
    const CSecString &Prefix);
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent) const;
  bool ExistsInTree(HTREEITEM &node, const CSecString &s, HTREEITEM &si) const; 
  void CollapseBranch(HTREEITEM hItem);
  CSecString GetPrefix(HTREEITEM hItem) const;
  bool CollectData(BYTE * &out_buffer, long &outLen);
  bool ProcessData(BYTE *in_buffer, const long &inLen, const CSecString &DropGroup);
  void GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem);
  void GetEntryData(CDDObList &out_oblist, CItemData *pci);
  CFont *GetFontBasedOnStatus(HTREEITEM &hItem, CItemData *pci, COLORREF &cf);

  // Notes Display
  UINT_PTR m_nHoverNDTimerID, m_nShowNDTimerID;
  CPoint m_HoverNDPoint;
  bool m_bShowNotes, m_bMouseInWindow;

  // Filter
  bool m_bTreeFilterActive;
  bool m_bEditLabelCompleted;

  bool m_bUseHighLighting;
  std::vector<StringX> m_vModifiedNodes;

  bool m_bUseNewProtectedSymbol, m_bUseNewAttachmentSymbol;
  std::wstring m_sProtectSymbol, m_sAttachmentSymbol;
};

/**
* typedef to hide the fact that CPWTreeCtrl is really a mixin.
*/

typedef CPWTouch< CPWTreeCtrlX > CPWTreeCtrl;
