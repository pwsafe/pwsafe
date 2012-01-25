/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "os/UUID.h"

#include <vector>

class CItemData;
class PWScore;

class CShowCompareDlg : public CPWDialog
{

public:
  // default constructor
  CShowCompareDlg(CItemData *pci, CItemData *pci_other, CWnd *pParent);
  virtual ~CShowCompareDlg();

  pws_os::CUUID GetUUID();
  enum { IDD = IDD_SHOW_COMPARE };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  virtual BOOL OnInitDialog();
  afx_msg void OnShowIdenticalFields();

  DECLARE_MESSAGE_MAP()

  int m_ShowIdenticalFields;

private:
  void PopulateResults(const bool bShowAll);
  CString GetDCAString(const int iValue, const bool isShift);

  CSCWListCtrl m_ListCtrl;
  CItemData *m_pci, *m_pci_other;

  std::vector<UINT> m_DCA;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
