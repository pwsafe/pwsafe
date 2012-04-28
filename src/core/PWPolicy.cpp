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
#include "StringXStream.h"
#include <iomanip>
/**
 * A policy is encoded as string (for persistence) as follows:
 * We need flags(4), length(3), lower_len(3), upper_len(3)
 *   digit_len(3), symbol_len(3) = 4 + 5 * 3 = 19
 * All fields are hexadecimal digits representing flags or lengths

 * Note: order of fields set by PWSprefs enum that can have minimum lengths.
 * Later releases must support these as a minimum.  Any fields added
 * by these releases will be lost if the user changes these field.
 */
#define POL_STR_ENC_LEN 19

PWPolicy::PWPolicy(const StringX &str) : usecount(0)
{
  ASSERT(str.empty() || str.length() == POL_STR_ENC_LEN);
  if (str.length() != POL_STR_ENC_LEN) {
    PWPolicy emptyPol;
    *this = emptyPol;
    return;
  }

  // String !empty and of right length: Get fields
  const stringT cs_pwp(str.c_str());
  istringstreamT is_flags(stringT(cs_pwp, 0, 4));
  istringstreamT is_length(stringT(cs_pwp, 4, 3));
  istringstreamT is_digitminlength(stringT(cs_pwp, 7, 3));
  istringstreamT is_lowreminlength(stringT(cs_pwp, 10, 3));
  istringstreamT is_symbolminlength(stringT(cs_pwp, 13, 3));
  istringstreamT is_upperminlength(stringT(cs_pwp, 16, 3));

  // Put them into PWPolicy structure
  unsigned int f; // dain bramaged istringstream requires this runaround
  if (!(is_flags >> std::hex >> f)) goto fail;
  flags = static_cast<unsigned short>(f);
  if (!(is_length >> std::hex >> length)) goto fail;
  if (!(is_digitminlength >> std::hex >> digitminlength)) goto fail;
  if (!(is_lowreminlength >> std::hex >> lowerminlength)) goto fail;
  if (!(is_symbolminlength >> std::hex >> symbolminlength)) goto fail;
  if (!(is_upperminlength >> std::hex >> upperminlength)) goto fail;

  // Sanity checks:
  // Must be some flags; however hex incompatible with other flags
  // lengths also have restrictions.
  bool bhex_flag = (flags & PWPolicy::UseHexDigits) != 0;
  bool bother_flags = (flags & (~PWPolicy::UseHexDigits)) != 0;
  const int total_sublength = digitminlength + lowerminlength +
    symbolminlength + upperminlength;

  if (flags == 0 || (bhex_flag && bother_flags) ||
      length > 1024 || total_sublength > length ||
      digitminlength > 1024 || lowerminlength > 1024 ||
      symbolminlength > 1024 || upperminlength > 1024) {
    goto fail;
  }
  return;

 fail:
  PWPolicy failPol; // empty
  *this = failPol;
}

PWPolicy::operator StringX() const
{
  StringX retval;
  if (flags == 0) {
    return retval;
  }
  ostringstreamT os;
  unsigned int f; // dain bramaged istringstream requires this runaround
  f = static_cast<unsigned int>(flags);
  os.fill(charT('0'));
  os << std::hex << std::setw(4) << f
     << std::setw(3) << length
     << std::setw(3) << digitminlength
     << std::setw(3) << lowerminlength
     << std::setw(3) << symbolminlength
     << std::setw(3) << upperminlength;
  retval = os.str().c_str();
  ASSERT(retval.length() == POL_STR_ENC_LEN);
  return retval;
}

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
                    upperminlength != that.upperminlength) ||
        symbols != that.symbols)
      return false;
  }
  return true;
}

// Following calls CPasswordCharPool::MakePassword()
// with arguments matching 'this' policy, or,
// preference-defined policy if this->flags == 0
StringX PWPolicy::MakeRandomPassword() const
{
  PWPolicy pol(*this); // small price to keep constness
  if (flags == 0)
    pol.SetToDefaults();
  pol.Normalize();

  CPasswordCharPool pwchars(pol);
  return pwchars.MakePassword();
}

inline void NormalizeField(int bitset, int &value)
{
  if (bitset != 0) {
    if (value == 0)
      value = 1;
  } else
    value = 0;
}

void PWPolicy::Normalize()
{
  /**
   * Protect agains inconsistent policy:
   * Make sure that if a Use* bit is set, the corresponding
   * min length is > 0.
   */
  NormalizeField((flags & PWPolicy::UseLowercase), lowerminlength);
  NormalizeField((flags & PWPolicy::UseUppercase), upperminlength);
  NormalizeField((flags & PWPolicy::UseDigits), digitminlength);
  NormalizeField((flags & PWPolicy::UseSymbols), symbolminlength);
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
  symbols = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultSymbols);
  Normalize();
}

void PWPolicy::UpdateDefaults(bool bUseCopy) const
{
  const_cast<PWPolicy *>(this)->Normalize(); // alternate: use a local copy
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

void PWPolicy::Policy2Table(PWPolicy::RowPutter rp, void *table)
{
  stringT yes, no;
  LoadAString(yes, IDSC_YES); LoadAString(no, IDSC_NO);

  stringT std_symbols, easyvision_symbols, pronounceable_symbols;
  CPasswordCharPool::GetDefaultSymbols(std_symbols);
  CPasswordCharPool::GetEasyVisionSymbols(easyvision_symbols);
  CPasswordCharPool::GetPronounceableSymbols(pronounceable_symbols);

  const bool bEV = (flags & PWPolicy::UseEasyVision) != 0;
  const bool bPR = (flags & PWPolicy::MakePronounceable) != 0;

  int nPos = 0;

// Length, Lowercase, Uppercase, Digits, Symbols, EasyVision, Pronounceable, Hexadecimal
  stringT col1, col2;

  LoadAString(col1, IDSC_PLENGTH);
  Format(col2, L"%d", length);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSELOWER);
  col2 = PolValueString((flags & PWPolicy::UseLowercase), bEV || bPR, lowerminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSEUPPER);
  col2 = PolValueString((flags & PWPolicy::UseUppercase), bEV || bPR, upperminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSEDIGITS);
  col2 = PolValueString((flags & PWPolicy::UseDigits), bEV || bPR, digitminlength);
  rp(nPos, col1, col2, table);
  nPos++;

  LoadAString(col1, IDSC_PUSESYMBOL);
  if ((flags & PWPolicy::UseSymbols) != 0) {
    if (bEV || bPR) {
      Format(col2, bEV ? IDSC_YESEASYVISON : IDSC_YESPRONOUNCEABLE,
             bEV ? easyvision_symbols.c_str() : pronounceable_symbols.c_str());
    } else {
      stringT tmp, sym;
      LoadAString(tmp, symbols.empty() ? IDSC_DEFAULTSYMBOLS : IDSC_SPECFICSYMBOLS);
      sym = symbols.empty() ? std_symbols.c_str() : symbols.c_str();
      Format(col2, IDSC_YESSYMBOLS, symbolminlength, tmp.c_str(), sym.c_str());
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
  rp(nPos, col1, (flags & PWPolicy::UseHexDigits) != 0 ? yes : no, table);
  nPos++;
}

