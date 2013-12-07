/*
 * Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "os/sleep.h"

void CYubiMixin::SetupMixin(wxWindow *btn, wxWindow *status)
{
  m_prompt1 = _("<- Click on button to the left"); // change via SetPrompt1
  m_prompt2 = _("Now touch your YubiKey's button"); // change via SetPrompt2
  m_btn = btn;
  m_status = status;
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer even
  // Hide Yubi controls if user doesn't have one:
  if (m_btn != NULL) m_btn->Show(yubiExists());
  if (m_status != NULL) m_status->Show(yubiExists());
}


bool CYubiMixin::yubiExists() const
{
  return PWYubi::YubiExists();
}

void CYubiMixin::yubiInserted(void)
{
  m_btn->Enable(true);
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(m_prompt1);
}

void CYubiMixin::yubiRemoved(void)
{
  m_btn->Enable(false);
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(_("Please insert your YubiKey"));
}

bool CYubiMixin::IsYubiInserted() const
{
  const PWYubi yubi;
  return yubi.IsYubiInserted();
}

void CYubiMixin::HandlePollingTimer()
{
  // Show Yubi controls when inserted first time:
  if (yubiExists()) {
    wxWindow *parent = NULL; // assume both have same parent
    if (m_btn != NULL) {m_btn->Show(true); parent = m_btn->GetParent();}
    if (m_status != NULL) {m_status->Show(true); parent = m_btn->GetParent();}
    if (parent != NULL) parent->Layout();
  }

  // Currently hmac check is blocking (ugh), so no need to check here
  // if a request is in-progress.
  bool inserted = IsYubiInserted();
  if (inserted != m_present) {
    m_present = inserted;
    UpdateStatus();
  }
}

void CYubiMixin::UpdateStatus()
{
  if (m_present)
    yubiInserted();
  else
    yubiRemoved();
}

bool CYubiMixin::PerformChallengeResponse(const StringX &challenge,
                                          StringX &response)
{
  bool retval = false;
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(m_prompt2);
  ::wxSafeYield(); // get text to update
  BYTE chalBuf[PWYubi::SHA1_MAX_BLOCK_SIZE];
  BYTE chalLength = BYTE(challenge.length()*sizeof(TCHAR));
  memset(chalBuf, 0, PWYubi::SHA1_MAX_BLOCK_SIZE);
  if (chalLength > PWYubi::SHA1_MAX_BLOCK_SIZE)
    chalLength = PWYubi::SHA1_MAX_BLOCK_SIZE;

  memcpy(chalBuf, challenge.c_str(), chalLength);

  PWYubi yubi;
  if (yubi.RequestHMacSHA1(chalBuf, chalLength)) {
    unsigned char hmac[PWYubi::RESPLEN];
    PWYubi::RequestStatus status = PWYubi::PENDING;
    do {
      status = yubi.GetResponse(hmac);
      if (status == PWYubi::PENDING)
        pws_os::sleep_ms(250); // Ugh.
      ::wxSafeYield(); // so as not to totally freeze the app...
    } while (status == PWYubi::PENDING);
    if (status == PWYubi::DONE) {
#if 0
      for (unsigned i = 0; i < sizeof(hmac); i++)
        std::cerr << std::hex << std::setw(2) << (int)hmac[i];
      std::cerr << std::endl;
#endif
      // The returned hash is the passkey
      response = Bin2Hex(hmac, PWYubi::RESPLEN);
      retval = true;
    } else {
      if (status == PWYubi::TIMEOUT) {
        m_status->SetForegroundColour(*wxRED);
        m_status->SetLabel(_("Timeout - please try again"));
      } else { // error
        m_status->SetForegroundColour(*wxRED);
        m_status->SetLabel(_("Error: Bad response from YubiKey"));
      }
    }
  } else {
    m_status->SetForegroundColour(*wxRED);
    m_status->SetLabel(_("Error: Unconfigured YubiKey?"));
  }
  return retval;
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

