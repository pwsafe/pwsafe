/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// SysColStatic.h
//-----------------------------------------------------------------------------

/*
This entire file was copied from
http://www.codeguru.com/staticctrl/syscol_static.shtml
and was written by Pål K. Tønder 
*/

#include <atlimage.h>

//-----------------------------------------------------------------------------
class CSysColStatic : public CStatic
{
  // Construction
public:
  CSysColStatic();
  ~CSysColStatic();
  void ReloadBitmap(int nImageID = -1);

protected:
  //{{AFX_MSG(CSysColStatic)
  afx_msg void OnSysColorChange();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CImage m_imt;
  int m_nImageID;
  HBITMAP m_hBmp;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
