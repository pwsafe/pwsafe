/// \file QueryAddName.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "QueryAddName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CQueryAddName::CQueryAddName(CWnd* pParent)
   : CDialog(CQueryAddName::IDD, pParent)
{
   m_dontqueryname = FALSE;
}


void CQueryAddName::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_QUERYCHECK, m_dontqueryname);
}


BEGIN_MESSAGE_MAP(CQueryAddName, CDialog)
END_MESSAGE_MAP()


void CQueryAddName::OnOK() 
{
   UpdateData(TRUE);
   app.WriteProfileInt("", "queryaddname", not(m_dontqueryname));
   CDialog::OnOK();
}


void CQueryAddName::OnCancel() 
{
   UpdateData(TRUE);
   app.WriteProfileInt("", "queryaddname", not(m_dontqueryname));
   CDialog::OnCancel();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
