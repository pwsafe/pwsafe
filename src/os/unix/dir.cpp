/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of dir.h
 */
#include "../dir.h"
#include "../utf8conv.h" // for pws_os::towc
#include "../env.h"
#include <unistd.h>
#include <limits.h>
#include <cassert>
#include <cstdlib>
#include <cstring> // for memset
#include <sys/stat.h>

stringT pws_os::getexecdir()
{
  char path[PATH_MAX];

  std::memset(path, 0, sizeof(path)); // keep valgrind happy
  if (::readlink("/proc/self/exe", path, PATH_MAX) < 0)
    return _T("?");
  else {
    stringT retval(pws_os::towc(path));
    stringT::size_type last_slash = retval.find_last_of(_T('/'));
    return retval.substr(0, last_slash + 1);
  }
}

stringT pws_os::getcwd()
{
  char curdir[PATH_MAX];
  if (::getcwd(curdir, PATH_MAX) == nullptr) {
    curdir[0] = '?'; curdir[1] = '\0';
  }
  stringT CurDir(pws_os::towc(curdir));
  return CurDir;
}

bool pws_os::chdir(const stringT &dir)
{
  assert(!dir.empty());
  const char *szdir = nullptr;
  size_t N = std::wcstombs(nullptr, dir.c_str(), 0) + 1;
  assert(N > 0);
  szdir = new char[N];
  std::wcstombs(const_cast<char *>(szdir), dir.c_str(), N);
  bool retval = (::chdir(szdir) == 0);
  delete[] szdir;
  return retval;
}

bool pws_os::splitpath(const stringT &path,
                       stringT &drive, stringT &dir,
                       stringT &file, stringT &ext)
{
  if (path.empty())
    return false;

  stringT::size_type last_slash = path.find_last_of(_T('/'));
  dir = path.substr(0, last_slash + 1);
  if (dir.empty())
    dir = _T("./");
  drive = (dir[0] == '/') ? _T("") : _T("./");
  stringT::size_type last_dot = path.find_last_of(_T('.'));
  if (last_dot != stringT::npos && last_dot > last_slash) {
    file = path.substr(last_slash + 1, last_dot - last_slash - 1);
    ext = path.substr(last_dot);
  } else {
    file = path.substr(last_slash + 1);
    ext = _T("");
  }
  return true;
}

stringT pws_os::makepath(const stringT &drive, const stringT &dir,
                         const stringT &file, const stringT &ext)
{
  stringT retval;
  /**
   * drive and dir can semantically be "./" with no ill effect,
   * but wxFileDialog doesn't like a "filename" with "/"s.
   */
  if (drive != L"./")
    retval = drive;
  if (dir != L"./") {
    retval += dir;
    if (!dir.empty() && retval[retval.length()-1] != '/')
      retval += _T("/");
  }
  retval += file;
  if (!ext.empty()) {
    if (ext[0] != L'.')
      retval += _T(".");
    retval += ext;
  }
  return retval;
}

stringT pws_os::fullpath(const stringT &relpath)
{
  stringT retval;
  char full[PATH_MAX];

  // relpath -> char *path
  size_t N = std::wcstombs(nullptr, relpath.c_str(), 0) + 1;
  assert(N > 0);
  char *path = new char[N];
  std::wcstombs(path, relpath.c_str(), N);

  if (realpath(path, full) != nullptr) {
    // full -> retval
    size_t wfull_len = ::mbstowcs(nullptr, full, 0) + 1;
    wchar_t *wfull = new wchar_t[wfull_len];
    std::mbstowcs(wfull, full, wfull_len);
    retval = wfull;
    delete[] wfull;
  }
  delete[] path;
  return retval;
}

static bool direxists(const stringT &path, bool createIfNeeded)
{
      struct stat statbuf;
      bool retval = false;
      int status = ::lstat(pws_os::tomb(path).c_str(), &statbuf);

      if (status == 0 && (S_ISDIR(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)))
        retval = true;
      
      // no existing dir, create one if so instructed
      if (!retval && createIfNeeded) {
        const mode_t oldmode = umask(0);
        retval = (mkdir(pws_os::tomb(path).c_str(), S_IRUSR|S_IWUSR|S_IXUSR) == 0);
        umask(oldmode);
      }
      return retval;
}


static stringT createuserprefsdir(void)
{
  /**
   * (1) We start by checking if ~/.pwsafe exists, as this was the default until FR902. If it exists, we use it.
   * (2) If not, then we try to respect Freedesktop.org's XDG Base Directory Specification:
   *       If $XDG_CONFIG_HOME is set, then we'll use $XDG_CONFIG_HOME/pwsafe, creating it if needed
   * (3) If ~/.pwsafe doesn't exist and $XDG_CONFIG_HOME isn't set, then:
   *       We test for ~/.config, creating if needed, then we test for ~/.config/pwsafe, creating if needed
  */

  stringT cfgdir = pws_os::getenv("HOME", true);

  if (!cfgdir.empty()) { // if $HOME's not defined, we have bigger problems...
 
    // (1)
    cfgdir += _T(".pwsafe");

    if (direxists(cfgdir, false))
      return cfgdir + _T("/");

    // (2)
    cfgdir = pws_os::getenv("XDG_CONFIG_HOME", true);
    if (!cfgdir.empty()) {
      cfgdir += _T("/pwsafe");
      if (direxists(cfgdir, true))
        return cfgdir + _T("/");
    }

    // (3)
    cfgdir = pws_os::getenv("HOME", true) + _T("/.config");
    if (direxists(cfgdir, true)) {
      cfgdir += _T("/pwsafe");
      if (direxists(cfgdir, true))
        return cfgdir + _T("/");
    }
  }

  return _T("");
}

stringT pws_os::getuserprefsdir(void)
{
  /**
   * Returns preference directory, creating it if needed.
   * See createuserprefsdir() for description of logic.
   * If creation failed, return empty string, caller
   * will fallback to something else or fail gracefully.
   * Note that we use a static string so that createuserprefsdir() is called exactly once.
   */
  static const stringT cfgdir = createuserprefsdir();
  return cfgdir;
}

stringT pws_os::getsafedir(void)
{
  return getuserprefsdir(); // same-same on linux
}

stringT pws_os::getxmldir(void)
{
   stringT xmldir = pws_os::getenv("PWS_XMLDIR", true);
  if (xmldir.empty()) {
#ifdef __FreeBSD__
  xmldir = _T("/usr/local/share/pwsafe/xml/");
#else
  xmldir = _T("/usr/share/passwordsafe/xml/");
#endif
  }
  return xmldir;
}

stringT pws_os::gethelpdir(void)
{
  stringT helpdir = pws_os::getenv("PWS_HELPDIR", true);
  if (helpdir.empty()) {
#if defined( __FreeBSD__) || defined(__OpenBSD)
    helpdir = _T("/usr/local/share/doc/passwordsafe/help/");
#else
    helpdir = _T("/usr/share/passwordsafe/help/");
#endif
  }
  return helpdir;
}
