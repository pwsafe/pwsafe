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
   return GetPassword(app.GetProfileInt("", "pwlendefault", 8),
		      app.GetProfileInt("", "pwuselowercase", TRUE),
		      app.GetProfileInt("", "pwuseuppercase", TRUE),
		      app.GetProfileInt("", "pwusedigits", TRUE),
		      app.GetProfileInt("", "pwusesymbols", TRUE));
}


CMyString
DboxMain::GetPassword( UINT pwlen, BOOL uselowercase, BOOL useuppercase,
		       BOOL usedigits, BOOL usesymbols)
{
  ASSERT(pwlen > 0);
  ASSERT(uselowercase || useuppercase || usedigits || usesymbols);

  int lowercaseneeded;
  int uppercaseneeded;
  int digitsneeded;
  int symbolsneeded;

  CMyString password = "";

  BOOL pwRulesMet;

   do
   {
      TCHAR ch;

      lowercaseneeded = (uselowercase) ? 1 : 0;
      uppercaseneeded = (useuppercase) ? 1 : 0;
      digitsneeded = (usedigits) ? 1 : 0;
      symbolsneeded = (usesymbols) ? 1 : 0;

      // If following assertion doesn't hold, we'll never exit the do loop!
      ASSERT(int(pwlen) >= lowercaseneeded + uppercaseneeded +
	     digitsneeded + symbolsneeded);

      CMyString temp = "";    // empty the password string
      PWCHARTYPE type;

      for (UINT x = 0; x < pwlen; x++)
      {
         ch = pwchars.GetRandomChar(&type);
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
   ASSERT(password.GetLength() == int(pwlen));
   return password;
}

