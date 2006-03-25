/// \file RemindSaveDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "corelib/PWSprefs.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "RemindSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CRemindSaveDlg::CRemindSaveDlg(CWnd* pParent)
   : CDialog(CRemindSaveDlg::IDD, pParent)
{
  m_dontask = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DontAskSaveMinimize) ? TRUE : FALSE;
}


void CRemindSaveDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontask);
}


BEGIN_MESSAGE_MAP(CRemindSaveDlg, CDialog)
END_MESSAGE_MAP()


void CRemindSaveDlg::OnCancel() 
{
   UpdateData(TRUE);
   PWSprefs::GetInstance()->SetPref(PWSprefs::DontAskSaveMinimize,
				    m_dontask == TRUE);
   CDialog::OnCancel();
}


void CRemindSaveDlg::OnOK() 
{
   UpdateData(TRUE);
   PWSprefs::GetInstance()->SetPref(PWSprefs::DontAskSaveMinimize,
				    m_dontask == TRUE);
   CDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
