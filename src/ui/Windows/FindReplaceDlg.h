/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FindReplaceDlg.h : header file
//

#pragma once

#include "PWDialog.h"

#include "FRListCtrl.h"
#include "FRHdrCtrl.h"

#include "SecString.h"
#include "TBMStatic.h"

#include "Fonts.h"

#include "core/Match.h" // for PWSMatch::MatchRule

// CFindReplaceDlg dialog

class CFindReplaceDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CFindReplaceDlg)

public:
	CFindReplaceDlg(CWnd *pParent);   // standard constructor
	virtual ~CFindReplaceDlg();

  void SetAllSelected(FRState state);
  void SortRows(int iColumn);

  FRState GetResultState(const size_t iresult);
  bool SetResultState(const size_t iresult, const FRState state);
  size_t GetNumberResults() { return m_vFRResults.size(); }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FINDREPLACE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  afx_msg void OnCbnSelchangeField();
  afx_msg void OnCbnSelchangeStringRule();
  afx_msg void OnEdtChangeOldText();
  afx_msg void OnEdtChangeNewText();
  afx_msg void OnCase();
  afx_msg void OnSearch();
  afx_msg void OnExit();
  afx_msg void OnChangeSelected();
  afx_msg void OnChangeRowClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnChangeRowChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnChangeRowRightClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);

	DECLARE_MESSAGE_MAP()

private:
  enum { FIELDSELECTED = 1, RULESELECTED = 2, ORIGINALTEXTPRESENT = 4, ALL = 7 };

  UINT uiColumns[5] = { IDS_GROUP, IDS_TITLE, IDS_USERNAME, IDS_CURRENT_VALUE,  IDS_NEW_VALUE };

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  void UpdateButtons(const BOOL bEnable);
  StringX ChangeField(StringX &sxOldFieldValue, StringX &sxOldText, StringX &sxNewText,
    const bool bAddColour = false);

  std::vector<st_FRResults> m_vFRResults;

  int m_iSortedColumn, m_state, m_ibmHeight;
  bool m_bSortAscending, m_bCaseSensitive, m_bResultsLoaded;

  CFRListCtrl m_lctChanges;
  CComboBox m_cbxRule, m_cbxField;
  CEdit m_edtOldText, m_edtNewText;
  CButton m_btnCase, m_btnSearch, m_btnChangeSelected, m_btnExit;
  CSecString m_secOldText, m_secNewText, m_secOldTextOld;
  CTBMStatic m_Help1, m_Help2, m_Help3, m_Help4;

  PWSMatch::MatchRule m_rule, m_ruleold;
  CItem::FieldType m_ft, m_ftold;
};
