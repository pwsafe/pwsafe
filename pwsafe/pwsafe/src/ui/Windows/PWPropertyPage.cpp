/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "DboxMain.h"
#include "PWPropertyPage.h"
#include "GeneralMsgBox.h"

#if defined(POCKET_PC)
#error "TBD - define proper PropertyPage base class for PPC"
#endif

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWPropertyPage, CPropertyPage)

CPWPropertyPage::CPWPropertyPage(UINT nID)
: CPropertyPage(nID)
{
  m_psp.dwFlags |= PSP_HASHELP;
}

static UINT chooseResource(UINT nID, UINT shortID)
{
  // based on current screen height, decide if we want to display
  // the normal (tall) page, or the "short" version (for netbooks)
  int Y = ::GetSystemMetrics(SM_CYSCREEN);
  const int THRESHOLD = 600;
  return (Y > THRESHOLD) ? nID : shortID;
}

CPWPropertyPage::CPWPropertyPage(UINT nID, UINT shortID)
  : CPropertyPage(chooseResource(nID, shortID))
{
  m_psp.dwFlags |= PSP_HASHELP;
}

LRESULT CPWPropertyPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  CWnd *pParent = GetParent();
  while (pParent != NULL) {
    DboxMain *pDbx = dynamic_cast<DboxMain *>(pParent);
    if (pDbx != NULL && pDbx->m_eye_catcher != NULL &&
        wcscmp(pDbx->m_eye_catcher, EYE_CATCHER) == 0) {
      pDbx->ResetIdleLockCounter(message);
      break;
    } else
      pParent = pParent->GetParent();
  }
  if (pParent == NULL)
    pws_os::Trace(L"CPWPropertyPage::WindowProc - couldn't find DboxMain ancestor\n");

  return CPropertyPage::WindowProc(message, wParam, lParam);
}

