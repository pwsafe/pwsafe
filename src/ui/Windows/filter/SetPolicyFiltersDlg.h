/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CSetPolicyFiltersDlg dialog

#include "PWFiltersDlg.h"

class CSetPolicyFiltersDlg : public CPWFiltersDlg
{
  DECLARE_DYNAMIC(CSetPolicyFiltersDlg)

public:
  CSetPolicyFiltersDlg(CWnd* pParent, st_filters *pfilters,
                       CString filtername);   // standard constructor
  virtual ~CSetPolicyFiltersDlg();

protected:

  DECLARE_MESSAGE_MAP()
};
