/// \file QuerySetDef.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "PwsPlatform.h"

#include "ThisMfcApp.h"
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "QuerySetDef.h"

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
   m_message = "";
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
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), m_querycheck ? 0 : 1);
   super::OnOK();
}


void CQuerySetDef::OnCancel()
{
   UpdateData(TRUE);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), m_querycheck ? 0 : 1);
   super::OnCancel();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
