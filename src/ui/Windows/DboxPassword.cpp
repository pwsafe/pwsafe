/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

  bool pwuselowercase, pwuseuppercase;
  bool pwusedigits, pwusesymbols, pweasyvision, pwusehexdigits;
  bool pwmakepronounceable;
  int pwdefaultlength;
  int pwdigitminlength, pwlowerminlength, pwsymbolminlength, pwupperminlength;

  if (pwp.flags != 0) {
    pwuselowercase = (pwp.flags & PWSprefs::PWPolicyUseLowercase) == 
                      PWSprefs::PWPolicyUseLowercase;
    pwuseuppercase = (pwp.flags & PWSprefs::PWPolicyUseUppercase) == 
                      PWSprefs::PWPolicyUseUppercase;
    pwusedigits = (pwp.flags & PWSprefs::PWPolicyUseDigits) == 
                   PWSprefs::PWPolicyUseDigits;
    pwusesymbols = (pwp.flags & PWSprefs::PWPolicyUseSymbols) == 
                    PWSprefs::PWPolicyUseSymbols;
    pwusehexdigits = (pwp.flags & PWSprefs::PWPolicyUseHexDigits) == 
                      PWSprefs::PWPolicyUseHexDigits;
    pweasyvision = (pwp.flags & PWSprefs::PWPolicyUseEasyVision) == 
                    PWSprefs::PWPolicyUseEasyVision;
    pwmakepronounceable = (pwp.flags & PWSprefs::PWPolicyMakePronounceable) == 
                           PWSprefs::PWPolicyMakePronounceable;
    pwdefaultlength = pwp.length;
    pwdigitminlength = pwp.digitminlength;
    pwlowerminlength = pwp.lowerminlength;
    pwsymbolminlength = pwp.symbolminlength;
    pwupperminlength = pwp.upperminlength;
  } else {
    pwuselowercase = prefs->GetPref(PWSprefs::PWUseLowercase);
    pwuseuppercase = prefs->GetPref(PWSprefs::PWUseUppercase);
    pwusedigits = prefs->GetPref(PWSprefs::PWUseDigits);
    pwusesymbols = prefs->GetPref(PWSprefs::PWUseSymbols);
    pwusehexdigits = prefs->GetPref(PWSprefs::PWUseHexDigits);
    pweasyvision = prefs->GetPref(PWSprefs::PWUseEasyVision);
    pwmakepronounceable = prefs->GetPref(PWSprefs::PWMakePronounceable);
    pwdefaultlength = prefs->GetPref(PWSprefs::PWDefaultLength);
    pwdigitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
    pwlowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
    pwsymbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
    pwupperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);
  }

  UINT numlowercase(0), numuppercase(0), numdigits(0), numsymbols(0);
  if (pwuselowercase)
    numlowercase = (pwlowerminlength == 0) ? 1 : pwlowerminlength;
  if (pwuseuppercase)
    numuppercase = (pwupperminlength == 0) ? 1 : pwupperminlength;
  if (pwusedigits)
    numdigits = (pwdigitminlength == 0) ? 1 : pwdigitminlength;
  if (pwusesymbols)
    numsymbols = (pwsymbolminlength == 0) ? 1 : pwsymbolminlength;
 
  CPasswordCharPool pwchars(pwdefaultlength,
                            numlowercase, numuppercase, numdigits, numsymbols,
                            pwusehexdigits == TRUE,
                            pweasyvision == TRUE,
                            pwmakepronounceable == TRUE);

  password = pwchars.MakePassword();

  SetClipboardData(password);
  UpdateLastClipboardAction(CItemData::PASSWORD);
}
