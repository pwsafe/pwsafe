//

#include "StdAfx.h"

#include "Unicode_Blocks.h"

std::vector<unicode_block> vUCBlocks_Subset;

bool CompareBlockNumberA(const unicode_block &a, const unicode_block &b)
{
  return a.iNumber < b.iNumber;
}

bool CompareBlockNumberD(const unicode_block &a, const unicode_block &b)
{
  return a.iNumber > b.iNumber;
}

bool CompareBlockNameA(const unicode_block &a, const unicode_block &b)
{
  return wcscmp(a.name, b.name) < 0;
}

bool CompareBlockNameD(const unicode_block &a, const unicode_block &b)
{
  return wcscmp(a.name, b.name) > 0;
}

bool CompareBlockFontA(const unicode_block &a, const unicode_block &b)
{
  int ia, ib;

  if (a.iUserFont >= 0) {
    ia = a.iUserFont;
  } else if (a.iPreferredFont >= 0) {
    ia = a.iPreferredFont;
  } else if (a.iBestAvailableFont >= 0) {
    ia = a.iBestAvailableFont;
  } else
    ia = -1;

  if (b.iUserFont >= 0) {
    ib = b.iUserFont;
  } else if (b.iPreferredFont >= 0) {
    ib = b.iPreferredFont;
  } else if (b.iBestAvailableFont >= 0) {
    ib = b.iBestAvailableFont;
  } else
    ib = -1;

  return ia < ib;
}

bool CompareBlockFontD(const unicode_block &a, const unicode_block &b)
{
  int ia, ib;

  if (a.iUserFont >= 0) {
    ia = a.iUserFont;
  } else if (a.iPreferredFont >= 0) {
    ia = a.iPreferredFont;
  } else if (a.iBestAvailableFont >= 0) {
    ia = a.iBestAvailableFont;
  } else
    ia = -1;

  if (b.iUserFont >= 0) {
    ib = b.iUserFont;
  } else if (b.iPreferredFont >= 0) {
    ib = b.iPreferredFont;
  } else if (b.iBestAvailableFont >= 0) {
    ib = b.iBestAvailableFont;
  } else
    ib = -1;

  return ia > ib;
}

const std::vector<int> viRsvd_000000_00007F = // Basic Latin
  {0, /*1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,*/ 127};
const std::vector<int> viRsvd_000080_0000FF = // Latin-1 Supplement
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
const std::vector<int> viRsvd_000370_0003FF = // Greek and Coptic
  {8, 9, 16, 17, 18, 19, 27, 29, 50};
const std::vector<int> viRsvd_000530_00058F = // Armenian
  {0, 39, 40, 48, 88, 91, 92};
const std::vector<int> viRsvd_000590_0005FF = // Hebrew
  {0, 56, 57, 58, 59, 60, 61, 62, 63, 91, 92, 93, 94, 95};
const std::vector<int> viRsvd_000600_0006FF = // Arabic
  {29};
const std::vector<int> viRsvd_000700_00074F = // Syriac
  {14, 75, 76};
const std::vector<int> viRsvd_000800_00083F = // Samaritan
  {46, 47};
const std::vector<int> viRsvd_000840_00085F = // Mandaic
  {28, 29};
const std::vector<int> viRsvd_0008A0_0008FF = // Arabic Extended A
  {21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
  38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66};
const std::vector<int> viRsvd_000980_0009FF = // Bengali
  {4, 13, 14, 17, 18, 41, 49, 51, 52, 53, 58, 59, 69, 70, 73, 74, 79,
  80, 81, 82, 83, 84, 85, 86, 88, 89, 90, 91, 94, 100, 101};
const std::vector<int> viRsvd_000A00_000A7F = // Gurmukhi
  {0, 4, 11, 12, 13, 14, 17, 18, 41, 49, 52, 55, 58, 59, 61, 67, 68,
  69, 70, 73, 74, 78, 79, 80, 82, 83, 84, 85, 86, 87, 88, 93, 95, 96,
  97, 98, 99, 100, 101};
const std::vector<int> viRsvd_000A80_000AFF = // Gujarati
  {0, 4, 14, 18, 41, 49, 52, 58, 59, 70, 74, 78, 79, 81, 82, 83, 84,
  85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 100, 101, 114, 115,
  116, 117, 118, 119, 120};
const std::vector<int> viRsvd_000B00_000B7F = // Oriya
  {0, 4, 13, 14, 17, 18, 41, 49, 52, 58, 59, 69, 70, 73, 74, 78, 79,
  80, 81, 82, 83, 84, 85, 88, 89, 90, 91, 94, 100, 101};
const std::vector<int> viRsvd_000B80_000BFF = // Tamil
  {0, 1, 4, 11, 12, 13, 17, 22, 23, 24, 27, 29, 32, 33, 34, 37, 38,
  39, 43, 44, 45, 58, 59, 60, 61, 67, 68, 69, 73, 78, 79, 81, 82, 83,
  84, 85, 86, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101};
const std::vector<int> viRsvd_000C00_000C7F = // Telugu
  {4, 13, 17, 41, 58, 59, 60, 69, 73, 78, 79, 80, 81, 82,
  83, 84, 87, 91, 92, 93, 94, 95, 100, 101, 112, 113, 114, 115,
  116, 117, 118, 119};
const std::vector<int> viRsvd_000C80_000CFF = // Kannada
  {0, 4, 13, 17, 41, 52, 58, 59, 69, 73, 78, 79, 80, 81,
  82, 83, 84, 87, 88, 89, 90, 91, 92, 93, 95, 100, 101, 112};
const std::vector<int> viRsvd_000D00_000D7F = // Malayalam
  {0, 4, 13, 17, 59, 60, 69, 73, 79, 80, 81, 82, 83, 84,
  85, 86, 88, 89, 90, 91, 92, 93, 94, 100, 101, 118, 119, 120};
const std::vector<int> viRsvd_000D80_000DFF = // Sinhala
  {0, 1, 4, 23, 24, 25, 50, 60, 62, 63, 71, 72, 73, 75,
  76, 77, 78, 85, 87, 96, 97, 98, 99, 100, 101, 112, 113};
const std::vector<int> viRsvd_000E00_000E7F = // Thai
  {0, 59, 60, 61, 62};
const std::vector<int> viRsvd_000E80_000EFF = // Lao
  {0, 3, 5, 6, 9, 11, 12, 14, 15, 16, 17, 18, 19, 24,
  32, 36, 38, 40, 41, 44, 58, 62, 63, 69, 71, 78, 79, 90,
  91};
const std::vector<int> viRsvd_000F00_000FFF = // Tibetan
  {72, 109, 110, 111, 112, 152, 189, 205};
const std::vector<int> viRsvd_0010A0_0010FF = // Georgian
  {38, 40, 41, 42, 43, 44, 46, 47};
const std::vector<int> viRsvd_001200_00137F = // Ethiopic
  {73, 78, 79, 87, 89, 94, 95, 137, 142, 143, 177, 182, 183, 191,
  193, 198, 199, 215, 273, 278, 279, 347, 348};
const std::vector<int> viRsvd_0013A0_0013FF = // Cherokee
  {86, 87};
const std::vector<int> viRsvd_001700_00171F = // Tagalog
  {13};
const std::vector<int> viRsvd_001760_00177F = // Tagbanwa
  {13, 17};
const std::vector<int> viRsvd_001780_0017FF = // Khmer
  {94, 95, 106, 107, 108, 109, 110, 111};
const std::vector<int> viRsvd_001800_0018AF = // Mongolian
  {15, 26, 27, 28, 29, 30, 31, 120, 121, 122, 123, 124, 125, 126,
  127};
const std::vector<int> viRsvd_001900_00194F = // Limbu
  {31, 44, 45, 46, 47, 60, 61, 62, 63, 65, 66, 67};
const std::vector<int> viRsvd_001950_00197F = // TaiLe
  {30, 31};
const std::vector<int> viRsvd_001980_0019DF = // NewTaiLue
  {44, 45, 46, 47, 74, 75, 76, 77, 78, 79, 91, 92, 93};
const std::vector<int> viRsvd_001A00_001A1F = // Buginese
  {28, 29};
const std::vector<int> viRsvd_001A20_001AAF = // TaiTham
  {63, 93, 94, 106, 107, 108, 109, 110, 111, 122, 123, 124, 125, 126,
  127};
const std::vector<int> viRsvd_001B00_001B7F = // Balinese
  {76, 77, 78, 79};
const std::vector<int> viRsvd_001BC0_001BFF = // Batak
  {52, 53, 54, 55, 56, 57, 58, 59};
const std::vector<int> viRsvd_001C00_001C4F = // Lepcha
  {56, 57, 58, 74, 75, 76};
const std::vector<int> viRsvd_001CD0_001CFF = // VedicExtensions
  {39};
const std::vector<int> viRsvd_001DC0_001DFF = // Combining Diacritical Marks Supplement
  {54, 55, 56, 57, 58, 59};
const std::vector<int> viRsvd_001F00_001FFF = // Greek Extended
  {22, 23, 30, 31, 70, 71, 78, 79, 88, 90, 92, 94, 126, 127,
  181, 197, 212, 213, 220, 240, 241, 245};
const std::vector<int> viRsvd_002000_00206F = // General Punctuation
  {101};
const std::vector<int> viRsvd_002070_00209F = // Superscripts and Subscripts
  {2, 3, 31};
const std::vector<int> viRsvd_002B00_002BFF = // Miscellaneou sSymbols and Arrows
  {116, 117, 150, 151, 186, 187, 188, 201, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
  230, 231, 232, 233, 234, 235, };
const std::vector<int> viRsvd_002C00_002C5F = // Glagolitic
  {47};
const std::vector<int> viRsvd_002C80_002CFF = // Coptic
  {116, 117, 118, 119, 120};
const std::vector<int> viRsvd_002D00_002D2F = // Georgian Supplement
  {38, 40, 41, 42, 43, 44};
const std::vector<int> viRsvd_002D30_002D7F = // Tifinagh
  {56, 57, 58, 59, 60, 61, 62, 65, 66, 67, 68, 69, 70, 71,
  72, 73, 74, 75, 76, 77, 78};
const std::vector<int> viRsvd_002D80_002DDF = // Ethiopic Extended
  {23, 24, 25, 26, 27, 28, 29, 30, 31, 39, 47, 55, 63, 71,
  79, 87};
const std::vector<int> viRsvd_002E80_002EFF = // CJK Radicals Supplement
  {26};
const std::vector<int> viRsvd_003040_00309F = // Hiragana
  {0, 87, 88};
const std::vector<int> viRsvd_003100_00312F = // Bopomofo
  {0, 1, 2, 3, 4};
const std::vector<int> viRsvd_003130_00318F = // Hangul Compatibility Jamo
  {0};
const std::vector<int> viRsvd_003200_0032FF = // Enclosed CJK Letters and Months
  {31};
const std::vector<int> viRsvd_00A720_00A7FF = // Latin Extended D
  {142, 143, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
  164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
  178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
  206, 207, 208, 209, 210, 211, 212, 213, 214};
const std::vector<int> viRsvd_00A880_00A8DF = // Saurashtra
  {69, 70, 71, 72, 73, 74, 75, 76, 77};
const std::vector<int> viRsvd_00A930_00A95F = // Rejang
  {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46};
const std::vector<int> viRsvd_00A980_00A9DF = // Javanese
  {78, 90, 91, 92, 93};
const std::vector<int> viRsvd_00AA00_00AA5F = // Cham
  {55, 56, 57, 58, 59, 60, 61, 62, 63, 78, 79, 90, 91};
const std::vector<int> viRsvd_00AA80_00AADF = // TaiViet
  {67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
  81, 82, 83, 84, 85, 86, 87, 88, 89, 90};
const std::vector<int> viRsvd_00AB00_00AB2F = // Ethiopic Extended A
  {0, 7, 8, 15, 16, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  39};
const std::vector<int> viRsvd_00ABC0_00ABFF = // Meetei Mayek
  {46, 47};
const std::vector<int> viRsvd_00D7B0_00D7FF = // Hangul Jamo Extended B
  {23, 24, 25, 26};
const std::vector<int> viRsvd_00FB00_00FB4F = // Alphabetic Presentation Forms
  {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 24, 25,
  26, 27, 28, 55, 61, 63, 66, 69};
const std::vector<int> viRsvd_00FB50_00FDFF = // Arabic Presentation Forms A
  {114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506,
  507, 508, 509, 510, 511, 576, 577, 632, 633, 634, 635, 636, 637, 638,
  639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652,
  653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666,
  667, 668, 669, 670, 671};
const std::vector<int> viRsvd_00FE50_00FE6F = // Small Form Variants
  {3, 23};
const std::vector<int> viRsvd_00FE70_00FEFF = // Arabic Presentation FormsB
  {5, 141, 142};
const std::vector<int> viRsvd_00FF00_00FFEF = // Halfwidth and Fullwidth Forms
  {0, 191, 192, 193, 200, 201, 208, 209, 216, 217, 221, 222, 223, 231};
const std::vector<int> viRsvd_00FFF0_00FFFF = // Specials
  {0, 1, 2, 3, 4, 5, 6, 7, 8};
const std::vector<int> viRsvd_010000_01007F = // LinearB Syllabary
  {12, 39, 59, 62, 78, 79};
const std::vector<int> viRsvd_010100_01013F = // Aegean Numbers
  {3, 4, 5, 6, 52, 53, 54};
const std::vector<int> viRsvd_010190_0101CF = // Ancient Symbols
  {12, 13, 14, 15};
const std::vector<int> viRsvd_010380_01039F = // Ugaritic
  {30};
const std::vector<int> viRsvd_0103A0_0103DF = // OldPersian
  {36, 37, 38, 39};
const std::vector<int> viRsvd_010480_0104AF = // Osmanya
  {30, 31};
const std::vector<int> viRsvd_010530_01056F = // Caucasian Albanian
  {52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62};
const std::vector<int> viRsvd_010600_01077F = // LinearA
  {311, 312, 313, 314, 315, 316, 317, 318, 319, 342, 343, 344, 345,
  346, 347, 348, 349, 350, 351};
const std::vector<int> viRsvd_010800_01083F = // Cypriot Syllabary
  {6, 7, 9, 54, 57, 58, 59, 61, 62};
const std::vector<int> viRsvd_010840_01085F = // Imperial Aramaic
  {22};
const std::vector<int> viRsvd_010880_0108AF = // Nabataean
  {31, 32, 33, 34, 35, 36, 37, 38};
const std::vector<int> viRsvd_0108E0_0108FF = // Hatran
  {19, 22, 23, 24, 25, 26};
const std::vector<int> viRsvd_010900_01091F = // Phoenician
  {28, 29, 30};
const std::vector<int> viRsvd_010920_01093F = // Lydian
  {26, 27, 28, 29, 30};
const std::vector<int> viRsvd_0109A0_0109FF = // Meroitic Cursive
  {24, 25, 26, 27, 48, 49};
const std::vector<int> viRsvd_010A00_010A5F = // Kharoshthi
  {4, 7, 8, 9, 10, 11, 20, 24, 52, 53, 54, 55, 59, 60,
  61, 62, 72, 73, 74, 75, 76, 77, 78, 79};
const std::vector<int> viRsvd_010AC0_010AFF = // Manichaean
  {39, 40, 41, 42};
const std::vector<int> viRsvd_010B00_010B3F = // Avestan
  {54, 55, 56};
const std::vector<int> viRsvd_010B40_010B5F = // Inscriptional Parthian
  {22, 23};
const std::vector<int> viRsvd_010B60_010B7F = // Inscriptional Pahlavi
  {19, 20, 21, 22, 23};
const std::vector<int> viRsvd_010B80_010BAF = // Psalter Pahlavi
  {18, 19, 20, 21, 22, 23, 24, 29, 30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40};
const std::vector<int> viRsvd_010C80_010CFF = // Old Hungarian
  {51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 115,
  116, 117, 118, 119, 120, 121};
const std::vector<int> viRsvd_011000_01107F = // Brahmi
  {78, 79, 80, 81, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
  122, 123, 124, 125, 126};
const std::vector<int> viRsvd_0110D0_0110FF = // Sora Sompeng
  {25, 26, 27, 28, 29, 30, 31};
const std::vector<int> viRsvd_011100_01114F = // Chakma
  {53};
const std::vector<int> viRsvd_011180_0111DF = // Sharada
  {78, 79};
const std::vector<int> viRsvd_0111E0_0111FF = // Sinhala Archaic Numbers
  {0};
const std::vector<int> viRsvd_011200_01124F = // Khojki
  {18};
const std::vector<int> viRsvd_011280_0112AF = // Multani
  {7, 9, 14, 30};
const std::vector<int> viRsvd_0112B0_0112FF = // Khudawadi
  {59, 60, 61, 62, 63};
const std::vector<int> viRsvd_011300_01137F = // Grantha
  {4, 13, 14, 17, 18, 41, 49, 52, 58, 59, 69, 70, 73, 74,
  78, 79, 81, 82, 83, 84, 85, 86, 88, 89, 90, 91, 92, 100,
  101, 109, 110, 111};
const std::vector<int> viRsvd_011480_0114DF = // Tirhuta
  {72, 73, 74, 75, 76, 77, 78, 79};
const std::vector<int> viRsvd_011580_0115FF = // Siddham
  {54, 55};
const std::vector<int> viRsvd_011600_01165F = // Modi
  {69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79};
const std::vector<int> viRsvd_011680_0116CF = // Takri
  {56, 57, 58, 59, 60, 61, 62, 63};
const std::vector<int> viRsvd_011700_01173F = // Ahom
  {26, 27, 28, 44, 45, 46, 47};
const std::vector<int> viRsvd_0118A0_0118FF = // WarangCiti
  {83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94};
const std::vector<int> viRsvd_012400_01247F = // Cuneifor mNumbers and Punctuation
  {111};
const std::vector<int> viRsvd_016A40_016A6F = // Mro
  {31, 42, 43, 44, 45};
const std::vector<int> viRsvd_016AD0_016AFF = // BassaVah
  {30, 31};
const std::vector<int> viRsvd_016B00_016B8F = // Pahawh Hmong
  {70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 90, 98, 120, 121,
  122, 123, 124};
const std::vector<int> viRsvd_016F00_016F9F = // Miao
  {69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 127, 128, 129,
  130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142};
const std::vector<int> viRsvd_01BC00_01BC9F = // Duployan
  {107, 108, 109, 110, 111, 125, 126, 127, 137, 138, 139, 140, 141, 142,
  143, 154, 155};
const std::vector<int> viRsvd_01D100_01D1FF = // Musical Symbols
  {39, 40};
const std::vector<int> viRsvd_01D400_01D7FF = // Mathematical Alphanumeric Symbols
  {85, 157, 160, 161, 163, 164, 167, 168, 173, 186, 188, 196, 262, 267,
  268, 277, 285, 314, 319, 325, 327, 328, 329, 337, 678, 679, 972, 973};
const std::vector<int> viRsvd_01D800_01DAAF = // Sutton Sign Writing
  {652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665,
  666, 672};
const std::vector<int> viRsvd_01E800_01E8DF = // Mende Kikakui
  {197, 198};
const std::vector<int> viRsvd_01EE00_01EEFF = // Arabic Mathematical Alphabetic Symbols
  {4, 32, 35, 37, 38, 40, 51, 56, 58, 60, 61, 62, 63, 64,
  65, 67, 68, 69, 70, 72, 74, 76, 80, 83, 85, 86, 88, 90,
  92, 94, 96, 99, 101, 102, 107, 115, 120, 125, 127, 138, 156, 157,
  158, 159, 160, 164, 170, 188, 189, 190, 191, 192, 193, 194, 195, 196,
  197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
  211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
  225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
  239};
const std::vector<int> viRsvd_01F0A0_01F0FF = // Playing Cards
  {15, 16, 32, 48};
const std::vector<int> viRsvd_01F100_01F1FF = // Enclosed Alphanumeric Supplement
  {13, 14, 15, 47, 108, 109, 110, 111, 155, 156, 157, 158, 159, 160,
  161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
  175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
  189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
  203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216,
  217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229};
const std::vector<int> viRsvd_01F200_01F2FF = // Enclosed Ideographic Supplement
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 59,
  60, 61, 62, 63, 73, 74, 75, 76, 77, 78, 79};
const std::vector<int> viRsvd_01F300_01F5FF = // Miscellaneous Symbols and Pictographs
  {634, 676};
const std::vector<int> viRsvd_01F680_01F6FF = // Transport and Map Symbols
  {81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
  95, 109, 110, 111};
const std::vector<int> viRsvd_01F800_01F8FF = // Supplemental Arrows C
  {12, 13, 14, 15, 72, 73, 74, 75, 76, 77, 78, 79, 90, 91,
  92, 93, 94, 95, 136, 137, 138, 139, 140, 141, 142, 143};
const std::vector<int> viRsvd_01F900_01F9FF = // Supplemental Symbols and Pictographs
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
  14, 15, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
  37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
  65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
  93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
  107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
  121, 122, 123, 124, 125, 126, 127, 133, 134, 135, 136, 137, 138, 139,
  140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
  154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
  182, 183, 184, 185, 186, 187, 188, 189, 190, 191};

const std::vector<int> vDFIdx_000000_00007F = {19, 190};  //  Basic Latin
const std::vector<int> vDFIdx_000080_0000FF = {19, 190};  //  Latin-1 Supplement
const std::vector<int> vDFIdx_000100_00017F = {19, 173, 22, 34, 28, 21, 38, 13};  //  Latin Extended-A
const std::vector<int> vDFIdx_000180_00024F = {19, 173, 22, 34, 28, 21, 38, 13};  //  Latin Extended-B
const std::vector<int> vDFIdx_000250_0002AF = {19, 173, 22, 34, 28, 21, 38, 13};  //  IPA Extensions
const std::vector<int> vDFIdx_0002B0_0002FF = {19, 173, 22, 34, 28, 21, 38, 13};  //  Spacing Modifier Letters
const std::vector<int> vDFIdx_000300_00036F = {19, 173, 22, 34, 28, 21, 38, 13};  //  Combining Diacritical Marks
const std::vector<int> vDFIdx_000370_0003FF = {72, 9, 173, 22, 187, 19, 13};  //  Greek and Coptic
const std::vector<int> vDFIdx_000400_0004FF = {173, 34, 187, 28, 21, 22};  //  Cyrillic
const std::vector<int> vDFIdx_000500_00052F = {173, 34, 187, 28, 21, 22};  //  Cyrillic Supplement
const std::vector<int> vDFIdx_000530_00058F = {164, 77, 186, 173, 22, 34};  //  Armenian
const std::vector<int> vDFIdx_000590_0005FF = {103, 35, 173, 22, 34};  //  Hebrew
const std::vector<int> vDFIdx_000600_0006FF = {76, 75, 177, 7, 59, 13, 22};  //  Arabic
const std::vector<int> vDFIdx_000700_00074F = {150, 149, 151, 32, 22};  //  Syriac
const std::vector<int> vDFIdx_000750_00077F = {76, 75, 177, 7, 59};  //  Arabic Supplement
const std::vector<int> vDFIdx_000780_0007BF = {70, 22, 34};  //  Thaana
const std::vector<int> vDFIdx_0007C0_0007FF = {127, 30, 24, 22};  //  NKo
const std::vector<int> vDFIdx_000800_00083F = {141, 34, 173};  //  Samaritan
const std::vector<int> vDFIdx_000840_00085F = {122};  //  Mandaic
const std::vector<int> vDFIdx_0008A0_0008FF = {76, 75, 177, 7};  //  Arabic Extended-A
const std::vector<int> vDFIdx_000900_00097F = {94, 181, 10, 51, 22, 13};  //  Devanagari
const std::vector<int> vDFIdx_000980_0009FF = {82, 197, 22, 13};  //  Bengali
const std::vector<int> vDFIdx_000A00_000A7F = {101, 174, 22, 13};  //  Gurmukhi
const std::vector<int> vDFIdx_000A80_000AFF = {100, 180, 22, 13};  //  Gujarati
const std::vector<int> vDFIdx_000B00_000B7F = {134, 44, 22, 13};  //  Oriya
const std::vector<int> vDFIdx_000B80_000BFF = {157, 196, 49, 22, 13};  //  Tamil
const std::vector<int> vDFIdx_000C00_000C7F = {158, 37, 195, 22, 13};  //  Telugu
const std::vector<int> vDFIdx_000C80_000CFF = {109, 193, 22, 13};  //  Kannada
const std::vector<int> vDFIdx_000D00_000D7F = {121, 45, 22, 13};  //  Malayalam
const std::vector<int> vDFIdx_000D80_000DFF = {144, 43};  //  Sinhala
const std::vector<int> vDFIdx_000E00_000E7F = {168, 159, 26, 13, 22};  //  Thai
const std::vector<int> vDFIdx_000E80_000EFF = {167, 114, 26, 13, 22};  //  Lao
const std::vector<int> vDFIdx_000F00_000FFF = {189, 54};  //  Tibetan
const std::vector<int> vDFIdx_001000_00109F = {125, 171, 22};  //  Myanmar
const std::vector<int> vDFIdx_0010A0_0010FF = {165, 97, 173, 22, 34, 13};  //  Georgian
const std::vector<int> vDFIdx_001100_0011FF = {194, 18, 27, 22, 13};  //  Hangul Jamo
const std::vector<int> vDFIdx_001200_00137F = {96, 1, 170, 22};  //  Ethiopic
const std::vector<int> vDFIdx_001380_00139F = {96, 1, 170, 22};  //  Ethiopic Supplement
const std::vector<int> vDFIdx_0013A0_0013FF = {89, 172, 0, 173, 22, 34};  //  Cherokee
const std::vector<int> vDFIdx_001400_00167F = {86, 0, 173, 34, 33, 22};  //  Unified Canadian Aboriginal Syllabics
const std::vector<int> vDFIdx_001680_00169F = {128, 178, 179, 173, 22, 34};  //  Ogham
const std::vector<int> vDFIdx_0016A0_0016FF = {140, 178, 179, 173, 22, 34};  //  Runic
const std::vector<int> vDFIdx_001700_00171F = {152, 173};  //  Tagalog
const std::vector<int> vDFIdx_001720_00173F = {102, 173};  //  Hanunoo
const std::vector<int> vDFIdx_001740_00175F = {85, 173, 22};  //  Buhid
const std::vector<int> vDFIdx_001760_00177F = {153, 173};  //  Tagbanwa
const std::vector<int> vDFIdx_001780_0017FF = {166, 112, 67, 46, 22};  //  Khmer
const std::vector<int> vDFIdx_001800_0018AF = {124, 66};  //  Mongolian
const std::vector<int> vDFIdx_0018B0_0018FF = {86, 0, 173, 34};  //  Unified Canadian Aboriginal Syllabics Extended
const std::vector<int> vDFIdx_001900_00194F = {116, 71, 22};  //  Limbu
const std::vector<int> vDFIdx_001950_00197F = {154, 58};  //  Tai Le
const std::vector<int> vDFIdx_001980_0019DF = {126, 56, 25};  //  New Tai Lue
const std::vector<int> vDFIdx_0019E0_0019FF = {166, 112, 67, 46, 22};  //  Khmer Symbols
const std::vector<int> vDFIdx_001A00_001A1F = {84, 22};  //  Buginese
const std::vector<int> vDFIdx_001A20_001AAF = {155, 48};  //  Tai Tham
const std::vector<int> vDFIdx_001AB0_001AFF = {-1};  //  Combining Diacritical Marks Extended
const std::vector<int> vDFIdx_001B00_001B7F = {79, 6};  //  Balinese
const std::vector<int> vDFIdx_001B80_001BBF = {146, 185};  //  Sundanese
const std::vector<int> vDFIdx_001BC0_001BFF = {81, 17};  //  Batak
const std::vector<int> vDFIdx_001C00_001C4F = {115, 65};  //  Lepcha
const std::vector<int> vDFIdx_001C50_001C7F = {129, 22};  //  Ol Chiki
const std::vector<int> vDFIdx_001CC0_001CCF = {146};  //  Sundanese Supplement
const std::vector<int> vDFIdx_001CD0_001CFF = {181};  //  Vedic Extensions
const std::vector<int> vDFIdx_001D00_001D7F = {19, 173, 22, 34, 28, 21, 38, 13};  //  Phonetic Extensions
const std::vector<int> vDFIdx_001D80_001DBF = {173, 22, 34, 28, 21, 38, 19, 13};  //  Phonetic Extensions Supplement
const std::vector<int> vDFIdx_001DC0_001DFF = {34};  //  Combining Diacritical Marks Supplement
const std::vector<int> vDFIdx_001E00_001EFF = {19, 173, 22, 34, 28, 21, 38, 13};  //  Latin Extended Additional
const std::vector<int> vDFIdx_001F00_001FFF = {9, 72, 8, 19, 173, 34, 38, 22, 13};  //  Greek Extended
const std::vector<int> vDFIdx_002000_00206F = {187, 34, 179, 9, 22, 173};  //  General Punctuation
const std::vector<int> vDFIdx_002070_00209F = {173, 187, 34, 15, 19};  //  Superscripts and Subscripts
const std::vector<int> vDFIdx_0020A0_0020CF = {148, 173, 187, 34, 190, 19};  //  Currency Symbols
const std::vector<int> vDFIdx_0020D0_0020FF = {148, 187, 34, 22};  //  Combining Diacritical Marks for Symbols
const std::vector<int> vDFIdx_002100_00214F = {148, 173, 187, 22, 34, 179};  //  Letterlike Symbols
const std::vector<int> vDFIdx_002150_00218F = {173, 187, 34, 28, 21, 22};  //  Number Forms
const std::vector<int> vDFIdx_002190_0021FF = {148, 19, 179, 173, 187, 22, 34};  //  Arrows
const std::vector<int> vDFIdx_002200_0022FF = {148, 179, 173, 187, 22, 34};  //  Mathematical Operators
const std::vector<int> vDFIdx_002300_0023FF = {148, 173, 187, 22, 34, 179};  //  Miscellaneous Technical
const std::vector<int> vDFIdx_002400_00243F = {148, 179, 22, 173, 34, 13};  //  Control Pictures
const std::vector<int> vDFIdx_002440_00245F = {148, 179, 22, 173, 34, 13, 187};  //  Optical Character Recognition
const std::vector<int> vDFIdx_002460_0024FF = {148, 22, 173, 15, 34, 52, 13};  //  Enclosed Alphanumerics
const std::vector<int> vDFIdx_002500_00257F = {148, 179, 173, 22, 34, 13, 187, 15};  //  Box Drawing
const std::vector<int> vDFIdx_002580_00259F = {148, 173, 22, 34, 187, 15};  //  Block Elements
const std::vector<int> vDFIdx_0025A0_0025FF = {148, 179, 173, 22, 34, 187, 13};  //  Geometric Shapes
const std::vector<int> vDFIdx_002600_0026FF = {148, 187, 34, 179, 22, 173};  //  Miscellaneous Symbols
const std::vector<int> vDFIdx_002700_0027BF = {148, 187, 179, 22, 34, 13, 173};  //  Dingbats
const std::vector<int> vDFIdx_0027C0_0027EF = {148, 173, 187, 22};  //  Miscellaneous Mathematical Symbols-A
const std::vector<int> vDFIdx_0027F0_0027FF = {148, 19, 179, 22, 173, 187, 34};  //  Supplemental Arrows-A
const std::vector<int> vDFIdx_002800_0028FF = {148, 179, 22, 173, 34};  //  Braille Patterns
const std::vector<int> vDFIdx_002900_00297F = {148, 19, 179, 173, 187, 22};  //  Supplemental Arrows-B
const std::vector<int> vDFIdx_002980_0029FF = {148, 179, 173, 187, 22};  //  Miscellaneous Mathematical Symbols-B
const std::vector<int> vDFIdx_002A00_002AFF = {148, 19, 179, 173, 22, 187};  //  Supplemental Mathematical Operators
const std::vector<int> vDFIdx_002B00_002BFF = {173, 187, 22};  //  Miscellaneous Symbols and Arrows
const std::vector<int> vDFIdx_002C00_002C5F = {98, 178, 173};  //  Glagolitic
const std::vector<int> vDFIdx_002C60_002C7F = {173, 34, 22, 28, 21, 38};  //  Latin Extended-C
const std::vector<int> vDFIdx_002C80_002CFF = {90, 73, 9, 72, 8, 173, 22};  //  Coptic
const std::vector<int> vDFIdx_002D00_002D2F = {97, 173, 34};  //  Georgian Supplement
const std::vector<int> vDFIdx_002D30_002D7F = {160, 34, 173, 30, 22};  //  Tifinagh
const std::vector<int> vDFIdx_002D80_002DDF = {96, 1, 170, 22};  //  Ethiopic Extended
const std::vector<int> vDFIdx_002DE0_002DFF = {173, 34};  //  Cyrillic Extended-A
const std::vector<int> vDFIdx_002E00_002E7F = {148, 173, 34, 22};  //  Supplemental Punctuation
const std::vector<int> vDFIdx_002E80_002EFF = {15, 42, 40, 22};  //  CJK Radicals Supplement
const std::vector<int> vDFIdx_002F00_002FDF = {15, 42, 40, 22, 52};  //  Kangxi Radicals
const std::vector<int> vDFIdx_002FF0_002FFF = {15, 42, 40, 22, 173};  //  Ideographic Description Characters
const std::vector<int> vDFIdx_003000_00303F = {55, 60, 40, 22, 13, 15};  //  CJK Symbols and Punctuation
const std::vector<int> vDFIdx_003040_00309F = {52, 55, 60, 15, 22};  //  Hiragana
const std::vector<int> vDFIdx_0030A0_0030FF = {52, 55, 60, 15, 22};  //  Katakana
const std::vector<int> vDFIdx_003100_00312F = {15, 22, 13, 55, 60};  //  Bopomofo
const std::vector<int> vDFIdx_003130_00318F = {194, 18, 27, 22, 13};  //  Hangul Compatibility Jamo
const std::vector<int> vDFIdx_003190_00319F = {52, 55, 60, 15, 40, 22, 13};  //  Kanbun
const std::vector<int> vDFIdx_0031A0_0031BF = {15, 22, 55, 60};  //  Bopomofo Extended
const std::vector<int> vDFIdx_0031C0_0031EF = {15, 42, 22};  //  CJK Strokes
const std::vector<int> vDFIdx_0031F0_0031FF = {52, 55, 60, 15, 42, 40, 22};  //  Katakana Phonetic Extensions
const std::vector<int> vDFIdx_003200_0032FF = {22, 13, 15, 52, 42};  //  Enclosed CJK Letters and Months
const std::vector<int> vDFIdx_003300_0033FF = {22, 13, 52};  //  CJK Compatibility
const std::vector<int> vDFIdx_003400_004DBF = {42, 62, 40, 15};  //  CJK Unified Ideographs Extension A
const std::vector<int> vDFIdx_004DC0_004DFF = {148, 179, 187, 22, 34, 15, 173};  //  Yijing Hexagram Symbols
const std::vector<int> vDFIdx_004E00_009FFF = {15, 42, 62, 40};  //  CJK Unified Ideographs
const std::vector<int> vDFIdx_00A000_00A48F = {163, 61, 169, 182, 22};  //  Yi Syllables
const std::vector<int> vDFIdx_00A490_00A4CF = {163, 61, 169, 182, 22};  //  Yi Radicals
const std::vector<int> vDFIdx_00A4D0_00A4FF = {118, 50, 173, 34};  //  Lisu
const std::vector<int> vDFIdx_00A500_00A63F = {162, 30, 29, 198, 22};  //  Vai
const std::vector<int> vDFIdx_00A640_00A69F = {173, 34, 22};  //  Cyrillic Extended-B
const std::vector<int> vDFIdx_00A6A0_00A6FF = {80};  //  Bamum
const std::vector<int> vDFIdx_00A700_00A71F = {148, 173, 28, 21, 38, 22};  //  Modifier Tone Letters
const std::vector<int> vDFIdx_00A720_00A7FF = {173, 34, 22, 28, 21, 38};  //  Latin Extended-D
const std::vector<int> vDFIdx_00A800_00A82F = {147};  //  Syloti Nagri
const std::vector<int> vDFIdx_00A830_00A83F = {181, 74};  //  Common Indic Number Forms
const std::vector<int> vDFIdx_00A840_00A87F = {136, 137, 57, 16, 22};  //  Phags-pa
const std::vector<int> vDFIdx_00A880_00A8DF = {142, 22};  //  Saurashtra
const std::vector<int> vDFIdx_00A8E0_00A8FF = {181};  //  Devanagari Extended
const std::vector<int> vDFIdx_00A900_00A92F = {110, 22};  //  Kayah Li
const std::vector<int> vDFIdx_00A930_00A95F = {139, 22, 34};  //  Rejang
const std::vector<int> vDFIdx_00A960_00A97F = {194};  //  Hangul Jamo Extended-A
const std::vector<int> vDFIdx_00A980_00A9DF = {107, 192};  //  Javanese
const std::vector<int> vDFIdx_00A9E0_00A9FF = {-1};  //  Myanmar Extended-B
const std::vector<int> vDFIdx_00AA00_00AA5F = {88, 22};  //  Cham
const std::vector<int> vDFIdx_00AA60_00AA7F = {125, 171};  //  Myanmar Extended-A
const std::vector<int> vDFIdx_00AA80_00AADF = {156, 188};  //  Tai Viet
const std::vector<int> vDFIdx_00AAE0_00AAFF = {123};  //  Meetei Mayek Extensions
const std::vector<int> vDFIdx_00AB00_00AB2F = {96, 1};  //  Ethiopic Extended-A
const std::vector<int> vDFIdx_00AB30_00AB6F = {34};  //  Latin Extended-E
const std::vector<int> vDFIdx_00AB70_00ABBF = {89, 172, 0, 173, 22, 34};  //  Cherokee Supplement
const std::vector<int> vDFIdx_00ABC0_00ABFF = {123, 31};  //  Meetei Mayek
const std::vector<int> vDFIdx_00AC00_00D7AF = {113, 183, 184, 18, 27, 39, 13, 22};  //  Hangul Syllables
const std::vector<int> vDFIdx_00D7B0_00D7FF = {194};  //  Hangul Jamo Extended-B
//const std::vector<int> vDFIdx_00D800_00DB7F = {-1};  //  High Surrogates
//const std::vector<int> vDFIdx_00DB80_00DBFF = {-1};  //  High Private Use Surrogates
//const std::vector<int> vDFIdx_00DC00_00DFFF = {-1};  //  Low Surrogates
const std::vector<int> vDFIdx_00E000_00F8FF = {-1};  //  Private Use Area
const std::vector<int> vDFIdx_00F900_00FAFF = {42, 15, 40};  //  CJK Compatibility Ideographs
const std::vector<int> vDFIdx_00FB00_00FB4F = {173, 22, 34, 13};  //  Alphabetic Presentation Forms
const std::vector<int> vDFIdx_00FB50_00FDFF = {76, 75, 7, 13, 11};  //  Arabic Presentation Forms-A
const std::vector<int> vDFIdx_00FE00_00FE0F = {15, 22};  //  Variation Selectors
const std::vector<int> vDFIdx_00FE10_00FE1F = {55, 60, 15, 187};  //  Vertical Forms
const std::vector<int> vDFIdx_00FE20_00FE2F = {187, 34};  //  Combining Half Marks
const std::vector<int> vDFIdx_00FE30_00FE4F = {55, 60, 187, 13, 22};  //  CJK Compatibility Forms
const std::vector<int> vDFIdx_00FE50_00FE6F = {55, 60, 15, 13, 22};  //  Small Form Variants
const std::vector<int> vDFIdx_00FE70_00FEFF = {76, 75, 191, 7, 22, 13};  //  Arabic Presentation Forms-B
const std::vector<int> vDFIdx_00FF00_00FFEF = {55, 60, 13, 22};  //  Halfwidth and Fullwidth Forms
const std::vector<int> vDFIdx_00FFF0_00FFFF = {148, 15, 189, 187, 34};  //  Specials
const std::vector<int> vDFIdx_010000_01007F = {117, 2, 23, 34};  //  Linear B Syllabary
const std::vector<int> vDFIdx_010080_0100FF = {117, 2, 23, 34};  //  Linear B Ideograms
const std::vector<int> vDFIdx_010100_01013F = {148, 2, 23, 34};  //  Aegean Numbers
const std::vector<int> vDFIdx_010140_01018F = {173, 14, 2, 72, 34};  //  Ancient Greek Numbers
const std::vector<int> vDFIdx_010190_0101CF = {173, 2, 72, 34};  //  Ancient Symbols
const std::vector<int> vDFIdx_0101D0_0101FF = {2, 34, 23};  //  Phaistos Disc
const std::vector<int> vDFIdx_010280_01029F = {119, 178, 173, 2, 34};  //  Lycian
const std::vector<int> vDFIdx_0102A0_0102DF = {87, 178, 173, 2, 34};  //  Carian
const std::vector<int> vDFIdx_0102E0_0102FF = {73};  //  Coptic Epact Numbers
const std::vector<int> vDFIdx_010300_01032F = {130, 178, 173, 2, 23, 72, 34};  //  Old Italic
const std::vector<int> vDFIdx_010330_01034F = {99, 178, 179, 176, 173, 8, 23, 34};  //  Gothic
const std::vector<int> vDFIdx_010350_01037F = {-1};  //  Old Permic
const std::vector<int> vDFIdx_010380_01039F = {161, 178, 2, 23, 34};  //  Ugaritic
const std::vector<int> vDFIdx_0103A0_0103DF = {131, 178, 2, 23};  //  Old Persian
const std::vector<int> vDFIdx_010400_01044F = {93, 179, 8, 23, 34};  //  Deseret
const std::vector<int> vDFIdx_010450_01047F = {143, 178, 23, 34};  //  Shavian
const std::vector<int> vDFIdx_010480_0104AF = {135, 30, 23, 34};  //  Osmanya
const std::vector<int> vDFIdx_010500_01052F = {-1};  //  Elbasan
const std::vector<int> vDFIdx_010530_01056F = {-1};  //  Caucasian Albanian
const std::vector<int> vDFIdx_010600_01077F = {2};  //  Linear A
const std::vector<int> vDFIdx_010800_01083F = {92, 178, 2, 34, 23};  //  Cypriot Syllabary
const std::vector<int> vDFIdx_010840_01085F = {104, 178, 12, 34, 173};  //  Imperial Aramaic
const std::vector<int> vDFIdx_010860_01087F = {-1};  //  Palmyrene
const std::vector<int> vDFIdx_010880_0108AF = {-1};  //  Nabataean
const std::vector<int> vDFIdx_0108E0_0108FF = {-1};  //  Hatran
const std::vector<int> vDFIdx_010900_01091F = {138, 178, 2, 34, 173, 23};  //  Phoenician
const std::vector<int> vDFIdx_010920_01093F = {120, 178, 173, 2, 34};  //  Lydian
const std::vector<int> vDFIdx_010980_01099F = {73};  //  Meroitic Hieroglyphs
const std::vector<int> vDFIdx_0109A0_0109FF = {178, 73};  //  Meroitic Cursive
const std::vector<int> vDFIdx_010A00_010A5F = {111, 178};  //  Kharoshthi
const std::vector<int> vDFIdx_010A60_010A7F = {132, 178, 173, 34};  //  Old South Arabian
const std::vector<int> vDFIdx_010A80_010A9F = {-1};  //  Old North Arabian
const std::vector<int> vDFIdx_010AC0_010AFF = {-1};  //  Manichaean
const std::vector<int> vDFIdx_010B00_010B3F = {78, 4};  //  Avestan
const std::vector<int> vDFIdx_010B40_010B5F = {106, 178, 199};  //  Inscriptional Parthian
const std::vector<int> vDFIdx_010B60_010B7F = {105, 178, 199};  //  Inscriptional Pahlavi
const std::vector<int> vDFIdx_010B80_010BAF = {-1};  //  Psalter Pahlavi
const std::vector<int> vDFIdx_010C00_010C4F = {133, 178, 173};  //  Old Turkic
const std::vector<int> vDFIdx_010C80_010CFF = {-1};  //  Old Hungarian
const std::vector<int> vDFIdx_010E60_010E7F = {42};  //  Rumi Numeral Symbols
const std::vector<int> vDFIdx_011000_01107F = {83, 178};  //  Brahmi
const std::vector<int> vDFIdx_011080_0110CF = {108};  //  Kaithi
const std::vector<int> vDFIdx_0110D0_0110FF = {74};  //  Sora Sompeng
const std::vector<int> vDFIdx_011100_01114F = {175};  //  Chakma
const std::vector<int> vDFIdx_011150_01117F = {-1};  //  Mahajani
const std::vector<int> vDFIdx_011180_0111DF = {-1};  //  Sharada
const std::vector<int> vDFIdx_0111E0_0111FF = {-1};  //  Sinhala Archaic Numbers
const std::vector<int> vDFIdx_011200_01124F = {-1};  //  Khojki
const std::vector<int> vDFIdx_011280_0112AF = {-1};  //  Multani
const std::vector<int> vDFIdx_0112B0_0112FF = {-1};  //  Khudawadi
const std::vector<int> vDFIdx_011300_01137F = {-1};  //  Grantha
const std::vector<int> vDFIdx_011480_0114DF = {-1};  //  Tirhuta
const std::vector<int> vDFIdx_011580_0115FF = {-1};  //  Siddham
const std::vector<int> vDFIdx_011600_01165F = {-1};  //  Modi
const std::vector<int> vDFIdx_011680_0116CF = {-1};  //  Takri
const std::vector<int> vDFIdx_011700_01173F = {-1};  //  Ahom
const std::vector<int> vDFIdx_0118A0_0118FF = {-1};  //  Warang Citi
const std::vector<int> vDFIdx_011AC0_011AFF = {-1};  //  Pau Cin Hau
const std::vector<int> vDFIdx_012000_0123FF = {91, 145, 178, 5};  //  Cuneiform
const std::vector<int> vDFIdx_012400_01247F = {91, 145, 178, 5};  //  Cuneiform Numbers and Punctuation
const std::vector<int> vDFIdx_012480_01254F = {91, 145, 178, 5};  //  Early Dynastic Cuneiform
const std::vector<int> vDFIdx_013000_01342F = {95, 178, 36, 3};  //  Egyptian Hieroglyphs
const std::vector<int> vDFIdx_014400_01467F = {-1};  //  Anatolian Hieroglyphs
const std::vector<int> vDFIdx_016800_016A3F = {80};  //  Bamum Supplement
const std::vector<int> vDFIdx_016A40_016A6F = {68};  //  Mro
const std::vector<int> vDFIdx_016AD0_016AFF = {-1};  //  Bassa Vah
const std::vector<int> vDFIdx_016B00_016B8F = {-1};  //  Pahawh Hmong
const std::vector<int> vDFIdx_016F00_016F9F = {53};  //  Miao
const std::vector<int> vDFIdx_01B000_01B0FF = {15, 42};  //  Kana Supplement
const std::vector<int> vDFIdx_01BC00_01BC9F = {-1};  //  Duployan
const std::vector<int> vDFIdx_01BCA0_01BCAF = {-1};  //  Shorthand Format Controls
const std::vector<int> vDFIdx_01D000_01D0FF = {187, 14, 69};  //  Byzantine Musical Symbols
const std::vector<int> vDFIdx_01D100_01D1FF = {187, 69, 173, 23};  //  Musical Symbols
const std::vector<int> vDFIdx_01D200_01D24F = {148, 187, 69, 14, 2, 173, 72, 34};  //  Ancient Greek Musical Notation
const std::vector<int> vDFIdx_01D300_01D35F = {148, 179, 173, 187, 23, 34, 15};  //  Tai Xuan Jing Symbols
const std::vector<int> vDFIdx_01D360_01D37F = {148, 173, 187, 23, 15};  //  Counting Rod Numerals
const std::vector<int> vDFIdx_01D400_01D7FF = {148, 187, 20, 23, 34, 173};  //  Mathematical Alphanumeric Symbols
const std::vector<int> vDFIdx_01D800_01DAAF = {-1};  //  Sutton SignWriting
const std::vector<int> vDFIdx_01E800_01E8DF = {-1};  //  Mende Kikakui
const std::vector<int> vDFIdx_01EE00_01EEFF = {7};  //  Arabic Mathematical Alphabetical Symbols
const std::vector<int> vDFIdx_01F000_01F02F = {148, 187, 173};  //  Mahjong Tiles
const std::vector<int> vDFIdx_01F030_01F09F = {148, 187, 34, 173, 23};  //  Domino Tiles
const std::vector<int> vDFIdx_01F0A0_01F0FF = {148, 187, 34, 173};  //  Playing Cards
const std::vector<int> vDFIdx_01F100_01F1FF = {173, 15};  //  Enclosed Alphanumeric Supplement
const std::vector<int> vDFIdx_01F200_01F2FF = {148, 15, 42};  //  Enclosed Ideographic Supplement
const std::vector<int> vDFIdx_01F300_01F5FF = {187, 173};  //  Miscellaneous Symbols and Pictographs
const std::vector<int> vDFIdx_01F600_01F64F = {187, 173};  //  Emoticons
const std::vector<int> vDFIdx_01F650_01F67F = {187};  //  Ornamental Dingbats
const std::vector<int> vDFIdx_01F680_01F6FF = {187};  //  Transport and Map Symbols
const std::vector<int> vDFIdx_01F700_01F77F = {148, 187, 34, 173};  //  Alchemical Symbols
const std::vector<int> vDFIdx_01F780_01F7FF = {187};  //  Geometric Shapes Extended
const std::vector<int> vDFIdx_01F800_01F8FF = {187};  //  Supplemental Arrows-C
const std::vector<int> vDFIdx_01F900_01F9FF = {187, 173};  //  Supplemental Symbols and Pictographs
const std::vector<int> vDFIdx_020000_02A6DF = {64, 63, 41, 15};  //  CJK Unified Ideographs Extension B
const std::vector<int> vDFIdx_02A700_02B73F = {15, 42};  //  CJK Unified Ideographs Extension C
const std::vector<int> vDFIdx_02B740_02B81F = {15, 42};  //  CJK Unified Ideographs Extension D
const std::vector<int> vDFIdx_02B820_02CEAF = {15, 42};  //  CJK Unified Ideographs Extension E
const std::vector<int> vDFIdx_02F800_02FA1F = {42, 41, 15};  //  CJK Compatibility Ideographs Supplement
const std::vector<int> vDFIdx_0E0000_0E007F = {15, 23};  //  Tags
const std::vector<int> vDFIdx_0E0100_0E01EF = {15};  //  Variation Selectors Supplement
const std::vector<int> vDFIdx_0F0000_0FFFFF = {-1};  //  Supplementary Private Use Area-A
const std::vector<int> vDFIdx_100000_10FFFF = {-1};  //  Supplementary Private Use Area-B

std::vector<unicode_block> vUCBlocks = {
  //{ Idx,UF,PF,BF,Low, High, Max, #, Name, Reserved Values, Fonts },
  {0, -1, -1, -1, 0x000000, 0x00007F, 0x00007E, 95, L"Basic Latin", &viRsvd_000000_00007F, &vDFIdx_000000_00007F},
  {1, -1, -1, -1, 0x000080, 0x0000FF, 0x0000FF, 96, L"Latin-1 Supplement", &viRsvd_000080_0000FF, &vDFIdx_000080_0000FF},
  {2, -1, -1, -1, 0x000100, 0x00017F, 0x00017F, 128, L"Latin Extended-A", NULL, &vDFIdx_000100_00017F},
  {3, -1, -1, -1, 0x000180, 0x00024F, 0x00024F, 208, L"Latin Extended-B", NULL, &vDFIdx_000180_00024F},
  {4, -1, -1, -1, 0x000250, 0x0002AF, 0x0002AF, 96, L"IPA Extensions", NULL, &vDFIdx_000250_0002AF},
  {5, -1, -1, -1, 0x0002B0, 0x0002FF, 0x0002FF, 80, L"Spacing Modifier Letters", NULL, &vDFIdx_0002B0_0002FF},
  {6, -1, -1, -1, 0x000300, 0x00036F, 0x00036F, 112, L"Combining Diacritical Marks", NULL, &vDFIdx_000300_00036F},
  {7, -1, -1, -1, 0x000370, 0x0003FF, 0x0003FF, 135, L"Greek and Coptic", &viRsvd_000370_0003FF, &vDFIdx_000370_0003FF},
  {8, -1, -1, -1, 0x000400, 0x0004FF, 0x0004FF, 256, L"Cyrillic", NULL, &vDFIdx_000400_0004FF},
  {9, -1, -1, -1, 0x000500, 0x00052F, 0x00052F, 48, L"Cyrillic Supplement", NULL, &vDFIdx_000500_00052F},
  {10, -1, -1, -1, 0x000530, 0x00058F, 0x00058F, 89, L"Armenian", &viRsvd_000530_00058F, &vDFIdx_000530_00058F},
  {11, -1, -1, -1, 0x000590, 0x0005FF, 0x0005F4, 87, L"Hebrew", &viRsvd_000590_0005FF, &vDFIdx_000590_0005FF},
  {12, -1, -1, -1, 0x000600, 0x0006FF, 0x0006FF, 255, L"Arabic", &viRsvd_000600_0006FF, &vDFIdx_000600_0006FF},
  {13, -1, -1, -1, 0x000700, 0x00074F, 0x00074F, 77, L"Syriac", &viRsvd_000700_00074F, &vDFIdx_000700_00074F},
  {14, -1, -1, -1, 0x000750, 0x00077F, 0x00077F, 48, L"Arabic Supplement", NULL, &vDFIdx_000750_00077F},
  {15, -1, -1, -1, 0x000780, 0x0007BF, 0x0007B1, 50, L"Thaana", NULL, &vDFIdx_000780_0007BF},
  {16, -1, -1, -1, 0x0007C0, 0x0007FF, 0x0007FA, 59, L"NKo", NULL, &vDFIdx_0007C0_0007FF},
  {17, -1, -1, -1, 0x000800, 0x00083F, 0x00083E, 61, L"Samaritan", &viRsvd_000800_00083F, &vDFIdx_000800_00083F},
  {18, -1, -1, -1, 0x000840, 0x00085F, 0x00085E, 29, L"Mandaic", &viRsvd_000840_00085F, &vDFIdx_000840_00085F},
  {19, -1, -1, -1, 0x0008A0, 0x0008FF, 0x0008FF, 50, L"Arabic Extended-A", &viRsvd_0008A0_0008FF, &vDFIdx_0008A0_0008FF},
  {20, -1, -1, -1, 0x000900, 0x00097F, 0x00097F, 128, L"Devanagari", NULL, &vDFIdx_000900_00097F},
  {21, -1, -1, -1, 0x000980, 0x0009FF, 0x0009FB, 93, L"Bengali", &viRsvd_000980_0009FF, &vDFIdx_000980_0009FF},
  {22, -1, -1, -1, 0x000A00, 0x000A7F, 0x000A75, 79, L"Gurmukhi", &viRsvd_000A00_000A7F, &vDFIdx_000A00_000A7F},
  {23, -1, -1, -1, 0x000A80, 0x000AFF, 0x000AF9, 85, L"Gujarati", &viRsvd_000A80_000AFF, &vDFIdx_000A80_000AFF},
  {24, -1, -1, -1, 0x000B00, 0x000B7F, 0x000B77, 90, L"Oriya", &viRsvd_000B00_000B7F, &vDFIdx_000B00_000B7F},
  {25, -1, -1, -1, 0x000B80, 0x000BFF, 0x000BFA, 72, L"Tamil", &viRsvd_000B80_000BFF, &vDFIdx_000B80_000BFF},
  {26, -1, -1, -1, 0x000C00, 0x000C7F, 0x000C7F, 96, L"Telugu", &viRsvd_000C00_000C7F, &vDFIdx_000C00_000C7F},
  {27, -1, -1, -1, 0x000C80, 0x000CFF, 0x000CF2, 87, L"Kannada", &viRsvd_000C80_000CFF, &vDFIdx_000C80_000CFF},
  {28, -1, -1, -1, 0x000D00, 0x000D7F, 0x000D7F, 100, L"Malayalam", &viRsvd_000D00_000D7F, &vDFIdx_000D00_000D7F},
  {29, -1, -1, -1, 0x000D80, 0x000DFF, 0x000DF4, 90, L"Sinhala", &viRsvd_000D80_000DFF, &vDFIdx_000D80_000DFF},
  {30, -1, -1, -1, 0x000E00, 0x000E7F, 0x000E5B, 87, L"Thai", &viRsvd_000E00_000E7F, &vDFIdx_000E00_000E7F},
  {31, -1, -1, -1, 0x000E80, 0x000EFF, 0x000EDF, 67, L"Lao", &viRsvd_000E80_000EFF, &vDFIdx_000E80_000EFF},
  {32, -1, -1, -1, 0x000F00, 0x000FFF, 0x000FDA, 211, L"Tibetan", &viRsvd_000F00_000FFF, &vDFIdx_000F00_000FFF},
  {33, -1, -1, -1, 0x001000, 0x00109F, 0x00109F, 160, L"Myanmar", NULL, &vDFIdx_001000_00109F},
  {34, -1, -1, -1, 0x0010A0, 0x0010FF, 0x0010FF, 88, L"Georgian", &viRsvd_0010A0_0010FF, &vDFIdx_0010A0_0010FF},
  {35, -1, -1, -1, 0x001100, 0x0011FF, 0x0011FF, 256, L"Hangul Jamo", NULL, &vDFIdx_001100_0011FF},
  {36, -1, -1, -1, 0x001200, 0x00137F, 0x00137C, 358, L"Ethiopic", &viRsvd_001200_00137F, &vDFIdx_001200_00137F},
  {37, -1, -1, -1, 0x001380, 0x00139F, 0x001399, 26, L"Ethiopic Supplement", NULL, &vDFIdx_001380_00139F},
  {38, -1, -1, -1, 0x0013A0, 0x0013FF, 0x0013FD, 92, L"Cherokee", &viRsvd_0013A0_0013FF, &vDFIdx_0013A0_0013FF},
  {39, -1, -1, -1, 0x001400, 0x00167F, 0x00167F, 640, L"Unified Canadian Aboriginal Syllabics", NULL, &vDFIdx_001400_00167F},
  {40, -1, -1, -1, 0x001680, 0x00169F, 0x00169C, 29, L"Ogham", NULL, &vDFIdx_001680_00169F},
  {41, -1, -1, -1, 0x0016A0, 0x0016FF, 0x0016F8, 89, L"Runic", NULL, &vDFIdx_0016A0_0016FF},
  {42, -1, -1, -1, 0x001700, 0x00171F, 0x001714, 20, L"Tagalog", &viRsvd_001700_00171F, &vDFIdx_001700_00171F},
  {43, -1, -1, -1, 0x001720, 0x00173F, 0x001736, 23, L"Hanunoo", NULL, &vDFIdx_001720_00173F},
  {44, -1, -1, -1, 0x001740, 0x00175F, 0x001753, 20, L"Buhid", NULL, &vDFIdx_001740_00175F},
  {45, -1, -1, -1, 0x001760, 0x00177F, 0x001773, 18, L"Tagbanwa", &viRsvd_001760_00177F, &vDFIdx_001760_00177F},
  {46, -1, -1, -1, 0x001780, 0x0017FF, 0x0017F9, 114, L"Khmer", &viRsvd_001780_0017FF, &vDFIdx_001780_0017FF},
  {47, -1, -1, -1, 0x001800, 0x0018AF, 0x0018AA, 156, L"Mongolian", &viRsvd_001800_0018AF, &vDFIdx_001800_0018AF},
  {48, -1, -1, -1, 0x0018B0, 0x0018FF, 0x0018F5, 70, L"Unified Canadian Aboriginal Syllabics Extended", NULL, &vDFIdx_0018B0_0018FF},
  {49, -1, -1, -1, 0x001900, 0x00194F, 0x00194F, 68, L"Limbu", &viRsvd_001900_00194F, &vDFIdx_001900_00194F},
  {50, -1, -1, -1, 0x001950, 0x00197F, 0x001974, 35, L"Tai Le", &viRsvd_001950_00197F, &vDFIdx_001950_00197F},
  {51, -1, -1, -1, 0x001980, 0x0019DF, 0x0019DF, 83, L"New Tai Lue", &viRsvd_001980_0019DF, &vDFIdx_001980_0019DF},
  {52, -1, -1, -1, 0x0019E0, 0x0019FF, 0x0019FF, 32, L"Khmer Symbols", NULL, &vDFIdx_0019E0_0019FF},
  {53, -1, -1, -1, 0x001A00, 0x001A1F, 0x001A1F, 30, L"Buginese", &viRsvd_001A00_001A1F, &vDFIdx_001A00_001A1F},
  {54, -1, -1, -1, 0x001A20, 0x001AAF, 0x001AAD, 127, L"Tai Tham", &viRsvd_001A20_001AAF, &vDFIdx_001A20_001AAF},
  {55, -1, -1, -1, 0x001AB0, 0x001AFF, 0x001ABE, 15, L"Combining Diacritical Marks Extended", NULL, &vDFIdx_001AB0_001AFF},
  {56, -1, -1, -1, 0x001B00, 0x001B7F, 0x001B7C, 121, L"Balinese", &viRsvd_001B00_001B7F, &vDFIdx_001B00_001B7F},
  {57, -1, -1, -1, 0x001B80, 0x001BBF, 0x001BBF, 64, L"Sundanese", NULL, &vDFIdx_001B80_001BBF},
  {58, -1, -1, -1, 0x001BC0, 0x001BFF, 0x001BFF, 56, L"Batak", &viRsvd_001BC0_001BFF, &vDFIdx_001BC0_001BFF},
  {59, -1, -1, -1, 0x001C00, 0x001C4F, 0x001C4F, 74, L"Lepcha", &viRsvd_001C00_001C4F, &vDFIdx_001C00_001C4F},
  {60, -1, -1, -1, 0x001C50, 0x001C7F, 0x001C7F, 48, L"Ol Chiki", NULL, &vDFIdx_001C50_001C7F},
  {61, -1, -1, -1, 0x001CC0, 0x001CCF, 0x001CC7, 8, L"Sundanese Supplement", NULL, &vDFIdx_001CC0_001CCF},
  {62, -1, -1, -1, 0x001CD0, 0x001CFF, 0x001CF9, 41, L"Vedic Extensions", &viRsvd_001CD0_001CFF, &vDFIdx_001CD0_001CFF},
  {63, -1, -1, -1, 0x001D00, 0x001D7F, 0x001D7F, 128, L"Phonetic Extensions", NULL, &vDFIdx_001D00_001D7F},
  {64, -1, -1, -1, 0x001D80, 0x001DBF, 0x001DBF, 64, L"Phonetic Extensions Supplement", NULL, &vDFIdx_001D80_001DBF},
  {65, -1, -1, -1, 0x001DC0, 0x001DFF, 0x001DFF, 58, L"Combining Diacritical Marks Supplement", &viRsvd_001DC0_001DFF, &vDFIdx_001DC0_001DFF},
  {66, -1, -1, -1, 0x001E00, 0x001EFF, 0x001EFF, 256, L"Latin Extended Additional", NULL, &vDFIdx_001E00_001EFF},
  {67, -1, -1, -1, 0x001F00, 0x001FFF, 0x001FFE, 233, L"Greek Extended", &viRsvd_001F00_001FFF, &vDFIdx_001F00_001FFF},
  {68, -1, -1, -1, 0x002000, 0x00206F, 0x00206F, 111, L"General Punctuation", &viRsvd_002000_00206F, &vDFIdx_002000_00206F},
  {69, -1, -1, -1, 0x002070, 0x00209F, 0x00209C, 42, L"Superscripts and Subscripts", &viRsvd_002070_00209F, &vDFIdx_002070_00209F},
  {70, -1, -1, -1, 0x0020A0, 0x0020CF, 0x0020BE, 31, L"Currency Symbols", NULL, &vDFIdx_0020A0_0020CF},
  {71, -1, -1, -1, 0x0020D0, 0x0020FF, 0x0020F0, 33, L"Combining Diacritical Marks for Symbols", NULL, &vDFIdx_0020D0_0020FF},
  {72, -1, -1, -1, 0x002100, 0x00214F, 0x00214F, 80, L"Letterlike Symbols", NULL, &vDFIdx_002100_00214F},
  {73, -1, -1, -1, 0x002150, 0x00218F, 0x00218B, 60, L"Number Forms", NULL, &vDFIdx_002150_00218F},
  {74, -1, -1, -1, 0x002190, 0x0021FF, 0x0021FF, 112, L"Arrows", NULL, &vDFIdx_002190_0021FF},
  {75, -1, -1, -1, 0x002200, 0x0022FF, 0x0022FF, 256, L"Mathematical Operators", NULL, &vDFIdx_002200_0022FF},
  {76, -1, -1, -1, 0x002300, 0x0023FF, 0x0023FA, 251, L"Miscellaneous Technical", NULL, &vDFIdx_002300_0023FF},
  {77, -1, -1, -1, 0x002400, 0x00243F, 0x002426, 39, L"Control Pictures", NULL, &vDFIdx_002400_00243F},
  {78, -1, -1, -1, 0x002440, 0x00245F, 0x00244A, 11, L"Optical Character Recognition", NULL, &vDFIdx_002440_00245F},
  {79, -1, -1, -1, 0x002460, 0x0024FF, 0x0024FF, 160, L"Enclosed Alphanumerics", NULL, &vDFIdx_002460_0024FF},
  {80, -1, -1, -1, 0x002500, 0x00257F, 0x00257F, 128, L"Box Drawing", NULL, &vDFIdx_002500_00257F},
  {81, -1, -1, -1, 0x002580, 0x00259F, 0x00259F, 32, L"Block Elements", NULL, &vDFIdx_002580_00259F},
  {82, -1, -1, -1, 0x0025A0, 0x0025FF, 0x0025FF, 96, L"Geometric Shapes", NULL, &vDFIdx_0025A0_0025FF},
  {83, -1, -1, -1, 0x002600, 0x0026FF, 0x0026FF, 256, L"Miscellaneous Symbols", NULL, &vDFIdx_002600_0026FF},
  {84, -1, -1, -1, 0x002700, 0x0027BF, 0x0027BF, 192, L"Dingbats", NULL, &vDFIdx_002700_0027BF},
  {85, -1, -1, -1, 0x0027C0, 0x0027EF, 0x0027EF, 48, L"Miscellaneous Mathematical Symbols-A", NULL, &vDFIdx_0027C0_0027EF},
  {86, -1, -1, -1, 0x0027F0, 0x0027FF, 0x0027FF, 16, L"Supplemental Arrows-A", NULL, &vDFIdx_0027F0_0027FF},
  {87, -1, -1, -1, 0x002800, 0x0028FF, 0x0028FF, 256, L"Braille Patterns", NULL, &vDFIdx_002800_0028FF},
  {88, -1, -1, -1, 0x002900, 0x00297F, 0x00297F, 128, L"Supplemental Arrows-B", NULL, &vDFIdx_002900_00297F},
  {89, -1, -1, -1, 0x002980, 0x0029FF, 0x0029FF, 128, L"Miscellaneous Mathematical Symbols-B", NULL, &vDFIdx_002980_0029FF},
  {90, -1, -1, -1, 0x002A00, 0x002AFF, 0x002AFF, 256, L"Supplemental Mathematical Operators", NULL, &vDFIdx_002A00_002AFF},
  {91, -1, -1, -1, 0x002B00, 0x002BFF, 0x002BEF, 206, L"Miscellaneous Symbols and Arrows", &viRsvd_002B00_002BFF, &vDFIdx_002B00_002BFF},
  {92, -1, -1, -1, 0x002C00, 0x002C5F, 0x002C5E, 94, L"Glagolitic", &viRsvd_002C00_002C5F, &vDFIdx_002C00_002C5F},
  {93, -1, -1, -1, 0x002C60, 0x002C7F, 0x002C7F, 32, L"Latin Extended-C", NULL, &vDFIdx_002C60_002C7F},
  {94, -1, -1, -1, 0x002C80, 0x002CFF, 0x002CFF, 123, L"Coptic", &viRsvd_002C80_002CFF, &vDFIdx_002C80_002CFF},
  {95, -1, -1, -1, 0x002D00, 0x002D2F, 0x002D2D, 40, L"Georgian Supplement", &viRsvd_002D00_002D2F, &vDFIdx_002D00_002D2F},
  {96, -1, -1, -1, 0x002D30, 0x002D7F, 0x002D7F, 59, L"Tifinagh", &viRsvd_002D30_002D7F, &vDFIdx_002D30_002D7F},
  {97, -1, -1, -1, 0x002D80, 0x002DDF, 0x002DDE, 79, L"Ethiopic Extended", &viRsvd_002D80_002DDF, &vDFIdx_002D80_002DDF},
  {98, -1, -1, -1, 0x002DE0, 0x002DFF, 0x002DFF, 32, L"Cyrillic Extended-A", NULL, &vDFIdx_002DE0_002DFF},
  {99, -1, -1, -1, 0x002E00, 0x002E7F, 0x002E42, 67, L"Supplemental Punctuation", NULL, &vDFIdx_002E00_002E7F},
  {100, -1, -1, -1, 0x002E80, 0x002EFF, 0x002EF3, 115, L"CJK Radicals Supplement", &viRsvd_002E80_002EFF, &vDFIdx_002E80_002EFF},
  {101, -1, -1, -1, 0x002F00, 0x002FDF, 0x002FD5, 214, L"Kangxi Radicals", NULL, &vDFIdx_002F00_002FDF},
  {102, -1, -1, -1, 0x002FF0, 0x002FFF, 0x002FFB, 12, L"Ideographic Description Characters", NULL, &vDFIdx_002FF0_002FFF},
  {103, -1, -1, -1, 0x003000, 0x00303F, 0x00303F, 64, L"CJK Symbols and Punctuation", NULL, &vDFIdx_003000_00303F},
  {104, -1, -1, -1, 0x003040, 0x00309F, 0x00309F, 93, L"Hiragana", &viRsvd_003040_00309F, &vDFIdx_003040_00309F},
  {105, -1, -1, -1, 0x0030A0, 0x0030FF, 0x0030FF, 96, L"Katakana", NULL, &vDFIdx_0030A0_0030FF},
  {106, -1, -1, -1, 0x003100, 0x00312F, 0x00312D, 41, L"Bopomofo", &viRsvd_003100_00312F, &vDFIdx_003100_00312F},
  {107, -1, -1, -1, 0x003130, 0x00318F, 0x00318E, 94, L"Hangul Compatibility Jamo", &viRsvd_003130_00318F, &vDFIdx_003130_00318F},
  {108, -1, -1, -1, 0x003190, 0x00319F, 0x00319F, 16, L"Kanbun", NULL, &vDFIdx_003190_00319F},
  {109, -1, -1, -1, 0x0031A0, 0x0031BF, 0x0031BA, 27, L"Bopomofo Extended", NULL, &vDFIdx_0031A0_0031BF},
  {110, -1, -1, -1, 0x0031C0, 0x0031EF, 0x0031E3, 36, L"CJK Strokes", NULL, &vDFIdx_0031C0_0031EF},
  {111, -1, -1, -1, 0x0031F0, 0x0031FF, 0x0031FF, 16, L"Katakana Phonetic Extensions", NULL, &vDFIdx_0031F0_0031FF},
  {112, -1, -1, -1, 0x003200, 0x0032FF, 0x0032FE, 254, L"Enclosed CJK Letters and Months", &viRsvd_003200_0032FF, &vDFIdx_003200_0032FF},
  {113, -1, -1, -1, 0x003300, 0x0033FF, 0x0033FF, 256, L"CJK Compatibility", NULL, &vDFIdx_003300_0033FF},
  {114, -1, -1, -1, 0x003400, 0x004DBF, 0x004DB5, 6582, L"CJK Unified Ideographs Extension A", NULL, &vDFIdx_003400_004DBF},
  {115, -1, -1, -1, 0x004DC0, 0x004DFF, 0x004DFF, 64, L"Yijing Hexagram Symbols", NULL, &vDFIdx_004DC0_004DFF},
  {116, -1, -1, -1, 0x004E00, 0x009FFF, 0x009FD5, 20950, L"CJK Unified Ideographs", NULL, &vDFIdx_004E00_009FFF},
  {117, -1, -1, -1, 0x00A000, 0x00A48F, 0x00A48C, 1165, L"Yi Syllables", NULL, &vDFIdx_00A000_00A48F},
  {118, -1, -1, -1, 0x00A490, 0x00A4CF, 0x00A4C6, 55, L"Yi Radicals", NULL, &vDFIdx_00A490_00A4CF},
  {119, -1, -1, -1, 0x00A4D0, 0x00A4FF, 0x00A4FF, 48, L"Lisu", NULL, &vDFIdx_00A4D0_00A4FF},
  {120, -1, -1, -1, 0x00A500, 0x00A63F, 0x00A62B, 300, L"Vai", NULL, &vDFIdx_00A500_00A63F},
  {121, -1, -1, -1, 0x00A640, 0x00A69F, 0x00A69F, 96, L"Cyrillic Extended-B", NULL, &vDFIdx_00A640_00A69F},
  {122, -1, -1, -1, 0x00A6A0, 0x00A6FF, 0x00A6F7, 88, L"Bamum", NULL, &vDFIdx_00A6A0_00A6FF},
  {123, -1, -1, -1, 0x00A700, 0x00A71F, 0x00A71F, 32, L"Modifier Tone Letters", NULL, &vDFIdx_00A700_00A71F},
  {124, -1, -1, -1, 0x00A720, 0x00A7FF, 0x00A7FF, 159, L"Latin Extended-D", &viRsvd_00A720_00A7FF, &vDFIdx_00A720_00A7FF},
  {125, -1, -1, -1, 0x00A800, 0x00A82F, 0x00A82B, 44, L"Syloti Nagri", NULL, &vDFIdx_00A800_00A82F},
  {126, -1, -1, -1, 0x00A830, 0x00A83F, 0x00A839, 10, L"Common Indic Number Forms", NULL, &vDFIdx_00A830_00A83F},
  {127, -1, -1, -1, 0x00A840, 0x00A87F, 0x00A877, 56, L"Phags-pa", NULL, &vDFIdx_00A840_00A87F},
  {128, -1, -1, -1, 0x00A880, 0x00A8DF, 0x00A8D9, 81, L"Saurashtra", &viRsvd_00A880_00A8DF, &vDFIdx_00A880_00A8DF},
  {129, -1, -1, -1, 0x00A8E0, 0x00A8FF, 0x00A8FD, 30, L"Devanagari Extended", NULL, &vDFIdx_00A8E0_00A8FF},
  {130, -1, -1, -1, 0x00A900, 0x00A92F, 0x00A92F, 48, L"Kayah Li", NULL, &vDFIdx_00A900_00A92F},
  {131, -1, -1, -1, 0x00A930, 0x00A95F, 0x00A95F, 37, L"Rejang", &viRsvd_00A930_00A95F, &vDFIdx_00A930_00A95F},
  {132, -1, -1, -1, 0x00A960, 0x00A97F, 0x00A97C, 29, L"Hangul Jamo Extended-A", &viRsvd_00A980_00A9DF, &vDFIdx_00A960_00A97F},
  {133, -1, -1, -1, 0x00A980, 0x00A9DF, 0x00A9DF, 91, L"Javanese", NULL, &vDFIdx_00A980_00A9DF},
  {134, -1, -1, -1, 0x00A9E0, 0x00A9FF, 0x00A9FE, 31, L"Myanmar Extended-B", NULL, &vDFIdx_00A9E0_00A9FF},
  {135, -1, -1, -1, 0x00AA00, 0x00AA5F, 0x00AA5F, 83, L"Cham", &viRsvd_00AA00_00AA5F, &vDFIdx_00AA00_00AA5F},
  {136, -1, -1, -1, 0x00AA60, 0x00AA7F, 0x00AA7F, 32, L"Myanmar Extended-A", &viRsvd_00AA80_00AADF, &vDFIdx_00AA60_00AA7F},
  {137, -1, -1, -1, 0x00AA80, 0x00AADF, 0x00AADF, 72, L"Tai Viet", NULL, &vDFIdx_00AA80_00AADF},
  {138, -1, -1, -1, 0x00AAE0, 0x00AAFF, 0x00AAF6, 23, L"Meetei Mayek Extensions", NULL, &vDFIdx_00AAE0_00AAFF},
  {139, -1, -1, -1, 0x00AB00, 0x00AB2F, 0x00AB2E, 32, L"Ethiopic Extended-A", &viRsvd_00AB00_00AB2F, &vDFIdx_00AB00_00AB2F},
  {140, -1, -1, -1, 0x00AB30, 0x00AB6F, 0x00AB65, 54, L"Latin Extended-E", NULL, &vDFIdx_00AB30_00AB6F},
  {141, -1, -1, -1, 0x00AB70, 0x00ABBF, 0x00ABBF, 80, L"Cherokee Supplement", NULL, &vDFIdx_00AB70_00ABBF},
  {142, -1, -1, -1, 0x00ABC0, 0x00ABFF, 0x00ABF9, 56, L"Meetei Mayek", &viRsvd_00ABC0_00ABFF, &vDFIdx_00ABC0_00ABFF},
  {143, -1, -1, -1, 0x00AC00, 0x00D7AF, 0x00D7A3, 11172, L"Hangul Syllables", NULL, &vDFIdx_00AC00_00D7AF},
  {144, -1, -1, -1, 0x00D7B0, 0x00D7FF, 0x00D7FB, 72, L"Hangul Jamo Extended-B", &viRsvd_00D7B0_00D7FF, &vDFIdx_00D7B0_00D7FF},
  //{ -,-1,-1,-1,0x00D800, 0x00DB7F, -1, 0, L"High Surrogates",NULL, &vDFIdx_00D800_00DB7F },
  //{ -,-1,-1,-1,0x00DB80, 0x00DBFF, -1, 0, L"High Private Use Surrogates",NULL, &vDFIdx_00DB80_00DBFF },
  //{ -,-1,-1,-1,0x00DC00, 0x00DFFF, -1, 0, L"Low Surrogates",NULL, &vDFIdx_00DC00_00DFFF },
  {145, -1, -1, -1, 0x00E000, 0x00F8FF, 0x00F8FF, 6400, L"Private Use Area", NULL, &vDFIdx_00E000_00F8FF},
  {146, -1, -1, -1, 0x00F900, 0x00FAFF, 0x00FAD9, 472, L"CJK Compatibility Ideographs", NULL, &vDFIdx_00F900_00FAFF},
  {147, -1, -1, -1, 0x00FB00, 0x00FB4F, 0x00FB4F, 58, L"Alphabetic Presentation Forms", &viRsvd_00FB00_00FB4F, &vDFIdx_00FB00_00FB4F},
  {148, -1, -1, -1, 0x00FB50, 0x00FDFF, 0x00FDFD, 611, L"Arabic Presentation Forms-A", &viRsvd_00FB50_00FDFF, &vDFIdx_00FB50_00FDFF},
  {149, -1, -1, -1, 0x00FE00, 0x00FE0F, 0x00FE0F, 16, L"Variation Selectors", NULL, &vDFIdx_00FE00_00FE0F},
  {150, -1, -1, -1, 0x00FE10, 0x00FE1F, 0x00FE19, 10, L"Vertical Forms", NULL, &vDFIdx_00FE10_00FE1F},
  {151, -1, -1, -1, 0x00FE20, 0x00FE2F, 0x00FE2F, 16, L"Combining Half Marks", NULL, &vDFIdx_00FE20_00FE2F},
  {152, -1, -1, -1, 0x00FE30, 0x00FE4F, 0x00FE4F, 32, L"CJK Compatibility Forms", NULL, &vDFIdx_00FE30_00FE4F},
  {153, -1, -1, -1, 0x00FE50, 0x00FE6F, 0x00FE6B, 26, L"Small Form Variants", &viRsvd_00FE50_00FE6F, &vDFIdx_00FE50_00FE6F},
  {154, -1, -1, -1, 0x00FE70, 0x00FEFF, 0x00FEFF, 141, L"Arabic Presentation Forms-B", &viRsvd_00FE70_00FEFF, &vDFIdx_00FE70_00FEFF},
  {155, -1, -1, -1, 0x00FF00, 0x00FFEF, 0x00FFEE, 225, L"Halfwidth and Fullwidth Forms", &viRsvd_00FF00_00FFEF, &vDFIdx_00FF00_00FFEF},
  {156, -1, -1, -1, 0x00FFF0, 0x00FFFF, 0x00FFFD, 5, L"Specials", &viRsvd_00FFF0_00FFFF, &vDFIdx_00FFF0_00FFFF},
  {157, -1, -1, -1, 0x010000, 0x01007F, 0x01005D, 88, L"Linear B Syllabary", &viRsvd_010000_01007F, &vDFIdx_010000_01007F},
  {158, -1, -1, -1, 0x010080, 0x0100FF, 0x0100FA, 123, L"Linear B Ideograms", NULL, &vDFIdx_010080_0100FF},
  {159, -1, -1, -1, 0x010100, 0x01013F, 0x01013F, 57, L"Aegean Numbers", &viRsvd_010100_01013F, &vDFIdx_010100_01013F},
  {160, -1, -1, -1, 0x010140, 0x01018F, 0x01018C, 77, L"Ancient Greek Numbers", NULL, &vDFIdx_010140_01018F},
  {161, -1, -1, -1, 0x010190, 0x0101CF, 0x0101A0, 13, L"Ancient Symbols", &viRsvd_010190_0101CF, &vDFIdx_010190_0101CF},
  {162, -1, -1, -1, 0x0101D0, 0x0101FF, 0x0101FD, 46, L"Phaistos Disc", NULL, &vDFIdx_0101D0_0101FF},
  {163, -1, -1, -1, 0x010280, 0x01029F, 0x01029C, 29, L"Lycian", NULL, &vDFIdx_010280_01029F},
  {164, -1, -1, -1, 0x0102A0, 0x0102DF, 0x0102D0, 49, L"Carian", NULL, &vDFIdx_0102A0_0102DF},
  {165, -1, -1, -1, 0x0102E0, 0x0102FF, 0x0102FB, 28, L"Coptic Epact Numbers", NULL, &vDFIdx_0102E0_0102FF},
  {166, -1, -1, -1, 0x010300, 0x01032F, 0x010323, 36, L"Old Italic", NULL, &vDFIdx_010300_01032F},
  {167, -1, -1, -1, 0x010330, 0x01034F, 0x01034A, 27, L"Gothic", NULL, &vDFIdx_010330_01034F},
  {168, -1, -1, -1, 0x010350, 0x01037F, 0x01037A, 43, L"Old Permic", NULL, &vDFIdx_010350_01037F},
  {169, -1, -1, -1, 0x010380, 0x01039F, 0x01039F, 31, L"Ugaritic", &viRsvd_010380_01039F, &vDFIdx_010380_01039F},
  {170, -1, -1, -1, 0x0103A0, 0x0103DF, 0x0103D5, 50, L"Old Persian", &viRsvd_0103A0_0103DF, &vDFIdx_0103A0_0103DF},
  {171, -1, -1, -1, 0x010400, 0x01044F, 0x01044F, 80, L"Deseret", NULL, &vDFIdx_010400_01044F},
  {172, -1, -1, -1, 0x010450, 0x01047F, 0x01047F, 48, L"Shavian", NULL, &vDFIdx_010450_01047F},
  {173, -1, -1, -1, 0x010480, 0x0104AF, 0x0104A9, 40, L"Osmanya", &viRsvd_010480_0104AF, &vDFIdx_010480_0104AF},
  {174, -1, -1, -1, 0x010500, 0x01052F, 0x010527, 40, L"Elbasan", NULL, &vDFIdx_010500_01052F},
  {175, -1, -1, -1, 0x010530, 0x01056F, 0x01056F, 53, L"Caucasian Albanian", &viRsvd_010530_01056F, &vDFIdx_010530_01056F},
  {176, -1, -1, -1, 0x010600, 0x01077F, 0x010767, 341, L"Linear A", &viRsvd_010600_01077F, &vDFIdx_010600_01077F},
  {177, -1, -1, -1, 0x010800, 0x01083F, 0x01083F, 55, L"Cypriot Syllabary", &viRsvd_010800_01083F, &vDFIdx_010800_01083F},
  {178, -1, -1, -1, 0x010840, 0x01085F, 0x01085F, 31, L"Imperial Aramaic", &viRsvd_010840_01085F, &vDFIdx_010840_01085F},
  {179, -1, -1, -1, 0x010860, 0x01087F, 0x01087F, 32, L"Palmyrene", NULL, &vDFIdx_010860_01087F},
  {180, -1, -1, -1, 0x010880, 0x0108AF, 0x0108AF, 40, L"Nabataean", &viRsvd_010880_0108AF, &vDFIdx_010880_0108AF},
  {181, -1, -1, -1, 0x0108E0, 0x0108FF, 0x0108FF, 26, L"Hatran", &viRsvd_0108E0_0108FF, &vDFIdx_0108E0_0108FF},
  {182, -1, -1, -1, 0x010900, 0x01091F, 0x01091F, 29, L"Phoenician", &viRsvd_010900_01091F, &vDFIdx_010900_01091F},
  {183, -1, -1, -1, 0x010920, 0x01093F, 0x01093F, 27, L"Lydian", &viRsvd_010920_01093F, &vDFIdx_010920_01093F},
  {184, -1, -1, -1, 0x010980, 0x01099F, 0x01099F, 32, L"Meroitic Hieroglyphs", NULL, &vDFIdx_010980_01099F},
  {185, -1, -1, -1, 0x0109A0, 0x0109FF, 0x0109FF, 90, L"Meroitic Cursive", &viRsvd_0109A0_0109FF, &vDFIdx_0109A0_0109FF},
  {186, -1, -1, -1, 0x010A00, 0x010A5F, 0x010A58, 65, L"Kharoshthi", &viRsvd_010A00_010A5F, &vDFIdx_010A00_010A5F},
  {187, -1, -1, -1, 0x010A60, 0x010A7F, 0x010A7F, 32, L"Old South Arabian", NULL, &vDFIdx_010A60_010A7F},
  {188, -1, -1, -1, 0x010A80, 0x010A9F, 0x010A9F, 32, L"Old North Arabian", NULL, &vDFIdx_010A80_010A9F},
  {189, -1, -1, -1, 0x010AC0, 0x010AFF, 0x010AF6, 51, L"Manichaean", &viRsvd_010AC0_010AFF, &vDFIdx_010AC0_010AFF},
  {190, -1, -1, -1, 0x010B00, 0x010B3F, 0x010B3F, 61, L"Avestan", &viRsvd_010B00_010B3F, &vDFIdx_010B00_010B3F},
  {191, -1, -1, -1, 0x010B40, 0x010B5F, 0x010B5F, 30, L"Inscriptional Parthian", &viRsvd_010B40_010B5F, &vDFIdx_010B40_010B5F},
  {192, -1, -1, -1, 0x010B60, 0x010B7F, 0x010B7F, 27, L"Inscriptional Pahlavi", &viRsvd_010B60_010B7F, &vDFIdx_010B60_010B7F},
  {193, -1, -1, -1, 0x010B80, 0x010BAF, 0x010BAF, 29, L"Psalter Pahlavi", &viRsvd_010B80_010BAF, &vDFIdx_010B80_010BAF},
  {194, -1, -1, -1, 0x010C00, 0x010C4F, 0x010C48, 73, L"Old Turkic", NULL, &vDFIdx_010C00_010C4F},
  {195, -1, -1, -1, 0x010C80, 0x010CFF, 0x010CFF, 108, L"Old Hungarian", &viRsvd_010C80_010CFF, &vDFIdx_010C80_010CFF},
  {196, -1, -1, -1, 0x010E60, 0x010E7F, 0x010E7E, 31, L"Rumi Numeral Symbols", NULL, &vDFIdx_010E60_010E7F},
  {197, -1, -1, -1, 0x011000, 0x01107F, 0x01107F, 109, L"Brahmi", &viRsvd_011000_01107F, &vDFIdx_011000_01107F},
  {198, -1, -1, -1, 0x011080, 0x0110CF, 0x0110C1, 66, L"Kaithi", NULL, &vDFIdx_011080_0110CF},
  {199, -1, -1, -1, 0x0110D0, 0x0110FF, 0x0110F9, 35, L"Sora Sompeng", &viRsvd_0110D0_0110FF, &vDFIdx_0110D0_0110FF},
  {200, -1, -1, -1, 0x011100, 0x01114F, 0x011143, 67, L"Chakma", &viRsvd_011100_01114F, &vDFIdx_011100_01114F},
  {201, -1, -1, -1, 0x011150, 0x01117F, 0x011176, 39, L"Mahajani", NULL, &vDFIdx_011150_01117F},
  {202, -1, -1, -1, 0x011180, 0x0111DF, 0x0111DF, 94, L"Sharada", &viRsvd_011180_0111DF, &vDFIdx_011180_0111DF},
  {203, -1, -1, -1, 0x0111E0, 0x0111FF, 0x0111F4, 20, L"Sinhala Archaic Numbers", &viRsvd_0111E0_0111FF, &vDFIdx_0111E0_0111FF},
  {204, -1, -1, -1, 0x011200, 0x01124F, 0x01123D, 61, L"Khojki", &viRsvd_011200_01124F, &vDFIdx_011200_01124F},
  {205, -1, -1, -1, 0x011280, 0x0112AF, 0x0112A9, 38, L"Multani", &viRsvd_011280_0112AF, &vDFIdx_011280_0112AF},
  {206, -1, -1, -1, 0x0112B0, 0x0112FF, 0x0112F9, 69, L"Khudawadi", &viRsvd_0112B0_0112FF, &vDFIdx_0112B0_0112FF},
  {207, -1, -1, -1, 0x011300, 0x01137F, 0x011374, 85, L"Grantha", &viRsvd_011300_01137F, &vDFIdx_011300_01137F},
  {208, -1, -1, -1, 0x011480, 0x0114DF, 0x0114D9, 82, L"Tirhuta", &viRsvd_011480_0114DF, &vDFIdx_011480_0114DF},
  {209, -1, -1, -1, 0x011580, 0x0115FF, 0x0115DD, 92, L"Siddham", &viRsvd_011580_0115FF, &vDFIdx_011580_0115FF},
  {210, -1, -1, -1, 0x011600, 0x01165F, 0x011659, 79, L"Modi", &viRsvd_011600_01165F, &vDFIdx_011600_01165F},
  {211, -1, -1, -1, 0x011680, 0x0116CF, 0x0116C9, 66, L"Takri", &viRsvd_011680_0116CF, &vDFIdx_011680_0116CF},
  {212, -1, -1, -1, 0x011700, 0x01173F, 0x01173F, 57, L"Ahom", &viRsvd_011700_01173F, &vDFIdx_011700_01173F},
  {213, -1, -1, -1, 0x0118A0, 0x0118FF, 0x0118FF, 84, L"Warang Citi", &viRsvd_0118A0_0118FF, &vDFIdx_0118A0_0118FF},
  {214, -1, -1, -1, 0x011AC0, 0x011AFF, 0x011AF8, 57, L"Pau Cin Hau", NULL, &vDFIdx_011AC0_011AFF},
  {215, -1, -1, -1, 0x012000, 0x0123FF, 0x012399, 922, L"Cuneiform", NULL, &vDFIdx_012000_0123FF},
  {216, -1, -1, -1, 0x012400, 0x01247F, 0x012474, 116, L"Cuneiform Numbers and Punctuation", &viRsvd_012400_01247F, &vDFIdx_012400_01247F},
  {217, -1, -1, -1, 0x012480, 0x01254F, 0x012543, 196, L"Early Dynastic Cuneiform", NULL, &vDFIdx_012480_01254F},
  {218, -1, -1, -1, 0x013000, 0x01342F, 0x01342E, 1071, L"Egyptian Hieroglyphs", NULL, &vDFIdx_013000_01342F},
  {219, -1, -1, -1, 0x014400, 0x01467F, 0x014646, 583, L"Anatolian Hieroglyphs", NULL, &vDFIdx_014400_01467F},
  {220, -1, -1, -1, 0x016800, 0x016A3F, 0x016A38, 569, L"Bamum Supplement", NULL, &vDFIdx_016800_016A3F},
  {221, -1, -1, -1, 0x016A40, 0x016A6F, 0x016A6F, 43, L"Mro", &viRsvd_016A40_016A6F, &vDFIdx_016A40_016A6F},
  {222, -1, -1, -1, 0x016AD0, 0x016AFF, 0x016AF5, 36, L"Bassa Vah", &viRsvd_016AD0_016AFF, &vDFIdx_016AD0_016AFF},
  {223, -1, -1, -1, 0x016B00, 0x016B8F, 0x016B8F, 127, L"Pahawh Hmong", &viRsvd_016B00_016B8F, &vDFIdx_016B00_016B8F},
  {224, -1, -1, -1, 0x016F00, 0x016F9F, 0x016F9F, 133, L"Miao", &viRsvd_016F00_016F9F, &vDFIdx_016F00_016F9F},
  {225, -1, -1, -1, 0x01B000, 0x01B0FF, 0x01B001, 2, L"Kana Supplement", NULL, &vDFIdx_01B000_01B0FF},
  {226, -1, -1, -1, 0x01BC00, 0x01BC9F, 0x01BC9F, 143, L"Duployan", &viRsvd_01BC00_01BC9F, &vDFIdx_01BC00_01BC9F},
  {227, -1, -1, -1, 0x01BCA0, 0x01BCAF, 0x01BCA3, 4, L"Shorthand Format Controls", NULL, &vDFIdx_01BCA0_01BCAF},
  {228, -1, -1, -1, 0x01D000, 0x01D0FF, 0x01D0F5, 246, L"Byzantine Musical Symbols", NULL, &vDFIdx_01D000_01D0FF},
  {229, -1, -1, -1, 0x01D100, 0x01D1FF, 0x01D1E8, 231, L"Musical Symbols", &viRsvd_01D100_01D1FF, &vDFIdx_01D100_01D1FF},
  {230, -1, -1, -1, 0x01D200, 0x01D24F, 0x01D245, 70, L"Ancient Greek Musical Notation", NULL, &vDFIdx_01D200_01D24F},
  {231, -1, -1, -1, 0x01D300, 0x01D35F, 0x01D356, 87, L"Tai Xuan Jing Symbols", NULL, &vDFIdx_01D300_01D35F},
  {232, -1, -1, -1, 0x01D360, 0x01D37F, 0x01D371, 18, L"Counting Rod Numerals", NULL, &vDFIdx_01D360_01D37F},
  {233, -1, -1, -1, 0x01D400, 0x01D7FF, 0x01D7FF, 996, L"Mathematical Alphanumeric Symbols", &viRsvd_01D400_01D7FF, &vDFIdx_01D400_01D7FF},
  {234, -1, -1, -1, 0x01D800, 0x01DAAF, 0x01DAAF, 672, L"Sutton SignWriting", &viRsvd_01D800_01DAAF, &vDFIdx_01D800_01DAAF},
  {235, -1, -1, -1, 0x01E800, 0x01E8DF, 0x01E8D6, 213, L"Mende Kikakui", &viRsvd_01E800_01E8DF, &vDFIdx_01E800_01E8DF},
  {236, -1, -1, -1, 0x01EE00, 0x01EEFF, 0x01EEF1, 143, L"Arabic Mathematical Alphabetic Symbols", &viRsvd_01EE00_01EEFF, &vDFIdx_01EE00_01EEFF},
  {237, -1, -1, -1, 0x01F000, 0x01F02F, 0x01F02B, 44, L"Mahjong Tiles", NULL, &vDFIdx_01F000_01F02F},
  {238, -1, -1, -1, 0x01F030, 0x01F09F, 0x01F093, 100, L"Domino Tiles", NULL, &vDFIdx_01F030_01F09F},
  {239, -1, -1, -1, 0x01F0A0, 0x01F0FF, 0x01F0F5, 82, L"Playing Cards", &viRsvd_01F0A0_01F0FF, &vDFIdx_01F0A0_01F0FF},
  {240, -1, -1, -1, 0x01F100, 0x01F1FF, 0x01F1FF, 173, L"Enclosed Alphanumeric Supplement", &viRsvd_01F100_01F1FF, &vDFIdx_01F100_01F1FF},
  {241, -1, -1, -1, 0x01F200, 0x01F2FF, 0x01F251, 57, L"Enclosed Ideographic Supplement", &viRsvd_01F200_01F2FF, &vDFIdx_01F200_01F2FF},
  {242, -1, -1, -1, 0x01F300, 0x01F5FF, 0x01F5FF, 766, L"Miscellaneous Symbols and Pictographs", &viRsvd_01F300_01F5FF, &vDFIdx_01F300_01F5FF},
  {243, -1, -1, -1, 0x01F600, 0x01F64F, 0x01F64F, 80, L"Emoticons", NULL, &vDFIdx_01F600_01F64F},
  {244, -1, -1, -1, 0x01F650, 0x01F67F, 0x01F67F, 48, L"Ornamental Dingbats", NULL, &vDFIdx_01F650_01F67F},
  {245, -1, -1, -1, 0x01F680, 0x01F6FF, 0x01F6F3, 98, L"Transport and Map Symbols", &viRsvd_01F680_01F6FF, &vDFIdx_01F680_01F6FF},
  {246, -1, -1, -1, 0x01F700, 0x01F77F, 0x01F773, 116, L"Alchemical Symbols", NULL, &vDFIdx_01F700_01F77F},
  {247, -1, -1, -1, 0x01F780, 0x01F7FF, 0x01F7D4, 85, L"Geometric Shapes Extended", NULL, &vDFIdx_01F780_01F7FF},
  {248, -1, -1, -1, 0x01F800, 0x01F8FF, 0x01F8AD, 148, L"Supplemental Arrows-C", &viRsvd_01F800_01F8FF, &vDFIdx_01F800_01F8FF},
  {249, -1, -1, -1, 0x01F900, 0x01F9FF, 0x01F9C0, 15, L"Supplemental Symbols and Pictographs", &viRsvd_01F900_01F9FF, &vDFIdx_01F900_01F9FF},
  {250, -1, -1, -1, 0x020000, 0x02A6DF, 0x02A6D6, 42711, L"CJK Unified Ideographs Extension B", NULL, &vDFIdx_020000_02A6DF},
  {251, -1, -1, -1, 0x02A700, 0x02B73F, 0x02B734, 4149, L"CJK Unified Ideographs Extension C", NULL, &vDFIdx_02A700_02B73F},
  {252, -1, -1, -1, 0x02B740, 0x02B81F, 0x02B81D, 222, L"CJK Unified Ideographs Extension D", NULL, &vDFIdx_02B740_02B81F},
  {253, -1, -1, -1, 0x02B820, 0x02CEAF, 0x02CEA1, 5762, L"CJK Unified Ideographs Extension E", NULL, &vDFIdx_02B820_02CEAF},
  {254, -1, -1, -1, 0x02F800, 0x02FA1F, 0x02FA1D, 542, L"CJK Compatibility Ideographs Supplement", NULL, &vDFIdx_02F800_02FA1F},
  {255, -1, -1, -1, 0x0E0000, 0x0E007F, 0x0E007F, 97, L"Tags", NULL, &vDFIdx_0E0000_0E007F},
  {256, -1, -1, -1, 0x0E0100, 0x0E01EF, 0x0E01EF, 240, L"Variation Selectors Supplement", NULL, NULL},
  {257, -1, -1, -1, 0x0F0000, 0x0FFFFF, 0x0FFFFD, 65534, L"Supplementary Private Use Area-A", NULL, NULL},
  {258, -1, -1, -1, 0x100000, 0x10FFFF, 0x10FFFD, 65534, L"Supplementary Private Use Area-B", NULL, NULL}
};
