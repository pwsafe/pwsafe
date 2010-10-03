/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CPWFileDialog : public CFileDialog
{
public:
  CPWFileDialog(BOOL bOpenFileDialog,
                LPCWSTR lpszDefExt = NULL,
                LPCWSTR lpszFileName = NULL,
                DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                LPCWSTR lpszFilter = NULL,
                CWnd* pParentWnd = NULL,
                DWORD dwSize = 0,
                bool bAttachment = false)
  : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter,
                pParentWnd, dwSize),
    m_bAttachment(bAttachment), m_bOpenFileDialog(bOpenFileDialog), m_csDesc(L"") {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual INT_PTR DoModal();

  DECLARE_DYNAMIC(CPWFileDialog)

  CString GetDescription() {return m_csDesc;}

protected:
  CStatic m_stcDesc;
  CEdit m_edtDesc;

  //{{AFX_MSG(CPWFileDialog)
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CString m_csDesc;
  bool m_bAttachment;
  BOOL m_bOpenFileDialog;
};
