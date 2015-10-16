/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Attachment.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "resource.h"

#include "afxwin.h"
#include "atlimage.h" // for CImage

#include <string>
#include <vector>

class CDragDropAttachment : public CStatic
{

  // Generated message map functions
protected:
  //{{AFX_MSG(CDragDropAttachment)
  afx_msg void OnDropFiles(HDROP hDropInfo);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

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

  void UpdateStats();

protected:
  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Attachment)
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  LRESULT OnDroppedFile(WPARAM wParam, LPARAM lParam);

  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Attachment)
  afx_msg void OnHelp();
  afx_msg void OnPaint();
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
  CRect m_initial_clientrect, m_initial_windowrect, m_clientrect;
  int m_xoffset, m_yoffset;

  CString m_AttName;
  CString m_AttFileName;
  CString m_csImageFilter;
  std::vector<std::wstring> m_image_extns;

  CString m_csSize, m_csFileCTime, m_csFileMTime, m_csMediaType;

  CStatic m_stcNoPreview;
  CImage m_AttImage;

  // Visible counterpart of CImage
  CDragDropAttachment m_AttStatic;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
