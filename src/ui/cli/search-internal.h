#pragma once

#include <iosfwd>

class PWScore;
class UserArgs;

int SearchInternal(PWScore &core, const UserArgs &ua, std::wostream &os);
