/*
 * Silly subclass of CTreeCtrl just to implement Drag&Drop.
 *
 * Based on MFC sample code from CMNCTRL1
 */

#ifndef _MYTREECTRL_H
#define _MYTREECTRL_H

class CMyTreeCtrl : public CTreeCtrl
{
public:
  CMyTreeCtrl();
  ~CMyTreeCtrl();

  enum {NODE=0, LEAF=1}; // indices of bitmaps in ImageList

private:
  bool        m_bDragging;
  HTREEITEM   m_hitemDrag;
  HTREEITEM   m_hitemDrop;
  CImageList  *m_pimagelist;

  void SetNewStyle(long lStyleMask, BOOL bSetBits);
  bool TransferItem(HTREEITEM hitem, HTREEITEM hNewParent);
  void OnButtonUp(void);
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
  bool IsLeafNode(HTREEITEM hItem);

 protected:
  //{{AFX_MSG(CMyTreeCtrl)
  afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnDestroy();
  //}}AFX_MSG

  void OnButtonUp(CPoint point);

  DECLARE_MESSAGE_MAP()
};


#endif /* _MYTREECTRL_H */
