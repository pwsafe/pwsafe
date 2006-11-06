/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// PwsBackend.h
// Password Safe - backend
//-----------------------------------------------------------------------------
#include "MyString.h"
#include "Util.h"
//-----------------------------------------------------------------------------
class PwsBackend
{
public:
   PwsBackend();
   ~PwsBackend();
   
   CMyString m_passkey; // the main one, in memory?!? yikes {jpr}

   unsigned char m_randstuff[StuffSize];
   unsigned char m_randhash[SaltSize];

};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
