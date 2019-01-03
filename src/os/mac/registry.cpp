/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of registry.h
 *
 * NULL, really, since registry's a Windows-ism
 */

#include "../typedefs.h"
#include "../registry.h"

bool pws_os::RegCheckExists(const TCHAR *)
{
  return false;
}

bool pws_os::RegWriteValue(const TCHAR *, const TCHAR *, int)
{
  return false;
}

bool pws_os::RegWriteValue(const TCHAR *, const TCHAR *, const TCHAR *)
{
  return false;
}
bool pws_os::RegDeleteEntry(const TCHAR *)
{
  return false;
}

int pws_os::RegReadValue(const TCHAR *, const TCHAR *, const int value)
{
  return value;
}

const stringT pws_os::RegReadValue(const TCHAR *, const TCHAR *,
                                   const TCHAR *value)
{
  return value;
}

void pws_os::RegDeleteSubtree(const TCHAR *)
{
}

bool pws_os::RegOpenSubtree(const TCHAR *)
{
  return false;
}

bool pws_os::RegReadSTValue(const TCHAR *, bool &)
{
  return false;
}

bool pws_os::RegReadSTValue(const TCHAR *, int &)
{
  return false;
}

bool pws_os::RegReadSTValue(const TCHAR *, stringT &)
{
  return false;
}

bool pws_os::RegCloseSubtree()
{
  return false;
}

bool pws_os::DeleteRegistryEntries()
{
  return false;
}
