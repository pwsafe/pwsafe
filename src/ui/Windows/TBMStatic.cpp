/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void CTBMStatic::Init(const UINT nImageID)
{
  // Save resource IDs (Static and required image)
  m_nID = GetDlgCtrlID();

  // Load bitmap
  VERIFY(m_Bitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED))));

  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  SetBitmapBackground(m_Bitmap, crCOLOR_3DFACE);

  SetBitmap((HBITMAP)m_Bitmap);
}

BEGIN_MESSAGE_MAP(CTBMStatic, CStatic)
  //{{AFX_MSG_MAP(CTBMStatic)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTBMStatic::SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour)
{
  // Get how many pixels in the bitmap
  BITMAP bmInfo;
  bm.GetBitmap(&bmInfo);

  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  ASSERT(pixels != NULL);

  const RGBTRIPLE newbkgrndColourRGB = {GetBValue(newbkgrndColour),
                                        GetGValue(newbkgrndColour),
                                        GetRValue(newbkgrndColour)};

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue  == 192 &&
        pixels[i].rgbtGreen == 192 &&
        pixels[i].rgbtRed   == 192) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}
