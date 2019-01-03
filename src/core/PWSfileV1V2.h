/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSfileV1V2.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#ifndef __PWSFILEV1V2_H
#define __PWSFILEV1V2_H

#include "PWSfile.h"
#include "BlowFish.h"

class PWSfileV1V2 : public PWSfile
{
public:
  static int CheckPasskey(const StringX &filename,
                          const StringX &passkey, FILE *a_fd = nullptr);

  PWSfileV1V2(const StringX &filename, RWmode mode, VERSION version);
  ~PWSfileV1V2();

  virtual int Open(const StringX &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

protected:
  virtual size_t WriteCBC(unsigned char type, const StringX &data);

private:
  PWSfileV1V2& operator=(const PWSfileV1V2&); // Do not implement
  size_t ReadCBC(unsigned char &type, StringX &data);
  // crypto stuff for reading/writing files:
  unsigned char m_salt[SaltLength];
  unsigned char m_ipthing[BlowFish::BLOCKSIZE]; // for CBC
  int WriteV2Header();
  int ReadV2Header();
};

#endif /*  __PWSFILEV1V2_H */
