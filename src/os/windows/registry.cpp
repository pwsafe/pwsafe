/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of registry.h
 *
 * Actually contains two implementations:
 * One for Windows/MFC, and one for wxWidgets
 */

#include "../typedefs.h"
#include "../registry.h"
#include "../debug.h"

#ifndef __WX__

static HKEY hSubTreeKey;      // not re-entrant, but who cares?
static bool bSubTreeKeyValid; // not me!

bool pws_os::RegCheckExists(const TCHAR *stree)
{
  if (stree == nullptr) {
    CWinApp *app = ::AfxGetApp();
    if (app == nullptr) // can happen in unit test framework
      return false;
    stree = app->m_pszRegistryKey;
  }

  if (stree == nullptr) // can happen too (no registry key for app)
	  return false;

  const stringT csSubkey = _T("Software\\") + stringT(stree);
  HKEY hSubkey;
  bool bExists = ::RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey.c_str(), 0L,
                                KEY_READ, &hSubkey) == ERROR_SUCCESS;
  if (bExists)
    ::RegCloseKey(hSubkey);
  return bExists;
}

bool pws_os::RegWriteValue(const TCHAR *section, const TCHAR *entry, int value)
{
  return ::AfxGetApp()->WriteProfileInt(section, entry, value) == TRUE;
}

bool pws_os::RegWriteValue(const TCHAR *section, const TCHAR *entry,
                           const TCHAR *value)
{
  return ::AfxGetApp()->WriteProfileString(section, entry, value) == TRUE;
}

bool pws_os::RegDeleteEntry(const TCHAR *name)
{
  HKEY hSubkey;
  DWORD dwResult, dwType;
  bool bRetVal;

  // Keys in registry are in:
  // "HKEY_CURRENT_USER\Software\Password Safe\Password Safe\"
  const stringT csSubkey = _T("Software\\") + 
    stringT(::AfxGetApp()->m_pszRegistryKey)
    + _T("\\") +
    stringT(::AfxGetApp()->m_pszRegistryKey);

  dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                          csSubkey.c_str(),
                          NULL,
                          KEY_ALL_ACCESS,
                          &hSubkey);

  if (dwResult != ERROR_SUCCESS)
    return false; // may have been called due to OldPrefs

  dwResult = RegQueryValueEx(hSubkey, name, NULL, &dwType, NULL, NULL);
  if (dwResult == ERROR_SUCCESS) {
    // Was there - now delete it
    dwResult = RegDeleteValue(hSubkey, name);
    ASSERT(dwResult == ERROR_SUCCESS);
    bRetVal = (dwResult == ERROR_SUCCESS);
  } else
    bRetVal = true;

  dwResult = RegCloseKey(hSubkey);
  ASSERT(dwResult == ERROR_SUCCESS);
  return bRetVal;
}

int pws_os::RegReadValue(const TCHAR *section, const TCHAR *entry, const int value)
{
  return ::AfxGetApp()->GetProfileInt(section, entry, value);
}

const stringT pws_os::RegReadValue(const TCHAR *section, const TCHAR *entry,
                                   const TCHAR *value)
{
  return (const TCHAR*)::AfxGetApp()->GetProfileString(section, entry, value);
}

void pws_os::RegDeleteSubtree(const TCHAR *stree)
{
  HKEY hSubkey;
  LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                           _T("Software"),
                           NULL,
                           KEY_ALL_ACCESS,
                           &hSubkey);
  if (dw != ERROR_SUCCESS) {
    pws_os::Trace0(_T("pws_os::RegDeleteSubtree: RegOpenKeyEx failed\n"));
    return;
  }

  dw = ::AfxGetApp()->DelRegTree(hSubkey, stree);
  if (dw != ERROR_SUCCESS) {
    pws_os::Trace0(_T("pws_os::RegDeleteSubtree: DelRegTree failed\n"));
  }
  dw = ::RegCloseKey(hSubkey);
  if (dw != ERROR_SUCCESS) {
    pws_os::Trace0(_T("pws_os::RegDeleteSubtree: RegCloseKey failed\n"));
  }
}

// Start of registry Subtree functions
// Use static variable hSubTreeKey to hold registry key handle between calls.
bool pws_os::RegOpenSubtree(const TCHAR *stree)
{
  const stringT streeT(stree);
  stringT OldAppKey(_T("Software\\"));
  OldAppKey += streeT; OldAppKey += _T("\\Password Safe");
  LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                           OldAppKey.c_str(),
                           NULL,
                           KEY_ALL_ACCESS,
                           &hSubTreeKey);
  bSubTreeKeyValid = (dw == ERROR_SUCCESS);
  return bSubTreeKeyValid;
}

bool pws_os::RegReadSTValue(const TCHAR *name, bool &value)
{
  if (!bSubTreeKeyValid)
    return bSubTreeKeyValid;

  int v;
  bool retval = RegReadSTValue(name, v);
  if (retval)
    value = (v != 0);
  return retval;
}

bool pws_os::RegReadSTValue(const TCHAR *name, int &value)
{
  if (!bSubTreeKeyValid)
    return bSubTreeKeyValid;

  bool retval = false;
  LONG rv;
  DWORD dwType, vData, DataLen(sizeof(vData));
  rv = ::RegQueryValueEx(hSubTreeKey,
                         name,
                         NULL,
                         &dwType,
                         LPBYTE(&vData),
                         &DataLen);
  if (rv == ERROR_SUCCESS && dwType == REG_DWORD) {
    value = vData;
    retval = true;
  }
  return retval;
}

bool pws_os::RegReadSTValue(const TCHAR *name, stringT &value)
{
  if (!bSubTreeKeyValid)
    return bSubTreeKeyValid;

  bool retval = false;
  LONG rv;
  DWORD dwType, DataLen;
  rv = ::RegQueryValueEx(hSubTreeKey,
                         name,
                         NULL,
                         &dwType,
                         NULL,
                         &DataLen);
  if (rv == ERROR_SUCCESS && dwType == REG_SZ) {
    DataLen++;
    TCHAR *pData = new TCHAR[DataLen];
    ::memset(pData, 0, DataLen);
    rv = ::RegQueryValueEx(hSubTreeKey,
                            name,
                            NULL,
                            &dwType,
                            LPBYTE(pData),
                            &DataLen);

    if (rv == ERROR_SUCCESS) {
      value = pData;
      retval = true;
    }
    delete[] pData;
  }
  return retval;
}

bool pws_os::RegCloseSubtree()
{
  if (!bSubTreeKeyValid)
    return bSubTreeKeyValid;

  bool retval = (::RegCloseKey(hSubTreeKey) == ERROR_SUCCESS);
  bSubTreeKeyValid = false;
  return retval;
}

// End of registry Subtree functions

bool pws_os::DeleteRegistryEntries()
{
  HKEY hSubkey;
  const stringT csSubkey = _T("Software\\");
  bool retval = true;

  LONG dw = RegOpenKeyEx(HKEY_CURRENT_USER,
                         csSubkey.c_str(),
                         NULL,
                         KEY_ALL_ACCESS,
                         &hSubkey);
  if (dw != ERROR_SUCCESS) {
    return false; // may have been called due to OldPrefs
  }

  dw = ::AfxGetApp()->DelRegTree(hSubkey, ::AfxGetApp()->m_pszAppName);
  if (dw != ERROR_SUCCESS) {
    pws_os::Trace0(_T("pws_os::DeleteRegistryEntries: DelRegTree() failed\n"));
    retval = false;
  }

  dw = RegCloseKey(hSubkey);
  if (dw != ERROR_SUCCESS) {
    pws_os::Trace0(_T("pws_os::DeleteRegistryEntries: RegCloseKey() failed\n"));
  }
  return retval;
}

#else /* __WX__ */

// XXX All TBD...

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

int pws_os::RegReadValue(const TCHAR *, const TCHAR *, int value)
{
  return value;
}

const stringT pws_os::RegReadValue(const TCHAR *, const TCHAR *, const TCHAR *value)
{
  return stringT(value);
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

#endif /* __WX__ */
