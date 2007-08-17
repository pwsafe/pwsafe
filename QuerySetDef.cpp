/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file QuerySetDef.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PwsPlatform.h"

#include "ThisMfcApp.h"
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "QuerySetDef.h"
#include "corelib/PWSprefs.h"

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
   m_message = _T("");
}


void CQuerySetDef::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_QUERYCHECK, m_querycheck);
   DDX_Text(pDX, IDC_MESSAGE, m_message);
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
