#pragma once

#include "./argutils.h"
#include <iosfwd>

class PWScore;

// This file has declarations internal to safeutils.cpp, and should not be
// included by other .cpp files in pwsafe-cli project.

// These declarations are not in safeutils.cpp but in a separate header
// for testing.
int AddEntryWithFields(PWScore &core, const UserArgs::FieldUpdates &fieldValues,
                      std::wostream &errstream);
