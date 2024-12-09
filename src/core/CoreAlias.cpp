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

#include "core.h"
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

  if (Password.empty())
    return false;

  // number of colons + 1 is the possible number of GTU elements
  int num_colonsP1 = static_cast<int>(std::count(Password.begin(), Password.end(), _T(':'))) + 1;

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
        // This may not be a valid combination of source+target entries - sorted out by CheckAliasValidity()
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

/*
 *struct BaseEntryParms {
  // All fields except "InputType" are 'output'.
  StringX csPwdGroup;
  StringX csPwdTitle;
  StringX csPwdUser;
  pws_os::CUUID base_uuid;
  CItemData::EntryType InputType;
  CItemData::EntryType TargetType;
  int ibasedata;  // > 0 -> base entry found, 0 -> not alias format, < 0 -> base entry not found
  bool bMultipleEntriesFound; // found but not unique
 */

/**
 * Check that an alias password is Kosher
 * @param pl - the base entry parameters, as returned by ParseAliasPassword() ???
 * @param selfGTU - the [g:t:u] string of the current entry, to test for self-reference.
 * @param errmess - if return value is false, this is the text to display to the user
 * @param yesNoError - true means caller should prompt user with Yes/No dialog with errmess, accept alias if Yes returned
 * @return - true if alias is valid, false if there's a problem
 */
bool PWScore::CheckAliasValidity(const BaseEntryParms& pl, const StringX &selfGTU, StringX &errmess, bool& yesNoError)
{
  yesNoError = false; // default

  if (pl.ibasedata == 0) {// shouldn't call me in this case, but whatever
    ASSERT(0);
    errmess = L"Internal Error - CheckAliasValidity called when it shouldn't have been";
    return false;
  }
  // Parsed correctly, let's see what we have
  if (pl.ibasedata > 0  && !pl.bMultipleEntriesFound)
  {
    // are we self-referential?
    if (selfGTU == L"[" + pl.csPwdGroup + L":" + pl.csPwdTitle + L":" + pl.csPwdUser + L"]")
    {
      LoadAString(errmess, IDSC_ALIASCANTREFERTOITSELF);
      return false;
    }
    // Now verify that the base entry indeed exists and is a regular entry

  }


  if (pl.ibasedata > 0) { // base entry exists
    if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
      // An alias can only point to a normal entry or an alias base entry
      Format(errmess, IDSC_BASEISALIAS,
        pl.csPwdGroup.c_str(), pl.csPwdTitle.c_str(), pl.csPwdUser.c_str());
      return false;
    }
    else { // <= 0, no base entry. What's TargetType in this case??
      if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
        // An alias can only point to a normal entry or an alias base entry
        Format(errmess, IDSC_ABASEINVALID,
          pl.csPwdGroup.c_str(), pl.csPwdTitle.c_str(), pl.csPwdUser.c_str());
        return false;
      }
      else {
        return true;
      }
    }
  }

  ASSERT(pl.ibasedata < 0); // otherwise we should have returned by now

#if 0 // no longer relevant, as only called for alias entry candidate
  if (InputType == CItemData::ET_SHORTCUT) {
    if (pl.bMultipleEntriesFound) // originally for InputType == CItemData::ET_SHORTCUT - don't think we need this anymore!
    {
      LoadAString(errmess, IDS_MULTIPLETARGETSFOUND);
    } else
    {
      LoadAString(errmess, IDS_TARGETNOTFOUND);
    }
#endif

    // ibasedata:
    //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
    //   0: password not in alias format
    //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

    // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
    // is only valid if n = -1 or -2.

    if (pl.ibasedata < 0) {
      StringX msgA, msgZ;
      LoadAString(msgA, IDSC_ALIASNOTFOUNDA);
      LoadAString(msgZ, IDSC_ALIASNOTFOUNDZ);

      switch (pl.ibasedata) {
      case -1: // [t] - must be title as this is the only mandatory field
        if (pl.bMultipleEntriesFound)
          Format(errmess, IDSC_ALIASNOTFOUND0A, pl.csPwdTitle.c_str());  // multiple entries exist with title=x
        else
          Format(errmess, IDSC_ALIASNOTFOUND0B, pl.csPwdTitle.c_str());  // no entry exists with title=x
        errmess = msgA + errmess + msgZ;
        yesNoError = true;
        break;
      case -2: // [g,t], [t:u]
        // In this case the 2 fields from the password are in Group & Title
        if (pl.bMultipleEntriesFound)
          Format(errmess, IDSC_ALIASNOTFOUND1A,
                  pl.csPwdGroup.c_str(),
                  pl.csPwdTitle.c_str(),
                  pl.csPwdGroup.c_str(),
                  pl.csPwdTitle.c_str());
        else
          Format(errmess, IDSC_ALIASNOTFOUND1B,
                  pl.csPwdGroup.c_str(),
                  pl.csPwdTitle.c_str(),
                  pl.csPwdGroup.c_str(),
                  pl.csPwdTitle.c_str());
        errmess = msgA + errmess + msgZ;
        yesNoError = true;
        break;
      case -3: // [g:t:u], [g:t:], [:t:u], [:t:] (title cannot be empty)
      {
        const bool bGE = pl.csPwdGroup.empty();
        const bool bTE = pl.csPwdTitle.empty();
        const bool bUE = pl.csPwdUser.empty();
        if (bTE) {
          // Title is mandatory for all entries!
          LoadAString(errmess, IDSC_BASEHASNOTITLE);
          break;
        }
        if (!bGE && !bUE)  // [x:y:z]
          Format(errmess, IDSC_ALIASNOTFOUND2A,
            pl.csPwdGroup.c_str(),
            pl.csPwdTitle.c_str(),
            pl.csPwdUser.c_str());
        else if (!bGE && bUE)     // [x:y:]
          Format(errmess, IDSC_ALIASNOTFOUND2B,
            pl.csPwdGroup.c_str(),
            pl.csPwdTitle.c_str());
        else if (bGE && !bUE)     // [:y:z]
          Format(errmess, IDSC_ALIASNOTFOUND2C,
            pl.csPwdTitle.c_str(),
            pl.csPwdUser.c_str());
        else if (bGE && bUE)      // [:y:]
          Format(errmess, IDSC_ALIASNOTFOUND0B,
            pl.csPwdTitle.c_str());

        errmess = msgA + errmess + msgZ;
        yesNoError = true;
        break;
      }
      default:
        // Never happens
        ASSERT(0);
        errmess = L"Internal error parsing alias string";
        return false;
      }
    }
  return true; // All OK
}
