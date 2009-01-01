/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __FILE_H
#define __FILE_H
#include "typedefs.h"
#include <cstdio>
#include <vector>

namespace pws_os {
  extern bool FileExists(const stringT &filename);
  extern bool FileExists(const stringT &filename, bool &bReadOnly);
  extern bool RenameFile(const stringT &oldname, const stringT &newname);
  extern bool CopyAFile(const stringT &from, const stringT &to); // creates dirs as needed!
  extern bool DeleteAFile(const stringT &filename);
  extern void FindFiles(const stringT &filter, std::vector<stringT> &res);
  extern bool LockFile(const stringT &filename, stringT &locker,
                       HANDLE &lockFileHandle, int &LockCount);
  extern bool IsLockedFile(const stringT &filename);
  extern void UnlockFile(const stringT &filename,
                         HANDLE &lockFileHandle, int &LockCount);

  extern std::FILE *FOpen(const stringT &filename, const TCHAR *mode);
  extern long fileLength(std::FILE *fp);
  extern const TCHAR *PathSeparator; // slash for Unix, backslash for Windows
};
#endif /* __FILE_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
