/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "SecString.h"

#include "core/ItemData.h"

class CCWTreeCtrl : public CTreeCtrl
{
public:
  CCWTreeCtrl();
  ~CCWTreeCtrl();

  // Indices of bitmaps in ImageList
  // NOTE for normal and base entries items, order MUST be:
  //    Not-Expired, Warn-Expired, Expired
  // MUST BE SAME AS IN PWTreeCtrl.h !!!!
  enum {GROUP = 0,
    NORMAL, WARNEXPIRED_NORMAL, EXPIRED_NORMAL,
    ALIASBASE, WARNEXPIRED_ALIASBASE, EXPIRED_ALIASBASE, ALIAS,
    SHORTCUTBASE, WARNEXPIRED_SHORTCUTBASE, EXPIRED_SHORTCUTBASE, SHORTCUT,
    NUM_IMAGES};

  HTREEITEM AddGroup(const CString &path);
  bool IsLeaf(HTREEITEM hItem) const;
  int CountChildren(HTREEITEM hStartItem) const;
  CSecString MakeTreeDisplayString(const CItemData &ci,
             CString &csProtect, CString &csAttachment) const;
  void OnCollapseAll();
  void OnExpandAll();
  HTREEITEM GetNextTreeItem(HTREEITEM hItem);
  int GetEntryImage(const CItemData &ci) const;

protected:
  //{{AFX_MSG(CCWTreeCtrl)
  afx_msg void OnBeginLabelEdit(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent) const;
  bool ExistsInTree(HTREEITEM &node, const CSecString &s, HTREEITEM &si) const;
  void CollapseBranch(HTREEITEM hItem);
  
  CString m_csProtect, m_csAttachment;
};
