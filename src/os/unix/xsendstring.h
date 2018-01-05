/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * xsendstring.h - interface for sending a given string as keyboard input
 * to the X application in focus
 */

#ifndef __XSENDSTRING_H__
#define __XSENDSTRING_H__

#include "../../core/StringX.h"

namespace pws_os {
  /* Set the method to AUTO if you're not sure what it should be */
  typedef enum { ATMETHOD_AUTO, ATMETHOD_XTEST, ATMETHOD_XSENDKEYS } AutotypeMethod;
}

/////////////////////////////////////////////////////////////////////
// CKeySendImpl - Linux specific bits
///////////////

// This is to keep Xlib headers in xsendstring.cpp only
class AutotypeMethodBase;
struct _XDisplay;
class  XErrorHandlerInstaller;
class ModifierFactory;
struct wchar2xevent_map_ptr;

class CKeySendImpl
{
    // This takes effect the next time SendString is called
    bool m_emulateModsSeparately = true;

    // The ctor throws if it can't create this, because all X functions need it
    _XDisplay *m_display;

    // Can't change autotype method in the middle of an autotype session
    AutotypeMethodBase *m_method;

    XErrorHandlerInstaller *m_errHandlerInstaller;

    ModifierFactory *m_modFactory;

    wchar2xevent_map_ptr  *m_wcharmap;

    // Does the actual autotype work with the help of m_method
    void DoSendString(const StringX& str, unsigned delayMS, bool emulateMods);

    CKeySendImpl() = delete;
    CKeySendImpl(const CKeySendImpl&) = delete;
    CKeySendImpl(CKeySendImpl&&) = delete;
    CKeySendImpl& operator=(const CKeySendImpl&) = delete;
    CKeySendImpl& operator=(CKeySendImpl&&) = delete;
  public:

    CKeySendImpl(pws_os::AutotypeMethod method);
    ~CKeySendImpl();

    // Autotypes all chars in data "delay" ms in between
    void SendString(const StringX &data, unsigned delay);
    void EmulateMods(bool emulate) { m_emulateModsSeparately = emulate; }
    bool IsEmulatingMods() const { return m_emulateModsSeparately; }
    // If code == 0, autotypes Ctrl-A
    void SelectAll(unsigned delayMS, int code = 0, int mask = 0);
};

#endif
