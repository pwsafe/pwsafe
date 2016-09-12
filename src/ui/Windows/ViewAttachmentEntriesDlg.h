/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ViewAttachmentEntriesDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "Windowsdefs.h"

#include "core/ItemData.h"

#include "afxcmn.h"

// CViewAttachmentEntriesDlg dialog

class CViewAttachmentEntriesDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CViewAttachmentEntriesDlg)

public:
	CViewAttachmentEntriesDlg(CWnd *pParent = NULL, std::vector<st_gtui> *pvst_gtui = NULL);   // standard constructor
	virtual ~CViewAttachmentEntriesDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEWATTACHMENTENTRIES };
#endif
  int m_iSortedColumn;
  BOOL m_bSortAscending;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  BOOL OnInitDialog();

  afx_msg void OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
	DECLARE_MESSAGE_MAP()

private:
  CListCtrl m_lcEntryList;
  CImageList *m_pImageList;
  std::vector<st_gtui> *m_pvst_gtui;

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
