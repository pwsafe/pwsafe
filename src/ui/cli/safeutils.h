#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"

class PWScore;


int OpenCore(PWScore& core, const StringX& safe);
StringX GetNewPassphrase();

int AddEntry(PWScore &core, const UserArgs &ua);
