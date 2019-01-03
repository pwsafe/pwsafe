/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWPListEntries dialog

#include "resource.h"
#include "PWDialog.h"

#include "../../core/coredefs.h"

#include <vector>

class CPWPListEntries : public CPWDialog
{
	DECLARE_DYNAMIC(CPWPListEntries)

public:
	CPWPListEntries(CWnd* pParent = NULL, StringX sxPolicyName = L"",
    std::vector<st_GroupTitleUser> *pventries = NULL);
	virtual ~CPWPListEntries();

// Dialog Data
	enum { IDD = IDD_NPWP_LIST_ENTRIES };

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);

	DECLARE_MESSAGE_MAP()

  CListCtrl m_PolicyEntries;
  int m_iSortedColumn;
  BOOL m_bSortAscending;

private:
  std::vector<st_GroupTitleUser> *m_pventries;
  StringX m_sxPolicyName;

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  CFont *m_pAddEditFont;
};
