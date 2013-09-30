/*
* Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSFILEV4_H
#define __PWSFILEV4_H

// PWSfileV4.h
// Abstract the gory details of reading and writing an encrypted database
//-----------------------------------------------------------------------------


// XXX Stub interface of V4, started as a copy of V3.

#include "PWSfile.h"
#include "PWSFilters.h"
#include "TwoFish.h"
#include "sha256.h"
#include "hmac.h"
#include "UTF8Conv.h"

class PWSfileV4 : public PWSfile
{
public:

  enum {HDR_VERSION           = 0x00,
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
    HDR_LAST,                             // Start of unknown fields!
    HDR_END                   = 0xff};    // header field types, per formatV{2,3}.txt

  static int CheckPasskey(const StringX &filename,
                          const StringX &passkey,
                          FILE *a_fd = NULL,
                          unsigned char *aPtag = NULL, uint32 *nIter = NULL);
  static bool IsV4x(const StringX &filename, VERSION &v);

  PWSfileV4(const StringX &filename, RWmode mode, VERSION version);
  ~PWSfileV4();

  virtual int Open(const StringX &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

  uint32 GetNHashIters() const {return m_nHashIters;}
  void SetNHashIters(uint32 N) {m_nHashIters = N;}
  
  void SetFilters(const PWSFilters &MapFilters) {m_MapFilters = MapFilters;}
  const PWSFilters &GetFilters() const {return m_MapFilters;}

  void SetPasswordPolicies(const PSWDPolicyMap &MapPSWDPLC) {m_MapPSWDPLC = MapPSWDPLC;}
  const PSWDPolicyMap &GetPasswordPolicies() const {return m_MapPSWDPLC;}

  void SetEmptyGroups(const std::vector<StringX> &vEmptyGroups) {m_vEmptyGroups = vEmptyGroups;}
  const std::vector<StringX> &GetEmptyGroups() const {return m_vEmptyGroups;}

private:
  uint32 m_nHashIters;
  unsigned char m_ipthing[TwoFish::BLOCKSIZE]; // for CBC
  unsigned char m_key[32];
  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> m_hmac;
  CUTF8Conv m_utf8conv;
  virtual size_t WriteCBC(unsigned char type, const StringX &data);
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          size_t length);

  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         size_t &length);
  int WriteHeader();
  int ReadHeader();
  PWSFilters m_MapFilters;
  PSWDPolicyMap m_MapPSWDPLC;

  // EmptyGroups
  std::vector<StringX> m_vEmptyGroups;

  static int SanityCheck(FILE *stream); // Check for TAG and EOF marker
  static void StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const StringX &passkey,
                         uint32 N, unsigned char *Ptag);
};
#endif /* __PWSFILEV4_H */
