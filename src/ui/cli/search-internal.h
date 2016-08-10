#pragma once

#include <iosfwd>

class PWScore;
struct UserArgs;

int SearchInternal(PWScore &core, const UserArgs &ua, std::wostream &os);
