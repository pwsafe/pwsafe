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
  m_usedigits(usedigits), m_usesymbols(usesymbols), m_length(0)
{
  ASSERT(m_pwlen > 0);
  ASSERT(m_uselowercase || m_useuppercase || m_usedigits || m_usesymbols);

   if (uselowercase)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_lowercase_chars);
         block.SetLength(easyvision_lowercase_len);
      }
      else
      {
         block.SetStr(std_lowercase_chars);
         block.SetLength(std_lowercase_len);
      }
      block.SetType(PWC_LOWER);
      m_pool.AddTail(block);
   }

   if (useuppercase)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_uppercase_chars);
         block.SetLength(easyvision_uppercase_len);
      }
      else
      {
         block.SetStr(std_uppercase_chars);
         block.SetLength(std_uppercase_len);
      }
      block.SetType(PWC_UPPER);
      m_pool.AddTail(block);
   }

   if (usedigits)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_digit_chars);
         block.SetLength(easyvision_digit_len);
      }
      else
      {
         block.SetStr(std_digit_chars);
         block.SetLength(std_digit_len);
      }
      block.SetType(PWC_DIGIT);
      m_pool.AddTail(block);
   }

   if (usesymbols)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_symbol_chars);
         block.SetLength(easyvision_symbol_len);
      }
      else
      {
         block.SetStr(std_symbol_chars);
         block.SetLength(std_symbol_len);
      }
      block.SetType(PWC_SYMBOL);
      m_pool.AddTail(block);
   }

   m_length = GetLength();
}


size_t
CPasswordCharPool::GetLength(void) const
{
   POSITION listPos = m_pool.GetHeadPosition();
   size_t len = 0;

   while (listPos != NULL)
   {
      len += m_pool.GetAt(listPos).GetLength();
      m_pool.GetNext(listPos);
   }
   return len;
}


char
CPasswordCharPool::GetRandomChar(PWCHARTYPE& type) const
{
   char ch = ' ';
   type = PWC_UNKNOWN;

   size_t rand = RangeRand(m_length);

   /*
   **  Iterate through the list of password character blocks
   **  to find the correct character.
   */
   POSITION listPos = m_pool.GetHeadPosition();
   size_t len;
   const TCHAR* str;

   while (listPos != NULL)
   {
      len = m_pool.GetAt(listPos).GetLength();
      if (rand < len)
      {
         type = m_pool.GetAt(listPos).GetType();
         str = m_pool.GetAt(listPos).GetStr();
         ch = str[rand];
         break;
      }
      else
      {
         rand -= len;
      }

      m_pool.GetNext(listPos);
   }
   return ch;
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

  BOOL pwRulesMet;

   do
   {
      TCHAR ch;

      lowercaseneeded = (m_uselowercase) ? 1 : 0;
      uppercaseneeded = (m_useuppercase) ? 1 : 0;
      digitsneeded = (m_usedigits) ? 1 : 0;
      symbolsneeded = (m_usesymbols) ? 1 : 0;

      // If following assertion doesn't hold, we'll never exit the do loop!
      ASSERT(int(m_pwlen) >= lowercaseneeded + uppercaseneeded +
	     digitsneeded + symbolsneeded);

      CMyString temp = "";    // empty the password string
      PWCHARTYPE type;

      for (UINT x = 0; x < m_pwlen; x++)
      {
         ch = GetRandomChar(type);
         temp += ch;
         /*
         **  Decrement the appropriate needed character type count.
         */
         switch (type)
         {
            case PWC_LOWER:
	      lowercaseneeded--;
               break;

            case PWC_UPPER:
	      uppercaseneeded--;
               break;

            case PWC_DIGIT:
	      digitsneeded--;
               break;

            case PWC_SYMBOL:
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


//-----------------------------------------------------------------------------

CPasswordCharBlock::CPasswordCharBlock() : m_type(PWC_UNKNOWN),
					   m_length(0), m_str(NULL)
{
}

CPasswordCharBlock::CPasswordCharBlock(const CPasswordCharBlock &that)
  : m_type(that.m_type), m_length(that.m_length), m_str(that.m_str)
{
}

void
CPasswordCharBlock::SetStr(const TCHAR* str)
{
   m_str = str;
}

void
CPasswordCharBlock::SetLength(size_t len)
{
   m_length = len;
}

void
CPasswordCharBlock::SetType(PWCHARTYPE type)
{
   m_type = type;
}

const TCHAR*
CPasswordCharBlock::GetStr(void) const
{
   return m_str;
}

PWCHARTYPE
CPasswordCharBlock::GetType(void) const
{
   return m_type;
}

size_t
CPasswordCharBlock::GetLength(void) const
{
   return m_length;
}

CPasswordCharBlock&
CPasswordCharBlock::operator=(const CPasswordCharBlock &second)
{
   //Check for self-assignment
   if (&second != this)
   {
      m_type = second.m_type;
      m_length = second.m_length;
      m_str = second.m_str;
   }

   return *this;
}

