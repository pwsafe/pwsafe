/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "Options_PropertySheet.h"
#include "Options_PropertyPage.h"

IMPLEMENT_DYNAMIC(COptions_PropertySheet, CPWPropertySheet)

COptions_PropertySheet::COptions_PropertySheet(UINT nID, CWnd* pParent)
  : CPWPropertySheet(nID, pParent)
{
}

COptions_PropertySheet::~COptions_PropertySheet()
{
}

BOOL COptions_PropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // There is no OnOK for classes derived from CPropertySheet,
  // so we make our own!
  if (LOWORD(wParam) == IDOK) {
    // First send a message to all loaded pages using base class function.
    // We want them all to update their variables in the Master Data area.
    // And call OnApply() rather than the default OnOK processing
    // Note: This message is only sent to PropertyPages that have been
    // loaded - i.e. the user has selected to view them, since obviously
    // the user would not have changed their values if not displayed. Duh!
    if (SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CPWPropertyPage::PP_UPDATE_VARIABLES, 0L) != 0)
      return TRUE;

    // Now end it all so that OnApply isn't called again
    CPWPropertySheet::EndDialog(IDOK);
    return TRUE;
  }
  return CPWPropertySheet::OnCommand(wParam, lParam);
}