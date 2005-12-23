// PWSfileV3.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef PWSfileV3_h
#define PWSfileV3_h

#include "PWSfile.h"
#include "TwoFish.h"
#include "sha256.h"

class PWSfileV3 : public PWSfile {
 public:
  static int CheckPassword(const CMyString &filename,
                           const CMyString &passkey, FILE *a_fd = NULL);

  PWSfileV3(const CMyString &filename, RWmode mode, VERSION version);
  ~PWSfileV3();

  virtual int Open(const CMyString &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

 private:
  unsigned char m_ipthing[TwoFish::BLOCKSIZE]; // for CBC
  unsigned char m_key[32];
  unsigned char m_L[32]; // for HMAC
  CMyString m_prefString; // prefererences stored in the file
  int WriteCBC(unsigned char type, const CString &data);
  int WriteCBC(unsigned char type, const unsigned char *data, unsigned int length);
  int ReadCBC( unsigned char &type, CMyString &data);
  int WriteHeader();
  int ReadHeader();
  void StretchKey(const unsigned char *salt, unsigned long saltLen,
                  unsigned char *Ptag);
};

#endif PWSfileV3_h

