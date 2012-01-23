/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "CWTreeCtrl.h"

#include "core/ItemData.h"
#include "core/PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCWTreeCtrl::CCWTreeCtrl()
{
}

CCWTreeCtrl::~CCWTreeCtrl()
{
}

BEGIN_MESSAGE_MAP(CCWTreeCtrl, CTreeCtrl)
  //{{AFX_MSG_MAP(CCWTreeCtrl)
  ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCWTreeCtrl::OnBeginLabelEdit(NMHDR *, LRESULT *pLResult)
{
  *pLResult = TRUE; // TRUE cancels label editing
}

bool CCWTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent) const
{
  do {
    if (hitemChild == hitemSuspectedParent)
      break;
  } while ((hitemChild = GetParentItem(hitemChild)) != NULL);

  return (hitemChild != NULL);
}

bool CCWTreeCtrl::IsLeaf(HTREEITEM hItem) const
{
  // ItemHasChildren() won't work in the general case
  int i, dummy;
  BOOL status = GetItemImage(hItem, i, dummy);
  ASSERT(status);
  return (i != NODE);
}

// Returns the number of children of this group
int CCWTreeCtrl::CountChildren(HTREEITEM hStartItem) const
{
  // Walk the Tree!
  int num = 0;
  if (hStartItem != NULL && ItemHasChildren(hStartItem)) {
    HTREEITEM hChildItem = GetChildItem(hStartItem);
    while (hChildItem != NULL) {
      if (ItemHasChildren(hChildItem)) {
        num += CountChildren(hChildItem);
      } else {
        num++;
      }
      hChildItem = GetNextSiblingItem(hChildItem);
    }
  }
  return num;
}

static CSecString GetPathElem(CSecString &path)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // (assuming GROUP_SEP is '.')

  CSecString retval;
  int N = path.Find(L'.');
  if (N == -1) {
    retval = path;
    path = L"";
  } else {
    const int Len = path.GetLength();
    retval = CSecString(path.Left(N));
    path = CSecString(path.Right(Len - N - 1));
  }
  return retval;
}

bool CCWTreeCtrl::ExistsInTree(HTREEITEM &node, const CSecString &s, HTREEITEM &si) const
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

HTREEITEM CCWTreeCtrl::AddGroup(const CString &group)
{
  // Add a group at the end of path
  HTREEITEM ti = TVI_ROOT;
  HTREEITEM si;
  if (!group.IsEmpty()) {
    CSecString path = group;
    CSecString s;
    StringX path2root(L""), sxDot(L".");
    do {
      s = GetPathElem(path);
      if (path2root.empty())
        path2root = (LPCWSTR)s;
      else
        path2root += sxDot + StringX(s);

      if (!ExistsInTree(ti, s, si)) {
        ti = InsertItem(s, ti, TVI_SORT);
        SetItemImage(ti, CCWTreeCtrl::NODE, CCWTreeCtrl::NODE);
      } else
        ti = si;
    } while (!path.IsEmpty());
  }
  return ti;
}

void CCWTreeCtrl::OnExpandAll()
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

void CCWTreeCtrl::OnCollapseAll()
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

void CCWTreeCtrl::CollapseBranch(HTREEITEM hItem)
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

HTREEITEM CCWTreeCtrl::GetNextTreeItem(HTREEITEM hItem)
{
  if (NULL == hItem)
    return GetRootItem();

  // First, try to go to this item's 1st child
  HTREEITEM hReturn = GetChildItem(hItem);

  // If no more child items...
  while (hItem && !hReturn) {
    // Get this item's next sibling
    hReturn = GetNextSiblingItem(hItem);

    // If hReturn is NULL, then there are no sibling items, and we're on a leaf node.
    // Backtrack up the tree one level, and we'll look for a sibling on the next
    // iteration (or we'll reach the root and quit).
    hItem = GetParentItem(hItem);
  }
  return hReturn;
}

CSecString CCWTreeCtrl::MakeTreeDisplayString(const CItemData &ci) const
{
  CSecString treeDispString = ci.GetTitle();

  treeDispString += L" [";
  treeDispString += ci.GetUser();
  treeDispString += L"]";

  if (ci.IsProtected())
    treeDispString += L" #";

  return treeDispString;
}

int CCWTreeCtrl::GetEntryImage(const CItemData &ci) const
{
  int entrytype = ci.GetEntryType();
  if (entrytype == CItemData::ET_ALIAS) {
    return ALIAS;
  }
  if (entrytype == CItemData::ET_SHORTCUT) {
    return SHORTCUT;
  }

  int nImage;
  switch (entrytype) {
    case CItemData::ET_NORMAL:
      nImage = NORMAL;
      break;
    case CItemData::ET_ALIASBASE:
      nImage = ALIASBASE;
      break;
    case CItemData::ET_SHORTCUTBASE:
      nImage = SHORTCUTBASE;
      break;
    default:
      nImage = NORMAL;
  }

  time_t tttXTime;
  ci.GetXTime(tttXTime);
  if (tttXTime > time_t(0) && tttXTime <= time_t(3650)) {
    time_t tttCPMTime;
    ci.GetPMTime(tttCPMTime);
    if ((long)tttCPMTime == 0L)
      ci.GetCTime(tttCPMTime);
    tttXTime = (time_t)((long)tttCPMTime + (long)tttXTime * 86400);
  }

  if (tttXTime != 0) {
    time_t now, warnexptime((time_t)0);
    time(&now);
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
      int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
      struct tm st;
#if (_MSC_VER >= 1400)
      errno_t err;
      err = localtime_s(&st, &now);  // secure version
      ASSERT(err == 0);
#else
      st = *localtime(&now);
      ASSERT(st != NULL); // null means invalid time
#endif
      st.tm_mday += idays;
      warnexptime = mktime(&st);

      if (warnexptime == (time_t)-1)
        warnexptime = (time_t)0;
    }
    if (tttXTime <= now) {
      nImage += 2;  // Expired
    } else if (tttXTime < warnexptime) {
      nImage += 1;  // Warn nearly expired
    }
  }
  return nImage;
}
