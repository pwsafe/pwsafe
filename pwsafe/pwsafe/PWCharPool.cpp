/// \file PWCharPool.cpp
//-----------------------------------------------------------------------------

#include "PWCharPool.h"
#include "Util.h"

// Following macro get length of std_*_chars less the trailing \0
// compile time equivalent of strlen()
#define LENGTH(s) (sizeof(s)/sizeof(s[0]) - sizeof(s[0]))

const TCHAR 
CPasswordCharPool::std_lowercase_chars[] = _T("abcdefghijklmnopqrstuvwxyz");
const size_t
CPasswordCharPool::std_lowercase_len = LENGTH(std_lowercase_chars);
const TCHAR CPasswordCharPool::std_uppercase_chars[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
const size_t
CPasswordCharPool::std_uppercase_len = LENGTH(std_uppercase_chars);
const TCHAR
CPasswordCharPool::std_digit_chars[] =
_T("0123456789");
const size_t
CPasswordCharPool::std_digit_len = LENGTH(std_digit_chars);
const TCHAR
CPasswordCharPool::std_symbol_chars[] = _T("+-=_@#$%^&;:,.<>/~\\[](){}?!|");
const size_t
CPasswordCharPool::std_symbol_len = LENGTH(std_symbol_chars);
const TCHAR
CPasswordCharPool::easyvision_lowercase_chars[] = _T("abcdefghijkmnopqrstuvwxyz");
const size_t
CPasswordCharPool::easyvision_lowercase_len = LENGTH(easyvision_lowercase_chars);
const TCHAR
CPasswordCharPool::easyvision_uppercase_chars[] = _T("ABCDEFGHJKLMNPQRTUVWXY");
const size_t
CPasswordCharPool::easyvision_uppercase_len = LENGTH(easyvision_uppercase_chars);
const TCHAR
CPasswordCharPool::easyvision_digit_chars[] = _T("346789");
const size_t
CPasswordCharPool::easyvision_digit_len = LENGTH(easyvision_digit_chars);
const TCHAR
CPasswordCharPool::easyvision_symbol_chars[] = _T("+-=_@#$%^&<>/~\\?");
const size_t
CPasswordCharPool::easyvision_symbol_len = LENGTH(easyvision_symbol_chars);

//-----------------------------------------------------------------------------

CPasswordCharPool::CPasswordCharPool(UINT pwlen,
				     BOOL uselowercase, BOOL useuppercase,
				     BOOL usedigits, BOOL usesymbols,
				     BOOL easyvision) :
  m_pwlen(pwlen), m_uselowercase(uselowercase), m_useuppercase(useuppercase),
  m_usedigits(usedigits), m_usesymbols(usesymbols)
{
  ASSERT(m_pwlen > 0);
  ASSERT(m_uselowercase || m_useuppercase || m_usedigits || m_usesymbols);

  if (easyvision) {
    m_char_arrays[LOWERCASE] = (TCHAR *)easyvision_lowercase_chars;
    m_char_arrays[UPPERCASE] = (TCHAR *)easyvision_uppercase_chars;
    m_char_arrays[DIGIT] = (TCHAR *)easyvision_digit_chars;
    m_char_arrays[SYMBOL] = (TCHAR *)easyvision_symbol_chars;
    m_lengths[LOWERCASE] = uselowercase ? easyvision_lowercase_len : 0;
    m_lengths[UPPERCASE] = useuppercase ? easyvision_uppercase_len : 0;
    m_lengths[DIGIT] = usedigits ? easyvision_digit_len : 0;
    m_lengths[SYMBOL] = usesymbols ? easyvision_symbol_len : 0;
  } else { // !easyvision
    m_char_arrays[LOWERCASE] = (TCHAR *)std_lowercase_chars;
    m_char_arrays[UPPERCASE] = (TCHAR *)std_uppercase_chars;
    m_char_arrays[DIGIT] = (TCHAR *)std_digit_chars;
    m_char_arrays[SYMBOL] = (TCHAR *)std_symbol_chars;
    m_lengths[LOWERCASE] = uselowercase ? std_lowercase_len : 0;
    m_lengths[UPPERCASE] = useuppercase ? std_uppercase_len : 0;
    m_lengths[DIGIT] = usedigits ? std_digit_len : 0;
    m_lengths[SYMBOL] = usesymbols ? std_symbol_len : 0;
  }

  m_x[0] = 0;
  m_sumlengths = 0;
  for (int i = 0; i< NUMTYPES; i++) {
    m_x[i+1] = m_x[i] + m_lengths[i];
    m_sumlengths += m_lengths[i];
  }
  ASSERT(m_sumlengths > 0);
}

CPasswordCharPool::CharType CPasswordCharPool::GetRandomCharType() const
{
  // select a chartype with weighted probability
   size_t rand = RangeRand(m_sumlengths);
   int i;
   for (i = 0; i < NUMTYPES; i++) {
     if (rand < m_x[i+1]) {
       break;
     }
   }

   ASSERT(m_lengths[i] > 0 && i < NUMTYPES);
   return CharType(i);
}


TCHAR CPasswordCharPool::GetRandomChar(CPasswordCharPool::CharType t) const
{
  ASSERT(t < NUMTYPES);
  ASSERT(m_lengths[t] > 0);
  size_t rand = RangeRand(m_lengths[t]);

  TCHAR retval = m_char_arrays[t][rand];
  return retval;
}

CMyString
CPasswordCharPool::MakePassword() const
{
  ASSERT(m_pwlen > 0);
  ASSERT(m_uselowercase || m_useuppercase || m_usedigits || m_usesymbols);

  int lowercaseneeded;
  int uppercaseneeded;
  int digitsneeded;
  int symbolsneeded;

  CMyString password = "";

  bool pwRulesMet;
  CMyString temp;

   do
   {
      TCHAR ch;
      CharType type;

      lowercaseneeded = (m_uselowercase) ? 1 : 0;
      uppercaseneeded = (m_useuppercase) ? 1 : 0;
      digitsneeded = (m_usedigits) ? 1 : 0;
      symbolsneeded = (m_usesymbols) ? 1 : 0;

      // If following assertion doesn't hold, we'll never exit the do loop!
      ASSERT(int(m_pwlen) >= lowercaseneeded + uppercaseneeded +
	     digitsneeded + symbolsneeded);

      temp = "";    // empty the password string

      for (UINT x = 0; x < m_pwlen; x++)
      {
	 type = GetRandomCharType();
         ch = GetRandomChar(type);
         temp += ch;
         /*
         **  Decrement the appropriate needed character type count.
         */
         switch (type)
         {
	    case LOWERCASE:
	      lowercaseneeded--;
               break;

            case UPPERCASE:
	      uppercaseneeded--;
               break;

            case DIGIT:
	      digitsneeded--;
               break;

            case SYMBOL:
	      symbolsneeded--;
               break;

            default:
	      ASSERT(0); // should never happen!
               break;
         }
      } // for

      /*
       * Make sure we have at least one representative of each required type
       * after the for loop. If not, try again. Arguably, recursion would have
       * been more elegant than a do loop, but this takes less stack...
       */
      pwRulesMet = (lowercaseneeded <= 0 && uppercaseneeded <= 0 &&
		    digitsneeded <= 0 && symbolsneeded <= 0);

      if (pwRulesMet)
      {
         password = temp;
      }
      // Otherwise, do not exit, do not collect $200, try again...
   } while (!pwRulesMet);
   ASSERT(password.GetLength() == int(m_pwlen));
   return password;
}
