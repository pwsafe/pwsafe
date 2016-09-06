/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ViewAttahmentDlg.h
//-----------------------------------------------------------------------------
#include "PWResizeDialog.h"
#include "ImgStatic.h"

#include "core/ItemAtt.h"

class CViewAttachmentDlg : public CPWResizeDialog
{
	DECLARE_DYNAMIC(CViewAttachmentDlg)

public:
	CViewAttachmentDlg(CWnd *pParent, CItemAtt *patt);   // standard constructor
	virtual ~CViewAttachmentDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEWATTACHMENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnOK();
  afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
  CItemAtt *m_pattachment;
  CImgStatic m_stImgAttachment;
};
