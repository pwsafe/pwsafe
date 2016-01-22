
#include "StdAfx.h"

#include "NListCtrl.h"

BEGIN_MESSAGE_MAP(CNListCtrl, CListCtrl)
  ON_WM_CHAR()        // OnChar
END_MESSAGE_MAP()

void CNListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (m_isortColumn == 0) {
    CListCtrl::OnChar(nChar, nRepCnt, nFlags);
    return;
  }

  TCHAR ch = (TCHAR)nChar;

  // No input within 2 seconds, resets the search
  if (m_LastSearchTime.GetCurrentTime() >= (m_LastSearchTime + 2)
      && m_LastSearchString.GetLength() > 0)
    m_LastSearchString = L"";

  // Changing cells, resets the search
  if (m_LastSearchColumn != m_isortColumn)
    m_LastSearchString = L"";

  // Changing rows, resets the search
  if (m_LastSearchRow != GetNextItem(-1, LVNI_FOCUSED))
    m_LastSearchString = L"";

  m_LastSearchColumn = m_isortColumn;
  m_LastSearchTime = m_LastSearchTime.GetCurrentTime();

  if (m_LastSearchString.GetLength() == 1 && m_LastSearchString.GetAt(0) == ch) {
    // When the same first character is entered again,
    // then just repeat the search
  } else {
    m_LastSearchString.AppendChar(ch);
  }

  int nRow = GetNextItem(-1, LVNI_FOCUSED);
  if (nRow < 0)
    nRow = 0;

  int nCol = m_isortColumn;

  int nRowCount = GetItemCount();

  // Perform the search loop twice
  //    - First search from current position down to bottom
  //    - Then search from top to current position
  for (int j = 0; j < 2; ++j) {
    for (int i = nRow + 1; i < nRowCount; ++i) {
      CString cellText = GetItemText(i, nCol);
      if (cellText.GetLength() >= m_LastSearchString.GetLength()) {
        cellText.Truncate(m_LastSearchString.GetLength());
        if (cellText.CompareNoCase(m_LastSearchString) == 0) {
          // De-select all other rows
          SetItemState(-1, 0, LVIS_SELECTED);
          // Select row found
          SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
          // Focus row found
          SetItemState(i, LVIS_FOCUSED, LVIS_FOCUSED);
          // Scroll to row found
          EnsureVisible(i, FALSE);
          m_LastSearchRow = i;
          return;
        }
      }
    }
    nRowCount = nRow;
    nRow = -1;
  }
}

