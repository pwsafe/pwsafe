/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "Options_PropertySheet.h"

IMPLEMENT_DYNAMIC(COptions_PropertySheet, CPWPropertySheet)

COptions_PropertySheet::COptions_PropertySheet(UINT nID, CWnd* pParent)
  : CPWPropertySheet(nID, pParent)
{
}

COptions_PropertySheet::~COptions_PropertySheet()
{
}
