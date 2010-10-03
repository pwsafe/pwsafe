/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Attachments.h
//-----------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "AddEdit_PropertyPage.h"
#include "PWAttLC.h"
#include "PWHdrCtrlNoChng.h"
#include "corelib/ItemData.h"
#include "corelib/attachments.h"

// CAddEdit_Attachments dialog

class CAddEdit_Attachments : public CAddEdit_PropertyPage
{
  DECLARE_DYNAMIC(CAddEdit_Attachments)

public:
  CAddEdit_Attachments(CWnd *pParent, st_AE_master_data *pAEMD);
  virtual ~CAddEdit_Attachments();

// Dialog Data
  enum { IDD = IDD_ADDEDIT_ATTACHMENTS };

  CStaticExtn m_stc_warning;
  void OnDropFiles(HDROP hDrop);

  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Attachments)
protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Attachments)
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnAddNewAttachment();
  afx_msg void OnDeleteNewAttachment();
  afx_msg void OnNewAttachmentListSelected(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg LRESULT OnAttachmentChanged(WPARAM wParam, LPARAM lParam);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg LRESULT DropFiles(WPARAM hDrop, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool HasExistingChanged();
  bool AnyNewAttachments();

  CToolTipCtrl *m_pToolTipCtrl;
  CPWAttLC m_AttLC, m_NewAttLC;
  CPWHdrCtrlNoChng m_AttLCHeader, m_NewAttLCHeader;

  std::vector<BYTE> m_vAttFlags;
  uuid_array_t m_entry_uuid;
  bool m_bInitdone;

  CStaticExtn m_stc_DropFiles;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
