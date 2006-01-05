#include "PWSfileV3.h"
#include "UUIDGen.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


PWSfileV3::PWSfileV3(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename,mode)
{
  m_curversion = version;
  m_IV = m_ipthing;
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
    status = ReadHeader();
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

const char V3TAG[4] = {'P','W','S','3'}; // ASCII chars, not wchar

int PWSfileV3::CheckPassword(const CMyString &filename,
                             const CMyString &passkey,
                             FILE *a_fd, unsigned char *aPtag)
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

  char tag[sizeof(V3TAG)];
  fread(tag, 1, sizeof(tag), fd);
  if (memcmp(tag, V3TAG, sizeof(tag)) != 0) {
    return NOT_PWS3_FILE;
  }

  unsigned char salt[SaltLengthV3];
  fread(salt, 1, sizeof(salt), fd);

  unsigned char Ptag[SHA256::HASHLEN];
  if (aPtag == NULL)
    aPtag = Ptag;
  LPCSTR passstr = LPCSTR(passkey); 
  StretchKey(salt, sizeof(salt),
             (const unsigned char *)passstr, passkey.GetLength(),
             aPtag);

  unsigned char HPtag[SHA256::HASHLEN];
  SHA256 H;
  H.Update(aPtag, SHA256::HASHLEN);
  H.Final(HPtag);
  unsigned char readHPtag[SHA256::HASHLEN];
  fread(readHPtag, 1, sizeof(readHPtag), fd);
  if (memcmp(readHPtag, HPtag, sizeof(readHPtag)) != 0) {
    return WRONG_PASSWORD;
  }

   if (a_fd == NULL) // if we opened the file, we close it...
     fclose(fd);
   return SUCCESS;
}

int PWSfileV3::WriteCBC(unsigned char type, const CString &data)
{
  LPCSTR d = LPCSTR(data);
  m_hmac.Update((const unsigned char *)d, data.GetLength());

  return PWSfile::WriteCBC(type, data);
}

int PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data,
                        unsigned int length)
{
  m_hmac.Update(data, length);
  return PWSfile::WriteCBC(type, data, length);
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
  int status = PWSfile::ReadCBC(type, data);

  if (status == SUCCESS) {
    LPCSTR d = LPCSTR(data);
    m_hmac.Update((const unsigned char *)d, data.GetLength());
  }

  return status;
}

int PWSfileV3::ReadCBC(unsigned char &type, unsigned char *data,
                       unsigned int &length)
{
  int status = PWSfile::ReadCBC(type, data, length);

  if (status == SUCCESS) {
    m_hmac.Update(data, length);
  }

  return status;
}


int PWSfileV3::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);
  // XXX TBD
  return SUCCESS;
}

void PWSfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const unsigned char *passkey, unsigned long passLen,
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
  H0.Update(passkey, passLen);
  H0.Update(salt, saltLen);
  H0.Final(X);

  const int N = 2048;
  for (int i = 0; i < N; i++) {
    SHA256 H;
    H.Update(X, sizeof(X));
    H.Final(X);
  }
}

const int VersionNum = 0x0300;

int PWSfileV3::WriteHeader()
{

  // See formatV3.txt for explanation of what's written here and why
  fwrite(V3TAG, 1, sizeof(V3TAG), m_fd);

  unsigned char salt[SaltLengthV3];
  GetRandomData(salt, sizeof(salt));
  fwrite(salt, 1, sizeof(salt), m_fd);
  
  unsigned char Ptag[SHA256::HASHLEN];
  LPCSTR passstr = LPCSTR(m_passkey); 
  StretchKey(salt, sizeof(salt),
             (const unsigned char *)passstr, m_passkey.GetLength(),
             Ptag);

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

  unsigned char L[32]; // for HMAC
  GetRandomData(L, sizeof(L));
  unsigned char B3B4[sizeof(L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  TF.Encrypt(L, B3B4);
  TF.Encrypt(L + 16, B3B4 + 16);
  fwrite(B3B4, 1, sizeof(B3B4), m_fd);

  m_hmac.Init(L, sizeof(L));
  
  GetRandomData(m_ipthing, sizeof(m_ipthing));
  fwrite(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  // write some actual data (at last!)
  int numWritten = 0;
  // Write version number
  numWritten = WriteCBC(0, (const unsigned char *)&VersionNum,
                        sizeof(VersionNum));
  
  // Write UUID
  // We should probably reuse the UUID when saving an existing
  // database, and generate a new one only from new dbs.
  // For now, this is Good Enough. XXX
  uuid_array_t uuid_array;
  CUUIDGen uuid;
  
  uuid.GetUUID(uuid_array);
  
  numWritten += WriteCBC(0, uuid_array, sizeof(uuid_array));

  if (numWritten <= 0) { // WriteCBC writes at least 2 blocks per datum.
    Close();
    return FAILURE;
  }

  // Write (non default) user preferences
  numWritten = WriteCBC(0, m_prefString);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  }

  // Write zero-length end-of-record type item
  // for future-proof (skip possible additional fields in read)
  WriteCBC(CItemData::END, NULL, 0);

  return SUCCESS;
}

int PWSfileV3::ReadHeader()
{
  unsigned char Ptag[SHA256::HASHLEN];
  int status = CheckPassword(m_filename, m_passkey, m_fd, Ptag);

  if (status != SUCCESS) {
    Close();
    return status;
  }

  unsigned char B1B2[sizeof(m_key)];
  ASSERT(sizeof(B1B2) == 32); // Generalize later
  fread(B1B2, 1, sizeof(B1B2), m_fd);
  TwoFish TF(Ptag, sizeof(Ptag));
  TF.Decrypt(B1B2, m_key);
  TF.Decrypt(B1B2 + 16, m_key + 16);

  unsigned char L[32]; // for HMAC
  unsigned char B3B4[sizeof(L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  fread(B3B4, 1, sizeof(B3B4), m_fd);
  TF.Decrypt(B3B4, L);
  TF.Decrypt(B3B4 + 16, L + 16);

  m_hmac.Init(L, sizeof(L));

  fread(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  int numRead = 0;
  unsigned char type;
  // Read version number
  int v;
  unsigned int vlen = sizeof(v);
  numRead = ReadCBC(type, (unsigned char *)&v, vlen);
  if (numRead <= 0  || vlen != sizeof(VersionNum)) {
    Close();
    return FAILURE;
  }

  if ((v & 0xff00) != (VersionNum & 0xff00)) {
    //major version mismatch
    Close();
    return UNSUPPORTED_VERSION;
  }
  // for now we assume that minor version changes will
  // be backward-compatible

  // Read UUID XXX should save into data member;
  uuid_array_t uuid_array;
  unsigned int ulen = sizeof(uuid_array);
  numRead = ReadCBC(type, (unsigned char *)uuid_array, ulen);
  if (numRead <= 0 || ulen != sizeof(uuid_array)) {
    Close();
    return FAILURE;
  }

  // Read (non default) user preferences
  numRead = ReadCBC(type, m_prefString);
  if (numRead <= 0) {
    Close();
    return FAILURE;
  }

  // ignore zero or more fields that may be addded by future versions
  // after prefString, until end-of-record read.

  CMyString dummy;
  do {
    ReadCBC(type, dummy);
  } while (type != CItemData::END);

  return SUCCESS;
}


bool PWSfileV3::IsV3x(const CMyString &filename, VERSION &v)
{
  // This is written so as to support V30, V31, V3x...

  ASSERT(FileExists(filename));
  FILE *fd;
#ifdef UNICODE
  fd = _wfopen((LPCTSTR) filename, _T("rb"));
#else
  fd = fopen((LPCTSTR) filename, _T("rb"));
#endif

  ASSERT(fd != NULL);
  char tag[sizeof(V3TAG)];
  fread(tag, 1, sizeof(tag), fd);
  if (memcmp(tag, V3TAG, sizeof(tag)) == 0) {
    v = V30;
    return true;
  } else {
    v = UNKNOWN_VERSION;
    return false;
  }

}
