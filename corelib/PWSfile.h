// PWSfile.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef PWSfile_h
#define PWSfile_h
#include "ItemData.h"
#include "MyString.h"

class PWSfile {
 public:
  enum VERSION {V17, V20, VCURRENT = V20,
		UNKNOWN_VERSION}; // supported file versions: V17 is last pre-2.0
  enum {CANT_OPEN_FILE, UNSUPPORTED_VERSION, WRONG_VERSION,
	WRONG_PASSWORD, END_OF_FILE, SUCCESS};

  static bool FileExists(const CMyString &filename);

  PWSfile(const CMyString &filename, const CMyString &passkey);
  ~PWSfile();

  int OpenWriteFile(VERSION v); //writes header
  int OpenReadFile(VERSION v);
  void CloseFile();
  VERSION GetFileVersion();
  int CheckPassword(); // opens for read and then closes iff not already open
  int WriteRecord(const CItemData &item);
  int ReadRecord(CItemData &item);

 private:
  const CMyString m_filename;
  const CMyString m_passkey;
  int m_fd;
  VERSION m_curversion;
  // crypto stuff for reading/writing files:
  unsigned char m_salt[SaltLength];
  unsigned char m_ipthing[8]; // for CBC
  int WriteCBC(unsigned char type, const CString &data);
  int ReadCBC( unsigned char &type, CMyString &data);
  int WriteV2Header();
  int ReadV2Header();
};

#endif PWSfile_h

