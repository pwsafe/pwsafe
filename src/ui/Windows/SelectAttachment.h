/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SelectAttachment.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"

#include "ViewAttachmentEntriesDlg.h"

#include "Windowsdefs.h"

// CSelectAttachment dialog

class CSelectAttachment : public CPWDialog
{
	DECLARE_DYNAMIC(CSelectAttachment)

public:
  CSelectAttachment(CWnd *pParent, pws_os::CUUID *pattuuid, bool *bOrphaned); 
	virtual ~CSelectAttachment();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEWAVAILABLEATTACHMENTS
  };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg BOOL OnInitDialog();
  afx_msg void OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnAttachmentRightClick(NMHDR * /*pNotifyStruct*/, LRESULT *pLResult);
  afx_msg void OnListAttEntries();
  afx_msg void OnViewAttachment();
  afx_msg void OnSelect();

	DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  CListCtrl m_lcAttachments;
  pws_os::CUUID *m_patt_uuid;
  bool *m_pbOrphaned;

  std::vector<st_att> m_vAttDetails;

  int m_iSortedColumn;
  BOOL m_bSortAscending;
};
