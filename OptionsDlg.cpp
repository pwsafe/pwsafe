/// \file OptionsDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"
#include "MyString.h"

#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define not(x) ((x) ? 0 : 1)

//-----------------------------------------------------------------------------
COptionsDlg::COptionsDlg(CWnd* pParent)
   : CDialog(COptionsDlg::IDD, pParent)
{
   m_confirmcopy =
      not(app.GetProfileInt("", "dontaskquestion", FALSE));
   m_clearclipboard =
      (app.GetProfileInt("", "dontaskminimizeclearyesno", FALSE));
   m_confirmdelete =
      not(app.GetProfileInt("", "deletequestion", FALSE));
   m_lockdatabase =
      (app.GetProfileInt("", "databaseclear", FALSE));
   m_confirmsaveonminimize =
      not(app.GetProfileInt("", "dontasksaveminimize", FALSE));
   m_pwshow =
      app.GetProfileInt("", "showpwdefault", FALSE);
   m_usedefuser =
      app.GetProfileInt("", "usedefuser", FALSE);

   m_defusername = CMyString(app.GetProfileString("", "defusername", ""));

   m_querysetdef =
      app.GetProfileInt("", "querysetdef", TRUE);
   m_queryaddname =
      app.GetProfileInt("", "queryaddname", TRUE);
}


void
COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARBOARD, m_clearclipboard);
   DDX_Check(pDX, IDC_CONFIRMCOPY, m_confirmcopy);
   DDX_Check(pDX, IDC_CONFIRMDELETE, m_confirmdelete);
   DDX_Check(pDX, IDC_LOCKBASE, m_lockdatabase);
   DDX_Check(pDX, IDC_SAVEMINIMIZE, m_confirmsaveonminimize);
   DDX_Check(pDX, IDC_DEFPWSHOW, m_pwshow);
   DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
   DDX_Text(pDX, IDC_DEFUSERNAME, (CString &)m_defusername);
   DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
   DDX_Check(pDX, IDC_QUERYADDNAME, m_queryaddname);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_LOCKBASE, OnLockbase)
   ON_BN_CLICKED(IDC_USEDEFUSER, OnDefaultuser)
END_MESSAGE_MAP()


void
COptionsDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
COptionsDlg::OnOK()
{
   UpdateData(TRUE);

   app.WriteProfileInt("", "dontaskquestion", not(m_confirmcopy));
   app.WriteProfileInt("", "dontaskminimizeclearyesno", m_clearclipboard);
   app.WriteProfileInt("", "deletequestion", not(m_confirmdelete));
   app.WriteProfileInt("", "databaseclear", m_lockdatabase);
   app.WriteProfileInt("", "dontasksaveminimize", not(m_confirmsaveonminimize));
   app.WriteProfileInt("", "showpwdefault", m_pwshow);
   app.WriteProfileInt("", "usedefuser", m_usedefuser);
   app.WriteProfileString("", "defusername", (CString &)m_defusername);
   app.WriteProfileInt("", "querysetdef", m_querysetdef);
   app.WriteProfileInt("", "queryaddname", m_queryaddname);
   app.m_pMainWnd = NULL;

   CDialog::OnOK();
}


void
COptionsDlg::OnHelp() 
{
   //WinHelp(0x20089, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_opts.htm",
              HH_DISPLAY_TOPIC, 0);
}


void
COptionsDlg::OnLockbase() 
{
   if (((CButton*)GetDlgItem(IDC_LOCKBASE))->GetCheck() == 1)
      GetDlgItem(IDC_SAVEMINIMIZE)->EnableWindow(TRUE);
   else
      GetDlgItem(IDC_SAVEMINIMIZE)->EnableWindow(FALSE);
}


void
COptionsDlg::OnDefaultuser()
{
   if (((CButton*)GetDlgItem(IDC_USEDEFUSER))->GetCheck() == 1)
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(FALSE);
   }
   else
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(TRUE);
   }
}


BOOL
COptionsDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
	
   //Set dependant fields on/off
   OnLockbase();
   OnDefaultuser();
	
   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
