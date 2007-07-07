/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __PWSFILEV3_H
#define __PWSFILEV3_H

// PWSfileV3.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#include "PWSfile.h"
#include "TwoFish.h"
#include "sha256.h"
#include "hmac.h"
#include  "UTF8Conv.h"

class PWSfileV3 : public PWSfile {
 public:

  enum {HDR_VERSION = 0x00, HDR_UUID = 0x01, HDR_NDPREFS = 0x02, HDR_DISPSTAT = 0x03,
    HDR_LASTUPDATETIME = 0x04, HDR_LASTUPDATEUSERHOST = 0x05,
    HDR_LASTUPDATEAPPLICATION = 0x06,
    HDR_LAST,        // Start of unknown fields!
    HDR_END = 0xff}; // header field types, per formatV{2,3}.txt

  static int CheckPassword(const CMyString &filename,
                           const CMyString &passkey,
                           FILE *a_fd = NULL,
                           unsigned char *aPtag = NULL, int *nIter = NULL);
  static bool IsV3x(const CMyString &filename, VERSION &v);

  PWSfileV3(const CMyString &filename, RWmode mode, VERSION version);
  ~PWSfileV3();

  virtual int Open(const CMyString &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

 private:
  unsigned char m_ipthing[TwoFish::BLOCKSIZE]; // for CBC
  unsigned char m_key[32];
  HMAC_SHA256 m_hmac;
  CUTF8Conv m_utf8conv;
  virtual size_t WriteCBC(unsigned char type, const CString &data);
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length);

  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         unsigned int &length);
  int WriteHeader();
  int ReadHeader();

  static void StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const CMyString &passkey,
                         unsigned int N, unsigned char *Ptag);
};
#endif /* __PWSFILEV3_H */
