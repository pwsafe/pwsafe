/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// jprdebug.h
//-----------------------------------------------------------------------------

#if !defined(jprdebug_h)
#define jprdebug_h

#if defined(JPRDEBUG)

#define DBGMSG(x) ::OutputDebugString(x)

inline void
DBGDUMP(const char* mem, size_t len)
{
  const int wrapsize = 20;
  char hexbuf[3];
  int wrap = 0;
  char ascbuf[wrapsize+1];
  size_t ix;

  DBGMSG("* DEBUGDUMP BEGIN *\n");

  for (ix = 0; ix < len; ix++) {
    _stprintf(hexbuf, "%02.2X", mem[ix]);
    DBGMSG(hexbuf);
    if (ix % 2) DBGMSG(" ");
    if (mem[ix] != 0)
      ascbuf[wrap] = mem[ix];
    else
      ascbuf[wrap] = '.';
    wrap++;
    if (wrap == wrapsize) {
      ascbuf[wrap] = 0;
      DBGMSG(" *");
      DBGMSG(ascbuf);
      DBGMSG("*\n");
      wrap = 0;
    }
  }
  if (wrap == 0) {
    DBGMSG("* DEBUGDUMP END *\n");
    return;
  }
  for (ix = wrap; ix < wrapsize; ix++) {
    DBGMSG("  ");
    if (ix % 2) DBGMSG(" ");

    ascbuf[ix]=' ';
  }
  ascbuf[wrapsize]=0;
  DBGMSG(" *");
  DBGMSG(ascbuf);
  DBGMSG("*\n");

  DBGMSG("* DEBUGDUMP END *\n");
}   

#else
#   define DBGMSG(x)
#   define DEBUGDUMP(mem, len)
#endif

#endif // jprdebug_h

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
