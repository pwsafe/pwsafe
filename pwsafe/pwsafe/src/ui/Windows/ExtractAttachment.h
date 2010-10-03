/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CExtractAttachment dialog

#include "PWDialog.h"
#include "PWAttLC.h"
#include "PWHdrCtrlNoChng.h"
#include "corelib/attachments.h"

class DboxMain;

class CExtractAttachment : public CPWDialog
{
  DECLARE_DYNAMIC(CExtractAttachment)

public:
  CExtractAttachment(CWnd* pParent);   // standard constructor
  virtual ~CExtractAttachment();

// Dialog Data
  enum { IDD = IDD_EXTRACT_ATTACHMENT };
  void SetAttachments(const ATRVector &vATRecords)
  {m_vATRecords = vATRecords;}

protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnAttachmentListSelected(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnExtract();
  afx_msg void OnHelp();

  DECLARE_MESSAGE_MAP()

private:
  CPWAttLC m_AttLC;
  CPWHdrCtrlNoChng m_AttLCHeader;
  CToolTipCtrl *m_pToolTipCtrl;

  ATRVector m_vATRecords;
  BOOL m_bInitdone;
  DboxMain *m_pDbx;
};
