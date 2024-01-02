/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file Clipboard.h
 *
 * Small wrapper for Clipboard operations
 */

#ifndef _CLIPBOARD_
#define _CLIPBOARD_

#include <wx/thread.h> // for wxMutex

#include "core/crypto/sha256.h"
#include "core/StringX.h"

class Clipboard
{
public:
  static Clipboard *self; //*< singleton pointer
  static Clipboard *GetInstance();
  static void DeleteInstance();
  bool SetData(const StringX &data);
  bool ClearCBData();
#if defined(__X__) || defined(__WXGTK__)
  void UsePrimarySelection(bool primary, bool clearOnChange=true);
#endif
private:
  Clipboard();
  ~Clipboard() {}
  Clipboard(const Clipboard &);
  Clipboard &operator=(const Clipboard &);

  bool m_set;//<* true if we stored our data
  unsigned char m_digest[SHA256::HASHLEN];//*< our data hash
  wxMutex m_clipboardMutex;//*< mutex for clipboard access
};

#endif /* _CLIPBOARD_ */
