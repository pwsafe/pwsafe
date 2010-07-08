/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertyPage.h"

class COptions_PropertySheet;

class COptions_PropertyPage : public CPWPropertyPage
{
public:
  COptions_PropertyPage(UINT nID);
  virtual ~COptions_PropertyPage() {}

  virtual BOOL OnQueryCancel();

  // Retrieve DoubleClickAction or ClearClipboardOnMimimize
  // or if Hot Key set
  // Make sure no overlap with 'PP_' enum in CPWPropertyPage
  enum {PPOPT_GET_DCA = 10, PPOPT_GET_CCOM, PPOPT_HOTKEY_SET};

  DECLARE_DYNAMIC(COptions_PropertyPage)

protected:
  virtual BOOL OnInitDialog();
  COptions_PropertySheet *m_options_psh;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
