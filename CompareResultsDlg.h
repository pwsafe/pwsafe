/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

/// CompareResultsDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "corelib/ItemData.h"
#include "corelib/MyString.h"
#include "corelib/PWScore.h"

// The following structure is needed for compare when record is in
// both databases (column = -1) but there are differences
// Subset used when record is in only one (column = 0 or 1)
// If entries made equal by copying, column set to -2.
struct st_CompareData {
  ItemListIter pos0;
  ItemListIter pos1;
  CItemData::FieldBits bsDiffs;
  CMyString group;
  CMyString title;
  CMyString user;
  int column;
  int listindex;
  bool unknflds0;
  bool unknflds1;
  int id;
};

// Vector of entries passed from DboxMain::Compare to CompareResultsDlg
// Used for "Only in Original DB", "only in Comparison DB" and
// in "Both with Differences"
typedef std::vector<st_CompareData> CompareData;

// The following structure is needed for compare to send back data
// to allow copying, viewing and editing of entries
struct st_CompareInfo {
  PWScore *pcore0;
  PWScore *pcore1;
  ItemListIter pos0;
  ItemListIter pos1;
  int column;
};

class CCompareResultsDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CCompareResultsDlg)

  // Construction
public:
  CCompareResultsDlg(CWnd* pParent,
                     CompareData &OnlyInCurrent,
                     CompareData &OnlyInComp,
                     CompareData &Conflicts,
                     CompareData &Identical,
                     CItemData::FieldBits &bsFields,
                     PWScore *pcore0, PWScore *pcore1);

  // st_CompareInfo Functions
  enum {EDIT = 0, VIEW, COPY_TO_ORIGINALDB, COPY_TO_COMPARISONDB};

  // Column indices
  enum {CURRENT = 0, COMPARE, GROUP, TITLE, USER, PASSWORD, NOTES, URL,
        AUTOTYPE, PWHIST, CTIME, ATIME, LTIME, PMTIME, RMTIME,
        LAST};

  // Dialog Data
  //{{AFX_DATA(CCompareResultsDlg)
  enum { IDD = IDD_COMPARE_RESULTS };
  CListCtrl m_LCResults;
  int m_iSortedColumn;
  bool m_bSortAscending;
  CMyString m_cs_Filename1, m_cs_Filename2;
  int m_ShowIdenticalEntries;
  //}}AFX_DATA

  bool m_bOriginalDBReadOnly, m_bComparisonDBReadOnly;
  bool m_OriginalDBChanged, m_ComparisonDBChanged;
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CCompareResultsDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

private:
  static int CALLBACK CRCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Implementation
protected:

  UINT statustext[1];
  CStatusBar m_statusBar;
  bool CopyLeftOrRight(const bool bCopyLeft);
  void UpdateStatusBar();
  bool ProcessFunction(const int ifunction);

  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CCompareResultsDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnCopyToClipboard();
  afx_msg void OnShowIdenticalEntries();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemDoubleClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnItemRightClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnCompareViewEdit();
  afx_msg void OnCompareCopyToOriginalDB();
  afx_msg void OnCompareCopyToComparisonDB();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
	CompareData m_OnlyInCurrent;
	CompareData m_OnlyInComp;
	CompareData m_Conflicts;
  CompareData m_Identical;
  CItemData::FieldBits m_bsFields;

	PWScore *m_pcore0, *m_pcore1;

  size_t m_numOnlyInCurrent, m_numOnlyInComp, m_numConflicts, m_numIdentical;
  int m_cxBSpace, m_cyBSpace, m_cySBar;
  int m_DialogMinWidth, m_DialogMinHeight;
  int m_DialogMaxWidth, m_DialogMaxHeight;
  int m_row, m_column;
  int m_nCols;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
