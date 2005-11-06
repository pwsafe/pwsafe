// ConfirmDeleteDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "ConfirmDeleteDlg.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CConfirmDeleteDlg::CConfirmDeleteDlg(CWnd* pParent)
   : super(CConfirmDeleteDlg::IDD, pParent)
{
  m_dontaskquestion =PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::DeleteQuestion);
}


void CConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
  BOOL B_dontaskquestion = m_dontaskquestion ? TRUE : FALSE;

  super::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CLEARCHECK, B_dontaskquestion);
  m_dontaskquestion = B_dontaskquestion == TRUE;
}


BEGIN_MESSAGE_MAP(CConfirmDeleteDlg, super)
END_MESSAGE_MAP()


void
CConfirmDeleteDlg::OnCancel() 
{
   super::OnCancel();
}


void
CConfirmDeleteDlg::OnOK() 
{
   UpdateData(TRUE);
   PWSprefs::GetInstance()->
     SetPref(PWSprefs::BoolPrefs::DeleteQuestion, m_dontaskquestion);
   super::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
