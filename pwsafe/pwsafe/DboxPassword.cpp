/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "Util.h"
#include "PWCharPool.h"
#include "DboxMain.h"


//-----------------------------------------------------------------------------
CMyString
DboxMain::GetPassword(void)
{
   return GetPassword(app.GetProfileInt("", "pwlendefault", 8));
}


CMyString
DboxMain::GetPassword( UINT pwlen )
{
   UINT pwminlowercase;
   UINT pwminuppercase;
   UINT pwmindigits;
   UINT pwminsymbols;
   UINT count_lowercase_chars;
   UINT count_uppercase_chars;
   UINT count_digit_chars;
   UINT count_symbol_chars;

   CMyString password = "";

   if (app.GetProfileInt("", "pwuselowercase", TRUE))
      pwminlowercase = app.GetProfileInt("", "pwminlowercase", 0);
   else
      pwminlowercase = 0;

   if (app.GetProfileInt("", "pwuseuppercase", TRUE))
      pwminuppercase = app.GetProfileInt("", "pwminuppercase", 0);
   else
      pwminuppercase = 0;

   if (app.GetProfileInt("", "pwusedigits", TRUE))
      pwmindigits = app.GetProfileInt("", "pwmindigits", 0);
   else
      pwmindigits = 0;

   if (app.GetProfileInt("", "pwusesymbols", TRUE))
      pwminsymbols = app.GetProfileInt("", "pwminsymbols", 0);
   else
      pwminsymbols = 0;

   BOOL pwRulesMet;

   do
   {
      char ch;

      count_uppercase_chars=0;
      count_lowercase_chars=0;
      count_digit_chars=0;
      count_symbol_chars=0;

      CMyString temp = "";    // empty the password string
      PWCHARTYPE type;

      for (UINT x = 0; x < pwlen; x++)
      {
         ch = pwchars.GetRandomChar(&type);
         temp += ch;
         /*
         **  Increment the appropriate character type count.
         */
         switch (type)
         {
            case PWC_LOWER:
               count_lowercase_chars++;
               break;

            case PWC_UPPER:
               count_uppercase_chars++;
               break;

            case PWC_DIGIT:
               count_digit_chars++;
               break;

            case PWC_SYMBOL:
               count_symbol_chars++;
               break;

            default:
               break;
         }
      }

      /*
      **  Set the 'pwRulesMet' to TRUE, indicating success.
      **  If we are not checking the rules the loop will end,
      **  and if we are checking rules we'll let the checks
      **  indicate failure if necessary.
      */
      pwRulesMet = TRUE;

      if (pwminuppercase > count_uppercase_chars)
         pwRulesMet = FALSE;
      else if (pwminlowercase > count_lowercase_chars)
         pwRulesMet = FALSE;
      else if (pwmindigits > count_digit_chars)
         pwRulesMet = FALSE;
      else if (pwminsymbols > count_symbol_chars)
         pwRulesMet = FALSE;

      if (pwRulesMet == TRUE)
      {
         password = temp;
      }

   } while (pwRulesMet == FALSE);

   return password;
}

