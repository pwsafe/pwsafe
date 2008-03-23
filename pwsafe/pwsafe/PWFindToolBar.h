/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWFindToolBar

#include "ControlExtns.h"
#include <vector>

class CPWFindToolBar : public CToolBar
{
  DECLARE_DYNAMIC(CPWFindToolBar)

public:
  CPWFindToolBar();
  virtual ~CPWFindToolBar();

  void Init(const int NumBits, CWnd *pMessageWindow, int iWMSGID);
  void LoadDefaultToolBar(const int toolbarMode);
  void AddExtraControls();
  void ChangeImages(const int toolbarMode);
  void Reset();
  void ShowFindToolBar(bool bShow);
  bool IsVisible() {return m_bVisible;}
  void GetSearchText(CString &csFindString)
  {m_findedit.GetWindowText(csFindString);}
  void Find();
  void ClearFind();
  void ShowFindAdvanced();
  void ToggleToolBarFindCase();
  BOOL IsFindCaseSet()
  {return m_bCaseSensitive ? TRUE : FALSE;}
  void RefreshImages();
  void InvalidateSearch() {m_lastshown = size_t(-1);}
  void GetSearchInfo(bool &bAdvanced, CItemData::FieldBits &bsFields, 
                     CString &subgroup_name, 
                     int &subgroup_set, int &subgroup_object, int &subgroup_function)
  {bAdvanced = m_bAdvanced; bsFields = m_bsFields;
   subgroup_name = m_subgroup_name; subgroup_set = m_subgroup_set;
   subgroup_object = m_subgroup_object; subgroup_function = m_subgroup_function;}

  std::vector<int> * GetSearchResults() {return &m_indices;}
  void SetStatus(CString cs_status) {m_findresults.SetWindowText(cs_status);}

  CEditExtn m_findedit;
  CStaticExtn m_findresults;

protected:
  //{{AFX_MSG(CPWFindToolBar)
  //}}AFX_MSG

  BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_MESSAGE_MAP()

private:
  static const UINT m_FindToolBarIDs[];
  static const UINT m_FindToolBarClassicBMs[];
  static const UINT m_FindToolBarNewBMs[];

  CImageList m_ImageLists[3];  // 1st = Classic; 2nd = New 8; 3rd = New 32;
  TBBUTTON *m_pOriginalTBinfo;
  CWnd *m_pDbx;
  CFont m_FindTextFont;
  int m_iMaxNumButtons, m_iNum_Bitmaps, m_NumBits;
  int m_iWMSGID;
  int m_toolbarMode, m_bitmode;
  bool m_bVisible, m_bCaseSensitive, m_bAdvanced;

  std::vector<int> m_indices; // array of found items

  bool m_cs_search, m_last_cs_search;
  CMyString m_search_text, m_last_search_text;
  CItemData::FieldBits m_bsFields, m_last_bsFields;
  CString m_subgroup_name, m_last_subgroup_name;
  int m_subgroup_set, m_last_subgroup_set;
  int m_subgroup_object, m_last_subgroup_object;
  int m_subgroup_function, m_last_subgroup_function;

  size_t m_lastshown; // last index selected, -1 indicates no search done yet
  size_t m_numFound; // number of matched items, as returned by DboxMain::FindAll

  int m_iCase_Insensitive_BM_offset, m_iCase_Sensitive_BM_offset;
};
