// Util.h
//-----------------------------------------------------------------------------
#ifndef Util_h
#define Util_h

#include "stdafx.h"
#include "sha1.h"

typedef unsigned char block[8];

#define bf_N 16
#define SaltLength 20
#define SaltSize 20
#define NumMem 30
#define StuffSize 10
#define CIPHERTEXT_SUFFIX ".PSF"
//Use non-standard dash (ANSI decimal 173) for separation
#define SPLTCHR '\xAD'
#define SPLTSTR "  \xAD  "
#define DEFUSERCHR '\xA0'
//Version defines
#define V10 0
#define V15 1

//Prototypes for stuff in variables.cpp
extern unsigned long bf_S[4][256];
extern unsigned long bf_P[bf_N + 2];
extern unsigned long tempbf_S[4][256];
extern unsigned long tempbf_P[bf_N + 2];

enum windows_t {Win32s,Win95,WinNT};

//Some extra typedefs -- I'm addicted to typedefs
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;


extern void trashMemory(SHA1_CTX &context);
extern void trashMemory(unsigned char* buffer, long length);
extern void trashMemory(unsigned char* buffer, long length, int numiter);
extern void trashMemory(CString &string);
extern void ErrorMessages(CMyString fn, int fp);
extern void GenRandhash(CMyString passkey, unsigned char* m_randstuff,
                        unsigned char* m_randhash);
extern int not(int x);
extern unsigned char newrand();
extern BOOL FileExists(CMyString filename);
extern windows_t GetOSVersion();
extern char GetRandAlphaNumChar();
int _readcbc(int fp, unsigned char* buffer, unsigned int buffer_len,
             unsigned char* salt, unsigned char* cbcbuffer);
int _readcbc(int fp, CMyString &deststring, unsigned char* salt,
             unsigned char* cbcbuffer);
int _writecbc(int fp, unsigned char* buffer, int length,
              unsigned char* salt, unsigned char* cbcbuffer);
int _writecbc(int fp, CMyString string, unsigned char* salt,
              unsigned char* cbcbuffer);
void _encryptFile(CString filepath);
void _decryptFile(CString filepath);
void convertToLongFilePath(CString &filepath);
void manageCmdLine(CString m_lpCmdLine);
void xormem(unsigned char* mem1, unsigned char* mem2, int length);
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
