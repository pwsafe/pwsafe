/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "core/StringX.h"
#include "core/UUIDGen.h"
#include "PWDialog.h"

#include <vector>

// Expired password Entry structure for CList
class CItemData;

struct ExpPWEntry {
  ExpPWEntry(const CItemData &ci);
  ExpPWEntry(const ExpPWEntry &ee);
  ExpPWEntry &operator=(const ExpPWEntry &that);
  uuid_array_t uuid;
  StringX group;
  StringX title;
  StringX user;
  StringX expirylocdate;  // user's long date/time  - format displayed in ListCtrl
  StringX expiryexpdate;  // "YYYY/MM/DD HH:MM:SS"  - format copied to clipboard - best for sorting
  time_t expirytttdate;
  int type;
};

class ExpiredList: public std::vector<ExpPWEntry>
{
 public:
  void Add(const CItemData &ci);
  void Update(const CItemData &ci) {Remove(ci); Add(ci);}
  void Remove(const CItemData &ci);
  ExpiredList GetExpired(int idays); // return a subset
};

// CExpPWListDlg dialog

class CExpPWListDlg : public CPWDialog
{
public:
  CExpPWListDlg(CWnd* pParent,
    const ExpiredList &expPWList,
    const CString& a_filespec = L"");
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
  afx_msg void OnDestroy();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedCopyExpToClipboard();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);

private:
  const ExpiredList &m_expPWList;
  static int CALLBACK ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
