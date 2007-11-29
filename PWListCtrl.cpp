/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "stdafx.h"
#include "PWListCtrl.h"
#include "DboxMain.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CPWListCtrl::CPWListCtrl()
{
}

CPWListCtrl::~CPWListCtrl()
{
}


BEGIN_MESSAGE_MAP(CPWListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CPWListCtrl)
	ON_MESSAGE(WM_CHAR, OnCharItemlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


LRESULT CPWListCtrl::OnCharItemlist(WPARAM wParam, LPARAM /* lParam */)
{
  //DboxMain *pDbx = static_cast<DboxMain *>(GetParent());
  CString cs_char = (TCHAR)wParam;
  m_pDbx->FindNext(cs_char);
  return 0L;
}