// Util.h
//-----------------------------------------------------------------------------
#ifndef Util_h
#define Util_h

#include "PasswordSafe.h"

#include "MyString.h"
#include "sha1.h"

#define SaltLength 20

// this is for the undocumented 'command line file encryption'
#define CIPHERTEXT_SUFFIX ".PSF"

//Use non-standard dash (ANSI decimal 173) for separation
#define SPLTCHR '\xAD'
#define SPLTSTR "  \xAD  "
#define DEFUSERCHR '\xA0'

//Version defines
#define V10 0
#define V15 1

enum windows_t {Win32s, Win95, WinNT};

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
#if 0
extern void trashMemory(unsigned char* buffer, 
                        long length);
#endif
extern void trashMemory(unsigned char* buffer,
                        long length,
                        int numiter = 30);
extern void trashMemory(CString& string);
#if 0
extern void ErrorMessages(CMyString fn, int fp);
#endif
extern void GenRandhash(CMyString passkey,
                        unsigned char* m_randstuff,
                        unsigned char* m_randhash);
#if 0
extern int not(int x);
#endif

extern unsigned char newrand();
#if 0
extern BOOL FileExists(CMyString filename);
#endif

#if 0
extern windows_t GetOSVersion();
#endif 

extern char GetRandAlphaNumChar();

int _readcbc(int fp, unsigned char* buffer, unsigned int buffer_len,
             unsigned char* salt, unsigned char* cbcbuffer);
int _readcbc(int fp, CMyString& deststring, unsigned char* salt,
             unsigned char* cbcbuffer);
int _writecbc(int fp, unsigned char* buffer, int length,
              unsigned char* salt, unsigned char* cbcbuffer);
int _writecbc(int fp, CMyString string, unsigned char* salt,
              unsigned char* cbcbuffer);

#if defined(WITH_LEGACY_CMDLINE)
//void _encryptFile(CString filepath);
//void _decryptFile(CString filepath);
//void convertToLongFilePath(CString& filepath);
void manageCmdLine(CString m_lpCmdLine);
#endif

int SplitName(CMyString, CMyString&, CMyString&);
void MakeName(CMyString&, CMyString, CMyString);

class CItemData;
void MakeFullNames(CList<CItemData, CItemData>* plist,
                   CMyString defusername);
void DropDefUsernames(CList<CItemData, CItemData>* plist,
                      CMyString defusername);
int CheckVersion(CList<CItemData, CItemData>* plist);
void SetBlankToDef(CList<CItemData, CItemData>* plist);
void SetBlankToName(CList<CItemData, CItemData>* plist, CMyString username);
BOOL CheckExtension(CMyString name, CMyString ext);

#endif // Util_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
