/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsclip.h
 *
 * Small wrapper for Clipboard operations
 */

#ifndef _PWSCLIP_H_
#define _PWSCLIP_H_
#include <wx/thread.h> // for wxMutex

#include "core/sha256.h"
#include "core/StringX.h"
class PWSclipboard
{
public:
  static PWSclipboard *self; //*< singleton pointer
  static PWSclipboard *GetInstance();
  static void DeleteInstance();
  bool SetData(const StringX &data);
  bool ClearCBData();
#if defined(__X__) || defined(__WXGTK__)
  void UsePrimarySelection(bool primary, bool clearOnChange=true);
#endif
private:
  PWSclipboard();
  ~PWSclipboard() {};
  PWSclipboard(const PWSclipboard &);
  PWSclipboard &operator=(const PWSclipboard &);

  bool m_set;//<* true if we stored our data
  unsigned char m_digest[SHA256::HASHLEN];//*< our data hash
  wxMutex m_clipboardMutex;//*< mutex for clipboard access
};

#endif /* _PWSCLIP_H_ */
