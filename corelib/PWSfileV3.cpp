#include "PWSfileV3.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


PWSfileV3::PWSfileV3(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename,mode)
{
  m_curversion = version;
}

PWSfileV3::~PWSfileV3()
{
}

int PWSfileV3::Open(const CMyString &passkey)
{
  int status = SUCCESS;

  ASSERT(m_curversion == V30);

  m_passkey = passkey;

  if (m_rw == Write) {
#ifdef UNICODE
    m_fd = _wfopen((LPCTSTR)m_filename, _T("wb") );
#else
    m_fd = fopen((LPCTSTR)m_filename, _T("wb") );
#endif

    if (m_fd == NULL)
      return CANT_OPEN_FILE;

    status = WriteHeader();
  } else { // open for read
#ifdef UNICODE
    m_fd = _wfopen((LPCTSTR) m_filename, _T("rb"));
#else
    m_fd = fopen((LPCTSTR) m_filename, _T("rb"));
#endif

    if (m_fd == NULL)
      return CANT_OPEN_FILE;
    status = CheckPassword(m_filename, m_passkey, m_fd);
    if (status != SUCCESS) {
      Close();
      return status;
    }
  }
  return status;
}


int PWSfileV3::Close()
{
  // XXX TBD - write or verify HMAC, depending on RWmode.
  return PWSfile::Close();
}


int PWSfileV3::CheckPassword(const CMyString &filename,
                               const CMyString &passkey, FILE *a_fd)
{
  FILE *fd = a_fd;
  if (fd == NULL) {
#ifdef UNICODE
    fd = _wfopen((LPCTSTR) filename, _T("rb"));
#else
    fd = fopen((LPCTSTR) filename, _T("rb"));
#endif
  }
  if (fd == NULL)
    return CANT_OPEN_FILE;

  // XXX do the actual check

   if (a_fd == NULL) // if we opened the file, we close it...
     fclose(fd);
   return SUCCESS;
}

int PWSfileV3::WriteCBC(unsigned char type, const CString &data)
{
  // XXX TBD
  return SUCCESS;
}

int PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data, unsigned int length)
{
  // XXX TBD
  return SUCCESS;
}


int PWSfileV3::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);
  int status = SUCCESS;
  // XXX TBD

  return status;
}

int
PWSfileV3::ReadCBC(unsigned char &type, CMyString &data)
{
  // XXX TBD
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);

  return SUCCESS;
}



int PWSfileV3::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);
  // XXX TBD
  return SUCCESS;
}

void PWSfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                         unsigned char *Ptag)
{
  /*
   * P' is the "stretched key" of the user's passphrase and the SALT, as defined
   * by the hash-function-based key stretching algorithm in
   * http://www.schneier.com/paper-low-entropy.pdf (Section 4.1), with SHA-256
   * as the hash function, and 2048 iterations (i.e., t = 11).
   */
  unsigned char *X = Ptag;
  SHA256 H0;
  // We do a double cast because the LPCSTR cast operator is overridden
  // by the CString class to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);
  H0.Update((const unsigned char *)passstr, m_passkey.GetLength());
  H0.Update(salt, saltLen);
  H0.Final(X);

  const int N = 2048;
  for (int i = 0; i < N; i++) {
    SHA256 H;
    H.Update(X, sizeof(X));
    H.Final(X);
  }
}

const char V3TAG[4] = {'P','W','S','3'}; // ASCII chars, not wchar

int PWSfileV3::WriteHeader()
{
  // See formatV3.txt for explanation of what's written here and why
  fwrite(V3TAG, 1, sizeof(V3TAG), m_fd);

  unsigned char salt[SaltLengthV3];
  GetRandomData(salt, sizeof(salt));
  fwrite(salt, 1, sizeof(salt), m_fd);

  unsigned char Ptag[SHA256::HASHLEN];
  StretchKey(salt, sizeof(salt), Ptag);

  unsigned char HPtag[SHA256::HASHLEN];
  SHA256 H;
  H.Update(Ptag, sizeof(Ptag));
  H.Final(HPtag);
  fwrite(HPtag, 1, sizeof(HPtag), m_fd);

  GetRandomData(m_key, sizeof(m_key));
  unsigned char B1B2[sizeof(m_key)];
  ASSERT(sizeof(B1B2) == 32); // Generalize later
  TwoFish TF(Ptag, sizeof(Ptag));
  TF.Encrypt(m_key, B1B2);
  TF.Encrypt(m_key + 16, B1B2 + 16);
  fwrite(B1B2, 1, sizeof(B1B2), m_fd);

  GetRandomData(m_L, sizeof(m_L));
  unsigned char B3B4[sizeof(m_L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  TF.Encrypt(m_L, B3B4);
  TF.Encrypt(m_L + 16, B3B4 + 16);
  fwrite(B3B4, 1, sizeof(B3B4), m_fd);

  GetRandomData(m_ipthing, sizeof(m_ipthing));
  fwrite(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  // write some actual data (at last!)
  return SUCCESS;
}

int PWSfileV3::ReadHeader()
{
  char tag[sizeof(V3TAG)];
  fread(tag, 1, sizeof(tag), m_fd);
  if (memcmp(tag, V3TAG, sizeof(tag)) != 0) {
    return NOT_PWS3_FILE;
  }
 
  unsigned char salt[SaltLengthV3];
  fread(salt, 1, sizeof(salt), m_fd);

  unsigned char Ptag[SHA256::HASHLEN];
  StretchKey(salt, sizeof(salt), Ptag);

  unsigned char HPtag[SHA256::HASHLEN];
  SHA256 H;
  H.Update(Ptag, sizeof(Ptag));
  H.Final(HPtag);
  unsigned char readHPtag[SHA256::HASHLEN];
  fread(readHPtag, 1, sizeof(readHPtag), m_fd);
  if (memcmp(readHPtag, HPtag, sizeof(readHPtag)) != 0) {
    return WRONG_PASSWORD;
  }

  unsigned char B1B2[sizeof(m_key)];
  ASSERT(sizeof(B1B2) == 32); // Generalize later
  fread(B1B2, 1, sizeof(B1B2), m_fd);
  TwoFish TF(Ptag, sizeof(Ptag));
  TF.Decrypt(B1B2, m_key);
  TF.Decrypt(B1B2 + 16, m_key + 16);

  unsigned char B3B4[sizeof(m_L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  fread(B3B4, 1, sizeof(B3B4), m_fd);
  TF.Decrypt(B3B4, m_L);
  TF.Decrypt(B3B4 + 16, m_L + 16);

  fread(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  // read the header data
  return SUCCESS;
}
