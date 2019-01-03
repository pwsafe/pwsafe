/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  CSetFiltersDlg(CWnd* pParent, st_filters *pfilters, 
    const int &iWMSGID, const bool bCanHaveAttachments,
    const std::set<StringX> *psMediaTypes = NULL,
    const bool bAllowSet = true);

  virtual ~CSetFiltersDlg();
  void EnableDisableApply();

protected:
  afx_msg void OnApply();

  DECLARE_MESSAGE_MAP()

private:
  // Following needed to be able to send a message to DboxMain (parent)
  // to apply the current filters without ending this dialog.
  int m_iWMSGID;
};
