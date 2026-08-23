/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __FILE_H
#define __FILE_H
#include "typedefs.h"

#include "../core/StringX.h"

#include <cstdio>
#include <vector>

namespace pws_os {
  enum class RWmode { Read, Write };

  extern void AddDrive(stringT &path);
  extern bool FileExists(const stringT &filename);
  extern bool FileExists(const stringT &filename, bool &bReadOnly);
  extern bool RenameFile(const stringT &oldname, const stringT &newname);
  extern bool CopyAFile(const stringT &from, const stringT &to); // creates dirs as needed!
  extern bool DeleteAFile(const stringT &filename);
  extern void FindFiles(const stringT &filter, std::vector<stringT> &res);
  extern bool LockFile(const stringT &filename, stringT &locker,
                       HANDLE &lockFileHandle);
  extern bool IsLockedFile(const stringT &filename);
  extern void UnlockFile(const stringT &filename, HANDLE &lockFileHandle);
  extern void TryUnlockFile(const stringT &filename, HANDLE &lockFileHandle);

  extern std::FILE *FOpen(const stringT &filename, const TCHAR *mode);
  // Flush stdio and OS buffers to storage. A null stream is a no-op.
  // Returns zero on success and EOF on error.
  extern int FFlush(std::FILE *fd);

  inline int FClose(std::FILE *fd)
  {
    return fd != nullptr ? std::fclose(fd) : 0;
  }

  // FFlushAndClose is a convenience function that does as the name implies.
  // It's meant to be used when we want to make an extra effort to physically write data to storage,
  // e.g., when saving a database file.
  inline int FFlushAndClose(std::FILE *fd)
  {
    // Closing is unconditional, including when flushing fails.
    const int flushResult = FFlush(fd);
    const int closeResult = FClose(fd);
    return flushResult == 0 && closeResult == 0 ? 0 : EOF;
  }

  extern size_t fileLength(std::FILE *fp);
  extern bool GetFileTimes(const stringT &filename,
      time_t &ctime, time_t &mtime, time_t &atime);
  extern bool SetFileTimes(const stringT &filename,
      time_t ctime, time_t mtime, time_t atime);
  extern bool ProgramExists(const stringT &filename);
  extern const TCHAR PathSeparator; // slash for Unix, backslash for Windows
  extern bool RenameFile(const stringT &oldname, const stringT &newname);
  // Most stdio.h routines return -1 for an error (not INVALID_HANDLE_VALUE)
  #define INVALID_FILE_DESCRIPTOR (int)-1
}
#endif /* __FILE_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
