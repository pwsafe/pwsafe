/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file SysColStatic.cpp
//-----------------------------------------------------------------------------
/*
This entire file was copied from
http://www.codeguru.com/staticctrl/syscol_static.shtml
and was written by Pål K. Tønder 
*/

#include "stdafx.h"
#include "SysColStatic.h"
#if defined(POCKET_PC)
#include "pocketpc/PocketPC.h"
#endif
#include "PasswordSafe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSysColStatic::CSysColStatic() :
  m_nImageID(-1), m_hBmp(NULL)
{
}

CSysColStatic::~CSysColStatic()
{
  ::DeleteObject(m_hBmp);
}

void CSysColStatic::ReloadBitmap(int nImageID)
{
  if(nImageID != -1)
    m_nImageID = nImageID;

  if(m_nImageID == -1)
    return;

  HBITMAP hBmpOld;
  HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
                                      MAKEINTRESOURCE(m_nImageID),
                                      IMAGE_BITMAP,
                                      0, 0,
                                      WCE_INS 0);  // WinCE only {kjp}
  WCE_DEL LR_LOADMAP3DCOLORS);  // not WinCE {kjp}

  if (hBmp == NULL)
    return;

  hBmpOld = SetBitmap(hBmp);
  ::DeleteObject(hBmpOld);
  ::DeleteObject(m_hBmp);
  m_hBmp = hBmp;
}

BEGIN_MESSAGE_MAP(CSysColStatic, CStatic)
  //{{AFX_MSG_MAP(CSysColStatic)
  ON_WM_SYSCOLORCHANGE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysColStatic message handlers

void CSysColStatic::OnSysColorChange() 
{
  CStatic::OnSysColorChange();
  ReloadBitmap(); 
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
