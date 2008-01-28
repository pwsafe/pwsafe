/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSfileV1V2.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef PWSfileV1V2_h
#define PWSfileV1V2_h

#include "PWSfile.h"
#include "BlowFish.h"

class PWSfileV1V2 : public PWSfile
{
public:
  static int CheckPassword(const CMyString &filename,
    const CMyString &passkey, FILE *a_fd = NULL);

  PWSfileV1V2(const CMyString &filename, RWmode mode, VERSION version);
  ~PWSfileV1V2();

  virtual int Open(const CMyString &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

protected:
  virtual size_t WriteCBC(unsigned char type, const CString &data);

private:
  size_t ReadCBC(unsigned char &type, CMyString &data);
  // crypto stuff for reading/writing files:
  unsigned char m_salt[SaltLength];
  unsigned char m_ipthing[BlowFish::BLOCKSIZE]; // for CBC
  int WriteV2Header();
  int ReadV2Header();
};

#endif /* PWSfileV1V2_h */

