/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 *  Abstract registry functions:
 * Dummy for Linux,
 * MFC or wxWidget base for Windows
 */

#ifndef __REGISTRY_H
#define __REGISTRY_H

#include "typedefs.h" // for TCHAR

namespace pws_os {
  bool RegCheckExists(const TCHAR *stree = NULL); // check for app's registry subtree by default
  bool RegWriteValue(const TCHAR *section, const TCHAR *entry, int value);
  inline bool RegWriteValue(const TCHAR *section, const TCHAR *entry, bool value)
  { return RegWriteValue(section, entry, value ? 1 : 0); }
  bool RegWriteValue(const TCHAR *section, const TCHAR *entry, const TCHAR *value);
  bool RegDeleteEntry(const TCHAR *name);
  int RegReadValue(const TCHAR *section, const TCHAR *entry, const int value);
  inline bool RegReadValue(const TCHAR *section, const TCHAR *entry, const bool value)
  { return RegReadValue(section, entry, value ? 1 : 0) != 0; }
  const stringT RegReadValue(const TCHAR *section, const TCHAR *entry, const TCHAR *value);
  void RegDeleteSubtree(const TCHAR *stree);
  bool DeleteRegistryEntries();
  // Following should be called in following order:
  // RegOpenSubtree(); RegReadSTValue() (repeatedly); RegCloseSubTree()
  // Not very elegant, but needed only for importing old prefs
  bool RegOpenSubtree(const TCHAR *stree);
  bool RegReadSTValue(const TCHAR *name, bool &value);
  bool RegReadSTValue(const TCHAR *name, int &value);
  bool RegReadSTValue(const TCHAR *name, stringT &value);
  bool RegCloseSubtree();
}
#endif /* __REGISTRY_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
