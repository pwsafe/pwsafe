/// \file PWCharPool.cpp
//-----------------------------------------------------------------------------

#include "PWCharPool.h"
#include "Util.h"


//-----------------------------------------------------------------------------
CPasswordCharPool::CPasswordCharPool()
{
   m_pool.RemoveAll();
   m_length = 0;

   std_lowercase_chars = _T("abcdefghijklmnopqrstuvwxyz");
   std_lowercase_len = strlen(std_lowercase_chars);
   std_uppercase_chars = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
   std_uppercase_len = strlen(std_uppercase_chars);
   std_digit_chars = _T("0123456789");
   std_digit_len = strlen(std_digit_chars);
   std_symbol_chars = _T("+-=_@#$%^&;:,.<>/~");
   std_symbol_len = strlen(std_symbol_chars);
   easyvision_lowercase_chars = _T("abcdefghijkmnopqrstuvwxyz");
   easyvision_lowercase_len = strlen(easyvision_lowercase_chars);
   easyvision_uppercase_chars = _T("ABCDEFGHIJKLMNPQRTUVWXY");
   easyvision_uppercase_len = strlen(easyvision_uppercase_chars);
   easyvision_digit_chars = _T("346789");
   easyvision_digit_len = strlen(easyvision_digit_chars);
   easyvision_symbol_chars = _T("+-=_@#$%^&;:,.<>/~");
   easyvision_symbol_len = strlen(easyvision_symbol_chars);
}


void
CPasswordCharPool::SetPool(BOOL easyvision, BOOL uselower, BOOL useupper, BOOL usedigits, BOOL usesymbols)
{
   m_pool.RemoveAll();
   m_length = 0;

   if (uselower)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_lowercase_chars);
      }
      else
      {
         block.SetStr(std_lowercase_chars);
      }
      block.SetType(PWC_LOWER);
      m_pool.AddTail(block);
   }

   if (useupper)
   {
      CPasswordCharBlock block;
      if (easyvision)
      {
         block.SetStr(easyvision_uppercase_chars);
      }
      else
      {
         block.SetStr(std_uppercase_chars);
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
      }
      else
      {
         block.SetStr(std_digit_chars);
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
      }
      else
      {
         block.SetStr(std_symbol_chars);
      }
      block.SetType(PWC_SYMBOL);
      m_pool.AddTail(block);
   }

   m_length = GetLength();
}


size_t
CPasswordCharPool::GetLength(void)
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
CPasswordCharPool::GetRandomChar(PWCHARTYPE* type)
{
   char ch = ' ';
   *type = PWC_UNKNOWN;

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
         *type = m_pool.GetAt(listPos).GetType();
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

//-----------------------------------------------------------------------------

CPasswordCharBlock::CPasswordCharBlock()
{
   m_type = PWC_UNKNOWN;
   m_length = 0;
   m_str = "";
}

void
CPasswordCharBlock::SetStr(const TCHAR* str)
{
   m_str = str;
   m_length = strlen(str);
}

void
CPasswordCharBlock::SetType(PWCHARTYPE type)
{
   m_type = type;
}

const TCHAR*
CPasswordCharBlock::GetStr(void)
{
   return m_str;
}

PWCHARTYPE
CPasswordCharBlock::GetType(void)
{
   return m_type;
}

size_t
CPasswordCharBlock::GetLength(void)
{
   return m_length;
}

CPasswordCharBlock&
CPasswordCharBlock::operator=(const CPasswordCharBlock &second)
{
   char *str;

   //Check for self-assignment
   if (&second != this)
   {
      m_type = second.m_type;
      m_length = second.m_length;

      str = new TCHAR[m_length+1];
      memcpy(str, second.m_str, m_length+1);
      m_str = str;
   }

   return *this;
}

