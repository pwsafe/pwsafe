#ifndef __POCKETPC_H__
#define __POCKETPC_H__

//-----------------------------------------------------------------------------
// An implementation of certain c runtime library functions not implemented in
// the standard Windows CE / PocketPC environment
//-----------------------------------------------------------------------------

#include "../PwsPlatform.h"

#if defined(POCKET_PC)
#include <stdlib.h>
#include <Afxwin.h>

#define EACCES	ERROR_ACCESS_DENIED
#define EEXIST	ERROR_FILE_EXISTS
#define EINVAL	ERROR_INVALID_PARAMETER
#define ENOENT	ERROR_FILE_NOT_FOUND
#define EMFILE	ERROR_TOO_MANY_OPEN_FILES

extern int errno;

#ifdef UNICODE

extern int rename( const wchar_t *oldname, const wchar_t *newname );

#else

extern int rename( const char *oldname, const char *newname );

#endif

extern time_t	time( time_t *timer );
extern void		centreWithin( CRect &larger, CRect &smaller, CRect &result );
extern void		centreWithin( CWnd *parent, CWnd *child, CRect &result );

extern DWORD	DisableWordCompletion( HWND hwnd );
extern void		EnableWordCompletion( HWND hwnd );

#endif

#endif