// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef PWSfile_h
#define PWSfile_h
#include <stdio.h> // for FILE *

#include "ItemData.h"
#include "MyString.h"

class Fish;

class PWSfile {
 public:
  enum VERSION {V17, V20, V30, VCURRENT = V30,
		UNKNOWN_VERSION}; // supported file versions: V17 is last pre-2.0
  enum RWmode {Read, Write};
  enum {SUCCESS = 0, FAILURE = 1, CANT_OPEN_FILE,
        UNSUPPORTED_VERSION, WRONG_VERSION, NOT_PWS3_FILE,
        WRONG_PASSWORD, BAD_DIGEST, END_OF_FILE};

  static PWSfile *MakePWSfile(const CMyString &a_filename, VERSION &version,
                              RWmode mode, int &status);

  static bool FileExists(const CMyString &filename);
  static VERSION ReadVersion(const CMyString &filename);
  static int RenameFile(const CMyString &oldname, const CMyString &newname);
  static int CheckPassword(const CMyString &filename,
                           const CMyString &passkey);

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

 protected:
  PWSfile(const CMyString &filename, RWmode mode);
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
  const RWmode m_rw;
  CMyString m_defusername; // for V17 conversion (read) only
  CMyString m_prefString; // prefererences stored in the file
  unsigned char *m_IV; // points to correct m_ipthing for *CBC()
  Fish *m_fish;
  unsigned char *m_terminal;
};

#endif PWSfile_h


