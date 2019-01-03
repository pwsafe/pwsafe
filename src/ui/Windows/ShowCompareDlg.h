/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ShowCompareDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "SCWListCtrl.h"

#include "core/ItemData.h"

#include "os/UUID.h"

#include <vector>

class PWScore;
class CInfoDisplay;

class CShowCompareDlg : public CPWDialog
{

public:
  // default constructor
  CShowCompareDlg(CItemData *pci, CItemData *pci_other, CWnd *pParent,
                  const bool bDifferentDBs);
  virtual ~CShowCompareDlg();

  pws_os::CUUID GetUUID();
  enum { IDD = IDD_SHOW_COMPARE };

  bool SetNotesWindow(const CPoint ptClient, const bool bVisible = true);

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnShowIdenticalFields();

  DECLARE_MESSAGE_MAP()

  int m_ShowIdenticalFields;

private:
  void PopulateResults(bool bShowAll);
  CString GetDCAString(int iValue, bool isShift) const;
  CString GetEntryTypeString(CItemData::EntryType et) const;

  CSCWListCtrl m_ListCtrl;
  CItemData *m_pci, *m_pci_other;
  CInfoDisplay *m_pNotesDisplay;

  bool m_bDifferentDB;

  std::vector<UINT> m_DCA;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
