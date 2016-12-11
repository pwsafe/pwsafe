/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWFindToolBar

#include "ControlExtns.h"
#include "AdvancedDlg.h"
#include <vector>

class CFindEditCtrl : public CEditExtn
{
  // Thanks to John Z. Czopowik VC++ MVP for code for vertically centred text in a
  // CEdit control
public:
  CFindEditCtrl();
  virtual ~CFindEditCtrl();

protected:
  // Needed to trap Entry Keyboard Shortcuts if we are in control
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  DECLARE_MESSAGE_MAP()

};

class CPWFindToolBar : public CToolBar
{
  DECLARE_DYNAMIC(CPWFindToolBar)

public:
  CPWFindToolBar();
  virtual ~CPWFindToolBar();

  void Init(const int NumBits, int iWMSGID, st_SaveAdvValues *pst_SADV);
  void LoadDefaultToolBar(const int toolbarMode);
  void AddExtraControls();
  void ChangeImages(const int toolbarMode);

  void ShowFindToolBar(bool bShow);
  bool IsVisible() {return m_bVisible;} // virt function, can't constify
  bool EntriesFound() const {return !m_indices.empty();}
  void GetSearchText(CString &csFindString)
  {m_findedit.GetWindowText(csFindString);}
  void Find();
  void ClearFind();
  void ShowFindAdvanced();
  void ToggleToolBarFindCase();
  BOOL IsFindCaseSet() const {return m_bCaseSensitive ? TRUE : FALSE;}
  void RefreshImages();
  void InvalidateSearch() {m_lastshown = size_t(-1);}
  void GetSearchInfo(bool &bAdvanced, 
                     CItemData::FieldBits &bsFields, CItemAtt::AttFieldBits &bsAttFields,
                     std::wstring &subgroup_name, 
                     bool &subgroup_bset, int &subgroup_object, int &subgroup_function)
  {
    bAdvanced = m_bAdvanced;
    bsFields = m_pst_SADV->bsFields; bsAttFields = m_pst_SADV->bsAttFields;
    subgroup_name = m_subgroup_name; subgroup_bset = m_subgroup_bset;
    subgroup_object = m_subgroup_object; subgroup_function = m_subgroup_function;}

  std::vector<int> *GetSearchResults() {return &m_indices;}
  void SetStatus(CString cs_status) {m_findresults.SetWindowText(cs_status);}

  CFindEditCtrl m_findedit;
  CStaticExtn m_findresults;
  void SetSearchDirection(int iFindDirection) {m_iFindDirection = iFindDirection;}
  void ChangeFont();

protected:
  BOOL PreTranslateMessage(MSG* pMsg);

  //{{AFX_MSG(CPWFindToolBar)
  afx_msg void OnDestroy();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static const UINT m_FindToolBarIDs[];
  static const UINT m_FindToolBarClassicBMs[];
  static const UINT m_FindToolBarNewBMs[];

  CImageList m_ImageLists[3];  // 1st = Classic; 2nd = New 8; 3rd = New 32;
  TBBUTTON *m_pOriginalTBinfo;
  CFont m_FindTextFont;
  int m_iMaxNumButtons, m_iNum_Bitmaps, m_NumBits;
  int m_iWMSGID;
  int m_toolbarMode, m_bitmode, m_iFindDirection;
  bool m_bVisible, m_bCaseSensitive;
  bool m_bAdvanced, m_bFontSet;

  std::vector<int> m_indices; // array of found items

  bool m_cs_search, m_last_cs_search;
  CSecString m_search_text, m_last_search_text;

  CItemData::FieldBits m_last_bsFields;
  CItemAtt::AttFieldBits m_last_bsAttFields;

  std::wstring m_subgroup_name, m_last_subgroup_name;

  bool m_subgroup_bset, m_last_subgroup_bset;
  int m_subgroup_object, m_last_subgroup_object;
  int m_subgroup_function, m_last_subgroup_function;

  size_t m_lastshown; // last index selected, -1 indicates no search done yet
  size_t m_numFound; // number of matched items, as returned by DboxMain::FindAll

  int m_iCase_Insensitive_BM_offset, m_iAdvanced_BM_offset;
  int m_iCase_Sensitive_BM_offset, m_iAdvancedOn_BM_offset;
  st_SaveAdvValues *m_pst_SADV;
};
