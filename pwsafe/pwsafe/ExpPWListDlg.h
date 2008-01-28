/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "corelib/MyString.h"
#include "PWDialog.h"

#include <vector>

// Expired password Entry structure for CList
class CItemData;

struct ExpPWEntry {
  ExpPWEntry(const CItemData &ci, time_t now, time_t LTime);
  CMyString group;
  CMyString title;
  CMyString user;
  CMyString expirylocdate;  // user's long dat/time   - format displayed in ListCtrl
  CMyString expiryexpdate;  // "YYYY/MM/DD HH:MM:SS"  - format copied to clipboard - best for sorting
  time_t expirytttdate;
  int type;
};

typedef std::vector<ExpPWEntry> ExpiredList;

// CExpPWListDlg dialog

class CExpPWListDlg : public CPWDialog
{
public:
  CExpPWListDlg(CWnd* pParent,
    const ExpiredList &expPWList,
    const CString& a_filespec = _T("")
    );
  virtual ~CExpPWListDlg();

  // Dialog Data
  enum { IDD = IDD_DISPLAY_EXPIRED_ENTRIES };
  CListCtrl m_expPWListCtrl;
  CImageList *m_pImageList;
  CString m_message;
  int m_iSortedColumn; 
  BOOL m_bSortAscending; 

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedCopyExpToClipboard();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);

private:
  const ExpiredList &m_expPWList;
  static int CALLBACK ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
