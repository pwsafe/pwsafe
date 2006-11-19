/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __PWSFILE_H
#define __PWSFILE_H
// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#include <stdio.h> // for FILE *

#include "ItemData.h"
#include "MyString.h"

class Fish;

class PWSfile {
 public:
  enum VERSION {V17, V20, V30, VCURRENT = V30,
		UNKNOWN_VERSION = 99}; // supported file versions: V17 is last pre-2.0
  enum RWmode {Read, Write};
  enum {SUCCESS = 0, FAILURE = 1, 
  		CANT_OPEN_FILE,					//  2
        UNSUPPORTED_VERSION,			//  3
        WRONG_VERSION,					//  4
        NOT_PWS3_FILE,					//  5
        WRONG_PASSWORD,					//  6 - see PWScore.h
        BAD_DIGEST,						//  7 - see PWScore.h
        END_OF_FILE						//  8
  };

  static PWSfile *MakePWSfile(const CMyString &a_filename, VERSION &version,
                              RWmode mode, int &status);

  static bool FileExists(const CMyString &filename);
  static bool FileExists(const CMyString &filename, bool &bReadOnly);
  static void FileError(int formatRes, int cause);
  static VERSION ReadVersion(const CMyString &filename);
  static int RenameFile(const CMyString &oldname, const CMyString &newname);
  static int CheckPassword(const CMyString &filename,
                           const CMyString &passkey, VERSION &version);

  enum {DATABASE_LOCK = 0, CONFIG_LOCK};
  static bool LockFile(const CMyString &filename, CMyString &locker, const bool bDB = true);
  static bool IsLockedFile(const CMyString &filename, const bool bDB = true);
  static void UnlockFile(const CMyString &filename, const bool bDB = true);
  static bool GetLocker(const CMyString &filename, CMyString &locker);

  virtual ~PWSfile();

  virtual int Open(const CMyString &passkey) = 0;
  virtual int Close();

  virtual int WriteRecord(const CItemData &item) = 0;
  virtual int ReadRecord(CItemData &item) = 0;
  void SetDefUsername(const CMyString &du) {m_defusername = du;} // for V17 conversion (read) only
  // The prefstring is read/written along with the rest of the file,
  // see code for details on where it's kept.
  void SetPrefString(const CMyString &prefStr) {m_prefString = prefStr;}
  const CMyString &GetPrefString() const {return m_prefString;}
  void SetDisplayStatus(const CString &displaystatus) {m_file_displaystatus = displaystatus;}
  const CString &GetDisplayStatus() const {return m_file_displaystatus;}
  void SetUseUTF8(bool flag) { m_useUTF8 = flag; } // nop for v1v2
  void SetUserHost(const CString &user, const CString &sysname)
		{m_user = user; m_sysname = sysname;}
  void SetApplicationVersion(const DWORD dwMajorMinor) 
		{m_dwMajorMinor = dwMajorMinor;}
  const CString &GetWhenLastSaved() const {return m_whenlastsaved;}
  const CString &GetWhoLastSaved() const {return m_wholastsaved;}
  const CString &GetWhatLastSaved() const {return m_whatlastsaved;}
  unsigned short GetCurrentMajorVersion() const {return m_nCurrentMajorVersion;}
  unsigned short GetCurrentMinorVersion() const {return m_nCurrentMinorVersion;}
  void SetCurVersion(VERSION v) {m_curversion = v;}

 protected:
  PWSfile(const CMyString &filename, RWmode mode);
  void FOpen(); // calls right variant of m_fd = fopen(m_filename);
  virtual int WriteCBC(unsigned char type, const CString &data);
  virtual int WriteCBC(unsigned char type, const unsigned char *data,
                       unsigned int length);
  virtual int ReadCBC(unsigned char &type, CMyString &data);
  virtual int ReadCBC(unsigned char &type, unsigned char *data,
                      unsigned int &length);
  const CMyString m_filename;
  CMyString m_passkey;
  FILE *m_fd;
  VERSION m_curversion;
  unsigned short m_nCurrentMajorVersion, m_nCurrentMinorVersion;
  const RWmode m_rw;
  CMyString m_defusername; // for V17 conversion (read) only
  CMyString m_prefString; // prefererences stored in the file
  CString m_file_displaystatus; // tree display sttaus stored in file
  CString m_whenlastsaved; // When last saved
  CString m_wholastsaved; // and by whom
  CString m_whatlastsaved; // and by what
  CString m_user, m_sysname; // current user & host
  DWORD m_dwMajorMinor;
  unsigned char *m_IV; // points to correct m_ipthing for *CBC()
  Fish *m_fish;
  unsigned char *m_terminal;
  bool m_useUTF8; // turn off for none-unicode os's, e.g. win98
};
#endif /* __PWSFILE_H */
