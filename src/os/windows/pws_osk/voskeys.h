/*
* Copyright (c) 2009 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// File: voskeys.h
//

#ifndef _VOSKEYS_H
#define _VOSKEYS_H

#include "OSK_voskeys.inc"

#include <bitset>
#include <vector>
#include <map>

typedef unsigned short int ushort;

// Per scancode - scancode, deadkey and 16 possible states
// with/without shift, alt, ctrl, alt+ctrl, capslock etc.
struct st_SC2CHAR {
  BYTE SC;
  std::bitset<16> bsDeadKey;
  wchar_t wcChar[16];
};

// For bitset below
enum eSpecials { b, bC, sb, sbC, l, lC, sl, slC, g, gC, sg, sgC, r, rC, sr, srC};
           
struct st_VKBD {
  unsigned short int uiCtrlID;
  BYTE numScanCodes;
  // l, lC, sl, slC, g, gC, sg, sgC, r, rC, sr, srC
  std::bitset<16> bsValidSpecials;
  // On Vista - 3 x 45, 5 x 46, 3 x 47, 11 x 48, 122 x 49, 2 x 51
  st_SC2CHAR stSC2CHAR[MAX_ROWS];
};

//----------------------------------------------------------------------
// Following structures (starting with I) are used Internally by pws_osk
//----------------------------------------------------------------------

// Pack storage to single byte boundaries -
// but should have no impact wih these structure definitions
#pragma pack(push, b1, 1)

// Ketboard scan code data
// Deadkey bitmap saved as integer
// Display Characters saved as offsets into common data areas
struct st_ISC2CHAR {
  BYTE SC;                // Scan code for this key
  ushort uiDeadKey;       // Bitmap indicating if any states of this key are Dead Keys (saved as an integer)
  ushort uiOffset1;       // Offset for base character quartet
  ushort uiOffset2;       // Offset for base character + Left Control quartet
  ushort uiOffset3;       // Offset for base character + AltGr quartet
  ushort uiOffset4;       // Offset for base character + Right Control quartet
};

// Keyboard data
struct st_IVKBD {
  ushort uiCtrlID;                  // Control ID of resource containing name of this keyboard
  BYTE numScanCodes;                // Number of scan codes valid in this keyboard
  st_ISC2CHAR stISC2CHAR[MAX_ROWS]; // Key data for each valid scan code
};

// Multi-character area:
// 1. SC + shiftstate 
// 2. Offset to appropriate common character table
struct st_SCSS2Offset {
  ushort uiSCSS;               // Scan code * 256 + shift state
  ushort uiOffset;             // Offset into multi-character table
};

// Dead Keydata area for application
// 1. Shiftstate 
// 2. Scan code
// 3. Dead Key
struct st_SCSS2DKChar {
  BYTE SC;                     // Scan code
  BYTE SS;                     // Shift state
  wchar_t wcDKchar;            // Dead Key combination character
};

// Dead Keydata area internally
// 1. Scan code 
// 2. Shiftstate
// 3. Offset to appropriate common character table
struct st_IDKSCSS2Offset {
  BYTE SC;                     // Scan code
  BYTE SS;                     // Shift state
  ushort uiDKOffset;           // Offset to Dead Key value
};

typedef std::vector<const st_SCSS2Offset> Vct_ISCSS2MC;
typedef Vct_ISCSS2MC::const_iterator CIter_Vct_ISCSS2MC;

// Disable warning can't create const fields - we never do - they are constant values
#pragma warning(push)
#pragma warning(disable: 4510 4610)
struct st_IKLID2SCSS2MC {
  const unsigned int uiKLID;
  const Vct_ISCSS2MC * pvctISCSS2MC;
};
#pragma warning(pop)

// Entry in an array of structures pointing to the keyboard data for a given KLID
struct st_IKLID2VKBD {
  unsigned int uiKLID;
  st_IVKBD * pstIVKBD;
};

// Revert packing
#pragma pack(pop, b1)

// Maps given keyboard's key's 'scancode + shiftstate' to
// a multi-character values
typedef std::map<ushort, ushort> Map_SCSS2Char;
typedef Map_SCSS2Char::const_iterator CIter_Map_SCSS2Char;

// Maps a given keyboard's DeadKey to map of scancode + shiftstate to character
typedef std::multimap<wchar_t, st_SCSS2DKChar> MMap_DK2SCSS;
typedef MMap_DK2SCSS::iterator Iter_MMap_DK2SCSS;

// Used to build the deadkey maps when user selects a keyboard.
// Input: KLID
// Output: Corresponding Map_DK2SCSS2WC & Map_SCSS2WL maps

// Vector of scan codes and corresponding offset into common Deadkey data
// for a specific deadkey for a specific keyboard
typedef std::vector<const st_IDKSCSS2Offset> Vct_IDeadkeys;
typedef Vct_IDeadkeys::const_iterator CIter_Vct_IDeadkeys;

// Maps a given KLID's DeadKey to data area of scancode + shiftstate to character
typedef std::map<wchar_t, const Vct_IDeadkeys *> Map_IDK2SCSS;
typedef Map_IDK2SCSS::iterator Iter_Map_IDK2SCSS;

// Maps a given KLID to map of DeadKeys to map of scancode
// to lower + upper case offset
typedef std::map<unsigned int, Map_IDK2SCSS *> Map_IKLID2DK2SCSS;
typedef Map_IKLID2DK2SCSS::iterator Iter_Map_IKLID2DK2SCSS;

// Implementation structure
struct st_KBImpl {
  // Standard Keyboard
  st_VKBD stVKBD;
  
  // Pointers to 2-, 3- and 4-multibyte characters (if any)
  wchar_t * wcMC2; // [NUM_UNIQUE_MC2][2];
  wchar_t * wcMC3; // [NUM_UNIQUE_MC3][3];
  wchar_t * wcMC4; // [NUM_UNIQUE_MC4][4];
  
  // Pointers to Dead Key characters (if any)
  wchar_t * wcDK;  // [ NUM_UNIQUE_DK];
  
  // Maps to 2-, 3- and 4-multibyte characters (if any)
  Map_SCSS2Char * pmapSCSS2Char2;
  Map_SCSS2Char * pmapSCSS2Char3;
  Map_SCSS2Char * pmapSCSS2Char4;
  // Map to DeadKeys (if any)
  MMap_DK2SCSS  * pmmapDK2SCSS;
};

#endif /* _VOSKEYS_H */
