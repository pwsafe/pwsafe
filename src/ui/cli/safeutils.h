#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"

class PWScore;
class PWPolicy;

int OpenCore(PWScore& core, const StringX& safe);
StringX GetNewPassphrase();

int AddEntry(PWScore &core, const UserArgs &ua);
int InitPWPolicy(PWPolicy &pwp, PWScore &core);
