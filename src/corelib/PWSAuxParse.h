/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * Declaration of utility functions that parse the two small
 * 'languages' used for 'autotype' and 'run' command processing.
 */

#ifndef _PWSAUXPARSE_H
#define _PWSAUXPARSE_H

#include "StringX.h"

#define DEFAULT_AUTOTYPE _T("\\u\\t\\p\\n")

class CItemData;

namespace PWSAuxParse {
  // Call following with NULL ci and/or empty sxCurrentDB
  // will only validate the run command (non-empty serrmsg means
  // parse failed, reason in same).
  StringX GetExpandedString(const StringX &sxRun_Command,
                            const StringX &sxCurrentDB,
                            CItemData *ci, bool &bAutoType,
                            StringX &sxAutotype, stringT &serrmsg,
                            StringX::size_type &st_column);

  StringX GetAutoTypeString(const StringX &sxAutoCmd,
                            const StringX &sxgroup, const StringX &sxtitle,
                            const StringX &sxuser,  const StringX &sxpwd,
                            const StringX &sxnotes);
};

#endif /* _PWSAUXPARSE_H */
