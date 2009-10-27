/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "../../corelib/ItemData.h"
////@end includes

////@begin forward declarations
class PasswordSafeFrame;
////@end forward declarations


/*!
 * PasswordSafeSearchData class declaration
 */

class PasswordSafeSearchData 
{
  
  PasswordSafeSearchData(const PasswordSafeSearchData&);

public:
  PasswordSafeSearchData():  m_fCaseSensitive(false),
                             m_fUseSubgroups(false),
                             m_subgroupObject(0),            // index into subgroups array defined in .cpp
                             m_subgroupFunction(0)           // index into subgroupFunctions array defined in .cpp
  {}

  bool                  m_fCaseSensitive;
  wxString              m_searchText;
  CItemData::FieldBits  m_bsFields;
  wxString              m_subgroupText;
  bool                  m_fUseSubgroups;
  int                   m_subgroupObject;
  int                   m_subgroupFunction;
};

inline bool operator==(const PasswordSafeSearchData& a, const PasswordSafeSearchData& b)
{
  return a.m_bsFields         == b.m_bsFields && 
         a.m_fCaseSensitive   == b.m_fCaseSensitive &&
         a.m_fUseSubgroups    == b.m_fUseSubgroups &&
         a.m_searchText       == b.m_searchText &&
         a.m_subgroupFunction == b.m_subgroupFunction &&
         a.m_subgroupText     == b.m_subgroupText &&
         a.m_subgroupObject   == b.m_subgroupObject; 
}



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



/*!
 * AdvancedSearchOptionsDlg class declaration
 */

class AdvancedSearchOptionsDlg: public wxDialog
{
  DECLARE_CLASS(AdvancedSearchOptionsDlg)
  DECLARE_EVENT_TABLE()

  DECLARE_NO_COPY_CLASS(AdvancedSearchOptionsDlg);

  PasswordSafeSearchContext& m_context;

  enum {ID_SELECT_SOME = 101, ID_SELECT_ALL, ID_REMOVE_SOME, ID_REMOVE_ALL, ID_LB_AVAILABLE_FIELDS, ID_LB_SELECTED_FIELDS };

public:
  AdvancedSearchOptionsDlg(wxWindow* wnd, PasswordSafeSearchContext& context);

  void OnOk( wxCommandEvent& evt );
  void OnSelectSome( wxCommandEvent& evt );
  void OnSelectAll( wxCommandEvent& evt );
  void OnRemoveSome( wxCommandEvent& evt );
  void OnRemoveAll( wxCommandEvent& evt );

private:
  void CreateControls(wxWindow* parentWnd);
  PasswordSafeSearchData m_searchData;
};

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

    void Reset() { m_currentIndex = m_indices.end(); m_label = wxT("No matches found"); }
    void Clear() { m_indices.clear() ; m_currentIndex = m_indices.end(); }
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
  void OnToggleCaseSensitivity(wxCommandEvent& evt);
  void FindNext(void);
  void FindPrevious(void);
  void UpdateView();
  
  void Activate(void);
  void FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr);
  
  void FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr,
                     const CItemData::FieldBits& bsFields, bool fUseSubgroups, const wxString& subgroupText,
                     CItemData::FieldType subgroupObject, PWSMatch::MatchRule subgroupFunction);

private:
  void CreateSearchBar(void);

  wxToolBar*           m_toolbar;
  PasswordSafeFrame*   m_parentFrame;
  bool                 m_fAdvancedSearch;

  SearchPointer        m_searchPointer;

  PasswordSafeSearchContext m_searchContext;
};

#endif
