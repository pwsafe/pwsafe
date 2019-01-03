/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  if (nImageID != -1)
    m_nImageID = nImageID;

  if (m_nImageID == -1)
    return;

  // Need to convert the background colour to the user's
  // selected dialog colour.
  const COLORREF cr = GetSysColor(COLOR_3DFACE);
  const COLORREF cr192 = RGB(192, 192, 192);

  if (!m_imt.IsNull())
    m_imt.Detach();

  m_imt.LoadFromResource(AfxGetInstanceHandle(), m_nImageID);

  // Need to handle images with <= 8bpp (colour tables) and
  // those with higher colour information
  const int noOfCTableEntries = m_imt.GetMaxColorTableEntries();
  if (noOfCTableEntries > 0) {
    RGBQUAD *ctable = new RGBQUAD[noOfCTableEntries]; 
    m_imt.GetColorTable(0, noOfCTableEntries, ctable);

    for (int ic = 0; ic < noOfCTableEntries; ic++) {
      if (ctable[ic].rgbBlue  == 192 &&
          ctable[ic].rgbGreen == 192 &&
          ctable[ic].rgbRed   == 192) {
        ctable[ic].rgbBlue  = GetBValue(cr);
        ctable[ic].rgbGreen = GetGValue(cr);
        ctable[ic].rgbRed   = GetRValue(cr);
        break;
      }
    }
    m_imt.SetColorTable(0, noOfCTableEntries, ctable);
    delete[] ctable;
  } else {
    for (int x = 0; x < m_imt.GetWidth(); x++) {
      for (int y = 0; y < m_imt.GetHeight(); y++) {
        if (m_imt.GetPixel(x, y) == cr192)
          m_imt.SetPixel(x, y, cr);
      }
    }
  }

  HBITMAP hBmpOld = SetBitmap(HBITMAP(m_imt));
  ::DeleteObject(hBmpOld);
  ::DeleteObject(m_hBmp);
  m_hBmp = HBITMAP(m_imt);
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
