// Properties.cpp : implementation file
//

#include "stdafx.h"
#include "Properties.h"

// CProperties dialog

IMPLEMENT_DYNAMIC(CProperties, CDialog)

CProperties::CProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CProperties::IDD, pParent)
{
}

CProperties::~CProperties()
{
}

void CProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProperties, CDialog)
	ON_BN_CLICKED(IDOK, &CProperties::OnOK)
END_MESSAGE_MAP()


// CProperties message handlers

void CProperties::OnOK()
{
	CDialog::OnOK();
}

BOOL CProperties::OnInitDialog()
{
	GetDlgItem(IDC_DATABASENAME)->SetWindowText(m_database);
	GetDlgItem(IDC_DATABASEFORMAT)->SetWindowText(m_databaseformat);
	GetDlgItem(IDC_NUMGROUPS)->SetWindowText(m_numgroups);
	GetDlgItem(IDC_NUMENTRIES)->SetWindowText(m_numentries);
	GetDlgItem(IDC_SAVEDON)->SetWindowText(m_whenlastsaved);
	GetDlgItem(IDC_SAVEDBY)->SetWindowText(m_wholastsaved);
	GetDlgItem(IDC_SAVEDAPP)->SetWindowText(m_whatlastsaved);

	return TRUE;
}
