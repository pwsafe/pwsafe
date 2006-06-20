// ImportXMLErrDlg.cpp : implementation file
//

#include "ImportXMLErrDlg.h"
#include "ThisMfcApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportXMLErrDlg dialog

CImportXMLErrDlg::CImportXMLErrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportXMLErrDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportXMLErrDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

void CImportXMLErrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportXMLErrDlg)
	DDX_Text(pDX, IDC_XML_IMPORT_ACTION, m_strActionText);
	DDX_Text(pDX, IDC_XML_IMPORT_RESULTS, m_strResultText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportXMLErrDlg, CDialog)
	//{{AFX_MSG_MAP(CImportXMLErrDlg)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_COPY_ERRORS_TO_CLIPBOARD, OnBnClickedCopyToClipboard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportXMLErrDlg message handlers

//	---------------------------------------------------------------------------
void CImportXMLErrDlg::OnOK()
{
	CDialog::OnOK();
}

void
CImportXMLErrDlg::OnBnClickedCopyToClipboard()
{
	app.SetClipboardData(m_strResultText);
}