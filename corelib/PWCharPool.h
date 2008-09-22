/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWCharPool.h
//-----------------------------------------------------------------------------

#ifndef PWCharPool_h
#define PWCharPool_h

#include "os/typedefs.h"
#include "StringX.h"

/*
 * This class is used to create a random password based on the policy
 * defined in the constructor.
 * The policy consists of the following attributes:
 * - The length of the password to be generated
 * - Which type of characters to use from the following: lowercase, uppercase,
 *   digits, symbols
 * - Whether or not to use only characters that are easily distinguishable
 *   (i.e., no '1', 'l', 'I', etc.)
 * The class ensures that if a character type is selected, then at least one
 * character from that type will be in the generated password. (i.e., at least
 * one digit if usedigits is set in the constructor).
 *
 * The usage scenario is something like:
 * CPasswordCharPool pwgen(-policy-);
 * StringX pwd = pwgen.MakePassword();
 *
 * CheckPassword() is used to verify the strength of existing passwords,
 * i.e., the password used to protect the database.
 */

class CPasswordCharPool
{
public:
  CPasswordCharPool(uint pwlen,
    uint numlowercase, uint numuppercase,
    uint numdigits, uint numsymbols, bool usehexdigits,
    bool easyvision, bool pronounceable);
  StringX MakePassword() const;

  static bool CheckPassword(const StringX &pwd, StringX &error);

private:
  enum CharType {LOWERCASE = 0, UPPERCASE = 1,
    DIGIT = 2, SYMBOL = 3, HEXDIGIT = 4, NUMTYPES = 5};
  CharType GetRandomCharType(unsigned int rand) const; // select a chartype with weighted probability
  charT GetRandomChar(CharType t, unsigned int rand) const;
  StringX MakePronounceable() const;

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
  charT *m_char_arrays[NUMTYPES];

  size_t m_sumlengths; // sum of all selected chartypes

  // Following state vars set by ctor, used by MakePassword()
  const uint m_pwlen;
  const uint m_numlowercase;
  const uint m_numuppercase;
  const uint m_numdigits;
  const uint m_numsymbols;
  const bool m_uselowercase;
  const bool m_useuppercase;
  const bool m_usedigits;
  const bool m_usesymbols;
  const bool m_usehexdigits;
  const bool m_pronounceable;

  CPasswordCharPool &operator=(const CPasswordCharPool &);
};

#endif
