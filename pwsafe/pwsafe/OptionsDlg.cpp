/// \file OptionsDlg.cpp
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
      not(app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskquestion"), FALSE));
   m_clearclipboard =
      (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskminimizeclearyesno"), FALSE));
   m_confirmdelete =
      not(app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), FALSE));
   m_lockdatabase =
      (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("databaseclear"), FALSE));
   m_confirmsaveonminimize =
      not(app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), FALSE));
   m_pwshowinedit =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwdefault"), FALSE);
   m_pwshowinlist =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwinlistdefault"), FALSE);
   m_usedefuser =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE);

   m_defusername = CMyString(app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T("")));

   m_querysetdef =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), TRUE);
   m_queryaddname =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("queryaddname"), TRUE);
   m_saveimmediately =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), TRUE);

	CString temp;

   UINT pwlen = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwlendefault"), 8);
   temp.Format(_T("%d"), pwlen);
   m_pwlendefault = (CMyString)temp;
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
   DDX_Check(pDX, IDC_DEFPWSHOWINEDIT, m_pwshowinedit);
   DDX_Check(pDX, IDC_DEFPWSHOWINLIST, m_pwshowinlist);
   DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
   DDX_Text(pDX, IDC_DEFUSERNAME, (CString &)m_defusername);
   DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
   DDX_Check(pDX, IDC_QUERYADDNAME, m_queryaddname);
   DDX_Check(pDX, IDC_ALWAYSONTOP, m_alwaysontop);
   DDX_Check(pDX, IDC_SAVEIMMEDIATELY, m_saveimmediately);
   DDX_Text(pDX, IDC_DEFPWLENGTH, (CString &)m_pwlendefault);
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

   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskquestion"), not(m_confirmcopy));
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskminimizeclearyesno"), m_clearclipboard);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), not(m_confirmdelete));
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("databaseclear"), m_lockdatabase);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), not(m_confirmsaveonminimize));
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("showpwdefault"), m_pwshowinedit);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("showpwinlistdefault"), m_pwshowinlist);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), m_usedefuser);
   app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), (CString &)m_defusername);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), m_querysetdef);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("queryaddname"), m_queryaddname);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("alwaysontop"), m_alwaysontop);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), m_saveimmediately);
#ifdef UNICODE
   int pwlendefault = (int) wcstol(m_pwlendefault, NULL, 10);
#else
   int pwlendefault = atoi(m_pwlendefault);
#endif
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("pwlendefault"), pwlendefault);
   app.m_pMainWnd = NULL;

   CDialog::OnOK();
}


void
COptionsDlg::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#options"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
   //WinHelp(0x20089, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_opts.htm",
              HH_DISPLAY_TOPIC, 0);
#endif
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
