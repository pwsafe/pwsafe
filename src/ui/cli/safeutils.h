#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"

class PWScore;


int OpenCore(PWScore& core, const StringX& safe);
StringX GetNewPassphrase();

int AddEntry(PWScore &core, const UserArgs &ua);
int AddEntryWithFields(PWScore &core, const UserArgs::FieldUpdates &fieldValues);
