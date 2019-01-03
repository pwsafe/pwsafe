/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

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
