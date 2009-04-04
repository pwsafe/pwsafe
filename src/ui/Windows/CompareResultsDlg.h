/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// CompareResultsDlg.h
//-----------------------------------------------------------------------------

#include "PWResizeDialog.h"
#include "corelib/ItemData.h"
#include "SecString.h"
#include "corelib/PWScore.h"
#include "corelib/Report.h"
#include "corelib/uuidgen.h"

#ifdef _DEBUG
#include <bitset>
#include <string>
#endif

// The following structure is needed for compare when record is in
// both databases (indatabase = -1) but there are differences
// Subset used when record is in only one (indatabase = 0 or 1)
// If entries made equal by copying, indatabase set to -1.
struct st_CompareData {
  uuid_array_t uuid0;  // original DB
  uuid_array_t uuid1;  // comparison DB
  CItemData::FieldBits bsDiffs;  // list of items compared
  CSecString group;
  CSecString title;
  CSecString user;
  int id;  // # in the appropriate list: "Only in Original", "Only in Comparison" or in "Both with Differences"
  int indatabase;  // see enum below
  int listindex;  // list index in CompareResultsDlg list control
  bool unknflds0;  // original DB
  bool unknflds1;  // comparison DB

  st_CompareData()
    : bsDiffs(0), group(_T("")), title(_T("")), user(_T("")),
    id(0), indatabase(0), listindex(0),
    unknflds0(false), unknflds1(false)
  {
    memset(uuid0, 0x00, sizeof(uuid_array_t));
    memset(uuid1, 0x00, sizeof(uuid_array_t));
  }

  st_CompareData(const st_CompareData &that)
    : bsDiffs(that.bsDiffs), group(that.group), title(that.title), user(that.user),
    id(that.id), indatabase(that.indatabase), listindex(that.listindex),
    unknflds0(that.unknflds0), unknflds1(that.unknflds1)
  {
    memcpy(uuid0, that.uuid0, sizeof(uuid_array_t));
    memcpy(uuid1, that.uuid1, sizeof(uuid_array_t));
  }

  st_CompareData &operator=(const st_CompareData &that)
  {
    if (this != &that) {
      memcpy(uuid0, that.uuid0, sizeof(uuid_array_t));
      memcpy(uuid1, that.uuid1, sizeof(uuid_array_t));
      bsDiffs = that.bsDiffs;
      group = that.group;
      title = that.title;
      user = that.user;
      id = that.id;
      indatabase = that.indatabase;
      listindex = that.listindex;
      unknflds0 = that.unknflds0;
      unknflds1 = that.unknflds1;
    }
    return *this;
  }
};

struct equal_id {
  equal_id(int const& id) : m_id(id) {}
  bool operator()(st_CompareData const& rdata) const
  {
    return (rdata.id == m_id);
  }

  int m_id;
};

// Vector of entries passed from DboxMain::Compare to CompareResultsDlg
// Used for "Only in Original DB", "Only in Comparison DB" and
// in "Both with Differences"
typedef std::vector<st_CompareData> CompareData;

// The following structure is needed for compare to send back data
// to allow copying, viewing and editing of entries
struct st_CompareInfo {
  PWScore *pcore0;     // original DB
  PWScore *pcore1;     // comparison DB
  uuid_array_t uuid0;  // original DB
  uuid_array_t uuid1;  // comparison DB
  int  clicked_column;
};

class CCompareResultsDlg : public CPWResizeDialog
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
    PWScore *pcore0, PWScore *pcore1,
    CReport *rpt);

  // st_CompareInfo Functions
  enum {EDIT = 0, VIEW, COPY_TO_ORIGINALDB, COPY_TO_COMPARISONDB};

  // Column indices
  // IDENTICAL means CURRENT + COMPARE but identical
  // BOTH means CURRENT + COMPARE but with differences
  enum {IDENTICAL = -2, BOTH = -1 , CURRENT = 0, COMPARE, 
    GROUP, TITLE, USER, PASSWORD, NOTES, URL, AUTOTYPE, PWHIST, 
    CTIME, ATIME, XTIME, PMTIME, RMTIME, POLICY,
    LAST};

  // Dialog Data
  //{{AFX_DATA(CCompareResultsDlg)
  enum { IDD = IDD_COMPARE_RESULTS };
  CListCtrl m_LCResults;
  int m_iSortedColumn;
  bool m_bSortAscending;
  CSecString m_cs_Filename1, m_cs_Filename2;
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
  bool CopyLeftOrRight(const bool bCopyLeft);
  void UpdateStatusBar();
  bool ProcessFunction(const int ifunction, st_CompareData *st_data);
  void WriteReportData();
  st_CompareData * GetCompareData(const DWORD dwItemData);
  static st_CompareData * GetCompareData(const DWORD dwItemData, CCompareResultsDlg *self);
  void AddCompareEntries(const bool bAddIdentical);

  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CCompareResultsDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnViewCompareReport();
  afx_msg void OnShowIdenticalEntries();
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemDoubleClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnItemRightClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnCompareViewEdit();
  afx_msg void OnCompareCopyToOriginalDB();
  afx_msg void OnCompareCopyToComparisonDB();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CompareData m_OnlyInCurrent;
  CompareData m_OnlyInComp;
  CompareData m_Conflicts;
  CompareData m_Identical;
  CItemData::FieldBits m_bsFields;

  PWScore *m_pcore0, *m_pcore1;
  CReport *m_prpt;

  size_t m_numOnlyInCurrent, m_numOnlyInComp, m_numConflicts, m_numIdentical;
  int m_row, m_column;
  int m_nCols;
};
