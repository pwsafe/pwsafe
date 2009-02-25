/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "UUIDGen.h"
#include "UnknownField.h"
#include "StringX.h"
#include "Proxy.h"
#include "YubiKey.h"

#define MIN_HASH_ITERATIONS 2048

class Fish;
class Asker;

class PWSfile
{
public:
  enum VERSION {V17, V20, V30, VCURRENT = V30,
    NEWFILE = 98,
    UNKNOWN_VERSION = 99}; // supported file versions: V17 is last pre-2.0
  enum RWmode {Read, Write};
  enum {SUCCESS = 0, FAILURE = 1, 
    CANT_OPEN_FILE,                          //  2
    UNSUPPORTED_VERSION,                     //  3
    WRONG_VERSION,                           //  4
    NOT_PWS3_FILE,                           //  5
    WRONG_PASSWORD,                          //  6 - see PWScore.h
    BAD_DIGEST,                              //  7 - see PWScore.h
    END_OF_FILE                              //  8
  };

  /**
  * The format defines a handful of fields in the file's header
  * Since the application needs these after the PWSfile object's
  * lifetime, it makes sense to define a nested header structure that
  * the app. can keep a copy of, rather than duplicating
  * data members, getters and setters willy-nilly.
  */
  struct HeaderRecord {
    HeaderRecord();
    HeaderRecord(const HeaderRecord &hdr);
    HeaderRecord &operator =(const HeaderRecord &hdr);

    unsigned short m_nCurrentMajorVersion, m_nCurrentMinorVersion;
    uuid_array_t m_file_uuid_array;
    int m_nITER; // Formally not part of the header.
    std::vector<bool> m_displaystatus; // tree expansion  state vector
    StringX m_prefString; // prefererences stored in the file
    time_t m_whenlastsaved; // When last saved
    StringX m_lastsavedby; // and by whom
    StringX m_lastsavedon; // and by which machine
    StringX m_whatlastsaved; // and by what application
    StringX m_dbname, m_dbdesc; // descriptive name, description
    struct {
      StringX PubID;  // public ID of YubiKey associated with this database
      unsigned int apiID; // apiID used to authenticate against server
      YubiApiKey_t apiKey; // apiKey used to authenticate against server
    } m_YubiKey; // Info used for YubiKey authentication
  };

  static PWSfile *MakePWSfile(const StringX &a_filename, VERSION &version,
                              RWmode mode, int &status, 
                              Asker *pAsker = NULL, Reporter *pReporter = NULL);

  static VERSION ReadVersion(const StringX &filename);
  static int CheckPassword(const StringX &filename,
                           const StringX &passkey, VERSION &version);

  // Following for 'legacy' use of pwsafe as file encryptor/decryptor
  static bool Encrypt(const stringT &fn, const StringX &passwd, stringT &errmess);
  static bool Decrypt(const stringT &fn, const StringX &passwd, stringT &errmess);

  virtual ~PWSfile();

  virtual int Open(const StringX &passkey) = 0;
  virtual int Close();

  virtual int WriteRecord(const CItemData &item) = 0;
  virtual int ReadRecord(CItemData &item) = 0;

  const HeaderRecord &GetHeader() const {return m_hdr;}
  void SetHeader(const HeaderRecord &h) {m_hdr = h;}

  void SetDefUsername(const StringX &du) {m_defusername = du;} // for V17 conversion (read) only
  void SetCurVersion(VERSION v) {m_curversion = v;}
  void GetUnknownHeaderFields(UnknownFieldList &UHFL);
  void SetUnknownHeaderFields(UnknownFieldList &UHFL);
  int GetNumRecordsWithUnknownFields()
  {return m_nRecordsWithUnknownFields;}
  
protected:
  PWSfile(const StringX &filename, RWmode mode);
  void FOpen(); // calls right variant of m_fd = fopen(m_filename);
  virtual size_t WriteCBC(unsigned char type, const StringX &data) = 0;
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length);
  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         unsigned int &length);
  const StringX m_filename;
  StringX m_passkey;
  FILE *m_fd;
  VERSION m_curversion;
  const RWmode m_rw;
  StringX m_defusername; // for V17 conversion (read) only
  unsigned char *m_IV; // points to correct m_ipthing for *CBC()
  Fish *m_fish;
  unsigned char *m_terminal;
  HeaderRecord m_hdr;
  // Save unknown header fields on read to put back on write unchanged
  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;
  size_t m_fileLength;
  Asker *m_pAsker;
  Reporter *m_pReporter;
};
#endif /* __PWSFILE_H */
