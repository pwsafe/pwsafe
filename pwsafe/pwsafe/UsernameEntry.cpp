/// \file UsernameEntry.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "UsernameEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CUsernameEntry::CUsernameEntry(CWnd* pParent)
   : CDialog(CUsernameEntry::IDD, pParent)
{
   m_makedefuser = FALSE;
   m_username = "";
}


void CUsernameEntry::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_MAKEDEF, m_makedefuser);
   DDX_Text(pDX, IDC_USERNAME, (CString &)m_username);
}


BEGIN_MESSAGE_MAP(CUsernameEntry, CDialog)
END_MESSAGE_MAP()


void
CUsernameEntry::OnOK() 
{
   UpdateData(TRUE);

   app.WriteProfileInt("", "usedefuser", m_makedefuser);
   if (m_makedefuser==TRUE)
      app.WriteProfileString("", "defusername", m_username);

   CDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
