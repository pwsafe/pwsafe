
#pragma once

#include <map>
#include <vector>
#include <string>

typedef std::map<int, std::map<int, int>> MapFont2UBlock2NumChars;
typedef std::map<int, int> MapFont2NumChars;

typedef std::map<int, int> MapBlock2NumChars;
typedef std::map<std::wstring, MapBlock2NumChars> MapFont2MapBlocks2NumChars;

extern const std::vector<int> vUCNameIndex;
extern const char *UCNames[];
extern const char *UCName_Words[];
