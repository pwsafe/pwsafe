// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef PWSfile_h
#define PWSfile_h
#include <stdio.h> // for FILE *

#include "ItemData.h"
#include "MyString.h"
#include "UUIDGen.h"

class PWSfile {
 public:
  enum VERSION {V17, V20, VCURRENT = V20,
		UNKNOWN_VERSION}; // supported file versions: V17 is last pre-2.0
  enum {SUCCESS = 0, FAILURE = 1, CANT_OPEN_FILE,
	UNSUPPORTED_VERSION, WRONG_VERSION,
	WRONG_PASSWORD, END_OF_FILE};

  static bool FileExists(const CMyString &filename);
  static int RenameFile(const CMyString &oldname, const CMyString &newname);

  PWSfile(const CMyString &filename, const CMyString &passkey);
  ~PWSfile();

  int OpenWriteFile(VERSION v); //writes header
  int OpenReadFile(VERSION v);
  void CloseFile();
  VERSION GetFileVersion();
  int CheckPassword(); // opens for read and then closes iff not already open
  int WriteRecord(const CItemData &item);
  int ReadRecord(CItemData &item);
  void SetDefUsername(const CMyString &du) {m_defusername = du;} // for V17 conversion (read) only

 private:
  const CMyString m_filename;
  const CMyString m_passkey;
  FILE *m_fd;
  VERSION m_curversion;
  CMyString m_defusername; // for V17 conversion (read) only
  // crypto stuff for reading/writing files:
  unsigned char m_salt[SaltLength];
  unsigned char m_ipthing[8]; // for CBC
  int WriteCBC(unsigned char type, const CString &data);
  int WriteCBC(unsigned char type, const unsigned char *data, unsigned int length);
  int ReadCBC( unsigned char &type, CMyString &data);
  int WriteV2Header();
  int ReadV2Header();
};

#endif PWSfile_h

