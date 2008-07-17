/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "../corelib/Itemdata.h"
#include "../corelib/filters.h"
#include "FilterStringDlg.h"
#include "FilterPasswordDlg.h"
#include "FilterIntegerDlg.h"
#include "FilterDateDlg.h"
#include "FilterBoolDlg.h"
#include "FilterEntryTypeDlg.h"

class CSetHistoryFiltersDlg;
class CSetPolicyFiltersDlg;

// Subitem indices
#define FLC_FILTER_NUMBER 0
#define FLC_ENABLE_BUTTON 1
#define FLC_ADD_BUTTON    2
#define FLC_REM_BUTTON    3
#define FLC_LGC_COMBOBOX  4
#define FLC_FLD_COMBOBOX  5
#define FLC_CRITERIA_TEXT 6
#define FLC_NUM_COLUMNS   7

// Item Data values (multiple of 2 since they are flags)
#define FLC_FILTER_ENABLED   0x01
#define FLC_CRITERIA_REDTXT  0x02
#define FLC_FLD_CBX_ENABLED  0x04
#define FLC_FLD_CBX_SET      0x08
#define FLC_LGC_CBX_ENABLED  0x10

enum FilterType {DFTYPE_MAIN, DFTYPE_PWHISTORY, DFTYPE_PWPOLICY};

class CPWFiltersDlg;

struct st_Fcbxdata {
  FieldType ftype;
  CString cs_text;
};

struct equal_ftype
{
  equal_ftype(const FieldType& ftype) : m_ftype(ftype) {}
  bool operator()(st_Fcbxdata const& rdata) const
  {
    return (rdata.ftype == m_ftype);
  }

  FieldType m_ftype;
};

struct st_Lcbxdata {
  LogicConnect ltype;
  CString cs_text;
};

struct equal_ltype
{
  equal_ltype(const LogicConnect& ltype) : m_ltype(ltype) {}
  bool operator()(st_Lcbxdata const& rdata) const
  {
    return (rdata.ltype == m_ltype);
  }

  LogicConnect m_ltype;
};

class CPWFilterLC : public CListCtrl
{
public:
  CPWFilterLC();
  ~CPWFilterLC();

  friend CPWFiltersDlg;

  void Init(CWnd * pParent, st_filters *pfilters, const int &filtertype);

protected:
  std::vector<FieldType> vlast_ft;           // Last combo selected item
  std::vector<PWSMatch::MatchType> vlast_mt; // Last selected matchtype
  std::vector<bool> vcbxChanged;             // Has combo selection changed?
  std::vector<bool> vCriteriaSet;            // Has criteria been set?
  std::vector<bool> vAddPresent;             // Do we add 'ISPRESENT' rule option?

  std::vector<st_Fcbxdata> vFcbx_data;     // Field combobox strings & fieldtypes
  std::vector<st_Lcbxdata> vLcbx_data;     // Logic (AND/OR) combobox strings
  std::vector<st_Fcbxdata> vWCFcbx_data;   // Working copy Field combobox & fieldtypes

  WCHAR *m_pwchTip;
  char *m_pchTip;

  BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;

  //{{AFX_MSG(CPWFilterLC)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult);
  virtual afx_msg BOOL OnToolTipText(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

public:
  st_filters *m_pfilters;
  bool IsPWHIST_Set() {return m_bPWHIST_Set;}
  bool IsPOLICY_Set() {return m_bPOLICY_Set;}
  bool IsHistoryGood() {return m_GoodHistory;}
  bool IsPolicyGood() {return m_GoodPolicy;}

private:
  void SetUpComboBoxData();
  void DrawComboBox(const int iSubItem, const int index);
  void DrawSubItemText(int iItem, int iSubItem, CDC *pDC,
                       COLORREF crText, COLORREF crBkgnd,
                       CRect &rect, bool bBold, bool bOpaque);
  void DrawImage(CDC *pDC, CRect &rect, int nImage);
  void CloseKillCombo();
  void DropDownCombo(const UINT nID);

  int AddFilter_Controls();
  void DeleteFilters();
  bool GetCriterion();
  FieldType EnableCriteria();
  bool SetField(const int iItem);
  void SetLogic(const int iItem);
  void CancelField(const int iItem);
  void CancelLogic(const int iItem);

  int AddFilter();
  void RemoveFilter();
  void ResetFilter(const int num);
  void RemoveLast();
  void MoveDown();
  void MoveUp(const int nAfter);
  void ResetAndOr();

  void OnProcessKey(UINT nID);

  // Remove History/Policy from the Comboboxes to prevent the
  // sub-dialogs being shown more than once.
  void DeleteEntry(FieldType ftype);

  CPWFiltersDlg* m_pPWF;
  CHeaderCtrl* m_pHeaderCtrl;
  CImageList *m_pImageList, *m_pCheckImageList;

  // Note - History & Policy are like my parent and they too have a ListCtrl
  // Incestuous?
  CFilterStringDlg m_fstring;
  CFilterPasswordDlg m_fpswd;
  CFilterIntegerDlg m_finteger;
  CFilterDateDlg m_fdate;
  CFilterBoolDlg m_fbool;
  CFilterEntryTypeDlg m_fentry;

  vfilterdata *m_pvfdata;
  int *m_pnumactive;

  bool m_bInitDone, m_bStatusBarOK, m_bSetFieldActive, m_bSetLogicActive;
  int m_numfilters;
  int m_iType;
  UINT m_FLD_ComboID, m_LGC_ComboID;

  // Needed to make the row height bigger
  CImageList m_imageList; 

  bool m_bPWHIST_Set, m_bPOLICY_Set;
  bool m_GoodHistory, m_GoodPolicy;

  COLORREF m_crGrayText, m_crWindow, m_crWindowText, m_crButtonFace, m_crRedText;
  int m_fwidth, m_lwidth, m_rowheight;
  CComboBox *m_pComboBox;
  CFont* m_pFont;
  int m_iItem;
};
