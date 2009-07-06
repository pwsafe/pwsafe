/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWPolicy.cpp
//-----------------------------------------------------------------------------

#include "PWPolicy.h"
#include "../os/typedefs.h"
#include "PWSprefs.h"
#include "PWCharPool.h"

bool PWPolicy::operator==(const PWPolicy &that) const
{
  if (this != &that) {
    if (flags           != that.flags ||
        length          != that.length ||
        ((flags & PWSprefs::PWPolicyUseDigits) == PWSprefs::PWPolicyUseDigits &&
         digitminlength  != that.digitminlength) ||
        ((flags & PWSprefs::PWPolicyUseLowercase) == PWSprefs::PWPolicyUseLowercase &&
         lowerminlength  != that.lowerminlength) ||
        ((flags & PWSprefs::PWPolicyUseSymbols) == PWSprefs::PWPolicyUseSymbols &&
         symbolminlength != that.symbolminlength) ||
        ((flags & PWSprefs::PWPolicyUseUppercase) == PWSprefs::PWPolicyUseUppercase &&
         upperminlength  != that.upperminlength))
      return false;
  }
  return true;
}

// Following calls CPasswordCharPool::MakePassword()
// with arguments matching 'this' policy, or,
// preference-defined policy if this->flags == 0
StringX PWPolicy::MakeRandomPassword() const
{
  bool pwuselowercase, pwuseuppercase;
  bool pwusedigits, pwusesymbols, pweasyvision, pwusehexdigits;
  bool pwmakepronounceable;
  int pwdefaultlength;
  int pwdigitminlength, pwlowerminlength, pwsymbolminlength, pwupperminlength;

  if (flags != 0) {
    pwuselowercase = (flags & PWSprefs::PWPolicyUseLowercase) == 
                      PWSprefs::PWPolicyUseLowercase;
    pwuseuppercase = (flags & PWSprefs::PWPolicyUseUppercase) == 
                      PWSprefs::PWPolicyUseUppercase;
    pwusedigits = (flags & PWSprefs::PWPolicyUseDigits) == 
                   PWSprefs::PWPolicyUseDigits;
    pwusesymbols = (flags & PWSprefs::PWPolicyUseSymbols) == 
                    PWSprefs::PWPolicyUseSymbols;
    pwusehexdigits = (flags & PWSprefs::PWPolicyUseHexDigits) == 
                      PWSprefs::PWPolicyUseHexDigits;
    pweasyvision = (flags & PWSprefs::PWPolicyUseEasyVision) == 
                    PWSprefs::PWPolicyUseEasyVision;
    pwmakepronounceable = (flags & PWSprefs::PWPolicyMakePronounceable) == 
                           PWSprefs::PWPolicyMakePronounceable;
    pwdefaultlength = length;
    pwdigitminlength = digitminlength;
    pwlowerminlength = lowerminlength;
    pwsymbolminlength = symbolminlength;
    pwupperminlength = upperminlength;
  } else {
    PWSprefs *prefs = PWSprefs::GetInstance();
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

  unsigned int numlowercase(0), numuppercase(0), numdigits(0), numsymbols(0);
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
                            pwusehexdigits, pweasyvision, pwmakepronounceable);

  return pwchars.MakePassword();
}
