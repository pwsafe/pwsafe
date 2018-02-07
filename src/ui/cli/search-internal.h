/*
 * Created by Saurav Ghosh on 19/06/16.
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#pragma once

#include <iosfwd>

class PWScore;
struct UserArgs;

int SearchInternal(PWScore &core, const UserArgs &ua, std::wostream &os);
