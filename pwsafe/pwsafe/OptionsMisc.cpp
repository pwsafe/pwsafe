// OptionsMisc.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif
#include "OptionsMisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc property page

IMPLEMENT_DYNCREATE(COptionsMisc, CPropertyPage)

COptionsMisc::COptionsMisc() : CPropertyPage(COptionsMisc::IDD)
{
	//{{AFX_DATA_INIT(COptionsMisc)
	//}}AFX_DATA_INIT
}

COptionsMisc::~COptionsMisc()
{
}

void COptionsMisc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsMisc)
	DDX_Check(pDX, IDC_CONFIRMDELETE, m_confirmdelete);
	DDX_Check(pDX, IDC_SAVEIMMEDIATELY, m_saveimmediately);
	DDX_Check(pDX, IDC_ESC_EXITS, m_escexits);
	DDX_Radio(pDX, IDC_DOUBLE_CLICK_COPIES, m_doubleclickaction);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsMisc, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsMisc)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers
