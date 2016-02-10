#pragma once

#include <vector>
#include <string>

enum {
  FONT_NOT_INSTALLED = 0, FONT_INSTALLED
};

struct Fonts_Index {
  short int installed;
  wchar_t *name;
};

Fonts_Index DefaultFontIndex[];

extern std::vector<std::wstring> vsInstalledFonts;

#define NUMFONTINDICES 200
