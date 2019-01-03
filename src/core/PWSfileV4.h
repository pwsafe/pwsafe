/*
* Copyright (c) 2013-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "PWSfile.h"
#include "TwoFish.h"
#include "sha256.h"
#include "hmac.h"
#include "UTF8Conv.h"

#include <vector>

class PWSfileV4 : public PWSfile
{
public:
  enum Cipher {PWTwoFish, PWAES};
  enum  {KLEN = 32};
  
  static int CheckPasskey(const StringX &filename,
                          const StringX &passkey,
                          FILE *a_fd = nullptr,
                          unsigned char *aPtag = nullptr, uint32 *nIter = nullptr);
  static bool IsV4x(const StringX &filename, const StringX &passkey, VERSION &v);

  PWSfileV4(const StringX &filename, RWmode mode, VERSION version);
  ~PWSfileV4();

  virtual int Open(const StringX &passkey);
  virtual int Close();

  virtual int WriteRecord(const CItemData &item);
  virtual int ReadRecord(CItemData &item);

  int WriteRecord(const CItemAtt &att);
  int ReadRecord(CItemAtt &att);

  // Following writes AttIV, AttEK, AttAK, AttContent
  // and AttContentHMAC per format spec.
  // All except the content are generated internally.
  size_t WriteContentFields(unsigned char *content, size_t len);
  // Following allocates content, caller responsible for deallocating
  size_t ReadContent(Fish *fish, unsigned char *cbcbuffer,
                     unsigned char *&content, size_t clen);

  uint32 GetNHashIters() const {return m_nHashIters;}
  void SetNHashIters(uint32 N) {m_nHashIters = N;}
  
  // Following for low-level details that changed between format versions
  virtual size_t timeFieldLen() const {return 5;} // Experimental

  // Following unique to V4

  // Following needs to be public so that we can manipulate it
  // prior to writing the database.
  class CKeyBlocks {
  public:
    CKeyBlocks();
    CKeyBlocks(const CKeyBlocks &ckb);
    ~CKeyBlocks();
    CKeyBlocks & operator=(const CKeyBlocks &that);
    bool AddKeyBlock(const StringX &current_passkey, const StringX &new_passkey,
                     uint nHashIters = MIN_HASH_ITERATIONS);
    bool RemoveKeyBlock(const StringX &passkey); // fails if m_keyblocks.size() <= 1...
    // ... or if passkey doesn't match.
  private:
    friend class PWSfileV4;
    struct KeyBlockFinder; // fwd decl for functor
    // V4 Format constants:
    enum {PWSaltLength = 32,KWLEN = (KLEN + 8)};
    struct KeyBlock { // See formatV4.txt
    KeyBlock() : m_nHashIters(MIN_HASH_ITERATIONS) {}
      KeyBlock(const KeyBlock &kb);
      KeyBlock &operator=(const KeyBlock &kb);
      unsigned char m_salt[PWSaltLength];
      uint32 m_nHashIters;
      unsigned char m_kw_k[KWLEN];
      unsigned char m_kw_l[KWLEN];
    };
    std::vector<KeyBlock> m_kbs;
    
    bool GetKeys(const StringX &passkey, uint32 nHashIters,
                 unsigned char K[KLEN], unsigned char L[KLEN]); // not const

    KeyBlock &operator[](unsigned i) {return m_kbs[i];}
    const KeyBlock &operator[](unsigned i) const {return m_kbs[i];}
    KeyBlock &at(unsigned i) {return m_kbs.at(i);}
    const KeyBlock &at(unsigned i) const {return m_kbs.at(i);}
    bool empty() const {return m_kbs.empty();}
    unsigned size() const {return (unsigned)m_kbs.size();}
    const size_t KBLEN = PWSaltLength + sizeof(uint32) + KWLEN + KWLEN;
  };

  void SetKeyBlocks(const CKeyBlocks &keyblocks) {m_keyblocks = keyblocks;}
  CKeyBlocks GetKeyBlocks() const {return m_keyblocks;}

 private:
  enum  {NONCELEN = 32};
  CKeyBlocks m_keyblocks;
  // Following set by CKeyBlocks::GetKeys(), call before writing database
  unsigned char m_key[KLEN]; // K
  unsigned char m_ell[KLEN]; // L
  unsigned char m_nonce[NONCELEN]; // 256 bit nonce
  ulong64 m_effectiveFileLength; // for read = fileLength - |HMAC|
  Cipher m_cipher;
  uint32 m_nHashIters; // mainly for single-user compatibility.
  unsigned char m_ipthing[TwoFish::BLOCKSIZE]; // for CBC
  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> m_hmac; // L
  CUTF8Conv m_utf8conv;
  // Forward declaration of functors:
  struct KeyBlockWriter;
  int ParseKeyBlocks(const StringX &passkey);
  int ReadKeyBlock(); // can return SUCCESS or END_OF_FILE
  int TryKeyBlock(unsigned index, const StringX &passkey,
                  unsigned char K[KLEN], unsigned char L[KLEN],
                  uint32 &nHashIters);
  void ComputeEndKB(const unsigned char hnonce[SHA256::HASHLEN],
                    unsigned char digest[SHA256::HASHLEN]);
  bool EndKeyBlocks(const unsigned char calc_hnonce[SHA256::HASHLEN]);
  bool VerifyKeyBlocks();
  virtual size_t WriteCBC(unsigned char type, const StringX &data);
  virtual size_t WriteCBC(unsigned char type, const unsigned char *data,
                          size_t length);

  virtual size_t ReadCBC(unsigned char &type, unsigned char* &data,
                         size_t &length);

  void GetCurrentKeys();
  bool WriteKeyBlocks();
  int WriteHeader();
  int ReadHeader();

  // Following to allow rollback when reverting an ItemAtt read
  // as an ItemData
  long m_savepos;
  unsigned char m_saveIV[TwoFish::BLOCKSIZE];
  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> m_savehmac;

  void SaveState();
  void RestoreState();

  static int SanityCheck(FILE *stream); // Check for TAG and EOF marker
  static void StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const StringX &passkey, uint32 N,
                         unsigned char *Ptag, unsigned long PtagLen);
};
#endif /* __PWSFILEV4_H */
