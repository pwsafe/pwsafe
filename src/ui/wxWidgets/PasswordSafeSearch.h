/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef __PASSWORDSAFESEARCH_H__
#define __PASSWORDSAFESEARCH_H__

/*!
 * Includes
 */

////@begin includes
#include <wx/event.h>
#include <wx/toolbar.h>

#include "../../core/ItemData.h"
#include "AdvancedSelectionDlg.h"
////@end includes

////@begin forward declarations
class PasswordSafeFrame;
////@end forward declarations

#if 0
/*!
 * PasswordSafeSearchContext class declaration.  This class tracks whether a  
 * contained PasswordSafeSearchData object has been modified
 */

class PasswordSafeSearchContext
{
  DECLARE_NO_COPY_CLASS(PasswordSafeSearchContext);

public:
  PasswordSafeSearchContext() : m_searchData(new PasswordSafeSearchData), m_fDirty(false)
  {}

  ~PasswordSafeSearchContext()
  {
    delete m_searchData;
    m_searchData = 0;
  }

  inline const PasswordSafeSearchData* operator->() const { return m_searchData; }
  inline void SetSearchText(const wxString& txt) { m_searchData->m_searchText = txt; m_fDirty = true;}
  inline void SetCaseSensitivity(bool ic) { m_searchData->m_fCaseSensitive = ic; m_fDirty = true; }
  inline bool IsSame(const PasswordSafeSearchData& data) const { return *m_searchData == data; }
  inline void Set(const PasswordSafeSearchData& data) { *m_searchData = data; m_fDirty = true; }
  inline const PasswordSafeSearchData& Get(void) const  { return *m_searchData; }
  inline bool IsDirty(void) const { return m_fDirty; }
  inline void Reset(void) { m_fDirty = false; }

private:
  PasswordSafeSearchData* m_searchData;
  bool                    m_fDirty;
};
#endif

/*!
 * Encapsulates a search index
 */
class SearchPointer
{
    typedef std::vector<CUUIDGen> SearchIndices;
    SearchIndices m_indices;

    SearchIndices::const_iterator m_currentIndex;
    wxString m_label;

public:
    SearchPointer() {
        m_currentIndex = m_indices.end();
        m_label = wxT("No matches found");
    }

    void Clear() { m_indices.clear() ; m_currentIndex = m_indices.end(); m_label = wxT("No matches found"); }
    bool IsEmpty() const { return m_indices.empty(); }
    const CUUIDGen& operator*() const { return *m_currentIndex; }
    size_t Size() const { return m_indices.size(); }

    void InitIndex(void) { 
        m_currentIndex = m_indices.begin();
    }

    void Add(const CUUIDGen& uuid) { m_indices.push_back(uuid); m_label.Printf(wxT("%d matches found"), m_indices.size());}

    SearchPointer& operator++();
    SearchPointer& operator--();

    const wxString& GetLabel(void) const { return m_label; }

private:
};


/*!
 * PasswordSafeSearch class declaration
 */

class PasswordSafeSearch : public wxEvtHandler
{
  DECLARE_CLASS( PasswordSafeSearch )
  DECLARE_EVENT_TABLE()

  DECLARE_NO_COPY_CLASS(PasswordSafeSearch);

public:
  /// Constructors
  PasswordSafeSearch(PasswordSafeFrame* parent);
  /// Destructor
  ~PasswordSafeSearch();

  // wxEVT_COMMAND_TEXT_ENTER event handler for ENTER key press in search text box
  void OnDoSearch( wxCommandEvent& evt );
  void OnSearchClose(wxCommandEvent& evt);
  void OnAdvancedSearchOptions(wxCommandEvent& evt);
  void OnChar(wxKeyEvent& evt);
  void FindNext(void);
  void FindPrevious(void);
  void UpdateView();
  
  void Activate(void);
  void RefreshButtons(void);
  
  //overridden from wxEvtHandler
  virtual bool ProcessEvent(wxEvent& evt);

private:
  template <class Iter, class Accessor>
  void FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr, Iter begin, Iter end, Accessor afn);
  
  template <class Iter, class Accessor>
  void FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr,
                     const CItemData::FieldBits& bsFields, bool fUseSubgroups, const wxString& subgroupText,
                     CItemData::FieldType subgroupObject, PWSMatch::MatchRule subgroupFunction, 
                     bool subgroupFunctionCaseSensitive, Iter begin, Iter end, Accessor afn);

  void CreateSearchBar(void);
  void HideSearchToolbar();
  
  template <class Iter, class Accessor>
  void OnDoSearchT( Iter begin, Iter end, Accessor afn); 

  wxToolBar*           m_toolbar;
  PasswordSafeFrame*   m_parentFrame;
  bool                 m_fAdvancedSearch;
  SelectionCriteria    m_criteria;
  SearchPointer        m_searchPointer;
};

#endif
