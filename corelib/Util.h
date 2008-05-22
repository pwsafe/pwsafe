/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __UTIL_H
#define __UTIL_H
// Util.h
//-----------------------------------------------------------------------------

#include "sha256.h"
#include "MyString.h"
#include "Fish.h"
#include "PwsPlatform.h"
#include "os/typedefs.h"

#define SaltLength 20
#define StuffSize 10

#define SaltLengthV3 32

// this is for the undocumented 'command line file encryption'
#define CIPHERTEXT_SUFFIX ".PSF"

//Use non-standard dash (ANSI decimal 173) for separation
#define SPLTCHR _T('\xAD')
#define SPLTSTR _T("  \xAD  ")
#define DEFUSERCHR _T('\xA0')

//Version defines
#define V10 0
#define V15 1

extern void trashMemory(void* buffer, size_t length );
extern void trashMemory( LPTSTR buffer, size_t length );
extern void trashMemory( CString &cs_buffer );
extern void burnStack(unsigned long len); // borrowed from libtomcrypt

extern void ConvertString(const CMyString &text,
                          unsigned char *&txt, int &txtlen);

extern void GenRandhash(const CMyString &passkey,
                        const unsigned char* m_randstuff,
                        unsigned char* m_randhash);

// buffer is allocated by _readcbc, *** delete[] is responsibility of caller ***
extern size_t _readcbc(FILE *fp, unsigned char* &buffer,
                       unsigned int &buffer_len,
                       unsigned char &type, Fish *Algorithm,
                       unsigned char* cbcbuffer,
                       const unsigned char *TERMINAL_BLOCK = NULL);
extern size_t _writecbc(FILE *fp, const unsigned char* buffer, int length,
                        unsigned char type, Fish *Algorithm,
                        unsigned char* cbcbuffer);

/*
* Get an integer that is stored in little-endian format
*/
inline int getInt32(const unsigned char buf[4])
{
  ASSERT(sizeof(int) == 4);
#if defined(PWS_LITTLE_ENDIAN)
#if defined(_DEBUG)
  if ( *(int*) buf != (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    TRACE0( "Warning: PWS_LITTLE_ENDIAN defined but architecture is big endian\n" );
  }
#endif
  return *(int *) buf;
#elif defined(PWS_BIG_ENDIAN)
#if defined(_DEBUG)
  // Folowing code works for big or little endian architectures but we'll warn anyway
  // if CPU is really little endian
  if ( *(int*) buf == (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    TRACE0( "Warning: PWS_BIG_ENDIAN defined but architecture is little endian\n" );
  }
#endif
  return (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24) );
#else
#error Is the target CPU big or little endian?
#endif
}

/*
* Store an integer that is stored in little-endian format
*/
inline void putInt32(unsigned char buf[4], const int val )
{
  ASSERT(sizeof(int) == 4);
#if defined(PWS_LITTLE_ENDIAN)
  *(int32 *) buf = val;
#if defined(_DEBUG)
  if ( *(int*) buf != (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    TRACE0( "Warning: PWS_LITTLE_ENDIAN defined but architecture is big endian\n" );
  }
#endif
#elif defined(PWS_BIG_ENDIAN)
  buf[0] = val & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
  buf[2] = (val >> 16) & 0xFF;
  buf[3] = (val >> 24) & 0xFF;
#if defined(_DEBUG)
  // Above code works for big or little endian architectures but we'll warn anyway
  // if CPU is really little endian
  if ( *(int*) buf == (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    TRACE0( "Warning: PWS_BIG_ENDIAN defined but architecture is little endian\n" );
  }
#endif
#else
#error Is the target CPU big or little endian?
#endif
}

// Time conversion result formats - powers of 2 as they can be combined!
enum {TMC_ASC_UNKNOWN = 1, TMC_ASC_NULL = 2, TMC_EXPORT_IMPORT = 4, TMC_XML = 8,
      TMC_LOCALE = 16};

namespace PWSUtil {
  // namespace of common utility functions

  // For  any comparing functions
  // SubGroup Function - if value used is negative, string compare IS case sensitive
  enum MatchRule {
    MR_INVALID = 0,  // Not valid value
    // For string, integer & date comparisons/filtering
    MR_EQUALS = 1, MR_NOTEQUAL,
    MR_ACTIVE, MR_INACTIVE,
    MR_PRESENT, MR_NOTPRESENT,
    MR_SET, MR_NOTSET,
    // For string comparisons/filters
    MR_BEGINS, MR_NOTBEGIN, 
    MR_ENDS, MR_NOTEND, 
    MR_CONTAINS, MR_NOTCONTAIN, 
    // For integer and date comparisons/filtering
    MR_BETWEEN,
    // For integer comparisons/filtering
    MR_LT, MR_LE, MR_GT, MR_GE,
    // For date comparisons/filtering
    MR_BEFORE, MR_AFTER,
    MR_LAST // MUST be last entry
  };

  // For Windows implementation, hide Unicode abstraction,
  // and use secure versions (_s) when available
  void strCopy(LPTSTR target, size_t tcount, const LPCTSTR source, size_t scount);
  size_t strLength(const LPCTSTR str);
  long fileLength(FILE *fp);
  CMyString ConvertToDateTimeString(const time_t &t, const int result_format);
  CMyString GetNewFileName(const CMyString &oldfilename, const CString &newExtn);
  extern const TCHAR *UNKNOWN_ASC_TIME_STR, *UNKNOWN_XML_TIME_STR;
  CString GetTimeStamp();
  CString Base64Encode(const BYTE *inData, size_t len);
  void Base64Decode(const LPCTSTR sz_inString, BYTE* &outData, size_t &out_len);
  CMyString NormalizeTTT(const CMyString &in);
  // Generalised checking
  bool MatchesString(CMyString string1, CMyString &csObject,
                     const int &iFunction);
  bool MatchesInteger(const int &num1, const int &num2, const int &iValue,
                      const int &iFunction);
  bool MatchesDateTime(const time_t &time1, const time_t &time2, const time_t &tValue,
                       const int &iFunction);
  bool MatchesBool(const bool bValue,
                   const int &iFunction);  // bool - if field present or not
};
#endif /* __UTIL_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
