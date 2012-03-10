/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
    if (flags != that.flags ||
        length != that.length ||
        ((flags & PWPolicy::UseDigits) == PWPolicy::UseDigits &&
                    digitminlength != that.digitminlength)  ||
        ((flags & PWPolicy::UseLowercase) == PWPolicy::UseLowercase &&
                    lowerminlength != that.lowerminlength)  ||
        ((flags & PWPolicy::UseSymbols) == PWPolicy::UseSymbols &&
                    symbolminlength != that.symbolminlength) ||
        ((flags & PWPolicy::UseUppercase) == PWPolicy::UseUppercase &&
                    upperminlength != that.upperminlength))
      return false;
  }
  return true;
}

// Following calls CPasswordCharPool::MakePassword()
// with arguments matching 'this' policy, or,
// preference-defined policy if this->flags == 0
StringX PWPolicy::MakeRandomPassword(const stringT &st_symbols) const
{
  bool pwuselowercase, pwuseuppercase;
  bool pwusedigits, pwusesymbols, pweasyvision, pwusehexdigits;
  bool pwmakepronounceable;
  int pwdefaultlength;
  int pwdigitminlength, pwlowerminlength, pwsymbolminlength, pwupperminlength;

  if (flags != 0) {
    pwuselowercase = (flags & PWPolicy::UseLowercase) == 
                            PWPolicy::UseLowercase;
    pwuseuppercase = (flags & PWPolicy::UseUppercase) == 
                            PWPolicy::UseUppercase;
    pwusedigits = (flags & PWPolicy::UseDigits) == 
                            PWPolicy::UseDigits;
    pwusesymbols = (flags & PWPolicy::UseSymbols) == 
                            PWPolicy::UseSymbols;
    pwusehexdigits = (flags & PWPolicy::UseHexDigits) == 
                            PWPolicy::UseHexDigits;
    pweasyvision = (flags & PWPolicy::UseEasyVision) == 
                            PWPolicy::UseEasyVision;
    pwmakepronounceable = (flags & PWPolicy::MakePronounceable) == 
                                 PWPolicy::MakePronounceable;
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

  // Sanity check:
  if ((numlowercase + numuppercase + numdigits + numsymbols == 0) &&
      !pwusehexdigits)
    return _T("");
 
  CPasswordCharPool pwchars(pwdefaultlength,
                            numlowercase, numuppercase, numdigits, numsymbols,
                            pwusehexdigits, pweasyvision, pwmakepronounceable,
                            st_symbols.c_str());

  return pwchars.MakePassword();
}

void PWPolicy::SetToDefaults()
{
  const PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs->GetPref(PWSprefs::PWUseLowercase))
    flags |= PWPolicy::UseLowercase;
  if (prefs->GetPref(PWSprefs::PWUseUppercase))
    flags |= PWPolicy::UseUppercase;
  if (prefs->GetPref(PWSprefs::PWUseDigits))
    flags |= PWPolicy::UseDigits;
  if (prefs->GetPref(PWSprefs::PWUseSymbols))
    flags |= PWPolicy::UseSymbols;
  if (prefs->GetPref(PWSprefs::PWUseHexDigits))
    flags |= PWPolicy::UseHexDigits;
  if (prefs->GetPref(PWSprefs::PWUseEasyVision))
    flags |= PWPolicy::UseEasyVision;
  if (prefs->GetPref(PWSprefs::PWMakePronounceable))
    flags |= PWPolicy::MakePronounceable;

  length = prefs->GetPref(PWSprefs::PWDefaultLength);
  digitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
  lowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
  symbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
  upperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);
}

void st_PSWDPolicy::SetToDefaults()
{
  pwp.SetToDefaults();
  symbols = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultSymbols);
}
