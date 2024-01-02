/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of lib.h
 */

#include "../lib.h"
#include "../debug.h"

#include <windows.h>
#include <wincrypt.h>
#include <Softpub.h>
#include <wintrust.h>

 // Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")

static bool VerifyEmbeddedSignature(LPCWSTR pwszFile)
{
  // based on  https://learn.microsoft.com/en-us/windows/win32/seccrypto/example-c-program--verifying-the-signature-of-a-pe-file
  DWORD dwLastError;
  bool retval = false;

  // Initialize the WINTRUST_FILE_INFO structure.

  WINTRUST_FILE_INFO FileData;
  memset(&FileData, 0, sizeof(FileData));
  FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
  FileData.pcwszFilePath = pwszFile;
  FileData.hFile = FileData.pgKnownSubject = nullptr;

  /*
  WVTPolicyGUID specifies the policy to apply on the file
  WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

  1) The certificate used to sign the file chains up to a root
  certificate located in the trusted root certificate store. This
  implies that the identity of the publisher has been verified by
  a certification authority.

  2) In cases where user interface is displayed (which this example
  does not do), WinVerifyTrust will check for whether the
  end entity certificate is stored in the trusted publisher store,
  implying that the user trusts content from this publisher.

  3) The end entity certificate has sufficient permission to sign
  code, as indicated by the presence of a code signing EKU or no
  EKU.
  */

  GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA WinTrustData;

  // Initialize the WinVerifyTrust input data structure.

  // Default all fields to 0.
  memset(&WinTrustData, 0, sizeof(WinTrustData));

  WinTrustData.cbStruct = sizeof(WinTrustData);

  // Use default code signing EKU.
  WinTrustData.pPolicyCallbackData = nullptr;

  // No data to pass to SIP.
  WinTrustData.pSIPClientData = nullptr;

  // Disable WVT UI.
  WinTrustData.dwUIChoice = WTD_UI_NONE;

  // No revocation checking.
  WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;

  // Verify an embedded signature on a file.
  WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

  // Verify action.
  WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;

  // Verification sets this value.
  WinTrustData.hWVTStateData = nullptr;

  // Not used.
  WinTrustData.pwszURLReference = nullptr;

  // No network retrieval, please.
  WinTrustData.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

  // This is not applicable if there is no UI
  WinTrustData.dwUIContext = 0;

  // Set pFile.
  WinTrustData.pFile = &FileData;

  // WinVerifyTrust verifies signatures as specified by the GUID 
  // and Wintrust_Data.
  LONG lStatus = WinVerifyTrust(
    nullptr,
    &WVTPolicyGUID,
    &WinTrustData);

  switch (lStatus)
  {
  case ERROR_SUCCESS:
    /*
    Signed file (UI disabled):
        - Hash that represents the subject is trusted.
        - Trusted publisher without any verification errors.
        - No publisher or time stamp chain errors.
    */
    pws_os::Trace(_T("The file \"%s\" is signed and the signature was verified.\n"), pwszFile);
    retval = true;
    break;

  case TRUST_E_NOSIGNATURE:
    // The file was not signed or had an invalid signature

    // Get the reason for no signature.
    dwLastError = GetLastError();
    if (TRUST_E_NOSIGNATURE == dwLastError ||
      TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
      TRUST_E_PROVIDER_UNKNOWN == dwLastError)
    {
      // The file was not signed.
      pws_os::Trace(L"The file \"%s\" is not signed.\n",
        pwszFile);
    }
    else
    {
      // The signature was not valid or there was an error 
      // opening the file.
      pws_os::Trace(L"An unknown error occurred trying to "
        L"verify the signature of the \"%s\" file.\n",
        pwszFile);
    }

    break;

  case TRUST_E_EXPLICIT_DISTRUST:
    // The hash that represents the subject or the publisher 
    // is not allowed by the admin or user.
    pws_os::Trace(L"The signature is present, but specifically disallowed.\n");
    break;

  case TRUST_E_SUBJECT_NOT_TRUSTED:
    // The user clicked "No" when asked to install and run.
    pws_os::Trace(L"The signature is present, but not trusted.\n");
    break;

  case CRYPT_E_SECURITY_SETTINGS:
    /*
    The hash that represents the subject or the publisher
    was not explicitly trusted by the admin and the
    admin policy has disabled user trust. No signature,
    publisher or time stamp errors.
    */
    pws_os::Trace(L"CRYPT_E_SECURITY_SETTINGS - The hash "
      L"representing the subject or the publisher wasn't "
      L"explicitly trusted by the admin and admin policy "
      L"has disabled user trust. No signature, publisher "
      L"or timestamp errors.\n");
    break;

  default:
    // The UI was disabled in dwUIChoice or the admin policy 
    // has disabled user trust. lStatus contains the 
    // publisher or time stamp chain error.
    pws_os::Trace(L"Error is: 0x%x.\n", lStatus);
    break;
  }

  // Any hWVTStateData must be released by a call with close.
  WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

  lStatus = WinVerifyTrust(
    nullptr,
    &WVTPolicyGUID,
    &WinTrustData);

  return retval;
}



void *pws_os::LoadLibrary(const TCHAR *lib, loadLibraryTypes type)
{
  ASSERT(lib != NULL);

  // We want to check the signature of the dll, but this makes sense only if the executable's signed:
  // - Assuming the user will see the warning of an unsigned exe, which is what is needed to bypass this check.
  // - This also saves the pain of having to sign dll's in every build.
  bool isExeSigned = false;

  
  // Qualify full path name.  (Lockheed Martin) Secure Coding  11-14-2007
  TCHAR szFilePath[MAX_PATH+1];
  memset(szFilePath, 0, MAX_PATH+1);
  if (type == loadLibraryTypes::SYS) {
    if (!GetSystemDirectory(szFilePath, MAX_PATH)) {
       pws_os::Trace(_T("GetSystemDirectory failed when loading dynamic library\n"));
       return nullptr;
    }
  }
  else if (type == loadLibraryTypes::APP || type == loadLibraryTypes::RESOURCE) {
    if (!GetModuleFileName(nullptr, szFilePath, MAX_PATH)) {
      pws_os::Trace(_T("GetModuleFileName failed when loading dynamic library\n"));
      return nullptr;
    }    else
    {
      isExeSigned = VerifyEmbeddedSignature(szFilePath);
       //set last slash to \0 for truncating app name
       *_tcsrchr(szFilePath, _T('\\')) = _T('\0');
    }

  }
  //Add slash after directory path
  if (type != loadLibraryTypes::CUSTOM) {
    size_t nLen = _tcslen(szFilePath);
    if (nLen > 0) {
      if (szFilePath[nLen - 1] != '\\')
        _tcscat_s(szFilePath, MAX_PATH, _T("\\"));
    }
  }

  _tcscat_s(szFilePath, MAX_PATH, lib);
  pws_os::Trace(_T("Loading Library: %s\n"), szFilePath);
  HMODULE hMod = nullptr;
  switch ((loadLibraryTypes)type) {
    // We load resource files (e.g language translation resource files) as
    // data files to avoid any problem with 32/64 bit DLLs. This allows
    // the use of 32-bit language DLLs in 64-bit builds and vice versa.
    case loadLibraryTypes::RESOURCE:
      hMod = ::LoadLibraryEx(szFilePath, nullptr, LOAD_LIBRARY_AS_DATAFILE);
	    break;
    // All other DLLs are loaded for execution
  case loadLibraryTypes::APP:
    if (isExeSigned && !VerifyEmbeddedSignature(szFilePath))
      return nullptr;
    // deliberate fallthru
    default:
	    hMod = ::LoadLibrary(szFilePath);
	    break;
  }
  return hMod;
  // End of change.  (Lockheed Martin) Secure Coding  11-14-2007
}

bool pws_os::FreeLibrary(void *handle)
{
  if (handle != nullptr)
    return ::FreeLibrary(HMODULE(handle)) == TRUE;
  else
    return false;
}

void *pws_os::GetFunction(void *handle, const char *name)
{
  ASSERT(handle != NULL && name != NULL);
  return ::GetProcAddress(HMODULE(handle), name);
}
