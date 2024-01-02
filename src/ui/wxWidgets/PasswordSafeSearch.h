/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordSafeSearch.h
*
*/

#ifndef _PASSWORDSAFESEARCH_H_
#define _PASSWORDSAFESEARCH_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/aui/auibar.h>

#include "core/ItemData.h"
////@end includes

#define SEARCHBAR_STYLE wxAUI_TB_DEFAULT_STYLE|wxAUI_TB_GRIPPER|wxAUI_TB_PLAIN_BACKGROUND

////@begin forward declarations
class PasswordSafeFrame;
struct SelectionCriteria;
////@end forward declarations


/*!
 * Encapsulates a search index
 */
class SearchPointer
{
  typedef UUIDVector SearchIndices;
  SearchIndices m_indices;

  SearchIndices::const_iterator m_currentIndex;
  wxString m_label;

public:
  SearchPointer() {
    m_currentIndex = m_indices.end();
    PrintLabel();
  }

  void Clear() { m_indices.clear() ; m_currentIndex = m_indices.end(); PrintLabel(); }
  bool IsEmpty() const { return m_indices.empty(); }
  const pws_os::CUUID& operator*() const {
    wxCHECK_MSG(!IsEmpty(), pws_os::CUUID::NullUUID(), wxT("Empty search pointer dereferenced"));
    return *m_currentIndex;
  }
  operator UUIDVector() const {return m_indices;}
  size_t Size() const { return m_indices.size(); }

  void InitIndex(void) { m_currentIndex = m_indices.begin(); }

  void Add(const pws_os::CUUID& uuid) {
    // every time we add to the array, we risk getting the iterators invalidated
    const bool restart = (m_indices.empty() || m_currentIndex == m_indices.begin());
    m_indices.push_back(uuid);
    if (restart) { InitIndex(); }
    PrintLabel();
  }
  
  const pws_os::CUUID& GetUUID(size_t idx) {
    if(idx < m_indices.size()) {
      return m_indices[idx];
    }
    return pws_os::CUUID::NullUUID();
  }

  SearchPointer& operator++();
  SearchPointer& operator--();

  const wxString& GetLabel(void) const { return m_label; }

private:
  void PrintLabel(const TCHAR* prefix = nullptr);
};

/*!
 * PasswordSafeSearch class declaration
 */

class PasswordSafeSearch : public wxAuiToolBar
{
public:
  PasswordSafeSearch(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &position=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=SEARCHBAR_STYLE);
  ~PasswordSafeSearch();

  void OnSearchClose(wxCommandEvent& event);
  void FindNext();
  void FindPrevious();

  void Activate();
  void UpdateBitmaps();
  bool HasTools() const { return GetToolCount() > 0; }
  void Invalidate() { m_searchPointer.Clear(); }
  void ReCreateSearchBar();
  void SetFocusIntoEditField();

private:
  template <class Iter, class Accessor>
  void FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr, Iter begin, Iter end, Accessor afn);

  // wxEVT_COMMAND_TEXT_ENTER event handler for ENTER key press in search text box
  void OnDoSearch( wxCommandEvent& event );
  void OnAdvancedSearchOptions(wxCommandEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnSearchClear(wxCommandEvent& event);
  void OnSize(wxSizeEvent& event);
  void UpdateView();
  void OnSearchTextChanged(wxCommandEvent& event);
  void OnSearchBarTextChar(wxKeyEvent& event);
  void OnToolBarFindReport(wxCommandEvent& event);

  bool CreateSearchBar();
  void HideSearchToolbar();
  void ClearToolbarStatusArea();
  void CalculateToolsWidth();
  wxSize CalculateSearchWidth();
  void UpdateStatusAreaWidth();
  void DoToolBarFindReport(int controlId);
  
  void GetAdvancedSearchOptions(int cointrolId);

  void SetModified(bool modified) { m_modified = modified; }
  bool IsModified() const { return m_modified; }
  
  template <class Iter, class Accessor>
  void OnDoSearchT( Iter begin, Iter end, Accessor afn);

  PasswordSafeFrame*   m_parentFrame;
  SelectionCriteria*   m_criteria;
  SearchPointer        m_searchPointer;
  size_t               m_ToolsWidth;
  bool                 m_modified;
};

#endif // _PASSWORDSAFESEARCH_H_
