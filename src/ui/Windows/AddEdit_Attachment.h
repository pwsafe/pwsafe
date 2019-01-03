/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Attachment.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "ImgStatic.h"

#include "resource.h"

#include "atlimage.h" // for CImage

class CAddEdit_Attachment : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Attachment)

  CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_Attachment();

  // Dialog Data
  //{{AFX_DATA(CAddEdit_Attachment)
  enum { IDD = IDD_ADDEDIT_ATT, IDD_SHORT = IDD_ADDEDIT_ATT_SHORT };

  enum ATT_TYPE { NO_ATTACHMENT = -1, ATTACHMENT_IS_IMAGE = 0, ATTACHMENT_NOT_IMAGE};

protected:
  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Attachment)
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  LRESULT OnDroppedFile(WPARAM wParam, LPARAM lParam);

  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Attachment)
  afx_msg void OnHelp();
  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);

  afx_msg void OnAttImport();
  afx_msg void OnAttExport();
  afx_msg void OnAttRemove();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void UpdateControls();
  void ShowPreview();

  bool m_bInitdone;
  ATT_TYPE m_attType;

  CSecString m_AttName;
  CSecString m_AttFileName;
  CString m_csSize, m_csFileCTime, m_csFileMTime, m_csMediaType;

  CStatic m_stcNoPreview;
  CImage m_AttImage;
  CImgStatic m_stImgAttachment;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
