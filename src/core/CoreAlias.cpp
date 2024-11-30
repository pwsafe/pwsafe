/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
* CoreAlias.cpp - parsing and validating alias 'passwords'
*/

#include "PWScore.h"


bool PWScore::ParseAliasPassword(const StringX& Password, BaseEntryParms& pl)
{
  // pl.ibasedata is:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but either no base entry found or 
  //      no unique entry found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
  //  is only valid if n = -1 or -2.

  // Returns true if in a valid alias format, false if not

  pl.bMultipleEntriesFound = false;

  // Take a copy of the Password field to do the counting!
  StringX passwd(Password);

  int num_colonsP1 = Replace(passwd, _T(':'), _T(';')) + 1;
  if ((Password[0] == _T('[')) &&
    (Password[Password.length() - 1] == _T(']')) &&
    num_colonsP1 <= 3) {
    StringX tmp;
    ItemListIter iter = m_pwlist.end();
    switch (num_colonsP1) {
    case 1:
      // [X] - OK if unique entry [g:X:u], [g:X:], [:X:u] or [:X:] exists for any value of g or u
      pl.csPwdTitle = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
      iter = GetUniqueBase(pl.csPwdTitle, pl.bMultipleEntriesFound);
      if (iter != m_pwlist.end()) {
        // Fill in the fields found during search
        pl.csPwdGroup = iter->second.GetGroup();
        pl.csPwdUser = iter->second.GetUser();
      }
      break;
    case 2:
      // [X:Y] - OK if unique entry [X:Y:u] or [g:X:Y] exists for any value of g or u
      pl.csPwdUser = _T("");
      tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
      pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(':')));
      pl.csPwdTitle = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
      iter = GetUniqueBase(pl.csPwdGroup, pl.csPwdTitle, pl.bMultipleEntriesFound);
      if (iter != m_pwlist.end()) {
        // Fill in the fields found during search
        pl.csPwdGroup = iter->second.GetGroup();
        pl.csPwdTitle = iter->second.GetTitle();
        pl.csPwdUser = iter->second.GetUser();
      }
      break;
    case 3:
      // [X:Y:Z], [X:Y:], [:Y:Z], [:Y:] (title cannot be empty)
      tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
      pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(':')));
      tmp = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
      pl.csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(':')));    // Skip over 'title:'
      pl.csPwdUser = tmp.substr(pl.csPwdTitle.length() + 1);
      iter = Find(pl.csPwdGroup, pl.csPwdTitle, pl.csPwdUser);
      break;
    default:
      ASSERT(0);
    }
    if (iter != m_pwlist.end()) {
      pl.TargetType = iter->second.GetEntryType();
      if (pl.InputType == CItemData::ET_ALIAS && pl.TargetType == CItemData::ET_ALIAS) {
        // Check if base is already an alias, if so, set this entry -> real base entry
        pl.base_uuid = iter->second.GetBaseUUID();
      }
      else {
        // This may not be a valid combination of source+target entries - sorted out by caller
        pl.base_uuid = iter->second.GetUUID();
      }
      // Valid and found
      pl.ibasedata = num_colonsP1;
      return true;
    }
    // Valid but either exact [g:t:u] not found or
    //  no or multiple entries satisfy [x] or [x:y]
    pl.ibasedata = -num_colonsP1;
    return true;
  }
  pl.ibasedata = 0; // invalid password format for an alias
  return false;
}
