/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "resource.h"
#include "resource3.h"  // String resources
#include "OptionsPasswordPolicy.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"

#include "DboxMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-----------------------------------------------------------------------------

// generate a random password, using the default settings or user overrides
// the generated password will be copied to the clipboard. Doing
// this leaves a problem where the user can generate a random password, have
// the password copied to the clipboard and then change the password. This could
// be avoided by putting the password into the clipboard when the entry is saved
// but that would be annoying when generating a new entry.

bool
DboxMain::MakeRandomPassword(CDialog * const pDialog, CMyString& password,
                             DWORD &dw_policy)
{
  bool is_override = (pDialog->IsDlgButtonChecked(IDC_OVERRIDE_POLICY) == BST_CHECKED);

  PWSprefs *prefs = PWSprefs::GetInstance();

  COptionsPasswordPolicy passwordpolicy;

  if (dw_policy != 0) {
    passwordpolicy.m_pwdefaultlength = (dw_policy & PWSprefs::PWPolicyMaxLength) + 1;
    passwordpolicy.m_pwuselowercase = 
      (dw_policy & PWSprefs::PWPolicyUseLowercase) ? TRUE : FALSE;
    passwordpolicy.m_pwuseuppercase = 
      (dw_policy & PWSprefs::PWPolicyUseUppercase) ? TRUE : FALSE;
    passwordpolicy.m_pwusedigits = 
      (dw_policy & PWSprefs::PWPolicyUseDigits) ? TRUE : FALSE;
    passwordpolicy.m_pwusesymbols = 
      (dw_policy & PWSprefs::PWPolicyUseSymbols) ? TRUE : FALSE;
    passwordpolicy.m_pwusehexdigits = 
      (dw_policy & PWSprefs::PWPolicyUseHexDigits) ? TRUE : FALSE;
    passwordpolicy.m_pweasyvision = 
      (dw_policy & PWSprefs::PWPolicyUseEasyVision) ? TRUE : FALSE;
    passwordpolicy.m_pwmakepronounceable = 
      (dw_policy & PWSprefs::PWPolicyMakePronounceable) ? TRUE : FALSE;
  } else {
    passwordpolicy.m_pwdefaultlength = prefs->
      GetPref(PWSprefs::PWDefaultLength);
    passwordpolicy.m_pwuselowercase = prefs->
      GetPref(PWSprefs::PWUseLowercase);
    passwordpolicy.m_pwuseuppercase = prefs->
      GetPref(PWSprefs::PWUseUppercase);
    passwordpolicy.m_pwusedigits = prefs->
      GetPref(PWSprefs::PWUseDigits);
    passwordpolicy.m_pwusesymbols = prefs->
      GetPref(PWSprefs::PWUseSymbols);
    passwordpolicy.m_pwusehexdigits = prefs->
      GetPref(PWSprefs::PWUseHexDigits);
    passwordpolicy.m_pweasyvision = prefs->
      GetPref(PWSprefs::PWUseEasyVision);
    passwordpolicy.m_pwmakepronounceable = prefs->
      GetPref(PWSprefs::PWMakePronounceable);
  }

  if (is_override) {
    // Start with existing password policy
    CPropertySheet optionsDlg(IDS_PASSWORDOVERRIDE, pDialog);

    // Display COptionsPasswordPolicy page
    optionsDlg.AddPage(&passwordpolicy);
    optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW; // remove "Apply Now" button

    // If the user cancels the dialog, the values will be left
    // at the default values, so the password will be generated
    // properly. That means we can actually ignore the return value here.
    (void)optionsDlg.DoModal();
  }

  CPasswordCharPool pwchars(
                            passwordpolicy.m_pwdefaultlength,
                            passwordpolicy.m_pwuselowercase,
                            passwordpolicy.m_pwuseuppercase,
                            passwordpolicy.m_pwusedigits,
                            passwordpolicy.m_pwusesymbols,
                            passwordpolicy.m_pwusehexdigits,
                            passwordpolicy.m_pweasyvision,
                            passwordpolicy.m_pwmakepronounceable);

  password = pwchars.MakePassword();

  SetClipboardData( password );
  return true;
}

bool
DboxMain::SetPasswordPolicy(DWORD &dw_policy)
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  COptionsPasswordPolicy passwordpolicy;

  if (dw_policy != 0) {
    passwordpolicy.m_pwdefaultlength = (dw_policy & PWSprefs::PWPolicyMaxLength) + 1;
    passwordpolicy.m_pwuselowercase = 
      (dw_policy & PWSprefs::PWPolicyUseLowercase) ? TRUE : FALSE;
    passwordpolicy.m_pwuseuppercase = 
      (dw_policy & PWSprefs::PWPolicyUseUppercase) ? TRUE : FALSE;
    passwordpolicy.m_pwusedigits = 
      (dw_policy & PWSprefs::PWPolicyUseDigits) ? TRUE : FALSE;
    passwordpolicy.m_pwusesymbols = 
      (dw_policy & PWSprefs::PWPolicyUseSymbols) ? TRUE : FALSE;
    passwordpolicy.m_pwusehexdigits = 
      (dw_policy & PWSprefs::PWPolicyUseHexDigits) ? TRUE : FALSE;
    passwordpolicy.m_pweasyvision = 
      (dw_policy & PWSprefs::PWPolicyUseEasyVision) ? TRUE : FALSE;
    passwordpolicy.m_pwmakepronounceable = 
      (dw_policy & PWSprefs::PWPolicyMakePronounceable) ? TRUE : FALSE;
  } else {
    passwordpolicy.m_pwdefaultlength = prefs->
      GetPref(PWSprefs::PWDefaultLength);
    passwordpolicy.m_pwuselowercase = prefs->
      GetPref(PWSprefs::PWUseLowercase);
    passwordpolicy.m_pwuseuppercase = prefs->
      GetPref(PWSprefs::PWUseUppercase);
    passwordpolicy.m_pwusedigits = prefs->
      GetPref(PWSprefs::PWUseDigits);
    passwordpolicy.m_pwusesymbols = prefs->
      GetPref(PWSprefs::PWUseSymbols);
    passwordpolicy.m_pwusehexdigits = prefs->
      GetPref(PWSprefs::PWUseHexDigits);
    passwordpolicy.m_pweasyvision = prefs->
      GetPref(PWSprefs::PWUseEasyVision);
    passwordpolicy.m_pwmakepronounceable = prefs->
      GetPref(PWSprefs::PWMakePronounceable);
  }

  // Start with existing password policy
  CPropertySheet optionsDlg(IDS_SETPASSWORDPOLICY);

  // Display COptionsPasswordPolicy page
  optionsDlg.AddPage(&passwordpolicy);
  optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW; // remove "Apply Now" button

  INT_PTR rc = optionsDlg.DoModal();
  if (rc == IDCANCEL)
    return false;

  dw_policy = 0;
  if (passwordpolicy.m_pwuselowercase)
    dw_policy |= PWSprefs::PWPolicyUseLowercase;
  if (passwordpolicy.m_pwuseuppercase)
    dw_policy |= PWSprefs::PWPolicyUseUppercase;
  if (passwordpolicy.m_pwusedigits)
    dw_policy |= PWSprefs::PWPolicyUseDigits;
  if (passwordpolicy.m_pwusesymbols)
    dw_policy |= PWSprefs::PWPolicyUseSymbols;
  if (passwordpolicy.m_pwusehexdigits)
    dw_policy |= PWSprefs::PWPolicyUseHexDigits;
  if (passwordpolicy.m_pweasyvision)
    dw_policy |= PWSprefs::PWPolicyUseEasyVision;
  if (passwordpolicy.m_pwmakepronounceable)
    dw_policy |= PWSprefs::PWPolicyMakePronounceable;
  dw_policy |= ((passwordpolicy.m_pwdefaultlength - 1) & PWSprefs::PWPolicyMaxLength);

  return true;
}
