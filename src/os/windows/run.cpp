/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of run.h
 */

#include "../typedefs.h"
#include <shellapi.h>

#include "../run.h"
#include "../debug.h"
#include "../dir.h"
#include "../env.h"
#include "../file.h"
#include "../utf8conv.h"
#include "../lib.h"

#include "pws_autotype/pws_at.h"

#include <vector>
#include <algorithm>

typedef AT_API BOOL (* AT_PROC_BOOL) (HWND);
typedef AT_API int  (* AT_PROC_INT) ();

struct st_run_impl {
  AT_PROC_BOOL pInit;   // Pointer to   Initialise function in pws_at(_D).dll
  AT_PROC_BOOL pUnInit; // Pointer to UnInitialise function in pws_at(_D).dll
  AT_PROC_INT  pGetVer; // Pointer to   GetVersion function in pws_at(_D).dll
  HWND hCBWnd;     // Handle to Window to receive SendMessage for processing
                   //   It is the main DboxMain window.
  HINSTANCE m_AT_HK_module;

  bool isValid() const {return m_AT_HK_module != NULL;}

  st_run_impl()
    : pInit(NULL), pUnInit(NULL), hCBWnd(NULL), m_AT_HK_module(NULL) {
    // Support Autotype with Launch Browser and Run Command
    // Try to load DLL to call back when window active for Autotype
#if defined( _DEBUG ) || defined( DEBUG )
    TCHAR *dll_name = _T("pws_at_D.dll");
#else
    TCHAR *dll_name = _T("pws_at.dll");
#endif
    m_AT_HK_module = HMODULE(pws_os::LoadLibrary(dll_name, pws_os::loadLibraryTypes::APP));
    if (m_AT_HK_module != NULL) {
      pws_os::Trace(_T("st_run_impl::st_run_impl - AutoType DLL Loaded: OK\n"));
      pInit  = AT_PROC_BOOL(pws_os::GetFunction(m_AT_HK_module, "AT_HK_Initialise"));
      pws_os::Trace(_T("st_run_impl::st_run_impl - Found AT_HK_Initialise: %s\n"),
            pInit != NULL ? _T("OK") : _T("FAILED"));

      pUnInit = AT_PROC_BOOL(pws_os::GetFunction(m_AT_HK_module, "AT_HK_UnInitialise"));
      pws_os::Trace(_T("st_run_impl::st_run_impl - Found AT_HK_UnInitialise: %s\n"),
            pUnInit != NULL ? _T("OK") : _T("FAILED"));

      pGetVer = AT_PROC_INT(pws_os::GetFunction(m_AT_HK_module, "AT_HK_GetVersion"));
      pws_os::Trace(_T("st_run_impl::st_run_impl - Found AT_HK_GetVersion: %s\n"),
            pGetVer != NULL ? _T("OK") : _T("FAILED"));

      if (pGetVer == NULL || pGetVer() != AT_DLL_VERSION) {
        pws_os::Trace(_T("st_run_impl::st_run_impl - Unable to determine DLL version")
                      _T(" or incorrect version\n"));
        BOOL brc = pws_os::FreeLibrary(m_AT_HK_module);
        pws_os::Trace(_T("st_run_impl::st_run_impl - Free Autotype DLL: %s\n"),
                      brc == TRUE ? _T("OK") : _T("FAILED"));
        m_AT_HK_module = NULL;
      }
    }
  }

  st_run_impl(const st_run_impl &that)
    : pInit(that.pInit), pUnInit(that.pUnInit), pGetVer(that.pGetVer),
      hCBWnd(that.hCBWnd), m_AT_HK_module(that.m_AT_HK_module) {}

  st_run_impl &operator=(const st_run_impl &that)
  {
    if (this != &that) {
      pInit = that.pInit;
      pUnInit = that.pUnInit;
      pGetVer = that.pGetVer;
      hCBWnd = that.hCBWnd;
      m_AT_HK_module = that.m_AT_HK_module;
    }
    return *this;
  }

  ~st_run_impl() {
    if (m_AT_HK_module != NULL) {
      // Autotype UnInitialise just in case - should have been done
      // during the callback process.  It will probably return failed
      // but we don't care.
      pUnInit(hCBWnd);
      BOOL brc = pws_os::FreeLibrary(m_AT_HK_module);
      pws_os::Trace(_T("st_run_impl::~st_run_impl - Free Autotype DLL: %s\n"),
                    brc == TRUE ? _T("OK") : _T("FAILED"));
      m_AT_HK_module = NULL;
    }
  }
};

PWSRun::PWSRun()
{
  pImpl = new st_run_impl;
}

PWSRun::~PWSRun()
{
  delete pImpl;
}

bool PWSRun::isValid() const
{
  return ((pImpl != NULL) && pImpl->isValid());
}

void PWSRun::Set(void *data) const
{
  if (pImpl != NULL)
    pImpl->hCBWnd = reinterpret_cast<HWND>(data);
}

bool PWSRun::UnInit()
{
  if (pImpl != NULL) {
    return pImpl->pUnInit(pImpl->hCBWnd) == TRUE;
  } else
    return false;
}

bool PWSRun::runcmd(const StringX &run_command, const bool &bAutotype) const
{
  // Get first parameter either enclosed by quotes or delimited by a space
  StringX full_string(run_command), first_part(_T("")), the_rest(_T(""));
  StringX env_var, sx_temp(_T(""));
  StringX::size_type end_delim;
  bool bfound(true);

  TrimLeft(full_string, _T(" "));
  if (full_string.c_str()[0] == _T('"')) {
    end_delim = full_string.find(_T('"'), 1);
    first_part = full_string.substr(1, end_delim - 1);
    the_rest = full_string.substr(end_delim + 1);
  } else {
    end_delim = full_string.find(_T(' '));
    if (end_delim != StringX::npos) {
      first_part = full_string.substr(0, end_delim);
      the_rest = full_string.substr(end_delim + 1);
    } else
      first_part = full_string;
  }

  // tokenize into separate elements using % as the field separator.
  // If this corresponds to a set environment variable - replace it
  // and rebuild the command
  for (StringX::size_type st_startpos = 0;
       st_startpos < first_part.size();
       /* st_startpos advanced in body */) {
    StringX::size_type st_next = first_part.find(_T('%'), st_startpos);
    if (st_next == StringX::npos) {
      sx_temp += first_part.substr(st_startpos);
      break;
    }
    if (st_next > 0) {
      env_var = first_part.substr(st_startpos, st_next - st_startpos);
      size_t mblen = pws_os::wcstombs(NULL, 0, env_var.c_str(), size_t(-1), false);
      unsigned char * mbtemp = new unsigned char[mblen + 1];
      // Finally get result
      size_t tmplen = pws_os::wcstombs((char *)mbtemp, mblen, env_var.c_str(),
                                       size_t(-1), false);
      if (tmplen != mblen) {
        return false;
      }
      mbtemp[mblen - 1] = '\0';
      StringX envar = (pws_os::getenv((char *)mbtemp, false)).c_str();
      if (!envar.empty()) {
        sx_temp += envar;
      } else {
        sx_temp += StringX(_T("%")) + env_var + StringX(_T("%"));
      }
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Replace string by rebuilt string
  first_part = sx_temp;

  first_part = getruncmd(first_part, bfound);

  bool rc;
  if (bfound)
    rc = issuecmd(first_part, the_rest, bAutotype);
  else
    rc = issuecmd(full_string, _T(""), bAutotype);

  return rc;
}

bool PWSRun::issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                      const bool &bAutotype) const
{
  SHELLEXECUTEINFO si;
  ZeroMemory(&si, sizeof(SHELLEXECUTEINFO));
  si.cbSize = sizeof(SHELLEXECUTEINFO);
  si.nShow = SW_SHOWNORMAL;
  si.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_DOENVSUBST;

  si.lpFile = sxFile.c_str();
  if (!sxParameters.empty()) {
    si.lpParameters = sxParameters.c_str();
  }

  BOOL bAT_init(FALSE);

  if (bAutotype && isValid()) {
    if (pImpl->pInit != NULL && pImpl->pUnInit != NULL &&
        pImpl->hCBWnd != NULL) {
      // OK - try and make it tell us!  Won't if another instance of
      // PWS is doing this at exactly the same time - silly user!
      bAT_init = pImpl->pInit(pImpl->hCBWnd);
      pws_os::Trace(_T("PWSRun::issuecmd - AT_HK_Initialise: %s\n"),
                    bAT_init == TRUE ? _T("OK") : _T("FAILED"));
    }
  }

  BOOL shellExecStatus = ::ShellExecuteEx(&si);
  if (shellExecStatus != TRUE) {
    // ShellExecute writes its own message on failure!
    if (bAT_init) {
      bAT_init = pImpl->pUnInit(pImpl->hCBWnd);
      pws_os::Trace(_T("PWSRun::issuecmd - AT_HK_UnInitialise: %s\n"),
                    bAT_init == TRUE ? _T("OK") : _T("FAILED"));
    }
    return false;
  }
  return true;
}

StringX PWSRun::getruncmd(const StringX &sxFile, bool &bfound) const
{
  // 1. If first parameter is in quotes - assume fully qualified - don't search.
  // 2. If first parameter starts with '%, assume it is going to be replaced by the
  // corresponding environment variable - no need to search directories.
  // 3. If the first parameter ends in '.xxx', and '.xxx' is in the PATHEXT variable,
  // search for it as-is.  If not, append each of the known extensions to it and then
  // search.
  // 4. If searched and could not find, just issue 'as-is'.

  std::vector<StringX> vpaths;
  std::vector<StringX> vextns;

  StringX full_pgm(sxFile), sx_cwd;
  StringX sx_temp, sx_dirs, sx_extns;

  stringT path, drive, dir, fname, extn;
  stringT s_temp;
  bool bsearch_dirs(true), bsearch_extns(true);

  bfound = false;

  if (full_pgm.length() == 0)
    return full_pgm;

  // Search order:

  // Current working directory
  s_temp = pws_os::getcwd();
  stringT::size_type Tlen = s_temp.length();
  if (Tlen == 0 || s_temp[Tlen - 1] != _T('\\')) {
    s_temp += _T("\\");
  }
  sx_cwd = StringX(s_temp.c_str());
  vpaths.push_back(sx_cwd);

  // Windows directory
  s_temp = pws_os::getenv("windir", true);
  if (s_temp.length() > 0)
    vpaths.push_back(StringX(s_temp.c_str()));

  // Windows/System32 directory
  if (!s_temp.empty()) {
    s_temp += stringT(_T("System32"));
    vpaths.push_back(StringX(s_temp.c_str()));
  }

  // Directories in PATH
  s_temp = pws_os::getenv("PATH", true);
  sx_temp = s_temp.c_str();
  // tokenize into separate elements using ; as the field separator
  for (StringX::size_type st_startpos = 0;
       st_startpos < sx_temp.size();
       /* st_startpos advanced in body */) {
    StringX::size_type st_next = sx_temp.find(_T(';'), st_startpos);
    if (st_next == StringX::npos)
      st_next = sx_temp.size();
    if (st_next > 0) {
      sx_dirs = sx_temp.substr(st_startpos, st_next - st_startpos);
      vpaths.push_back(sx_dirs);
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Apps Paths registry key - see below

  // Get support program extensions
  s_temp = pws_os::getenv("PATHEXT", false);
  sx_temp = s_temp.c_str();
  // tokenize into separate elements using ; as the field separator
  for (StringX::size_type st_startpos = 0;
       st_startpos < sx_temp.size();
       /* st_startpos advanced in body */) {
    StringX::size_type st_next = sx_temp.find(_T(';'), st_startpos);
    if (st_next == StringX::npos)
      st_next = sx_temp.size();
    if (st_next > 0) {
      sx_extns = sx_temp.substr(st_startpos, st_next - st_startpos);
      for(StringX::size_type i = 1; i < sx_extns.size(); i++) {
        sx_extns[i] = _totlower(sx_extns[i]);
      }
      vextns.push_back(sx_extns);
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Just need drive, directory and file extension
  path = full_pgm.c_str();
  pws_os::splitpath(path, drive, dir, fname, extn);

  if (!extn.empty()) {
    // ends with '.x-x'
    sx_temp = StringX(extn.c_str());
    for (StringX::size_type i = 1; i < extn.size(); i++) {
      sx_temp[i] = _totlower(sx_temp[i]);
    }
    // Check if a known command extn
    if (std::find(vextns.begin(), vextns.end(), sx_temp) != vextns.end()) {
      bsearch_extns = false;
    }
  }

  if (!drive.empty() || !dir.empty()) {
    // Don't search directories but do search extensions
    bsearch_dirs = false;
    if (drive.empty()) {
      // Drive not specified - so could be relative to current directory
      sx_temp = sx_cwd + full_pgm;
      if (pws_os::FileExists(sx_temp.c_str())) {
        full_pgm = sx_temp;
        bfound = true;
        goto exit;
      }
      // Doesn't exist - add on know program extensions
      for (StringX::size_type ie = 0; ie < vextns.size(); ie++) {
        sx_extns = vextns[ie];
        if (pws_os::FileExists((sx_temp + sx_extns).c_str())) {
          full_pgm = full_pgm + sx_extns;
          bfound = true;
          goto exit;
        }
      }
    } else {
      // Drive specified - so should be full path.
      // Check if file exists as-is
      if (pws_os::FileExists(full_pgm.c_str())) {
        bfound = true;
        goto exit;
      }
      // Doesn't exist - add on know program extensions
      for (StringX::size_type ie = 0; ie < vextns.size(); ie++) {
        sx_extns = vextns[ie];
        if (pws_os::FileExists((full_pgm + sx_extns).c_str())) {
          full_pgm = full_pgm + sx_extns;
          bfound = true;
          goto exit;
        }
      }
    }
  }

  // Now search directories!
  if (bsearch_dirs) {
    // Ensure directory ends in a '/'
    for (StringX::size_type id = 0; id < vpaths.size(); id++) {
      sx_dirs = vpaths[id];
      if (sx_dirs.empty()) {
        // Prevent out of bounds if string is empty
        sx_dirs += _T("\\");
      } else
      if (sx_dirs[sx_dirs.length() - 1] != _T('\\'))
        sx_dirs += _T("\\");

      if (bsearch_extns) {
        for (StringX::size_type ie = 0; ie < vextns.size(); ie++) {
          sx_extns = vextns[ie];
          if (pws_os::FileExists((sx_dirs + full_pgm + sx_extns).c_str())) {
            full_pgm = sx_dirs + full_pgm + sx_extns;
            bfound = true;
            goto exit;
          }
        }
      } else {
        if (pws_os::FileExists(stringT((sx_dirs + full_pgm).c_str()))) {
          full_pgm = sx_dirs + full_pgm;
          bfound = true;
          goto exit;
        }
      }
    }
  }

  // If not found directly or within current directory structure, 
  // we so need to search registry.
  // Either: we had to search extensions - 
  //    so full_pgm does not end with known program extn;
  // Or: we didn't have to search -
  //    so full_pgm may end with '.exe' and we must not add
  if (!bfound &&
      (bsearch_extns || (!bsearch_extns && extn == _T(".exe")))) {
    // Check via registry
    if (bsearch_extns)
      full_pgm += _T(".exe");

    // Look for registry key
    bool bexists;
    HKEY hSubkey;
    StringX csSubkey = StringX(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\")) +
      full_pgm;
    bexists = (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              csSubkey.c_str(),
                              0L,
                              KEY_READ,
                              &hSubkey) == ERROR_SUCCESS);
    if (bexists) {
      LONG rv;
      DWORD dwType, dwDataLen(0);
      rv = ::RegQueryValueEx(hSubkey,
                             _T("Path"),
                             NULL,
                             &dwType,
                             NULL,
                             &dwDataLen);
      if (rv == ERROR_SUCCESS && dwType == REG_SZ) {
        dwDataLen++;
        TCHAR *pData = new TCHAR[dwDataLen];
        ::memset(pData, 0, dwDataLen);
        rv = ::RegQueryValueEx(hSubkey,
                               _T("Path"),
                               NULL,
                               &dwType,
                               LPBYTE(pData),
                               &dwDataLen);

        if (rv == ERROR_SUCCESS) {
          sx_temp = pData;
          StringX::size_type len = sx_temp.length();
          if (sx_temp[len - 1] == _T(';'))
            sx_temp = sx_temp.substr(0, len - 1) + _T('\\');
          else
            if (sx_temp[len - 1] != _T('\\') && sx_temp[len - 1] != _T('/'))
              sx_temp = sx_temp + _T('\\');
          full_pgm =  sx_temp + full_pgm;
          bfound = true;
        }

        delete[] pData;
      } // Get the value
      ::RegCloseKey(hSubkey);
    }
  }

 exit:
  return full_pgm;
}
