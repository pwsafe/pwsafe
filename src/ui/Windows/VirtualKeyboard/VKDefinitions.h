/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// It is Unicode ONLY.

#pragma once

// VKdefinitions.h : header file
//-----------------------------------------------------------------------------

/*

NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!

*/

#include "../../../os/windows/pws_osk/pws_osk.h"

#include <vector>
#include <map>

#define NUM_KEYS (IDC_VKBBTN_KBD51 - IDC_VKBBTN_KBD01 + 1)
#define NUM_DIGITS (IDC_VKBBTN_N9 - IDC_VKBBTN_N0 + 1)

typedef OSK_API void(*LP_OSK_ListKeyboards) (UINT &uiKLID, UINT &uiCtrlID);
typedef OSK_API BOOL(*LP_OSK_GetKeyboardData) (UINT uiKLID, st_KBImpl &stKBImpl);
typedef OSK_API int(*LP_OSK_GetVersion) ();

enum { USER_FONT, ARIALMS_FONT, ARIAL_FONT, LUCIDA_FONT };

// See DboxMain.h as well, since all PWS messages are either defined or documented there
#define PWS_MSG_INSERTBUFFER (WM_APP + 70)

enum eJapanese { ENGLISH = 0, JAPANESE };    // Used for m_Kana
enum eHK       { HIRAGANA = 0, KATAKANA };   // Used for m_Hiragana
enum eSize     { HALF = 0, FULL };           // Used for m_Size

struct st_Keyboard_Layout {
  UINT uiKLID;
  UINT uiCtrlID;

  st_Keyboard_Layout()
    : uiKLID(0), uiCtrlID(0)
  {
  }

  st_Keyboard_Layout(const st_Keyboard_Layout &that)
    : uiKLID(that.uiKLID), uiCtrlID(that.uiCtrlID)
  {
  }

  st_Keyboard_Layout &operator=(const st_Keyboard_Layout &that)
  {
    if (this != &that) {
      uiKLID = that.uiKLID;
      uiCtrlID = that.uiCtrlID;
    }
    return *this;
  }
};

class CVKBButton;
class CVKBButton;

typedef std::vector<st_Keyboard_Layout> vKeyboard_Layouts;
typedef vKeyboard_Layouts::const_iterator KBL_citer;

typedef std::map<BYTE, const st_SC2CHAR> Map_st_SC2Char;
typedef Map_st_SC2Char::iterator Iter_Map_st_SC2CHAR;

typedef std::map<UINT, CVKBButton *> Map_ID2XButton;
typedef Map_ID2XButton::const_iterator Iter_Map_ID_XButton;

typedef std::map<UINT, CVKBButton *> Map_ID2Button;
typedef Map_ID2Button::const_iterator Iter_Map_ID_Button;

typedef OSK_API void(*LP_OSK_ListKeyboards) (UINT &uiKLID, UINT &uiCtrlID);
typedef OSK_API BOOL(*LP_OSK_GetKeyboardData) (UINT uiKLID, st_KBImpl &stKBImpl);
typedef OSK_API int(*LP_OSK_GetVersion) ();
