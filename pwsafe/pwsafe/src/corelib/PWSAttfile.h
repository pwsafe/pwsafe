/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWSFILEA_H
#define __PWSFILEA_H

// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#include <stdio.h> // for FILE *
#include <vector>

#include "attachments.h"
#include "UUIDGen.h"
#include "UnknownField.h"
#include "StringX.h"
#include "Proxy.h"
#include "sha256.h"

#include "coredefs.h"

#define MIN_HASH_ITERATIONS 2048

#define ATT_DEFAULT_ATTMT_SUFFIX   _T(".psatt3")
#define ATT_DEFAULT_ATTBKUP_SUFFIX _T(".ibakatt3")
#define ATT_DEFAULT_ATTDUP_SUFFIX _T(".idupatt3")
#define ATT_DEFAULT_ATTIMP_SUFFIX _T(".iimpatt3")

typedef std::map<st_UUID, ATRecord> UUIDATRMap;
typedef UUIDATRMap::const_iterator UAMciter;
typedef UUIDATRMap::iterator UAMiter;

typedef std::multimap<st_UUID, ATRecord> UUIDATRMMap;
typedef UUIDATRMMap::const_iterator UAMMciter;
typedef UUIDATRMMap::iterator UAMMiter;

typedef std::vector<st_UUID> UUIDAVector;
typedef UUIDVector::iterator UViter;

class Fish;
class Asker;

class PWSAttfile
{
public:
  enum VERSION {V30, VCURRENT = V30,
    NEWFILE = 98,
    UNKNOWN_VERSION = 99};

  enum RWmode {Read, Write};

  /**
  * The format defines a handful of fields in the file's header
  * Since the application needs these after the PWSfile object's
  * lifetime, it makes sense to define a nested header structure that
  * the app. can keep a copy of, rather than duplicating
  * data members, getters and setters willy-nilly.
  */

  struct AttHeaderRecord {
    AttHeaderRecord();
    AttHeaderRecord(const AttHeaderRecord &ahr);
    AttHeaderRecord &operator=(const AttHeaderRecord &ahr);

    void Clear() {
      nITER = 0;
      nCurrentMajorVersion = nCurrentMinorVersion = 0;
      memset(attfile_uuid, 0 , sizeof(uuid_array_t));
      memset(DBfile_uuid, 0 , sizeof(uuid_array_t));
      whenlastsaved = (time_t)0;
      whatlastsaved.clear();
      lastsavedby.clear();
      lastsavedon.clear();
    }

    int nITER; // Formally not part of the header.

    unsigned short nCurrentMajorVersion, nCurrentMinorVersion;
    uuid_array_t attfile_uuid;
    uuid_array_t DBfile_uuid;

    time_t whenlastsaved;   // When last saved
    StringX whatlastsaved;  // and by what application
    StringX lastsavedby;    // and by whom
    StringX lastsavedon;    // and by which machine
  };

  enum AttachmentFields {
    ATTMT_UUID         = 0x01,
    ATTMT_ENTRY_UUID   = 0x02,
    ATTMT_FLAGS        = 0x03,
    ATTMT_FNAME        = 0x04,
    ATTMT_PATH         = 0x05,
    ATTMT_DESC         = 0x06,
    ATTMT_UNCSIZE      = 0x07,
    ATTMT_BLKSIZE      = 0x08,
    ATTMT_CTIME        = 0x09,
    ATTMT_ATIME        = 0x0a,
    ATTMT_MTIME        = 0x0b,
    ATTMT_DTIME        = 0x0c,
    ATTMT_LAST_PREDATA,

    ATTMT_DATA         = 0x80,
    ATTMT_LASTDATA     = 0x81,

    ATTMT_CMPSIZE      = 0xc0,
    ATTMT_CRC          = 0xc1,
    ATTMT_ODIGEST      = 0xc2,
    ATTMT_CDIGEST      = 0xc3,
    ATTMT_LAST_POSTDATA,

    ATTMT_END          = 0xff
  };

  static PWSAttfile *MakePWSfile(const StringX &a_filename, VERSION &version,
                                 RWmode mode, int &status,
                                 Asker *pAsker = NULL, Reporter *pReporter = NULL);

  static VERSION ReadVersion(const StringX &filename);
  static int CheckPasskey(const StringX &filename,
                          const StringX &passkey, VERSION &version);

  virtual ~PWSAttfile();

  virtual int Open(const StringX &passkey) = 0;
  virtual int Close();

  virtual int ReadAttmntRecordPreData(ATRecord &atr) = 0;
  virtual int ReadAttmntRecordData(unsigned char * &pCmpData, unsigned int &uiCmpLen,
                                   unsigned char &readtype, const bool bSkip = false) = 0;
  virtual int ReadAttmntRecordPostData(ATRecord &atr) = 0;

  virtual int WriteAttmntRecordPreData(const ATRecord &adr) = 0;
  virtual int WriteAttmntRecordData(unsigned char *pData, const unsigned int len,
                                    const unsigned char type) = 0;
  virtual int WriteAttmntRecordPostData(const ATRecord &adr) = 0;

  const AttHeaderRecord &GetHeader() const {return m_atthdr;}
  void SetHeader(const AttHeaderRecord &ah) {m_atthdr = ah;}

  void SetCurVersion(VERSION v) {m_curversion = v;}

protected:
  PWSAttfile(const StringX &filename, RWmode mode);
  void FOpen(); // calls right variant of m_fd = fopen(m_filename);
  virtual size_t WriteCBC(unsigned char type, const StringX &data) = 0;
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length);
  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         unsigned int &length,
                         bool bSkip = false, unsigned char *pSkipTypes = NULL);

  const StringX m_filename;
  StringX m_passkey;
  FILE *m_fd;
  VERSION m_curversion;
  const RWmode m_rw;
  unsigned char *m_IV; // points to correct m_ipthing for *CBC()
  Fish *m_fish;
  unsigned char *m_terminal;
  AttHeaderRecord m_atthdr;

  size_t m_fileLength;
  Asker *m_pAsker;
  Reporter *m_pReporter;
};

#endif /* __PWSFILEA_H */
