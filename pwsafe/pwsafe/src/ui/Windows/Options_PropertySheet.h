/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertySheet.h"

class COptions_PropertySheet : public CPWPropertySheet
{
public:
  COptions_PropertySheet(UINT nID, CWnd* pDbx);
  ~COptions_PropertySheet();

  // Retrieve DoubleClickAction or ClearClipboardOnMimimize
  enum {PP_GET_DCA = 0, PP_GET_CCOM};

  DECLARE_DYNAMIC(COptions_PropertySheet)
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
