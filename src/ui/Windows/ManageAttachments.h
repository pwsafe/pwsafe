/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ManageAttachments.h.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"

#include "ViewAttachmentEntriesDlg.h"
#include "Windowsdefs.h"

// CManageAttachments.h dialog

class DboxMain;

// class CAttListCtrl

class CAttListCtrl : public CListCtrl {
public:
  CAttListCtrl();
  ~CAttListCtrl();

protected:
  //{{AFX_MSG(CAttListCtrl)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CFont *m_pOldFont, *m_pstkFont;
  COLORREF m_crWindowText, m_crRedText;
};

// class CManageAttachments

class CManageAttachments : public CPWDialog
{
	DECLARE_DYNAMIC(CManageAttachments)

public:
  CManageAttachments(CWnd* pParent = NULL); 
	virtual ~CManageAttachments();

  std::vector<st_att> &GetAttDetails() { return m_vAttDetails; }
  bool GetPurgeStatus(size_t i) { return m_vAttDetails[i].bToBePurged; }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MANAGEATTACHMENTS
  };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg BOOL OnInitDialog();
  afx_msg void OnAttachmentRightClick(NMHDR * /*pNotifyStruct*/, LRESULT *pLResult);
  afx_msg void OnListAttEntries();
  afx_msg void OnViewAttachment();
  afx_msg void OnPurgeAttachment();
  afx_msg void OnPerformAction();

	DECLARE_MESSAGE_MAP()

private:
  CAttListCtrl m_lcAttachments;

  std::vector<st_att> m_vAttDetails;
};
