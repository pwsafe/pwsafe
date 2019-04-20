/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"

class PWScore;
struct PWPolicy;

int OpenCore(PWScore &core, const StringX &safe, const StringX &passphrase);
StringX GetNewPassphrase();

int AddEntry(PWScore &core, const UserArgs &ua);
int InitPWPolicy(PWPolicy &pwp, PWScore &core);
