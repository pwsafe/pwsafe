/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWCharPool.h
//-----------------------------------------------------------------------------

#ifndef __PWCHARPOOL_H
#define __PWCHARPOOL_H

#include "os/typedefs.h"
#include "StringX.h"
#include "PWPolicy.h"

#include <algorithm>

/*
 * This class is used to create a random password based on the policy
 * defined in the constructor.
 * The class ensures that if a character type is selected, then at least one
 * character from that type will be in the generated password. (i.e., at least
 * one digit if usedigits is set in the constructor).
 *
 * The usage scenario is something like:
 * CPasswordCharPool pwgen(policy);
 * StringX pwd = pwgen.MakePassword();
 *
 * CheckPassword() is used to verify the strength of existing passwords,
 * i.e., the password used to protect the database.
 */

class CPasswordCharPool
{
public:
  CPasswordCharPool(const PWPolicy &policy);
  StringX MakePassword() const;

  ~CPasswordCharPool();

  static bool CheckPassword(const StringX &pwd, StringX &error);
  static stringT GetDefaultSymbols();
  static stringT GetEasyVisionSymbols() {return easyvision_symbol_chars;}
  static stringT GetPronounceableSymbols() {return pronounceable_symbol_chars;}
  static void ResetDefaultSymbols(); // reset the preference string

private:
  enum CharType {LOWERCASE = 0, UPPERCASE = 1,
                 DIGIT = 2, SYMBOL = 3, HEXDIGIT = 4, NUMTYPES = 5};
  // select a chartype with weighted probability
  CharType GetRandomCharType(unsigned int rand) const;
  charT GetRandomChar(CharType t, unsigned int rand) const;
  charT GetRandomChar(CharType t) const;
  StringX MakePronounceable() const;
  StringX MakeHex() const;

  // here are all the character types, in both full and "easyvision" versions
  static const charT std_lowercase_chars[];
  static const charT std_uppercase_chars[];
  static const charT std_digit_chars[];
  static const charT std_symbol_chars[];
  static const charT std_hexdigit_chars[];
  static const charT easyvision_lowercase_chars[];
  static const charT easyvision_uppercase_chars[];
  static const charT easyvision_digit_chars[];
  static const charT easyvision_symbol_chars[];
  static const charT easyvision_hexdigit_chars[];
  static const charT pronounceable_symbol_chars[];
  // and here are the lengths of the above arrays
  static const size_t std_lowercase_len;
  static const size_t std_uppercase_len;
  static const size_t std_digit_len;
  static const size_t std_symbol_len;
  static const size_t std_hexdigit_len;
  static const size_t easyvision_lowercase_len;
  static const size_t easyvision_uppercase_len;
  static const size_t easyvision_digit_len;
  static const size_t easyvision_symbol_len;
  static const size_t easyvision_hexdigit_len;

  // The following arrays are set by the constructor based on the policy
  // These determine the probability of a CharType being chosen
  // in GetRandomCharType.
  size_t m_lengths[NUMTYPES];
  size_t m_x[NUMTYPES+1]; // spread lengths along X axis
  const charT *m_char_arrays[NUMTYPES];

  size_t m_sumlengths; // sum of all selected chartypes

  // Following state vars set by ctor, used by MakePassword()
  const uint m_pwlen;
  uint m_numlowercase;
  uint m_numuppercase;
  uint m_numdigits;
  uint m_numsymbols;
  const bool m_uselowercase;
  const bool m_useuppercase;
  const bool m_usedigits;
  const bool m_usesymbols;
  const bool m_usehexdigits;
  const bool m_pronounceable;

  bool m_bDefaultSymbols;

  // helper struct for MakePassword
  struct typeFreq_s {
    uint numchars;
    StringX vchars;
    typeFreq_s(const CPasswordCharPool *parent, CharType ct, uint nc);
  };

  CPasswordCharPool &operator=(const CPasswordCharPool &);
};

#endif /*  __PWCHARPOOL_H */
