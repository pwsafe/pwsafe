// ShowPasswordDlg.cpp
//-----------------------------------------------------------------------------
// Currently only compiled into the Pocket PC build, but available to the
// desktop build if required.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "ShowPasswordDlg.h"
#include "PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#endif

//-----------------------------------------------------------------------------
CShowPasswordDlg::CShowPasswordDlg(CWnd* pParent)
   : super(CShowPasswordDlg::IDD, pParent)
{
}


void CShowPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	super::DoDataExchange(pDX);
	DDX_Text( pDX, IDC_PASSWORD, m_Password );
	DDX_Text( pDX, IDC_MESSAGE, m_Message );
}


BEGIN_MESSAGE_MAP(CShowPasswordDlg, super)
END_MESSAGE_MAP()


BOOL CShowPasswordDlg::OnInitDialog()
{
	super::OnInitDialog();

	if ( m_Title.GetLength() > 0 )
	{
		SetWindowText( m_Title );
	}

	return TRUE;
}


void CShowPasswordDlg::OnCancel() 
{
	app.m_pMainWnd = NULL;
	super::OnCancel();
}


void CShowPasswordDlg::OnOK() 
{
	app.m_pMainWnd = NULL;
	super::OnOK();
}


void CShowPasswordDlg::SetPassword( CMyString &pw ) 
{
	m_Password	= pw;
}


void CShowPasswordDlg::SetTitle( CMyString &title ) 
{
	m_Title = title;
	m_Message.Format( IDS_SHOWPWD_STATIC, LPCTSTR(m_Title) );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
