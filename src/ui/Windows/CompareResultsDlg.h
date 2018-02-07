/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// CompareResultsDlg.h
//-----------------------------------------------------------------------------

#include "PWTouch.h"
#include "PWResizeDialog.h"
#include "core/ItemData.h"
#include "SecString.h"
#include "core/PWScore.h"
#include "core/Report.h"
#include "core/DBCompareData.h"

class CCompareResultsDlg;

// Trivial class for results CListCtrl for detecting left mouse click and 
// performing custom draw

class CCPListCtrlX : public CListCtrl
{
public:
  CCPListCtrlX();
  ~CCPListCtrlX();

  virtual BOOL PreTranslateMessage(MSG *pMsg);

  inline int GetRow() {return m_row;}
  inline int GetColumn() {return m_column;}
  void SetRow(const int iRow) {m_row = iRow;}
  void SetColumn(const int iColumn) {m_column = iColumn;}
  bool IsSelected(DWORD_PTR iRow);
  void UpdateRowHeight(bool bInvalidate);

protected:
  //{{AFX_MSG(CCPListCtrl)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  int m_row, m_column;
};

/**
* typedef to hide the fact that CCPListCtrl is really a mixin.
*/

typedef CPWTouch< CCPListCtrlX > CCPListCtrl;

// The following structure is needed for compare to send back data
// to allow copying, viewing, editing and synching of entries
struct st_CompareInfo {
  PWScore *pcore0;     // original DB
  PWScore *pcore1;     // comparison DB
  pws_os::CUUID uuid0; // original DB
  pws_os::CUUID uuid1; // comparison DB
  int  clicked_column;
};

class CCompareResultsDlg : public CPWResizeDialog
{
  DECLARE_DYNAMIC(CCompareResultsDlg)

  // Construction
public:
  CCompareResultsDlg(CWnd *pParent, CompareData &OnlyInCurrent, CompareData &OnlyInComp,
                     CompareData &Conflicts, CompareData &Identical,
                     CItemData::FieldBits &bsFields,
                     PWScore *pcore0, PWScore *pcore1,
                     CString &csProtect, CString &csAttachment,
                     CReport *pRpt);

  virtual ~CCompareResultsDlg();

  // st_CompareInfo Functions
  enum {EDIT = 0, VIEW, COPY_TO_ORIGINALDB, SYNCH, COPYALL_TO_ORIGINALDB, SYNCHALL};

  // Column indices
  // IDENTICAL means CURRENT + COMPARE but identical
  // BOTH      means CURRENT + COMPARE but with differences

  // NOTE: BOTH = -1 , CURRENT = 0, COMPARE = 1
  // MUST be the same as in "core/DBCompareData.h"

  enum {
    // Used to determine if entries are in both DBs and whether different
    IDENTICAL = -2, BOTH = -1 ,
    // Fixed columns - MUST be same order as FixedCols member variable data
    CURRENT = 0, COMPARE = 1, 
    GROUP, TITLE, USER,
    // Optional columns - do NOT have to be in same order as OptCols member variable data
    PASSWORD, NOTES, URL, AUTOTYPE, PWHIST, 
    POLICY, POLICYNAME, SYMBOLS, RUNCMD, EMAIL,
    DCA, SHIFTDCA, PROTECTED, KBSHORTCUT, ATTREF,
    CTIME, ATIME, XTIME, XTIME_INT, PMTIME, RMTIME,
    LAST};

  // Dialog Data
  //{{AFX_DATA(CCompareResultsDlg)
  enum { IDD = IDD_COMPARE_RESULTS };

  CCPListCtrl m_LCResults;
  int m_iSortedColumn;
  bool m_bSortAscending;
  CSecString m_scFilename1, m_scFilename2;
  int m_ShowIdenticalEntries;
  //}}AFX_DATA

  bool m_bOriginalDBReadOnly, m_bComparisonDBReadOnly;
  bool m_OriginalDBChanged;
  bool m_bTreatWhiteSpaceasEmpty;
  CString GetResults() {return m_results;}

protected:
  static int CALLBACK CRCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CCompareResultsDlg)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  //}}AFX_VIRTUAL

  // Implementation

  // Generated message map functions
  //{{AFX_MSG(CCompareResultsDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnItemDoubleClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemRightClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnShowIdenticalEntries();
  afx_msg void OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnCompareViewEdit();
  afx_msg void OnCompareSynchronize();
  afx_msg void OnCompareCopyToOriginalDB();
  afx_msg void OnCompareCopyAllToOriginalDB();
  afx_msg void OnCompareSynchronizeAll();
  afx_msg void OnCompareBothEntries();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void UpdateStatusBar();
  bool ProcessFunction(const int ifunction, st_CompareData *st_data);
  LRESULT ProcessAllFunction(const int ifunction, std::vector<st_CompareData *> vpst_data);
  void WriteReportData();
  st_CompareData * GetCompareData(const LONG_PTR dwItemData);
  static st_CompareData * GetCompareData(const LONG_PTR dwItemData, CCompareResultsDlg *self);
  void AddCompareEntries(const bool bAddIdentical);
  void DoAllFunctions(const int ifunction);
  bool CompareEntries(st_CompareData *pst_data);
  
  CompareData m_OnlyInCurrent;
  CompareData m_OnlyInComp;
  CompareData m_Conflicts;
  CompareData m_Identical;
  CItemData::FieldBits m_bsFields;

  PWScore *m_pcore0, *m_pcore1;
  CReport *m_pRpt;
  CCoolMenuManager m_menuManager;

  CString m_results;
  size_t m_numOnlyInCurrent, m_numOnlyInComp, m_numConflicts, m_numIdentical;
  int m_nCols;
  bool m_bFirstInCompare;
  bool m_bDBNotificationState;
  
  CString m_csProtect, m_csAttachment;

  // These columns always shown
  static const UINT FixedCols[USER + 1];

  // These columns are optional
  static struct OptionalColumns {
    CItemData::FieldType ft; UINT ids;
  }   OptCols[LAST - PASSWORD];
};
