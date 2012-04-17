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
#include "core.h"

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

void PWPolicy::UpdateDefaults(bool bUseCopy) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  prefs->SetPref(PWSprefs::PWUseLowercase,
                 (flags & PWPolicy::UseLowercase) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWUseUppercase,
                 (flags & PWPolicy::UseUppercase) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWUseDigits,
                 (flags & PWPolicy::UseDigits) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWUseSymbols,
                 (flags & PWPolicy::UseSymbols) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWUseHexDigits,
                 (flags & PWPolicy::UseHexDigits) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWUseEasyVision,
                 (flags & PWPolicy::UseEasyVision) != 0, bUseCopy);
  prefs->SetPref(PWSprefs::PWMakePronounceable,
                 (flags & PWPolicy::MakePronounceable) != 0, bUseCopy);

  prefs->SetPref(PWSprefs::PWDefaultLength, length, bUseCopy);
  prefs->SetPref(PWSprefs::PWDigitMinLength, digitminlength, bUseCopy);
  prefs->SetPref(PWSprefs::PWLowercaseMinLength, lowerminlength, bUseCopy);
  prefs->SetPref(PWSprefs::PWSymbolMinLength, symbolminlength, bUseCopy);
  prefs->SetPref(PWSprefs::PWUppercaseMinLength, upperminlength, bUseCopy);
}


void st_PSWDPolicy::SetToDefaults()
{
  pwp.SetToDefaults();
  symbols = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultSymbols);
}

void st_PSWDPolicy::UpdateDefaults(bool bUseCopy) const
{
  pwp.UpdateDefaults(bUseCopy);
  PWSprefs::GetInstance()->SetPref(PWSprefs::DefaultSymbols,
                                   symbols, bUseCopy);
}

static stringT PolValueString(int flag, bool override, int count)
{
  // helper function for Policy2Table
  stringT yes, no;
  LoadAString(yes, IDSC_YES); LoadAString(no, IDSC_NO);

  stringT retval;
  if (flag != 0) {
    if (override)
      retval = yes;
    else {
      Format(retval, IDSC_YESNUMBER, count);
    }
  } else {
    retval = no;
  }
  return retval;
}

void st_PSWDPolicy::Policy2Table(st_PSWDPolicy::RowPutter rp, void *table)
{
  stringT yes, no;
  LoadAString(yes, IDSC_YES); LoadAString(no, IDSC_NO);

  stringT std_symbols, easyvision_symbols, pronounceable_symbols;
  CPasswordCharPool::GetDefaultSymbols(std_symbols);
  CPasswordCharPool::GetEasyVisionSymbols(easyvision_symbols);
  CPasswordCharPool::GetPronounceableSymbols(pronounceable_symbols);

  const bool bEV = (pwp.flags & PWPolicy::UseEasyVision) != 0;
  const bool bPR = (pwp.flags & PWPolicy::MakePronounceable) != 0;

  int nPos = 0;

// Length, Lowercase, Uppercase, Digits, Symbols, EasyVision, Pronounceable, Hexadecimal
  stringT col1, col2;

  LoadAString(col1, IDSC_PLENGTH);
  Format(col2, L"%d", pwp.length);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSELOWER);
  col2 = PolValueString((pwp.flags & PWPolicy::UseLowercase), bEV || bPR, pwp.lowerminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSEUPPER);
  col2 = PolValueString((pwp.flags & PWPolicy::UseUppercase), bEV || bPR, pwp.upperminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSEDIGITS);
  col2 = PolValueString((pwp.flags & PWPolicy::UseDigits), bEV || bPR, pwp.digitminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSESYMBOL);
  if ((pwp.flags & PWPolicy::UseSymbols) != 0) {
    if (bEV || bPR) {
      Format(col2, bEV ? IDSC_YESEASYVISON : IDSC_YESPRONOUNCEABLE,
             bEV ? easyvision_symbols.c_str() : pronounceable_symbols.c_str());
    } else {
      stringT tmp, sym;
      LoadAString(tmp, symbols.empty() ? IDSC_DEFAULTSYMBOLS : IDSC_SPECFICSYMBOLS);
      sym = symbols.empty() ? std_symbols.c_str() : symbols.c_str();
      Format(col2, IDSC_YESSYMBOLS, pwp.symbolminlength, tmp.c_str(), sym.c_str());
    }
  } else {
    col2 = no;
  }
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PEASYVISION);
  rp(nPos, col1, bEV ? yes : no, table);
  nPos++;

  LoadAString(col1, IDSC_PPRONOUNCEABLE);
  rp(nPos, col1, bPR ? yes : no, table);
  nPos++;

  LoadAString(col1, IDSC_PHEXADECIMAL);
  rp(nPos, col1, (pwp.flags & PWPolicy::UseHexDigits) != 0 ? yes : no, table);
  nPos++;
}

