/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// PasswordDb.h
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include <vector>

//-----------------------------------------------------------------------------
class PasswordDb
{
public:
   PasswordDb()
   {}

   ~PasswordDb()
   {}

   void Clear()
   { m_db.clear(); }

   void Add(const CItemData& a_item);
   CItemData Item(size_t a_ix);
   void Remove(size_t a_ix);
   size_t Find(const string& a_key);
   short Version();

private:
   vector<CItemData> m_db;
   string m_defaultUsername;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
