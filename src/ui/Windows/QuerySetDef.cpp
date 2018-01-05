/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file QuerySetDef.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "core/PwsPlatform.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "QuerySetDef.h"
#include "core/PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CQuerySetDef::CQuerySetDef(CWnd* pParent)
  : CPWDialog(CQuerySetDef::IDD, pParent)
{
  m_querycheck = FALSE;
  m_defaultusername = L"";
}

void CQuerySetDef::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_QUERYCHECK, m_querycheck);
  DDX_Text(pDX, IDC_DEFAULT_USERNAME, m_defaultusername);
}

BEGIN_MESSAGE_MAP(CQuerySetDef, CPWDialog)
END_MESSAGE_MAP()

void CQuerySetDef::OnOK() 
{
  UpdateData(TRUE);
  PWSprefs::GetInstance()->SetPref(PWSprefs::QuerySetDef,
    m_querycheck == FALSE);
  CPWDialog::OnOK();
}

void CQuerySetDef::OnCancel()
{
  UpdateData(TRUE);
  PWSprefs::GetInstance()->SetPref(PWSprefs::QuerySetDef,
    m_querycheck == FALSE);
  CPWDialog::OnCancel();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
