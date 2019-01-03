/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWPolicy.cpp
//-----------------------------------------------------------------------------

#include "PWPolicy.h"
#include "PWSprefs.h"
#include "PWCharPool.h"
#include "core.h"
#include "StringXStream.h"

#include "../os/typedefs.h"
#include "../os/debug.h"

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
  const int maxPWLength = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength);
  const int maxPWDigitLength = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDigitMinLength);
  const int maxPWLowercaseLength = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWLowercaseMinLength);
  const int maxPWUppercaseLength = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWUppercaseMinLength);
  const int maxPWSymbolLength = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWSymbolMinLength);

  if (str.empty() || str.length() != POL_STR_ENC_LEN) {
    if (!str.empty()) {
      ASSERT(str.length() == POL_STR_ENC_LEN);
      pws_os::Trace(_T("Malformed policy string: %ls\n"), str.c_str());
    }
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
  bool bhex_flag;
  bool bother_flags;
  int total_sublength;
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
  bhex_flag = (flags & PWPolicy::UseHexDigits) != 0;
  bother_flags = (flags & (~PWPolicy::UseHexDigits)) != 0;
  total_sublength = digitminlength + lowerminlength + symbolminlength + upperminlength;

  if (flags == 0 || (bhex_flag && bother_flags) ||
      length > maxPWLength || total_sublength > length ||
      digitminlength > maxPWDigitLength || lowerminlength > maxPWLowercaseLength ||
      symbolminlength > maxPWSymbolLength || upperminlength > maxPWUppercaseLength) {
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
    pol = PWSprefs::GetInstance()->GetDefaultPolicy();

  CPasswordCharPool pwchars(pol);
  return pwchars.MakePassword();
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

  stringT std_symbols = CPasswordCharPool::GetDefaultSymbols();
  stringT easyvision_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  stringT pronounceable_symbols = CPasswordCharPool::GetPronounceableSymbols();

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
    // Assume that when user selects either EasyVision or Pronounceable
    // Then default appropriate symbol set loaded for user to change
    if (bEV || bPR) {
      stringT sym;
      sym = symbols.empty() ? (bEV ? easyvision_symbols.c_str() : pronounceable_symbols.c_str()) : symbols.c_str();
      Format(col2, bEV ? IDSC_YESEASYVISON : IDSC_YESPRONOUNCEABLE, sym.c_str());
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

StringX PWPolicy::GetDisplayString()
{
  // Display string for policy in List View and Show entries' differences
  // when comparing entries in this or in different databases
  if (flags != 0) {
    stringT st_pwp(_T("")), st_text;
    if (flags & PWPolicy::UseLowercase) {
      st_pwp += _T("L");
      if (lowerminlength > 1) {
        Format(st_text, L"(%d)", lowerminlength);
        st_pwp += st_text;
      }
    }
    if (flags & PWPolicy::UseUppercase) {
      st_pwp += _T("U");
      if (upperminlength > 1) {
        Format(st_text, L"(%d)", upperminlength);
        st_pwp += st_text;
      }
    }
    if (flags & PWPolicy::UseDigits) {
      st_pwp += _T("D");
      if (digitminlength > 1) {
        Format(st_text, L"(%d)", digitminlength);
        st_pwp += st_text;
      }
    }
    if (flags & PWPolicy::UseSymbols) {
      st_pwp += _T("S");
        if (symbolminlength > 1) {
        Format(st_text, L"(%d)", symbolminlength);
          st_pwp += st_text;
      }
    }
    if (flags & PWPolicy::UseHexDigits)
      st_pwp += _T("H");
    if (flags & PWPolicy::UseEasyVision)
      st_pwp += _T("E");
    if (flags & PWPolicy::MakePronounceable)
      st_pwp += _T("P");
    oStringXStream osx;
    osx << st_pwp << _T(":") << length;
    return osx.str().c_str();
  }
  return _T("");
}
