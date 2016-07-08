#pragma once

#include "../../core/StringX.h"

class PWScore;


int OpenCore(PWScore& core, const StringX& safe);
StringX GetNewPassphrase();

