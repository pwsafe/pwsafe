/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * Declaration of utility functions that parse the two small
 * 'languages' used for 'autotype' and 'run' command processing.
 */

#ifndef __PWSAUXPARSE_H
#define __PWSAUXPARSE_H

#include "StringX.h"
#include <vector>

#define DEFAULT_AUTOTYPE _T("\\u\\t\\p\\n")

class CItemData;
class PWScore;

namespace PWSAuxParse {
  
  // GetEffectiveValues() should be used whenever entry values are needed for copy, autotype or run command
  // For shortcuts and aliases, get values from base (pbci) when appropriate
  // For others, just get values from pci.
  // effectiveItemData will have the "normal" fields.
  // prevPassword will be extracted from password history, and totpAuthCode will be calculated from TOTP values.
  void GetEffectiveValues(const CItemData* pci, const CItemData* pbci, CItemData& effectiveItemData, StringX& prevPassword, StringX& totpAuthCode);

  // Call following with nullptr ci and/or empty sxCurrentDB
  // will only validate the run command (non-empty serrmsg means
  // parse failed, reason in same).
  StringX GetExpandedString(const StringX &sxRun_Command,
                            const StringX &sxCurrentDB,
                            const CItemData *pci, const CItemData *pbci,
                            bool &bAutoType,
                            StringX &sxAutotype, stringT &serrmsg,
                            StringX::size_type &st_column,
                            bool &bURLSpecial);

  StringX GetAutoTypeString(const StringX &sxAutoCmd,
                            const StringX &sxgroup, const StringX &sxtitle,
                            const StringX &sxuser,
                            const StringX &sxpwd, const StringX &sxlastpwd,
                            const StringX &sxnotes, const StringX &sx_url,
                            const StringX &sx_email, const StringX &sx_totpauthcode,
                            std::vector<size_t> &vactionverboffsets);
  StringX GetAutoTypeString(const CItemData &ci,
                            const PWScore &core,
                            std::vector<size_t> &vactionverboffsets);
  // Do some runtime parsing (mainly delay commands) and send it to PC
  // as keystrokes:
  void SendAutoTypeString(const StringX &sx_autotype,
                          const std::vector<size_t> &vactionverboffsets);
}

#endif /* __PWSAUXPARSE_H */
