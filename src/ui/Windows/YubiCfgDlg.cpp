// YubiCfgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "YubiCfgDlg.h"
#include "afxdialogex.h"


// CYubiCfgDlg dialog


CYubiCfgDlg::CYubiCfgDlg(CWnd* pParent /*=NULL*/)
	: CPWDialog(CYubiCfgDlg::IDD, pParent)
    , m_YubiSN(_T(""))
    , m_YubiSK(_T(""))
{

}

CYubiCfgDlg::~CYubiCfgDlg()
{
}

void CYubiCfgDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_YUBI_SN, m_YubiSN);
    DDX_Text(pDX, IDC_YUBI_SK, m_YubiSK);
}


BEGIN_MESSAGE_MAP(CYubiCfgDlg, CPWDialog)
    ON_BN_CLICKED(IDC_YUBI_GEN_BN, &CYubiCfgDlg::OnYubiGenBn)
    ON_BN_CLICKED(IDOK, &CYubiCfgDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CYubiCfgDlg message handlers


void CYubiCfgDlg::OnYubiGenBn()
{
    // TODO: Add your control notification handler code here
}


void CYubiCfgDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
}
