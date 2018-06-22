/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWSFILE_H
#define __PWSFILE_H

// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#include <stdio.h> // for FILE *
#include <vector>

#include "ItemData.h"
#include "os/UUID.h"
#include "UnknownField.h"
#include "PWSFilters.h"
#include "StringX.h"
#include "PWSfileHeader.h"
#include "Proxy.h"
#include "sha256.h"

#include "coredefs.h"

// HASH_ITERATIONS is used by the key stretching algorithm.
// MIN_HASH_ITERATIONS is a lower limit - anything lower than this
// is considered inherently insecure.
#define MIN_HASH_ITERATIONS 2048
// MAX_USABLE_HASH_ITERS is a guesstimate on what's acceptable to a user
// with a reasonably powerful CPU. Real limit's 2^32-1.
#define MAX_USABLE_HASH_ITERS (1 << 22)

#define V3_SUFFIX      _T("psafe3")
#define V4_SUFFIX      _T("psafe4")
// For now, default to V3, at least until we're sufficiently tested.
#define DEFAULT_SUFFIX      V3_SUFFIX

class Fish;
class Asker;

class PWSfile
{
public:
  enum VERSION {V17, V20, V30, V40, VCURRENT = V30,
    NEWFILE = 98,
    UNKNOWN_VERSION = 99}; // supported file versions: V17 is last pre-2.0

  enum RWmode {Read, Write};

  enum {SUCCESS = 0, FAILURE = 1, 
    UNSUPPORTED_VERSION,                     //  2
    WRONG_VERSION,                           //  3
    NOT_PWS3_FILE,                           //  4
    WRONG_PASSWORD,                          //  5 - see PWScore.h
    BAD_DIGEST,                              //  6 - see PWScore.h
    END_OF_FILE,                             //  7
    TRUNCATED_FILE,                          //  8 (missing EOF marker)
    READ_FAIL,                               //  9
    WRITE_FAIL,                              //  10
    WRONG_RECORD,                            // 11
    CANT_OPEN_FILE = -10                     //  -10 - see PWScore.h
  };

  /**
  * The format defines a handful of fields in the file's header
  * Since the application needs these after the PWSfile object's
  * lifetime, it makes sense to define a nested header structure that
  * the app. can keep a copy of, rather than duplicating
  * data members, getters and setters willy-nilly.
  */
  enum HeaderType {HDR_VERSION               = 0x00,
                   HDR_UUID                  = 0x01,
                   HDR_NDPREFS               = 0x02,
                   HDR_DISPSTAT              = 0x03,
                   HDR_LASTUPDATETIME        = 0x04,
                   HDR_LASTUPDATEUSERHOST    = 0x05,     // DEPRECATED in format 0x0302
                   HDR_LASTUPDATEAPPLICATION = 0x06,
                   HDR_LASTUPDATEUSER        = 0x07,     // added in format 0x0302
                   HDR_LASTUPDATEHOST        = 0x08,     // added in format 0x0302
                   HDR_DBNAME                = 0x09,     // added in format 0x0302
                   HDR_DBDESC                = 0x0a,     // added in format 0x0302
                   HDR_FILTERS               = 0x0b,     // added in format 0x0305
                   HDR_RESERVED1             = 0x0c,     // added in format 0x030?
                   HDR_RESERVED2             = 0x0d,     // added in format 0x030?
                   HDR_RESERVED3             = 0x0e,     // added in format 0x030?
                   HDR_RUE                   = 0x0f,     // added in format 0x0307
                   HDR_YUBI_OLD_SK           = 0x10,     // Yubi-specific: format 0x030a
                   HDR_PSWDPOLICIES          = 0x10,     // added in format 0x030A
                   HDR_EMPTYGROUP            = 0x11,     // added in format 0x030B
                   HDR_YUBI_SK               = 0x12,     // Yubi-specific: format 0x030c
                   HDR_LASTPWDUPDATETIME     = 0x13,     // added in format 0x030E
                   HDR_LAST,                             // Start of unknown fields!
                   HDR_END                   = 0xff};    // header field types, per formatV{2,3}.txt

  static PWSfile *MakePWSfile(const StringX &a_filename, const StringX &passkey,
                              VERSION &version, RWmode mode, int &status, 
                              Asker *pAsker = nullptr, Reporter *pReporter = nullptr);

  static VERSION ReadVersion(const StringX &filename, const StringX &passkey);
  static int CheckPasskey(const StringX &filename, const StringX &passkey,
                          VERSION &version);

  // Following for 'legacy' use of pwsafe as file encryptor/decryptor
  static bool Encrypt(const stringT &fn, const StringX &passwd, stringT &errmess);
  static bool Decrypt(const stringT &fn, const StringX &passwd, stringT &errmess);

  virtual ~PWSfile();

  virtual int Open(const StringX &passkey) = 0;
  virtual int Close();

  virtual int WriteRecord(const CItemData &item) = 0;
  virtual int ReadRecord(CItemData &item) = 0;

  const PWSfileHeader &GetHeader() const {return m_hdr;}
  void SetHeader(const PWSfileHeader &h) {m_hdr = h;}

  void SetDefUsername(const StringX &du) {m_defusername = du;} // for V17 conversion (read) only
  void SetCurVersion(VERSION v) {m_curversion = v;}
  void GetUnknownHeaderFields(UnknownFieldList &UHFL);
  void SetUnknownHeaderFields(UnknownFieldList &UHFL);
  int GetNumRecordsWithUnknownFields() const
  {return m_nRecordsWithUnknownFields;}

  long GetOffset() const;
  
  // Following implemented in V3 and later
  virtual uint32 GetNHashIters() const {return 0;}
  virtual void SetNHashIters(uint32 ) {}

  void SetDBFilters(const PWSFilters &MapDBFilters) { m_MapDBFilters = MapDBFilters;}
  const PWSFilters *GetDBFilters() const {return &m_MapDBFilters;}

  void SetPasswordPolicies(const PSWDPolicyMap &MapPSWDPLC) {m_MapPSWDPLC = MapPSWDPLC;}
  const PSWDPolicyMap *GetPasswordPolicies() const {return &m_MapPSWDPLC;}

  void SetEmptyGroups(const std::vector<StringX> &vEmptyGroups) {m_vEmptyGroups = vEmptyGroups;}
  const std::vector<StringX> *GetEmptyGroups() const {return &m_vEmptyGroups;}

  // Following for low-level details that changed between format versions
  virtual size_t timeFieldLen() const {return 4;} // changed in V4
  
  size_t WriteField(unsigned char type,
                    const StringX &data) {return WriteCBC(type, data);}
  size_t WriteField(unsigned char type,
                    const unsigned char *data,
                    size_t length) {return WriteCBC(type, data, length);}
  size_t ReadField(unsigned char &type,
                   unsigned char* &data,
                   size_t &length) {return ReadCBC(type, data, length);}
  
protected:
  PWSfile(const StringX &filename, RWmode mode, VERSION v = UNKNOWN_VERSION);
  void FOpen(); // calls right variant of m_fd = fopen(m_filename);
  virtual size_t WriteCBC(unsigned char type, const StringX &data) = 0;
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          size_t length);
  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         size_t &length);
  
  static void HashRandom256(unsigned char *p256); // when we don't want to expose our RNG

  const StringX m_filename;
  StringX m_passkey;
  FILE *m_fd;
  VERSION m_curversion;
  const RWmode m_rw;
  StringX m_defusername; // for V17 conversion (read) only
  unsigned char *m_IV; // points to correct m_ipthing for *CBC()
  Fish *m_fish;
  unsigned char *m_terminal;
  int m_status;
  // Following are only used by V3 and later
  PWSfileHeader m_hdr;
  // Save unknown header fields on read to put back on write unchanged
  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;
  PWSFilters m_MapDBFilters;
  PSWDPolicyMap m_MapPSWDPLC;
  std::vector<StringX> m_vEmptyGroups;
  ulong64 m_fileLength;
  Asker *m_pAsker;
  Reporter *m_pReporter;

private:
  PWSfile& operator=(const PWSfile&); // Do not implement
};

// A quick way to determine if two files are equal,
// or if a given file has been modified. For large files,
// this may miss changes made to the middle. This is due
// to a performance trade-off.
class PWSFileSig
{
public:
  PWSFileSig(const stringT &fname);
  PWSFileSig(const PWSFileSig &pfs);
  PWSFileSig &operator=(const PWSFileSig &that);

  bool IsValid() {return m_iErrorCode == PWSfile::SUCCESS;}
  int GetErrorCode() {return m_iErrorCode;}

  bool operator==(const PWSFileSig &that);
  bool operator!=(const PWSFileSig &that) {return !(*this == that);}

private:
  ulong64 m_length; // -1 if file doesn't exist or zero length
  unsigned char m_digest[SHA256::HASHLEN];
  int m_iErrorCode;
};
#endif /* __PWSFILE_H */
