// PocketPC.cpp
//
//-----------------------------------------------------------------------------
#include "../PwsPlatform.h"

#if defined(POCKET_PC)

#include "pocketpc.h"


extern int errno = 0;


/**
 * Renames a file, uses the WinCE API to implement the function that is not
 * provided in the WinCE / PocketPC standard C library.
 */
#ifdef UNICODE
int rename( const wchar_t *oldname, const wchar_t *newname )
#else
int rename( const char *oldname, const char *newname )
#endif
{
	int	ret;

	ret = ::MoveFile( oldname, newname );
	if ( ret == 0 )
	{
		errno = ::GetLastError();
		return -1;
	}
	errno = 0;
	return errno;
}


/**
 * Gets the number of
 */
time_t time( time_t *timer )
{
	CTime now = CTime::GetCurrentTime();

	return now.GetTime();
}

void centreWithin( CRect &larger, CRect &smaller, CRect &result )
{
	result.left   = larger.left + ((larger.Width() - smaller.Width()) >> 1);
	result.top    = larger.top + ((larger.Height() - smaller.Height()) >> 1);
	result.bottom = result.top + smaller.Height();
	result.right  = result.left + smaller.Width();
}

void centreWithin( CWnd *parent, CWnd *child, CRect &result )
{
	CRect	larger;
	CRect	smaller;

	parent->GetWindowRect( larger );
	child->GetWindowRect( smaller );
	centreWithin( larger, smaller, result );
}
#endif