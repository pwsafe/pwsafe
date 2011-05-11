/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "WZPropertyPage.h"
#include "ControlExtns.h"

#include "resource.h"

class CVKeyBoardDlg;

class CWZSelectDB : public CWZPropertyPage
{
public:
  DECLARE_DYNAMIC(CWZSelectDB)

  CWZSelectDB(CWnd *pParent, UINT nIDCaption, const int nType);
  ~CWZSelectDB();

  enum {IDD = IDD_WZSELECTDB};

  CString m_defexpdelim;

protected:
  CSecEditExtn *m_pctlPasskey;
  CEditExtn *m_pctlDB;
  CSecString m_passkey;
  CStaticExtn m_stc_warning;
  CString m_filespec;
  int m_tries, m_state;
  int m_bAdvanced;

  BOOL OnInitDialog();
  void DoDataExchange(CDataExchange* pDX);
  LRESULT OnWizardNext();

  // Generated message map functions
  //{{AFX_MSG(CWZSelectDB)
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  virtual BOOL OnSetActive();

  afx_msg void OnHelp();
  afx_msg void OnPassKeyChange();
  afx_msg void OnDatabaseChange();
  afx_msg void OnOpenFileBrowser();
  afx_msg void OnVirtualKeyboard();
  afx_msg void OnAdvanced();
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX,
                                    const CString &delimiter);
  enum {KEYPRESENT = 1, DBPRESENT = 2, BOTHPRESENT = 3};

  COLORREF m_syncwarning_cfOldColour;

  bool ProcessPhrase(const StringX &filename, const StringX &passkey);
  bool m_bFileExistsUserAsked;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  st_SaveAdvValues *m_pst_SADV;
  CFont m_WarningFont;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
