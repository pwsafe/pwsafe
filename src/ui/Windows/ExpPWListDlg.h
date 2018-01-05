/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "PWDialog.h"
#include "ExpPswdLC.h"

#include "core/ExpiredList.h"

// CExpPWListDlg dialog

class ExpiredList;

// Local vector - has the group/title/user and locale version of expiry date
// Generated from similar vector in core, which doesn't need these.
struct st_ExpLocalListEntry {
  st_ExpLocalListEntry()
  : sx_group(L""), sx_title(L""), sx_user(L""), sx_expirylocdate(L""),
    expirytttXTime(time_t(0)), et(CItemData::ET_INVALID),
    bIsProtected(false), bHasAttachment(false),
    uuid(pws_os::CUUID::NullUUID()) {}

  st_ExpLocalListEntry(const st_ExpLocalListEntry &elle)
  : sx_group(elle.sx_group), sx_title(elle.sx_title), sx_user(elle.sx_user),
    sx_expirylocdate(elle.sx_expirylocdate),
    expirytttXTime(elle.expirytttXTime), et(elle.et),
    bIsProtected(elle.bIsProtected), bHasAttachment(elle.bHasAttachment),
    uuid(elle.uuid) {}

  st_ExpLocalListEntry &operator =(const st_ExpLocalListEntry &elle)
  {
    if (this != &elle) {
      sx_group = elle.sx_group;
      sx_title = elle.sx_title;
      sx_user = elle.sx_user;
      sx_expirylocdate = elle.sx_expirylocdate;
      expirytttXTime = elle.expirytttXTime;
      et = elle.et;
      bIsProtected = elle.bIsProtected;
      bHasAttachment = elle.bHasAttachment;
      uuid = elle.uuid;
    }
    return *this;
  }
 
  StringX sx_group;
  StringX sx_title;
  StringX sx_user;
  StringX sx_expirylocdate;  // user's long date/time  - format displayed to user in UI
  time_t expirytttXTime;
  CItemData::EntryType et; // Used to select image for display to user e.g.
                           // 'warn will expire' or 'has expired' &
                           // 'normal, aliasbase or shortcut base' entry
  bool bIsProtected;
  bool bHasAttachment;
  pws_os::CUUID uuid;
};

class CExpPWListDlg : public CPWDialog
{
public:
  CExpPWListDlg(CWnd* pParent, ExpiredList &expPWList,
    const CString &a_filespec = L"",
    const CString &csProtect = L"#", const CString &csAttachment = L"+");
    
  virtual ~CExpPWListDlg();

  // Dialog Data
  enum { IDD = IDD_DISPLAY_EXPIRED_ENTRIES };
  CExpPswdLC m_expPWListCtrl;
  CImageList *m_pImageList;
  CString m_Database;
  int m_iSortedColumn;
  BOOL m_bSortAscending;

  bool IsExpiryEntryProtected(size_t i) { return m_vExpLocalListEntries[i].bIsProtected; }

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnInitDialog();

  afx_msg void OnOK();
  afx_msg void OnDestroy();
  afx_msg void OnIconHelp();
  afx_msg void OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemDoubleClick(NMHDR *pNotifyStruct, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

private:
  int GetEntryImage(const st_ExpLocalListEntry &elle);
  ExpiredList &m_expPWList;

  std::vector<st_ExpLocalListEntry> m_vExpLocalListEntries;
  int m_idays;
  static int CALLBACK ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  
  CString m_csProtect, m_csAttachment;
};
