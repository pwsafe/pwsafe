/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SetPolicyFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"

#include "PWFiltersDlg.h"
#include "SetPolicyFiltersDlg.h"

#include "../resource3.h"

// CSetPolicyFiltersDlg dialog

IMPLEMENT_DYNAMIC(CSetPolicyFiltersDlg, CPWFiltersDlg)

CSetPolicyFiltersDlg::CSetPolicyFiltersDlg(CWnd* pParent, st_filters *pfilters,
                                           CString filtername)
  : CPWFiltersDlg(pParent, DFTYPE_PWPOLICY, filtername)
{
  ASSERT(pParent != NULL);
  ASSERT(pfilters != NULL);

  m_bAllowSet = false;
  m_pParent = pParent;
  m_pfilters = pfilters;

  m_cstitle.LoadString(IDS_SETPWPOLICYFILTER);
}

CSetPolicyFiltersDlg::~CSetPolicyFiltersDlg()
{
}

BEGIN_MESSAGE_MAP(CSetPolicyFiltersDlg, CPWFiltersDlg)
END_MESSAGE_MAP()
