/// \file QuerySetDef.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "QuerySetDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CQuerySetDef::CQuerySetDef(CWnd* pParent)
   : CDialog(CQuerySetDef::IDD, pParent)
{
   m_querycheck = FALSE;
   m_message = "";
}


void CQuerySetDef::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_QUERYCHECK, m_querycheck);
   DDX_Text(pDX, IDC_MESSAGE, m_message);
}


BEGIN_MESSAGE_MAP(CQuerySetDef, CDialog)
END_MESSAGE_MAP()


void CQuerySetDef::OnOK() 
{
   UpdateData(TRUE);
   app.WriteProfileInt("", "querysetdef", m_querycheck ? 0 : 1);
   CDialog::OnOK();
}


void CQuerySetDef::OnCancel()
{
   UpdateData(TRUE);
   app.WriteProfileInt("", "querysetdef", m_querycheck ? 0 : 1);
   CDialog::OnCancel();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
