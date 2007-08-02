/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

/** \file
 * Silly subclass of CTreeCtrl just to implement Drag&Drop.
 */

#include <Afxcmn.h>
#include "corelib/MyString.h"

// Need a set to keep track of what nodes are expanded, to re-expand
// after minimize
#include <set>
class CItemData;
typedef std::set<CItemData *> SetTreeItem_t;

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
  enum {NODE=0, LEAF=1, EXPIRED_LEAF = 2, WARNEXPIRED_LEAF = 3};

  void Initialize();
  void DeleteWithParents(HTREEITEM hItem); // if a parent node becomes a leaf
  void DeleteFromSet(HTREEITEM hItem);
  CString GetGroup(HTREEITEM hItem); // get group path to hItem
  HTREEITEM AddGroup(const CString &path);
  bool IsLeafNode(HTREEITEM hItem);
  void RestoreExpanded();
  void ClearExpanded(); // use when items will be invalid
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

 protected:
  //{{AFX_MSG(CPWTreeCtrl)
  afx_msg void OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *result);
  afx_msg void OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *result);
  afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnDestroy();
  //
  //afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG

  BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_MESSAGE_MAP()

private:
  HTREEITEM   m_hitemDrag;
  HTREEITEM   m_hitemDrop;
  SetTreeItem_t m_expandedItems;

  bool m_isRestoring; // don't repopulate m_expandedItems in restore
  int m_nDragPathLen;

  // in an ideal world, following would be is-a, rather than has-a (multiple inheritance)
  // Microsoft doesn't really support this, however...
  CPWTDropTarget *m_DropTarget;
  CPWTDropSource *m_DropSource;
  CPWTDataSource *m_DataSource;
  // Clipboard format for our Drag & Drop
  CLIPFORMAT m_tcddCPFID;

  void SetNewStyle(long lStyleMask, BOOL bSetBits);
  bool MoveItem(HTREEITEM hitem, HTREEITEM hNewParent);
  bool CopyItem(HTREEITEM hitem, HTREEITEM hNewParent, const CMyString &prefix);
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
  void UpdateLeafsGroup(HTREEITEM hItem, CString prefix);
  void CollapseBranch(HTREEITEM hItem);
  CMyString GetPrefix(HTREEITEM hItem) const;
  bool CollectData(BYTE * &out_buffer, long &outLen);
  bool ProcessData(BYTE *in_buffer, const long &inLen, const CMyString &DropGroup);
  void GetGroupEntriesData(CDDObList &out_oblist, HTREEITEM hItem);
  void GetEntryData(CDDObList &out_oblist, CItemData *ci);
};
