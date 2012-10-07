/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file YubiMixin.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <iomanip>
#include <sstream>

#include "YubiMixin.h"
#include "os/linux/PWYubi.h"

void CYubiMixin::SetupMixin(wxWindow *btn, wxWindow *status)
{
  m_btn = btn;
  m_status = status;
  m_pollingTimer = new wxTimer(reinterpret_cast<wxEvtHandler *>(this), POLLING_TIMER_ID);
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer even
  m_pollingTimer->Start(250); // check for Yubikey every 250ms.
}


void CYubiMixin::yubiInserted(void)
{
  m_btn->Enable(true);
  m_status->SetLabel(_("<- Click on button to the left"));
}

void CYubiMixin::yubiRemoved(void)
{
  m_btn->Enable(false);
  m_status->SetLabel(_("Please insert your YubiKey"));
}

bool CYubiMixin::IsYubiInserted() const
{
  const PWYubi yubi;
  return yubi.IsYubiInserted();
}

void CYubiMixin::HandlePollingTimer()
{
  // Currently hmac check is blocking (ugh), so no need to check here
  // if a request is in-progress.
  bool inserted = IsYubiInserted();
  // call relevant callback if something's changed
  if (inserted != m_present) {
    m_present = inserted;
    if (m_present)
      yubiInserted();
    else
      yubiRemoved();
  }
}

StringX CYubiMixin::Bin2Hex(const unsigned char *buf, int len) const
{
  std::wostringstream os;
  os << std::setw(2);
  os << std::setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << std::hex << std::setw(2) << int(buf[i]);
  }
  return StringX(os.str().c_str());
}

