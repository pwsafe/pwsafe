/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// TBMStatic.cpp : implementation file
//

/*
 * Transparent Bitmap Static control
*/

#include "stdafx.h"

#include "TBMStatic.h"
#include "winutils.h" // for ResizeBitmap

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void CTBMStatic::Init(const UINT nImageID)
{
  // Save resource IDs (Static and required image)
  m_nID = GetDlgCtrlID();

  VERIFY(WinUtil::LoadScaledBitmap(m_Bitmap, nImageID, true, m_hWnd));
 
  SetBitmap((HBITMAP)m_Bitmap);
}

BEGIN_MESSAGE_MAP(CTBMStatic, CStatic)
  //{{AFX_MSG_MAP(CTBMStatic)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
