/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "core/StringX.h"
#include "core/PWScore.h"  // for st_DBProperties

#include "PWDialog.h"
#include "EBListCtrl.h"
#include "resource.h"

#include <vector>

struct st_recfile {
  int rc;
  StringX filename;
  st_DBProperties dbp;
};

// CDisplayFSBkupFiles dialog

class CDisplayFSBkupFiles : public CPWDialog
{
public:
  CDisplayFSBkupFiles(CWnd* pParent, std::wstring &wsDBDrive,
                    std::wstring &wsDBPath, 
                    st_DBProperties &st_dbpcore,
                    std::vector<st_recfile> &vValidEBackupfiles);

  // Dialog Data
  enum { IDD = IDD_DISPLAYFSBKUPFILES };
  CEBListCtrl m_RFListCtrl;

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  afx_msg void OnHelp();
  afx_msg void OnContinue();
  afx_msg void OnSelect();
  afx_msg void OnDelete();
  afx_msg void OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

private:
  int m_iSelectedItem;
  UINT m_DriveType;
  std::wstring m_wsDBPath;
  st_DBProperties m_st_dbpcore;
  std::vector<st_recfile> m_vValidEBackupfiles;
  std::vector<int> m_vrc;
};
