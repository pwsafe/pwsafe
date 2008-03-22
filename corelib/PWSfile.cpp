/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "PWSfile.h"
#include "PWSfileV1V2.h"
#include "PWSfileV3.h"
#include "SysInfo.h"
#include "corelib.h"

#include "sha1.h" // for simple encrypt/decrypt
#include "PWSrand.h" // ditto

#include <LMCONS.H> // for UNLEN definition
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <vector>

using namespace std;

PWSfile *PWSfile::MakePWSfile(const CMyString &a_filename, VERSION &version,
                              RWmode mode, int &status)
{
  if (mode == Read && !FileExists(a_filename)) {
    status = CANT_OPEN_FILE;
    return NULL;
  }

  switch (version) {
    case V17:
    case V20:
      status = SUCCESS;
      return new PWSfileV1V2(a_filename, mode, version);
    case V30:
      status = SUCCESS;
      return new PWSfileV3(a_filename, mode, version);
    case UNKNOWN_VERSION:
      ASSERT(mode == Read);
      if (PWSfile::ReadVersion(a_filename) == V30) {
        version = V30;
        status = SUCCESS;
        return new PWSfileV3(a_filename, mode, version);
      } else {
        version = V20; // may be inaccurate (V17)
        status = SUCCESS;
        return new PWSfileV1V2(a_filename, mode, version);
      }
    default:
      ASSERT(0);
      status = FAILURE; return NULL;
  }
}

bool PWSfile::FileExists(const CMyString &filename)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  return (status == 0);
}

bool PWSfile::FileExists(const CMyString &filename, bool &bReadOnly)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);

  // As "stat" gives "user permissions" not "file attributes"....
  if (status == 0) {
    DWORD dwAttr = GetFileAttributes(filename);
    bReadOnly = (FILE_ATTRIBUTE_READONLY & dwAttr) == FILE_ATTRIBUTE_READONLY;
    return true;
  } else {
    bReadOnly = false;
    return false;
  }
}
void PWSfile::FileError(int formatRes, int cause)
{
  // present error from FileException to user
  CString cs_error, cs_msg;

  ASSERT(cause >= 0 && cause <= 14);
  cs_error.LoadString(IDSC_FILEEXCEPTION00 + cause);
  cs_msg.Format(formatRes, cs_error);
  AfxMessageBox(cs_msg, MB_OK);
}

PWSfile::VERSION PWSfile::ReadVersion(const CMyString &filename)
{
  if (FileExists(filename)) {
    VERSION v;
    if (PWSfileV3::IsV3x(filename, v))
      return v;
    else
      return V20;
  } else
    return UNKNOWN_VERSION;
}

int PWSfile::RenameFile(const CMyString &oldname, const CMyString &newname)
{
  _tremove(newname); // otherwise rename will fail if newname exists
  int status = _trename(oldname, newname);

  return (status == 0) ? SUCCESS : FAILURE;
}

PWSfile::PWSfile(const CMyString &filename, RWmode mode)
  : m_filename(filename), m_passkey(_T("")),  m_defusername(_T("")),
  m_curversion(UNKNOWN_VERSION), m_rw(mode),
  m_fd(NULL), m_fish(NULL), m_terminal(NULL),
  m_nRecordsWithUnknownFields(0)
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
}

PWSfile::HeaderRecord::HeaderRecord()
  : m_nCurrentMajorVersion(0), m_nCurrentMinorVersion(0),
  m_nITER(0), m_prefString(_T("")), m_whenlastsaved(0),
  m_lastsavedby(_T("")), m_lastsavedon(_T("")),
  m_whatlastsaved(_T("")),
  m_dbname(_T("")), m_dbdesc(_T(""))
{
  memset(m_file_uuid_array, 0x00, sizeof(m_file_uuid_array));
}

PWSfile::HeaderRecord::HeaderRecord(const PWSfile::HeaderRecord &h) 
  : m_nCurrentMajorVersion(h.m_nCurrentMajorVersion),
  m_nCurrentMinorVersion(h.m_nCurrentMinorVersion),
  m_nITER(h.m_nITER), m_displaystatus(h.m_displaystatus),
  m_prefString(h.m_prefString), m_whenlastsaved(h.m_whenlastsaved),
  m_lastsavedby(h.m_lastsavedby), m_lastsavedon(h.m_lastsavedon),
  m_whatlastsaved(h.m_whatlastsaved),
  m_dbname(h.m_dbname), m_dbdesc(h.m_dbdesc)
{
  memcpy(m_file_uuid_array, h.m_file_uuid_array,
    sizeof(m_file_uuid_array));
}

PWSfile::HeaderRecord &PWSfile::HeaderRecord::operator=(const PWSfile::HeaderRecord &h)
{
  if (this != &h) {
    m_nCurrentMajorVersion = h.m_nCurrentMajorVersion;
    m_nCurrentMinorVersion = h.m_nCurrentMinorVersion;
    m_nITER = h.m_nITER;
    m_displaystatus = h.m_displaystatus;
    m_prefString = h.m_prefString;
    m_whenlastsaved = h.m_whenlastsaved;
    m_lastsavedby = h.m_lastsavedby;
    m_lastsavedon = h.m_lastsavedon;
    m_whatlastsaved = h.m_whatlastsaved;
    m_dbname = h.m_dbname;
    m_dbdesc = h.m_dbdesc;
    memcpy(m_file_uuid_array, h.m_file_uuid_array,
      sizeof(m_file_uuid_array));
  }
  return *this;
}

void PWSfile::FOpen()
{
  TCHAR* m = (m_rw == Read) ? _T("rb") : _T("wb");
  // calls right variant of m_fd = fopen(m_filename);
#if _MSC_VER >= 1400
  _tfopen_s(&m_fd, (LPCTSTR) m_filename, m);
#else
  m_fd = _tfopen((LPCTSTR) m_filename, m);
#endif
}

int PWSfile::Close()
{
  delete m_fish;
  m_fish = NULL;
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
  }
  return SUCCESS;
}

size_t PWSfile::WriteCBC(unsigned char type, const unsigned char *data,
                         unsigned int length)
{
  ASSERT(m_fish != NULL && m_IV != NULL);
  return _writecbc(m_fd, data, length, type, m_fish, m_IV);
}

size_t PWSfile::ReadCBC(unsigned char &type, unsigned char* &data,
                        unsigned int &length)
{
  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  size_t retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type,
    m_fish, m_IV, m_terminal);

  if (buffer_len > 0) {
    if (buffer_len < length || data == NULL)
      length = buffer_len; // set to length read
    // if buffer_len > length, data is truncated to length
    // probably an error.
    if (data != NULL) {
      memcpy(data, buffer, length);
      trashMemory(buffer, buffer_len);
      delete[] buffer;
    } else { // NULL data means pass buffer directly to caller
      data = buffer; // caller must trash & delete[]!
    }
  } else {
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}

int PWSfile::CheckPassword(const CMyString &filename,
                           const CMyString &passkey, VERSION &version)
{
  int status;

  version = UNKNOWN_VERSION;
  status = PWSfileV3::CheckPassword(filename, passkey);
  if (status == SUCCESS)
    version = V30;
  if (status == NOT_PWS3_FILE) {
    status = PWSfileV1V2::CheckPassword(filename, passkey);
    if (status == SUCCESS)
      version = V20; // or V17?
  }
  return status;
}

/*
* The file lock/unlock functions were first implemented (in 2.08)
* with Posix semantics (using open(_O_CREATE|_O_EXCL) to detect
* an existing lock.
* This fails to check liveness of the locker process, specifically,
* if a user just turns of her PC, the lock file will remain.
* So, I'm keeping the Posix code under idef POSIX_FILE_LOCK,
* and re-implementing using the Win32 API, whose semantics
* supposedly protect against this scenario.
* Thanks to Frank (xformer) for discussion on the subject.
*/

/*
* Originally, the lock/unlock functions were designed for working with the
* passsword database file alone.
* As of 3.05, there's a need to lock the application's configuration
* file as well, potentially concurrently with the database file.
* Problem is, we need to keep state information (HANDLE, LockCount) per
* locked file. since the filenames are unique, indeally we'd maintain a map
* between the filename and the resources. For now, we require the application
* to differentiate between them for us by the bool param bDB. Less elegant,
* but *much* easier than setting up a map...
*/

static void GetLockFileName(const CMyString &filename,
                            CMyString &lock_filename)
{
  ASSERT(!filename.IsEmpty());
  // derive lock filename from filename
  lock_filename = CMyString(filename.Left(MAX_PATH - 4) + _T(".plk"));
}

bool PWSfile::LockFile(const CMyString &filename, CMyString &locker, 
                       HANDLE &lockFileHandle, int &LockCount)
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  int fh = _open(lock_filename, (_O_CREAT | _O_EXCL | _O_WRONLY),
    (_S_IREAD | _S_IWRITE));

  if (fh == -1) { // failed to open exclusively. Already locked, or ???
    switch (errno) {
      case EACCES:
        // Tried to open read-only file for writing, or file’s
        // sharing mode does not allow specified operations, or given path is directory
        locker.LoadString(IDSC_NOLOCKACCESS);
        break;
      case EEXIST: // filename already exists
      {
        // read locker data ("user@machine:nnnnnnnn") from file
        TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR) * 11];
        int fh2 = _open(lock_filename, _O_RDONLY);
        if (fh2 == -1) {
          locker.LoadString(IDSC_CANTGETLOCKER);
        } else {
          int bytesRead = _read(fh2, lockerStr, sizeof(lockerStr)-1);
          _close(fh2);
          if (bytesRead > 0) {
            lockerStr[bytesRead] = TCHAR('\0');
            locker = lockerStr;
          } else { // read failed for some reason
            locker = _T("Unable to read locker?");
          } // read info from lock file
        } // open lock file for read
        break;
      } // EEXIST block
      case EINVAL: // Invalid oflag or pmode argument
        locker.LoadString(IDSC_INTERNALLOCKERROR);
        break;
      case EMFILE: // No more file handles available (too many open files)
        locker.LoadString(IDSC_SYSTEMLOCKERROR);
        break;
      case ENOENT: //File or path not found
        locker.LoadString(IDSC_LOCKFILEPATHNF);
        break;
      default:
        locker.LoadString(IDSC_LOCKUNKNOWNERROR);
        break;
    } // switch (errno)
    return false;
  } else { // valid filehandle, write our info
    int numWrit;
    numWrit = _write(fh, m_user, m_user.GetLength() * sizeof(TCHAR));
    numWrit += _write(fh, _T("@"), sizeof(TCHAR));
    numWrit += _write(fh, m_sysname, m_sysname.GetLength() * sizeof(TCHAR));
    numWrit += _write(fh, _T(":"), sizeof(TCHAR));
    numWrit += _write(fh, m_ProcessID, m_ProcessID.GetLength() * sizeof(TCHAR));
    ASSERT(numWrit > 0);
    _close(fh);
    return true;
  }
#else
  const SysInfo *si = SysInfo::GetInstance();
  const stringT user = si->GetRealUser();
  const stringT host = si->GetRealHost();
  const stringT pid = si->GetCurrentPID();

  TCHAR fname[_MAX_FNAME];
  TCHAR ext[_MAX_EXT];
#if _MSC_VER >= 1400
  _tsplitpath_s(lock_filename, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
#else
  _tsplitpath(lock_filename, NULL, NULL, fname, ext);
#endif

  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (lockFileHandle != INVALID_HANDLE_VALUE) {
    // here if we've open another (or same) dbase previously,
    // need to unlock it. A bit inelegant...
    // If app was minimized and ClearData() called, we've a small
    // potential for a TOCTTOU issue here. Worse case, lock
    // will fail.

    const stringT cs_me = user + _T("@") + host + _T(":") + pid;
    GetLockFileName(filename, lock_filename);
    GetLocker(lock_filename, locker);

    if (cs_me == stringT(locker)) {
      LockCount++;
      TRACE(_T("%s Lock1  ; Count now %d; File: %s%s\n"), 
        PWSUtil::GetTimeStamp(), LockCount, fname, ext);
      locker.Empty();
      return true;
    } else {
      // XXX UnlockFile(bDB ? GetCurFile() : filename, bDB);
      UnlockFile(filename, lockFileHandle, LockCount);
    }
  }
  lockFileHandle = ::CreateFile(LPCTSTR(lock_filename),
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS, // rely on share to fail if exists!
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | 
                                // (Lockheed Martin) Secure Coding  11-14-2007
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                                NULL);

	// Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
	if (lockFileHandle != INVALID_HANDLE_VALUE) {
		if (::GetFileType( lockFileHandle ) != FILE_TYPE_DISK) {
			::CloseHandle( lockFileHandle );
			lockFileHandle = INVALID_HANDLE_VALUE;
		}
	}
	// End of Change.  (Lockheed Martin) Secure Coding  11-14-2007

  if (lockFileHandle == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    switch (error) {
      case ERROR_SHARING_VIOLATION: // already open by a live process
        GetLocker(lock_filename, locker);
        break;
      default:
        locker = _T("Cannot create lock file - no permission in directory?");
        break;
    } // switch (error)
    return false;
  } else { // valid filehandle, write our info
    DWORD numWrit, sumWrit;
    BOOL write_status;
    write_status = ::WriteFile(lockFileHandle,
                               CString(user.c_str()), user.length() * sizeof(TCHAR),
                               &sumWrit, NULL);
    write_status &= ::WriteFile(lockFileHandle,
                                _T("@"), sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                CString(host.c_str()), host.length() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                _T(":"), sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                CString(pid.c_str()), pid.length() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    ASSERT(sumWrit > 0);
    LockCount++;
    TRACE(_T("%s Lock1  ; Count now %d; File Created; File: %s%s\n"), 
      PWSUtil::GetTimeStamp(), LockCount, fname, ext);
    return (write_status == TRUE);
  }
#endif // POSIX_FILE_LOCK
}

void PWSfile::UnlockFile(const CMyString &filename,
                         HANDLE &lockFileHandle, int &LockCount)
{
#ifdef POSIX_FILE_LOCK
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
  _unlink(lock_filename);
#else
  const SysInfo *si = SysInfo::GetInstance();
  const stringT user = si->GetRealUser();
  const stringT host = si->GetRealHost();
  const stringT pid = si->GetCurrentPID();

  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (lockFileHandle != INVALID_HANDLE_VALUE) {
    CMyString lock_filename, locker;
    const stringT cs_me = user + _T("@") + host + _T(":") + pid;
    GetLockFileName(filename, lock_filename);
    GetLocker(lock_filename, locker);

    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
#if _MSC_VER >= 1400
    _tsplitpath_s(lock_filename, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
#else
    _tsplitpath(lock_filename, NULL, NULL, fname, ext);
#endif

    if (cs_me == stringT(locker) && LockCount > 1) {
      LockCount--;
      TRACE(_T("%s Unlock2; Count now %d; File: %s%s\n"), 
        PWSUtil::GetTimeStamp(), LockCount, fname, ext);
    } else {
      LockCount = 0;
      TRACE(_T("%s Unlock1; Count now %d; File Deleted; File: %s%s\n"), 
        PWSUtil::GetTimeStamp(), LockCount, fname, ext);
      CloseHandle(lockFileHandle);
      lockFileHandle = INVALID_HANDLE_VALUE;
      DeleteFile(LPCTSTR(lock_filename));
    }
  }
#endif // POSIX_FILE_LOCK
}

bool PWSfile::IsLockedFile(const CMyString &filename)
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  return PWSfile::FileExists(lock_filename);
#else
  // under this scheme, we need to actually try to open the file to determine
  // if it's locked.
  HANDLE h = CreateFile(LPCTSTR(lock_filename),
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING, // don't create one!
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH |
                        // (Lockheed Martin) Secure Coding  11-14-2007
                        SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                        NULL);
 
 	// Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
 	if (h != INVALID_HANDLE_VALUE) {
 		if (::GetFileType( h ) != FILE_TYPE_DISK) {
 			::CloseHandle( h );
 			h = INVALID_HANDLE_VALUE;
 		}
 	}
 	// End of Change.  (Lockheed Martin) Secure Coding  11-14-2007
 
  if (h == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION)
      return true;
    else
      return false; // couldn't open it, probably doesn't exist.
  } else {
    CloseHandle(h); // here if exists but lockable.
    return false;
  }
#endif // POSIX_FILE_LOCK
}

bool PWSfile::GetLocker(const CMyString &lock_filename, CMyString &locker)
{
  bool bResult = false;
  // read locker data ("user@machine:nnnnnnnn") from file
  TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + 11];
  // flags here counter (my) intuition, but see
  // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp
  HANDLE h2 = ::CreateFile(LPCTSTR(lock_filename),
                           GENERIC_READ,
                           FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           (FILE_ATTRIBUTE_NORMAL |
                            // (Lockheed Martin) Secure Coding  11-14-2007
                            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION),
                           NULL);
 	// Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
 	if (h2 != INVALID_HANDLE_VALUE) {
 		if (::GetFileType( h2 ) != FILE_TYPE_DISK) {
 			::CloseHandle( h2 );
 			h2 = INVALID_HANDLE_VALUE;
 		}
 	}
 	// End of Change.  (Lockheed Martin) Secure Coding  11-14-2007
 
  if (h2 == INVALID_HANDLE_VALUE) {
    locker.LoadString(IDSC_CANTGETLOCKER);
  } else {
    DWORD bytesRead;
    (void)::ReadFile(h2, lockerStr, sizeof(lockerStr)-1,
                     &bytesRead, NULL);
    CloseHandle(h2);
    if (bytesRead > 0) {
      lockerStr[bytesRead/sizeof(TCHAR)] = TCHAR('\0');
      locker = lockerStr;
      bResult = true;
    } else { // read failed for some reason
      locker.LoadString(IDSC_CANTREADLOCKER);
    } // read info from lock file
  }
  return bResult;
}

void PWSfile::GetUnknownHeaderFields(UnknownFieldList &UHFL)
{
  if (!m_UHFL.empty())
    UHFL = m_UHFL;
  else
    UHFL.clear();
}

void PWSfile::SetUnknownHeaderFields(UnknownFieldList &UHFL)
{
  if (!UHFL.empty())
    m_UHFL = UHFL;
  else
    m_UHFL.clear();
}

// Following for 'legacy' use of pwsafe as file encryptor/decryptor

//Complain if the file has not opened correctly

static void
ErrorMessages(const CString &fn, FILE *fp)
{
  if (fp == NULL) {
    CString cs_text;

    switch (errno) {
      case EACCES:
        cs_text.LoadString(IDSC_FILEREADONLY);
        break;
      case EEXIST:
        cs_text.LoadString(IDSC_FILEEXISTS);
        break;
      case EINVAL:
        cs_text.LoadString(IDSC_INVALIDFLAG);
        break;
      case EMFILE:
        cs_text.LoadString(IDSC_NOMOREHANDLES);
        break;
      case ENOENT:
        cs_text.LoadString(IDSC_FILEPATHNOTFOUND);
        break;
      default:
        break;
    }

    CString cs_title = _T("Password Safe - ") + fn;
    AfxGetMainWnd()->MessageBox(cs_text, cs_title, MB_ICONEXCLAMATION|MB_OK);
  }
}

bool PWSfile::Encrypt(const CString &fn, const CMyString &passwd)
{
  unsigned int len;
  unsigned char* buf;

  FILE *in;
#if _MSC_VER >= 1400
  _tfopen_s(&in, fn, _T("rb"));
#else
  in = _tfopen(fn, _T("rb"));
#endif
  if (in != NULL) {
    len = PWSUtil::fileLength(in);
    buf = new unsigned char[len];

    fread(buf, 1, len, in);
    fclose(in);
  } else {
    ErrorMessages(fn, in);
    return false;
  }

  CString out_fn = fn;
  out_fn += CIPHERTEXT_SUFFIX;

  FILE *out;
#if _MSC_VER >= 1400
  _tfopen_s(&out, out_fn, _T("wb"));
#else
  out = _tfopen(out_fn, _T("wb"));
#endif
  if (out != NULL) {
#ifdef KEEP_FILE_MODE_BWD_COMPAT
    fwrite( &len, 1, sizeof(len), out);
#else
    unsigned char randstuff[StuffSize];
    unsigned char randhash[SHA1::HASHLEN];   // HashSize
    PWSrand::GetInstance()->GetRandomData( randstuff, 8 );
    // miserable bug - have to fix this way to avoid breaking existing files
    randstuff[8] = randstuff[9] = TCHAR('\0');
    GenRandhash(passwd, randstuff, randhash);
    fwrite(randstuff, 1,  8, out);
    fwrite(randhash,  1, sizeof(randhash), out);
#endif // KEEP_FILE_MODE_BWD_COMPAT

    unsigned char thesalt[SaltLength];
    PWSrand::GetInstance()->GetRandomData( thesalt, SaltLength );
    fwrite(thesalt, 1, SaltLength, out);

    unsigned char ipthing[8];
    PWSrand::GetInstance()->GetRandomData( ipthing, 8 );
    fwrite(ipthing, 1, 8, out);

    unsigned char *pwd = NULL;
    int passlen = 0;
    ConvertString(passwd, pwd, passlen);
    Fish *fish = BlowFish::MakeBlowFish(pwd, passlen, thesalt, SaltLength);
    trashMemory(pwd, passlen);
#ifdef UNICODE
    delete[] pwd; // gross - ConvertString allocates only if UNICODE.
#endif
    _writecbc(out, buf, len, (unsigned char)0, fish, ipthing);
    delete fish;
    fclose(out);

  } else {
    ErrorMessages(out_fn, out);
    delete [] buf;
    return false;
  }
  delete[] buf;
  return true;
}

bool PWSfile::Decrypt(const CString &fn, const CMyString &passwd)
{
  unsigned int len;
  unsigned char* buf;

  FILE *in;
#if _MSC_VER >= 1400
  _tfopen_s(&in, fn, _T("rb"));
#else
  in = _tfopen(fn, _T("rb"));
#endif
  if (in != NULL) {
    unsigned char salt[SaltLength];
    unsigned char ipthing[8];
    unsigned char randstuff[StuffSize];
    unsigned char randhash[SHA1::HASHLEN];

#ifdef KEEP_FILE_MODE_BWD_COMPAT
    fread(&len, 1, sizeof(len), in); // XXX portability issue
#else
    fread(randstuff, 1, 8, in);
    randstuff[8] = randstuff[9] = TCHAR('\0'); // ugly bug workaround
    fread(randhash, 1, sizeof(randhash), in);

    unsigned char temphash[SHA1::HASHLEN];
    GenRandhash(passwd, randstuff, temphash);
    if (memcmp((char*)randhash, (char*)temphash, SHA1::HASHLEN != 0)) {
      fclose(in);
      AfxMessageBox(IDSC_BADPASSWORD);
      return false;
    }
#endif // KEEP_FILE_MODE_BWD_COMPAT
    buf = NULL; // allocated by _readcbc - see there for apologia

    fread(salt,    1, SaltLength, in);
    fread(ipthing, 1, 8,          in);

    unsigned char dummyType;
    unsigned char *pwd = NULL;
    int passlen = 0;
    ConvertString(passwd, pwd, passlen);
    Fish *fish = BlowFish::MakeBlowFish(pwd, passlen, salt, SaltLength);
    trashMemory(pwd, passlen);
#ifdef UNICODE
    delete[] pwd; // gross - ConvertString allocates only if UNICODE.
#endif
    if (_readcbc(in, buf, len,dummyType, fish, ipthing) == 0) {
      delete fish;
      delete[] buf; // if not yet allocated, delete[] NULL, which is OK
      return false;
    }
    delete fish;
    fclose(in);
  } else {
    ErrorMessages(fn, in);
    return false;
  }

  size_t suffix_len = strlen(CIPHERTEXT_SUFFIX);
  size_t filepath_len = fn.GetLength();

  CString out_fn = fn;
  out_fn = out_fn.Left(static_cast<int>(filepath_len - suffix_len));

#if _MSC_VER >= 1400
  FILE *out;
  _tfopen_s(&out, out_fn, _T("wb"));
#else
  FILE *out = _tfopen(out_fn, _T("wb"));
#endif
  if (out != NULL) {
    fwrite(buf, 1, len, out);
    fclose(out);
  } else
    ErrorMessages(out_fn, out);

  delete[] buf; // allocated by _readcbc
  return true;
}
