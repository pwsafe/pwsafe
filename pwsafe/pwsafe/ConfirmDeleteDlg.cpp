/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ConfirmDeleteDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

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
    GetPref(PWSprefs::DeleteQuestion);
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
     SetPref(PWSprefs::DeleteQuestion, m_dontaskquestion);
   super::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
