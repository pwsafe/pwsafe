// Util.h
//-----------------------------------------------------------------------------
#ifndef Util_h
#define Util_h

#include "MyString.h"
#include "Fish.h"
#include "PwsPlatform.h"

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

#ifdef _WIN32
//Some extra typedefs -- I'm addicted to typedefs
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;

typedef unsigned __int64   ulong64;
typedef unsigned long      ulong32;
#else
#include <sys/types.h>
typedef int8_t	int8;
typedef int16_t	int16;
typedef int32_t	int32;
typedef int64_t	int64;

typedef u_int8_t	uint8;
typedef u_int16_t	uint16;
typedef u_int32_t	uint32;
typedef u_int64_t	uint64;
#endif

extern void trashMemory(void* buffer, long length );
extern void trashMemory( LPTSTR buffer, long length );
extern void burnStack(unsigned long len); // borrowed from libtomcrypt

extern void GenRandhash(const CMyString &passkey,
                        const unsigned char* m_randstuff,
                        unsigned char* m_randhash);

//  fill this buffer with random data
void GetRandomData( void * const buffer, unsigned long length );

//  generate a random number between 0 and len
unsigned int RangeRand(size_t len);

// buffer is allocated by _readcbc, *** delete[] is responsibility of caller ***
extern int _readcbc(FILE *fp, unsigned char* &buffer, unsigned int &buffer_len,
                    unsigned char &type, Fish *Algorithm,
                    unsigned char* cbcbuffer,
                    const unsigned char *TERMINAL_BLOCK = NULL);
extern int _writecbc(FILE *fp, const unsigned char* buffer, int length,
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

inline char * strCopy( char *target, const char *source )
{
  return strcpy( target, source );
}

#if defined(UNICODE)
inline LPTSTR strCopy( LPTSTR target, LPCTSTR source )
{
	return wcscpy( target, source );
}
#endif

inline size_t strLength( const char *str )
{
	return strlen( str );
}

#if defined(UNICODE)
inline size_t strLength( const wchar_t *str )
{
	return wcslen( str );
}
#endif

inline char * strFind( const char *str, const char *fstr )
{
	return strstr( str, fstr );
}

#if defined(UNICODE)
inline wchar_t * strFind( const wchar_t *str, const wchar_t *fstr )
{
	return wcsstr( str, fstr );
}
#endif

extern long		fileLength( FILE *fp );

#endif // Util_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
