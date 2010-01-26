/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "corelib/StringX.h"
#include "PWDialog.h"

#include "corelib/PWScore.h"  // for st_DBProperties

#include <vector>

struct st_recfile {
  int rc;
  StringX filename;
  st_DBProperties dbp;
};

// CDisplayEmerBkupFiles dialog

class CDisplayEmerBkupFiles : public CDialog
{
public:
  CDisplayEmerBkupFiles(CWnd* pParent, std::wstring &wsDBDrive,
                    std::wstring &wsDBPath, 
                    st_DBProperties &st_dbpcore,
                    std::vector<st_recfile> &vValidEBackupfiles);
  virtual ~CDisplayEmerBkupFiles();

  // Dialog Data
  enum { IDD = IDD_DISPLAYEMERBKUPFILES };
  CListCtrl m_RFListCtrl;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnIgnore();
  afx_msg void OnSelect();
  afx_msg void OnDelete();
  afx_msg void OnItemSelected(NMHDR *pNMHDR, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

private:
  int m_iSelectedItem;
  UINT m_DriveType;
  std::wstring m_wsDBPath;
  st_DBProperties m_st_dbpcore;
  std::vector<st_recfile> m_vValidEBackupfiles;
  std::vector<int> m_vrc;
};
