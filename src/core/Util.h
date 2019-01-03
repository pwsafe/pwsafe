/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "StringX.h"
#include "StringXStream.h"
#include "Fish.h"
#include "PwsPlatform.h"
#include "UTF8Conv.h"

#include "../os/debug.h"
#include "../os/typedefs.h"
#include "../os/mem.h"

#include <sstream>
#include <stdarg.h>

// For V1V2 and file encryption, NOT for V3 and later:
#define SaltLength 20
#define StuffSize 10

//Use non-standard dash (ANSI decimal 173) for separation
#define SPLTCHR _T('\xAD')
#define SPLTSTR _T("  \xAD  ")
#define DEFUSERCHR _T('\xA0')

//Version defines
#define V10 0
#define V15 1

extern void trashMemory(void *buffer, size_t length);
extern void trashMemory(LPTSTR buffer, size_t length);
extern void burnStack(unsigned long len); // borrowed from libtomcrypt

extern void ConvertPasskey(const StringX &text,
                          unsigned char *&txt, size_t &txtlen);

extern void GenRandhash(const StringX &passkey,
                        const unsigned char *m_randstuff,
                        unsigned char *m_randhash);

// buffer is allocated by _readcbc, *** delete[] is responsibility of caller ***
extern size_t _readcbc(FILE *fp, unsigned char * &buffer,
                       size_t &buffer_len,
                       unsigned char &type, Fish *Algorithm,
                       unsigned char *cbcbuffer,
                       const unsigned char *TERMINAL_BLOCK = nullptr,
                       ulong64 file_len = 0);

// typeless version for V4 content (caller pre-allocates buffer)
extern size_t _readcbc(FILE *fp, unsigned char *buffer,
                       const size_t buffer_len, Fish *Algorithm,
                       unsigned char *cbcbuffer);

// _writecbc will throw(EIO) iff a write fail occurs!
extern size_t _writecbc(FILE *fp, const unsigned char *buffer, size_t length,
                        unsigned char type, Fish *Algorithm,
                        unsigned char *cbcbuffer);

// typeless version for V4 content:
extern size_t _writecbc(FILE *fp, const unsigned char *buffer, size_t length,
                        Fish *Algorithm, unsigned char *cbcbuffer);

// The following can be used directly or via template functions getInt<> / putInt<>

/*
* Get an integer that is stored in little-endian format
*/

inline int16 getInt16(const unsigned char buf[2])
{
#if defined(PWS_LITTLE_ENDIAN)
  return *reinterpret_cast<const int16 *>(buf);
#elif defined(PWS_BIG_ENDIAN)
  return (buf[0] | (buf[1] << 8) );
#else
#error Is the target CPU big or little endian?
#endif
}

inline int32 getInt32(const unsigned char buf[4])
{
#if defined(PWS_LITTLE_ENDIAN)
#if defined(_DEBUG)
  if ( *reinterpret_cast<const int32 *>(buf) != (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    pws_os::Trace0(_T("Warning: PWS_LITTLE_ENDIAN defined but architecture is big endian\n"));
  }
#endif
  return *reinterpret_cast<const int32 *>(buf);
#elif defined(PWS_BIG_ENDIAN)
#if defined(_DEBUG)
  // Following code works for big or little endian architectures but we'll warn anyway
  // if CPU is really little endian
  if (
      *reinterpret_cast<const int32 *>(buf) != 0 &&
      *reinterpret_cast<const int32 *>(buf) == (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    pws_os::Trace0(_T("Warning: PWS_BIG_ENDIAN defined but architecture is little endian\n"));
  }
#endif
  return (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24) );
#else
#error Is the target CPU big or little endian?
#endif
}

inline int64 getInt64(const unsigned char buf[8])
{
#if defined(PWS_LITTLE_ENDIAN)
  return *reinterpret_cast<const int64 *>(buf);
#elif defined(PWS_BIG_ENDIAN)
  return (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24) |
          ((int64)buf[4] << 32) | ((int64)buf[5] << 40) | ((int64)buf[6] << 48) | ((int64)buf[7] << 56));
#else
#error Is the target CPU big or little endian?
#endif
}

/*
* Store an integer that is stored in little-endian format
*/
inline void putInt16(unsigned char buf[2], const int16 val )
{
#if defined(PWS_LITTLE_ENDIAN)
  *reinterpret_cast<int16 *>(buf) = val;
#elif defined(PWS_BIG_ENDIAN)
  buf[0] = val & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
#else
#error Is the target CPU big or little endian?
#endif
}

inline void putInt32(unsigned char buf[4], const int32 val )
{
#if defined(PWS_LITTLE_ENDIAN)
  *reinterpret_cast<int32 *>(buf) = val;
#if defined(_DEBUG)
  if ( *reinterpret_cast<int32*>(buf) != (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    pws_os::Trace0(_T("Warning: PWS_LITTLE_ENDIAN defined but architecture is big endian\n"));
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
  if (
      *reinterpret_cast<const int32 *>(buf) != 0 &&
      *reinterpret_cast<int32*>(buf) == (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)) )
  {
    pws_os::Trace0(_T("Warning: PWS_BIG_ENDIAN defined but architecture is little endian\n"));
  }
#endif
#else
#error Is the target CPU big or little endian?
#endif
}

inline void putInt64(unsigned char buf[8], const int64 val )
{
#if defined(PWS_LITTLE_ENDIAN)
  *reinterpret_cast<int64 *>(buf) = val;
#elif defined(PWS_BIG_ENDIAN)
  buf[0] = val & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
  buf[2] = (val >> 16) & 0xFF;
  buf[3] = (val >> 24) & 0xFF;
  buf[4] = (val >> 32) & 0xFF;
  buf[5] = (val >> 40) & 0xFF;
  buf[6] = (val >> 48) & 0xFF;
  buf[7] = (val >> 56) & 0xFF;
#else
#error Is the target CPU big or little endian?
#endif
}

/**
 * And now let's wrap some template functions around the above:
 * The idea is to templatize on the size of the datatype, s.t. calling
 * one of these for a time_t, for example, will work regardless of the
 * sizeof(time_t) (which can be 4 or 8).
 * The only trick is that we cannot partially specialize a template function,
 * so we use a template class to map from the sizeof(type) to the right get/put,
 * and template functions as wrappers.
 */

// Template for partial specialization
template<typename T, std::size_t Size>
struct GetPutImpl {
  static T GetInt(const unsigned char *);
  static void PutInt(unsigned char *, const T);
};

// and here are the partial specializations, on the sizeof(T):
template<typename T>
struct GetPutImpl<T, 2> {
  static T GetInt(const unsigned char *buf) {return T(getInt16(buf));}
  static void PutInt(unsigned char *buf, const T val) { putInt16(buf, int16(val)); }
};

template<typename T>
struct GetPutImpl<T, 4> {
  static T GetInt(const unsigned char *buf) {return T(getInt32(buf));}
  static void PutInt(unsigned char *buf, const T val) { putInt32(buf, int32(val)); }
};

template<typename T>
struct GetPutImpl<T, 8> {
  static T GetInt(const unsigned char *buf) {return T(getInt64(buf));}
  static void PutInt(unsigned char *buf, const T val) { putInt64(buf, int64(val)); }
};

// And here are the wrapper template functions. Simple, no?
template<typename T>
T inline getInt(const unsigned char *buf) { return GetPutImpl<T, sizeof(T)>::GetInt(buf); }
template<typename T>
void inline putInt(unsigned char *buf, const T val) { GetPutImpl<T, sizeof(T)>::PutInt(buf, val); }

bool operator==(const std::string& str1, const stringT& str2);
inline bool operator==(const stringT& str1, const std::string &str2) { return str2 == str1; }

inline void byteswap(unsigned char * begin, unsigned char * end) {
    unsigned char *a = begin;
    unsigned char *b = end;
    ASSERT(a < b);
    unsigned char tmp;
    while (a < b) {
        tmp = *a;
        *a++ = *b;
        *b-- = tmp;
    }
}

namespace PWSUtil {
  // namespace of common utility functions

  // For Windows implementation, hide Unicode abstraction,
  // and use secure versions (_s) when available
  void strCopy(LPTSTR target, size_t tcount, const LPCTSTR source, size_t scount);
  size_t strLength(const LPCTSTR str);
  // Time conversion result formats:
  enum TMC {TMC_ASC_UNKNOWN, TMC_ASC_NULL, TMC_EXPORT_IMPORT, TMC_XML,
            TMC_LOCALE, TMC_LOCALE_DATE_ONLY};
  StringX ConvertToDateTimeString(const time_t &t, TMC result_format);
  stringT GetNewFileName(const stringT &oldfilename, const stringT &newExtn);
  extern const TCHAR *UNKNOWN_ASC_TIME_STR, *UNKNOWN_XML_TIME_STR;
  void GetTimeStamp(stringT &sTimeStamp, const bool bShort = false);
  stringT GetTimeStamp(const bool bShort = false);
  stringT Base64Encode(const BYTE *inData, size_t len);
  void Base64Decode(const StringX &inString, BYTE* &outData, size_t &out_len);
  StringX NormalizeTTT(const StringX &in, size_t maxlen = 64);
  bool WriteXMLField(std::ostream &os, const char *fname,
                     const StringX &value, CUTF8Conv &utf8conv,
                     const char *tabs = "\t\t");
  std::string GetXMLTime(int indent, const char *name,
                         time_t t, CUTF8Conv &utf8conv);

  StringX DeDupString(StringX &in_string);
  stringT GetSafeXMLString(const StringX &sxInString);

  bool pull_time(time_t &t, const unsigned char *data, size_t len);
  // load file to stream
  bool loadFile(const StringX &filename, StringXStream &stream);
}

///////////////////////////////////////////////////////
// Following two templates lets us use the two types
// of iterators in a common (templatized) function when
// all we need to do is to access the underlying value
template <typename PairAssociativeContainer>
class get_second {
  public:
    typedef typename PairAssociativeContainer::mapped_type mapped_type;
    typedef typename PairAssociativeContainer::const_iterator const_iterator;
    const mapped_type& operator()(const_iterator val) const { return val->second; }
};

template <typename SequenceContainer>
class dereference {
  public:
    typedef typename SequenceContainer::value_type value_type;
    typedef typename SequenceContainer::const_iterator const_iterator;
    const value_type& operator()(const_iterator itr) const { return *itr; }
};

extern unsigned int GetStringBufSize(const TCHAR *fmt, va_list args);

bool FindNoCase( const StringX& src, const StringX& dest);


std::string toutf8(const std::wstring &w);

#endif /* __UTIL_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
