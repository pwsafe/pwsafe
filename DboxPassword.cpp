/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "corelib/itemdata.h"

#include "DboxMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-----------------------------------------------------------------------------

// Generate a random password.
// The generated password will be copied to the clipboard. Doing
// this leaves a problem where the user can generate a random password, have
// the password copied to the clipboard and then change the password. This could
// be avoided by putting the password into the clipboard when the entry is saved
// but that would be annoying when generating a new entry.

void DboxMain::MakeRandomPassword(StringX &password, PWPolicy &pwp)
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  COptionsPasswordPolicy passwordpolicy;

  if (pwp.flags != 0) {
    passwordpolicy.m_pwuselowercase = 
      (pwp.flags & PWSprefs::PWPolicyUseLowercase) ? TRUE : FALSE;
    passwordpolicy.m_pwuseuppercase = 
      (pwp.flags & PWSprefs::PWPolicyUseUppercase) ? TRUE : FALSE;
    passwordpolicy.m_pwusedigits = 
      (pwp.flags & PWSprefs::PWPolicyUseDigits) ? TRUE : FALSE;
    passwordpolicy.m_pwusesymbols = 
      (pwp.flags & PWSprefs::PWPolicyUseSymbols) ? TRUE : FALSE;
    passwordpolicy.m_pwusehexdigits = 
      (pwp.flags & PWSprefs::PWPolicyUseHexDigits) ? TRUE : FALSE;
    passwordpolicy.m_pweasyvision = 
      (pwp.flags & PWSprefs::PWPolicyUseEasyVision) ? TRUE : FALSE;
    passwordpolicy.m_pwmakepronounceable = 
      (pwp.flags & PWSprefs::PWPolicyMakePronounceable) ? TRUE : FALSE;
    passwordpolicy.m_pwdefaultlength = pwp.length;
    passwordpolicy.m_pwdigitminlength = pwp.digitminlength;
    passwordpolicy.m_pwlowerminlength = pwp.lowerminlength;
    passwordpolicy.m_pwsymbolminlength = pwp.symbolminlength;
    passwordpolicy.m_pwupperminlength = pwp.upperminlength;
  } else {
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
    passwordpolicy.m_pwdefaultlength = prefs->
      GetPref(PWSprefs::PWDefaultLength);
    passwordpolicy.m_pwdigitminlength = prefs->
      GetPref(PWSprefs::PWDigitMinLength);
    passwordpolicy.m_pwlowerminlength = prefs->
      GetPref(PWSprefs::PWLowercaseMinLength);
    passwordpolicy.m_pwsymbolminlength = prefs->
      GetPref(PWSprefs::PWSymbolMinLength);
    passwordpolicy.m_pwupperminlength = prefs->
      GetPref(PWSprefs::PWUppercaseMinLength);
  }


  UINT numlowercase(0), numuppercase(0), numdigits(0), numsymbols(0);
  if (passwordpolicy.m_pwuselowercase)
    numlowercase = (passwordpolicy.m_pwlowerminlength == 0) ? 1 : passwordpolicy.m_pwlowerminlength;
  if (passwordpolicy.m_pwuseuppercase)
    numuppercase = (passwordpolicy.m_pwupperminlength == 0) ? 1 : passwordpolicy.m_pwupperminlength;
  if (passwordpolicy.m_pwusedigits)
    numdigits = (passwordpolicy.m_pwdigitminlength == 0) ? 1 : passwordpolicy.m_pwdigitminlength;
  if (passwordpolicy.m_pwusesymbols)
    numsymbols = (passwordpolicy.m_pwsymbolminlength == 0) ? 1 : passwordpolicy.m_pwsymbolminlength;  

  CPasswordCharPool pwchars(passwordpolicy.m_pwdefaultlength,
                            numlowercase, numuppercase, numdigits, numsymbols,
                            passwordpolicy.m_pwusehexdigits == TRUE,
                            passwordpolicy.m_pweasyvision == TRUE,
                            passwordpolicy.m_pwmakepronounceable == TRUE);

  password = pwchars.MakePassword();

  SetClipboardData(password);
}

bool DboxMain::SetPasswordPolicy(PWPolicy &pwp)
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  COptionsPasswordPolicy passwordpolicy;

  if (pwp.flags != 0) {
    passwordpolicy.m_pwuselowercase = 
      (pwp.flags & PWSprefs::PWPolicyUseLowercase) ? TRUE : FALSE;
    passwordpolicy.m_pwuseuppercase = 
      (pwp.flags & PWSprefs::PWPolicyUseUppercase) ? TRUE : FALSE;
    passwordpolicy.m_pwusedigits = 
      (pwp.flags & PWSprefs::PWPolicyUseDigits) ? TRUE : FALSE;
    passwordpolicy.m_pwusesymbols = 
      (pwp.flags & PWSprefs::PWPolicyUseSymbols) ? TRUE : FALSE;
    passwordpolicy.m_pwusehexdigits = 
      (pwp.flags & PWSprefs::PWPolicyUseHexDigits) ? TRUE : FALSE;
    passwordpolicy.m_pweasyvision = 
      (pwp.flags & PWSprefs::PWPolicyUseEasyVision) ? TRUE : FALSE;
    passwordpolicy.m_pwmakepronounceable = 
      (pwp.flags & PWSprefs::PWPolicyMakePronounceable) ? TRUE : FALSE;
    passwordpolicy.m_pwdefaultlength = pwp.length;
    passwordpolicy.m_pwdigitminlength = pwp.digitminlength;
    passwordpolicy.m_pwlowerminlength = pwp.lowerminlength;
    passwordpolicy.m_pwsymbolminlength = pwp.symbolminlength;
    passwordpolicy.m_pwupperminlength = pwp.upperminlength;
  } else {
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
    passwordpolicy.m_pwdefaultlength = prefs->
      GetPref(PWSprefs::PWDefaultLength);
    passwordpolicy.m_pwdigitminlength = prefs->
      GetPref(PWSprefs::PWDigitMinLength);
    passwordpolicy.m_pwlowerminlength = prefs->
      GetPref(PWSprefs::PWLowercaseMinLength);
    passwordpolicy.m_pwsymbolminlength = prefs->
      GetPref(PWSprefs::PWSymbolMinLength);
    passwordpolicy.m_pwupperminlength = prefs->
      GetPref(PWSprefs::PWUppercaseMinLength);
  }

  // Start with existing password policy
  CPropertySheet optionsDlg(IDS_SETPASSWORDPOLICY);

  // Display COptionsPasswordPolicy page
  optionsDlg.AddPage(&passwordpolicy);
  optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW; // remove "Apply Now" button

  INT_PTR rc = optionsDlg.DoModal();
  if (rc == IDCANCEL)
    return false;

  pwp.Empty();
  if (passwordpolicy.m_pwuselowercase)
    pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (passwordpolicy.m_pwuseuppercase)
    pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (passwordpolicy.m_pwusedigits)
    pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (passwordpolicy.m_pwusesymbols)
    pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (passwordpolicy.m_pwusehexdigits)
    pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (passwordpolicy.m_pweasyvision)
    pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (passwordpolicy.m_pwmakepronounceable)
    pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
  pwp.length = passwordpolicy.m_pwdefaultlength;
  pwp.digitminlength = passwordpolicy.m_pwdigitminlength;
  pwp.lowerminlength = passwordpolicy.m_pwlowerminlength;
  pwp.symbolminlength = passwordpolicy.m_pwsymbolminlength;
  pwp.upperminlength = passwordpolicy.m_pwupperminlength;
  return true;
}
