/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CSetFiltersDlg dialog

#include "PWFiltersDlg.h"
#include "SetHistoryFiltersDlg.h"
#include "SetPolicyFiltersDlg.h"

class CSetFiltersDlg : public CPWFiltersDlg
{
  DECLARE_DYNAMIC(CSetFiltersDlg)

public:
  CSetFiltersDlg(CWnd* pParent, st_filters *pfilters, const int &iWMSGID);
  virtual ~CSetFiltersDlg();
  void EnableDisableApply();

protected:

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnApply();
  afx_msg void OnCancel();

private:
  // Following needed to be able to send a message to DboxMain (parent)
  // to apply the current filters without ending this dialog.
  int m_iWMSGID;
  bool m_applied;
};
