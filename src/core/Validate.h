/*
* Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __VALIDATE_H
#define __VALIDATE_H

#include "coredefs.h"


// Helper struct for results of a database verification
struct st_ValidateResults {
  int num_invalid_UUIDs;
  int num_duplicate_UUIDs;
  int num_empty_titles;
  int num_empty_passwords;
  int num_duplicate_GTU_fixed;
  int num_PWH_fixed;
  int num_excessivetxt_found;
  int num_alias_warnings;
  int num_shortcuts_warnings;
  int num_missing_att;
  int num_orphan_att;

  st_ValidateResults()
    : num_invalid_UUIDs(0), num_duplicate_UUIDs(0),
    num_empty_titles(0), num_empty_passwords(0),
    num_duplicate_GTU_fixed(0),
    num_PWH_fixed(0), num_excessivetxt_found(0),
    num_alias_warnings(0), num_shortcuts_warnings(0),
    num_missing_att(0), num_orphan_att(0)
  {}

  st_ValidateResults(const st_ValidateResults &that)
    : num_invalid_UUIDs(that.num_invalid_UUIDs),
    num_duplicate_UUIDs(that.num_duplicate_UUIDs),
    num_empty_titles(that.num_empty_titles),
    num_empty_passwords(that.num_empty_passwords),
    num_duplicate_GTU_fixed(that.num_duplicate_GTU_fixed),
    num_PWH_fixed(that.num_PWH_fixed),
    num_excessivetxt_found(that.num_excessivetxt_found),
    num_alias_warnings(that.num_alias_warnings),
    num_shortcuts_warnings(that.num_shortcuts_warnings),
    num_missing_att(that.num_missing_att), num_orphan_att(that.num_orphan_att)
  {}

  st_ValidateResults &operator=(const st_ValidateResults &that) {
    if (this != &that) {
      num_invalid_UUIDs = that.num_invalid_UUIDs;
      num_duplicate_UUIDs = that.num_duplicate_UUIDs;
      num_empty_titles = that.num_empty_titles;
      num_empty_passwords = that.num_empty_passwords;
      num_duplicate_GTU_fixed = that.num_duplicate_GTU_fixed;
      num_PWH_fixed = that.num_PWH_fixed;
      num_excessivetxt_found = that.num_excessivetxt_found;
      num_alias_warnings = that.num_alias_warnings;
      num_shortcuts_warnings = that.num_shortcuts_warnings;
      num_missing_att = that.num_missing_att;
      num_orphan_att = that.num_orphan_att;
    }
    return *this;
  }

  int TotalIssues() const
  {
    return (num_invalid_UUIDs + num_duplicate_UUIDs +
      num_empty_titles + num_empty_passwords +
      num_duplicate_GTU_fixed +
      num_PWH_fixed + num_excessivetxt_found +
      num_alias_warnings + num_shortcuts_warnings +
      num_missing_att + num_orphan_att);
  }
};


// Comparison functions
// Following structure used in ReadFile and Validate and entries using
// Named Password Policy
inline bool GTUCompareV1(const st_GroupTitleUser& gtu1, const st_GroupTitleUser& gtu2)
{
  if (gtu1.group != gtu2.group)
    return gtu1.group.compare(gtu2.group) < 0;
  else if (gtu1.title != gtu2.title)
    return gtu1.title.compare(gtu2.title) < 0;
  else
    return gtu1.user.compare(gtu2.user) < 0;
}

#endif /* __VALIDATE_H */ 
