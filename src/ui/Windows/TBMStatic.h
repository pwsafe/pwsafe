/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

/*
* Transparent Bitmap Static control
*/

class CTBMStatic : public CStatic
{
public:
  CTBMStatic() {}
  virtual ~CTBMStatic() {}

// Attributes

// Operations
public:
  void Init(const UINT nImageID);

protected:
  //{{AFX_MSG(CTBMStatic)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour);

  UINT m_nID;
  CBitmap m_Bitmap;
};
