// Util.h
//-----------------------------------------------------------------------------
#ifndef Util_h
#define Util_h

#include "PasswordSafe.h"

#include "MyString.h"
#include "sha1.h"

#define SaltLength 20
#define SaltSize 20
#define StuffSize 10

// this is for the undocumented 'command line file encryption'
#define CIPHERTEXT_SUFFIX ".PSF"

//Use non-standard dash (ANSI decimal 173) for separation
#define SPLTCHR '\xAD'
#define SPLTSTR "  \xAD  "
#define DEFUSERCHR '\xA0'

//Version defines
#define V10 0
#define V15 1

//Some extra typedefs -- I'm addicted to typedefs
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;


extern void trashMemory(SHA1_CTX& context);
extern void trashMemory(unsigned char* buffer,
                        long length,
                        int numiter = 30);
extern void GenRandhash(const CMyString &passkey,
                        const unsigned char* m_randstuff,
                        unsigned char* m_randhash);
extern unsigned char newrand();

extern unsigned int  RangeRand(size_t len);
extern char GetRandAlphaNumChar();
extern char GetRandAlphaNumSymbolChar();

class BlowFish;
extern BlowFish *MakeBlowFish(const unsigned char *pass, int passlen,
			      const unsigned char *salt, int saltlen);

// buffer is allocated by _readcbc, *** delete[] is responsibility of caller ***
extern int _readcbc(int fp, unsigned char* &buffer, unsigned int &buffer_len,
		    const unsigned char *pass, int passlen,
		    const unsigned char* salt, int saltlen,
		    unsigned char* cbcbuffer);
extern int _writecbc(int fp, const unsigned char* buffer, int length,
		     const unsigned char *pass, int passlen,
		     const unsigned char* salt, int saltlen,
		     unsigned char* cbcbuffer);

#if defined(WITH_LEGACY_CMDLINE)
//void _encryptFile(CString filepath);
//void _decryptFile(CString filepath);
//void convertToLongFilePath(CString& filepath);
void manageCmdLine(CString m_lpCmdLine);
#endif


#endif // Util_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
