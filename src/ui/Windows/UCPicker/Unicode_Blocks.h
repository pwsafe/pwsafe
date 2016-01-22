
#pragma once

#include <vector>
#include <string>

struct unicode_block {
  int iNumber;
  int iUserFont;
  int iPreferredFont;
  int iBestAvailableFont;
  int imin;
  int imax;
  int imax_used;
  int numchars;
  wchar_t *name;
  const std::vector<int> *pviReserved;
  const std::vector<int> *pvDefaultFontIndex;
};

extern std::vector<unicode_block> vUCBlocks;
extern std::vector<unicode_block> vUCBlocks_Subset;

#define NUMUNICODERANGES 259

bool CompareBlockNumberA(const unicode_block &a, const unicode_block &b);
bool CompareBlockNumberD(const unicode_block &a, const unicode_block &b);
bool CompareBlockNameA(const unicode_block &a, const unicode_block &b);
bool CompareBlockNameD(const unicode_block &a, const unicode_block &b);
bool CompareBlockFontA(const unicode_block &a, const unicode_block &b);
bool CompareBlockFontD(const unicode_block &a, const unicode_block &b);
