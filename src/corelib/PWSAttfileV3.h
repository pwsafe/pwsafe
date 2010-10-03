/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSFILEAV3_H
#define __PWSFILEAV3_H

// PWSfileV3.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------

#include "PWSAttfile.h"
#include "TwoFish.h"
#include "sha256.h"
#include "hmac.h"
#include "UTF8Conv.h"

class PWSAttfileV3 : public PWSAttfile
{
public:

  enum {
    ATTHDR_VERSION               = 0x00,
    ATTHDR_FILEUUID              = 0x01,
    ATTHDR_DBUUID                = 0x02,
    ATTHDR_LASTUPDATETIME        = 0x03,
    ATTHDR_LASTUPDATEAPPLICATION = 0x04,
    ATTHDR_LASTUPDATEUSER        = 0x05,
    ATTHDR_LASTUPDATEHOST        = 0x06,
    ATTHDR_LAST,                             // Start of unknown fields!
    END                          = 0xff      // header field types, per FormatV3-attachments.txt
  };

  static int CheckPasskey(const StringX &filename,
                          const StringX &passkey,
                          FILE *a_fd = NULL,
                          unsigned char *aPtag = NULL, int *nIter = NULL);

  static bool IsV3x(const StringX &filename, VERSION &v);

  PWSAttfileV3(const StringX &filename, RWmode mode, VERSION version);
  ~PWSAttfileV3();

  virtual int Open(const StringX &passkey);
  virtual int Close();

  virtual int ReadAttmntRecordPreData(ATRecord &atr);
  virtual int ReadAttmntRecordData(unsigned char * &pCmpData, unsigned int &uiCmpLen,
                                   unsigned char &readtype, const bool bSkip);
  virtual int ReadAttmntRecordPostData(ATRecord &atr);

  virtual int WriteAttmntRecordPreData(const ATRecord &adr);
  virtual int WriteAttmntRecordData(unsigned char *pData, const unsigned int len,
                                    const unsigned char type);
  virtual int WriteAttmntRecordPostData(const ATRecord &adr);

private:
  virtual size_t WriteCBC(unsigned char type, const StringX &data);
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length);
  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         unsigned int &length,
                         bool bSkip = false, unsigned char *pSkipTypes = NULL);
  int WriteHeader();
  int ReadHeader();

  static void StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const StringX &passkey, unsigned int N, unsigned char *Ptag);

  unsigned char m_ipthing[TwoFish::BLOCKSIZE]; // for CBC
  unsigned char m_key[32];
  HMAC_SHA256 m_hmac;
  CUTF8Conv m_utf8conv;
};
#endif /* __PWSFILEAV3_H */
