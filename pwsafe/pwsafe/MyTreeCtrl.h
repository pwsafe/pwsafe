#pragma once

/*
 * Silly subclass of CTreeCtrl just to implement Drag&Drop.
 *
 * Based on MFC sample code from CMNCTRL1
 */

#include <Afxcmn.h>

class CMyTreeCtrl : public CTreeCtrl
{
public:
  CMyTreeCtrl();
  ~CMyTreeCtrl();

  enum {NODE=0, LEAF=1, EXPIRED_LEAF = 2}; // indices of bitmaps in ImageList

  void DeleteWithParents(HTREEITEM hItem); // if a parent node becomes a leaf
  CString GetGroup(HTREEITEM hItem); // get group path to hItem
  HTREEITEM AddGroup(const CString &path);
  bool IsLeafNode(HTREEITEM hItem);
  void RestoreExpanded();
  void ClearExpanded(); // use when items will be invalid
  void OnCollapseAll();
  void OnExpandAll();
  void SetDboxPointer(void *parent) {m_parent = parent;}
  HTREEITEM GetNextTreeItem(HTREEITEM hItem);

 protected:
  //{{AFX_MSG(CMyTreeCtrl)
  afx_msg void OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnExpandCollapse(NMHDR *pNotifyStruct, LRESULT *result);
  afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT nIDEvent);
  //
  //afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG

  void EndDragging(BOOL bCancel);
  BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_MESSAGE_MAP()

private:
  bool        m_bDragging;
  HTREEITEM   m_hitemDrag;
  HTREEITEM   m_hitemDrop;
  CImageList  *m_pimagelist;
  void *m_parent;
  void *m_expandedItems; // Internally this is a SetTreeItem_t, don't want to include stl file here...

  bool m_isRestoring; // don't repopulate m_expandedItems in restore
  
  void SetNewStyle(long lStyleMask, BOOL bSetBits);
  bool TransferItem(HTREEITEM hitem, HTREEITEM hNewParent);
  void OnButtonUp(void);
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
  void UpdateLeafsGroup(HTREEITEM hItem, CString prefix);
  void CollapseBranch(HTREEITEM hItem);
  
protected:
	UINT    m_nTimerID;
	UINT    m_timerticks;
	UINT	m_nHoverTimerID;
	POINT	m_HoverPoint;

};

