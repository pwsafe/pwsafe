/// \file PWCharPool.h
//-----------------------------------------------------------------------------

#ifndef PWCharPool_h
#define PWCharPool_h

#include "MyString.h"

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
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
 * CMyString pwd = pwgen.MakePassword();
 *
 * CheckPassword() is used to verify the strength of existing passwords, i.e., the password
 * used to protect the database.
 */

class CPasswordCharPool
{
public:
   CPasswordCharPool::CPasswordCharPool(UINT pwlen,
					BOOL uselowercase, BOOL useuppercase,
					BOOL usedigits, BOOL usesymbols,
					BOOL easyvision);
   CMyString MakePassword() const;

   static bool CheckPassword(const CMyString &pwd, CMyString &error);

private:
   enum CharType {LOWERCASE = 0, UPPERCASE = 1,
		  DIGIT = 2, SYMBOL = 3, NUMTYPES = 4};
   CharType GetRandomCharType(size_t rand) const; // select a chartype with weighted probability
   TCHAR GetRandomChar(CharType t, size_t rand) const;

   // here are all the character types, in both full and "easyvision" versions
   static const TCHAR std_lowercase_chars[];
   static const TCHAR std_uppercase_chars[];
   static const TCHAR std_digit_chars[];
   static const TCHAR std_symbol_chars[];
   static const TCHAR easyvision_lowercase_chars[];
   static const TCHAR easyvision_uppercase_chars[];
   static const TCHAR easyvision_digit_chars[];
   static const TCHAR easyvision_symbol_chars[];
   // and here are the lengths of the above arrays
   static const size_t std_lowercase_len;
   static const size_t std_uppercase_len;
   static const size_t std_digit_len;
   static const size_t std_symbol_len;
   static const size_t easyvision_lowercase_len;
   static const size_t easyvision_uppercase_len;
   static const size_t easyvision_digit_len;
   static const size_t easyvision_symbol_len;

   // The following arrays are set by the constructor based on the policy
   // These determine the probability of a CharType being chosen
   // in GetRandomCharType.
   size_t m_lengths[NUMTYPES];
   size_t m_x[NUMTYPES+1]; // spread lengths along X axis
   TCHAR *m_char_arrays[NUMTYPES];

   int m_sumlengths; // sum of all selected chartypes

   // Following state vars set by ctor, used by MakePassword()
   const UINT m_pwlen;
   const BOOL m_uselowercase;
   const BOOL m_useuppercase;
   const BOOL m_usedigits;
   const BOOL m_usesymbols;
};

#endif
