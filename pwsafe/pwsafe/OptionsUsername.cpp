// OptionsUsername.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "resource.h"
#include "OptionsUsername.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsUsername property page

IMPLEMENT_DYNCREATE(COptionsUsername, CPropertyPage)

COptionsUsername::COptionsUsername() : CPropertyPage(COptionsUsername::IDD)
{
	//{{AFX_DATA_INIT(COptionsUsername)
	//}}AFX_DATA_INIT
}

COptionsUsername::~COptionsUsername()
{
}

void COptionsUsername::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsUsername)
	DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
	DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
	DDX_Text(pDX, IDC_DEFUSERNAME, m_defusername);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsUsername, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsUsername)
	ON_BN_CLICKED(IDC_USEDEFUSER, OnUsedefuser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsUsername message handlers

void COptionsUsername::OnUsedefuser() 
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

BOOL COptionsUsername::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
   OnUsedefuser();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
