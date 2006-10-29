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
   : super(CQuerySetDef::IDD, pParent)
{
   m_querycheck = FALSE;
   m_message = _T("");
}


void CQuerySetDef::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_QUERYCHECK, m_querycheck);
   DDX_Text(pDX, IDC_MESSAGE, m_message);
}


BEGIN_MESSAGE_MAP(CQuerySetDef, super)
END_MESSAGE_MAP()


void CQuerySetDef::OnOK() 
{
   UpdateData(TRUE);
   PWSprefs::GetInstance()->SetPref(PWSprefs::QuerySetDef,
				    m_querycheck == FALSE);
   super::OnOK();
}


void CQuerySetDef::OnCancel()
{
   UpdateData(TRUE);
   PWSprefs::GetInstance()->SetPref(PWSprefs::QuerySetDef,
				    m_querycheck == FALSE);
   super::OnCancel();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
