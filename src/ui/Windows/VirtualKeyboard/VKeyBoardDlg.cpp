/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file VKeyBoard.cpp
//-----------------------------------------------------------------------------

/*

NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!

*/

// It is Unicode ONLY.

#ifndef UNICODE
  #error On Screen Virtual Keyboard requires a UNICODE configuration
#endif

#include "../stdafx.h"

#include "VKeyBoardDlg.h"
#include "VKShiftState.h"
#include "VKresource.h"
#include "VKresource3.h"

#include "../resource3.h"  // String resources

#include "../../../os/lib.h"
#include "../../../os/windows/pws_osk/pws_osk.h"
#include "../../../core/PWSprefs.h"
#include "../../../core/PWSrand.h"

#include <sstream>
#include <iomanip>  // For setbase and setw
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

extern int iStartTime;

/*

Keyboard - Key Buttons and their scan codes:

101 = 51 - 4 = 47
        01  02  03  04  05  06  07  08  09  10  11  12  13  --
              15  16  17  18  19  20  21  22  23  24  25  26  27
               28  29  30  31  32  33  34  35  36  37  38  --
             --  41  42  43  44  45  46  47  48  49  50  --

102 = 51 - 3 = 48
        01  02  03  04  05  06  07  08  09  10  11  12  13  --
              15  16  17  18  19  20  21  22  23  24  25  26  --
               28  29  30  31  32  33  34  35  36  37  38  39
              40  41  42  43  44  45  46  47  48  49  50  --

106 = 51 - 3 = '48 + 3' special = 51
        01  02  03  04  05  06  07  08  09  10  11  12  13  --
              15  16  17  18  19  20  21  22  23  24  25  26  27
               28  29  30  31  32  33  34  35  36  37  38  00
             --  41  42  43  44  45  46  47  48  49  50  51

*/

const BYTE defscancodes101[] = {
           0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x00,
           0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x2b,
           0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x00,
           0x00, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00};

const BYTE defscancodes102[] = {
           0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x00,
           0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x00,
           0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2b,
           0x56, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00};

const BYTE defscancodes106[] = {
           0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x00,
           0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x7d,
           0x00, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
           0x00, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00};

const wchar_t *pXdefnumbers[] = {
           L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9"};

// Keyboards with 101 keys by default (user can change)
const UINT k101[] = {
                     0x00000401,  // Arabic (101)
                     0x00000409,  // US
                     0x0000040D,  // Hebrew
                     0x00000415,  // Polish (Programmers)
                     0x00000419,  // Russian
                     0x0000041E,  // Thai Kedmanee
                     0x00000420,  // Urdu
                     0x00000423,  // Belarusian
                     0x00000428,  // Tajik
                     0x00000429,  // Persian
                     0x0000042A,  // Vietnamese
                     0x0000042B,  // Armenian Eastern
                     0x0000042C,  // Azeri Latin
                     0x00000432,  // Setswana
                     0x00000437,  // Georgian
                     0x0000043F,  // Kazakh
                     0x00000440,  // Kyrgyz Cyrillic
                     0x00000445,  // Bengali
                     0x00000446,  // Punjabi
                     0x00000448,  // Oriya
                     0x00000449,  // Tamil
                     0x0000044A,  // Telugu
                     0x0000044B,  // Kannada
                     0x0000044E,  // Marathi
                     0x00000450,  // Mongolian Cyrillic
                     0x00000468,  // Hausa
                     0x0000046A,  // Yoruba
                     0x0000046C,  // Sesotho Sa Leboa
                     0x00000470,  // Igbo
                     0x00000481,  // Maori
                     0x0000082C,  // Azeri Cyrillic
                     0x00000843,  // Uzbek Cyrillic
                     0x00010402,  // Bulgarian (Latin)
                     0x0001040E,  // Hungarian 101-Key
                     0x00010419,  // Russian (Typewriter)
                     0x0001041E,  // Thai Pattachote
                     0x00010427,  // Lithuanian
                     0x0001042B,  // Armenian Western
                     0x00010439,  // Hindi Traditional
                     0x00010445,  // Bengali - Inscript (Legacy)
                     0x00010480,  // Uyghur
                     0x00020405,  // Czech Programmers
                     0x00020409,  // United States-International
                     0x00020418,  // Romanian (Programmers)
                     0x0002041E,  // Thai Kedmanee (Non-Shiftlock)
                     0x00020445,  // Bengali - Inscript
                     0x00030402,  // Bulgarian
                     0x0003041E,  // Thai Pattachote (Non-Shiftlock)
                     0x00040402,  // Bulgarian (Phonetic Traditional)
                     0x00050408,  // Greek Latin
                     // Via XML input
                     0x0001FFFF,  // Chinese ChaJei
                     0x0002FFFF,  // Chinese Bopomofo
                     0x0003FFFF   // Korean
           };

const std::vector<UINT> vk101(k101, k101 + _countof(k101));

// Constants for characters on Japanese Keys (Hiragana, Katakana, Half-Width, Full-Width)
const wchar_t wcHiragana[3] = {0x3072, 0x3089, 0};
const wchar_t wcKatakana[3] = {0x30ab, 0xb0bf, 0};
const wchar_t wcHalfWidth[2] = {0x534a, 0};
const wchar_t wcFullWidth[2] = {0x5168, 0};

// State to index into scancode to characters array [16] - NOT used for Japanese keyboard
// b = Base, s = Shift, l = Ctrl, g = AltGr, C = caps Lock, r = Right Ctrl (a = Alt: not used)
const int state2index [] = { 0,  //  0 - b
                             2,  //  1 - sb
                             4,  //  2 - l
                             6,  //  3 - sl
                            -1,  //  4 - a   - not used
                            -1,  //  5 - sa  - not used
                             8,  //  6 - g
                            10,  //  7 - sg
                            12,  //  8 - r
                            14,  //  9 - sr
                            -1, -1, -1, -1, -1, -1,   // Invalid combinations
                             1,  // 16 - bC
                             3,  // 17 - sbC
                             5,  // 18 - lC
                             7,  // 19 - slC
                            -1,  // 20 - aC  - not used
                            -1,  // 21 - saC - not used
                             9,  // 22 - gC
                            11,  // 23 - sgC
                            13,  // 24 - rC
                            15,  // 25 - srC
                            -1, -1, -1, -1, -1, -1};  // Invalid combinations

//-----------------------------------------------------------------

// Callback Routine to find Unicode font for Virtual Keyboard
// The following code IS used by the correct method.
static int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX *, NEWTEXTMETRICEX *,
                                           DWORD , LPARAM lParam)
{
  // Found one
  bool *pFound = (bool *)lParam;
  *pFound = true;

  // Don't call me anymore - I'm done
  return 0;
}

int  CVKeyBoardDlg::m_iFont = -1;
bool CVKeyBoardDlg::m_bUserSpecifiedFont = false;
wchar_t *CVKeyBoardDlg::ARIALUMS = L"Arial Unicode MS";
wchar_t *CVKeyBoardDlg::ARIALU   = L"Arial Unicode";
wchar_t *CVKeyBoardDlg::LUCIDAUS = L"Lucida Sans Unicode";

bool CVKeyBoardDlg::IsOSKAvailable()
{
  /**
   *Check if we can support On-Screen Keyboards. Return true iff:
   * 1. Can load the dll
   * 2. Version matches + has required functions
   * 3. Can find the required font.
   */
  bool bVKAvailable(false);
  static bool warnedAlready(false); // warn only once per process.

  // Try to load DLL
#if defined(_DEBUG) || defined(DEBUG)
  wchar_t *dll_name = L"pws_osk_D.dll";
#else
  wchar_t *dll_name = L"pws_osk.dll";
#endif
  HINSTANCE OSK_module = HINSTANCE(pws_os::LoadLibrary(dll_name, pws_os::LOAD_LIBRARY_APP));

  if (OSK_module == NULL) {
    OutputDebugString(L"CVKeyBoardDlg::IsOSKAvailable - Unable to load OSK DLL. OSK not available.\n");
    return false;
  } else {
    LP_OSK_GetKeyboardData pGetKBData =
      LP_OSK_GetKeyboardData(pws_os::GetFunction(OSK_module, "OSK_GetKeyboardData"));
    LP_OSK_ListKeyboards pListKBs =
      LP_OSK_ListKeyboards(pws_os::GetFunction(OSK_module, "OSK_ListKeyboards"));
    LP_OSK_GetVersion pOSKVersion =
      LP_OSK_GetVersion(pws_os::GetFunction(OSK_module, "OSK_GetVersion"));

    if (pListKBs == NULL || pGetKBData == NULL || pOSKVersion == NULL)
      OutputDebugString(L"CVKeyBoardDlg::IsOSKAvailable - Unable to get all required OSK functions. OSK not available.\n");
    else if (pOSKVersion() == VK_DLL_VERSION) {
      bVKAvailable = true;
    } else if (!warnedAlready) {
      warnedAlready = true;
      stringT sText;
      LoadAString(sText, IDS_OSK_VERSION_MISMATCH);
      MessageBox(NULL, sText.c_str(), NULL, MB_ICONERROR);
    }

    pws_os::FreeLibrary(OSK_module);
   }

  if (!bVKAvailable)
    return false;

  // We have the DLL, now check Unicode font installed
  bool bFound(false);
  LOGFONT lf = {0, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0,
                L""};

  HDC hDC = ::GetDC(NULL);

  // First check user's font (if any)
 stringT cs_VKeyboardFont = L"";
  if (cs_VKeyboardFont.length() != 0 &&
      cs_VKeyboardFont.length() <= LF_FACESIZE) {
    m_bUserSpecifiedFont = true;
    memcpy_s(lf.lfFaceName, LF_FACESIZE * sizeof(wchar_t),
             cs_VKeyboardFont.c_str(), cs_VKeyboardFont.length() * sizeof(wchar_t));

    EnumFontFamiliesEx(hDC, &lf,
                       (FONTENUMPROC)&EnumFontFamiliesExProc,
                       (LPARAM)(&bFound), 0);
  }

  if (bFound) {
    m_iFont = USER_FONT;
    goto exit;
  }

  // Next check for Arial Unicode MS
  memcpy_s(lf.lfFaceName, LF_FACESIZE * sizeof(wchar_t),
           ARIALUMS, wcslen(ARIALUMS) * sizeof(wchar_t));
  EnumFontFamiliesEx(hDC, &lf,
                     (FONTENUMPROC)&EnumFontFamiliesExProc,
                     (LPARAM)(&bFound), 0);

  if (bFound) {
    m_iFont = ARIALMS_FONT;
    goto exit;
  }

  // Next check for Arial Unicode (commercial version of MS font)
  SecureZeroMemory(lf.lfFaceName, sizeof(lf.lfFaceName));
  memcpy_s(lf.lfFaceName, sizeof(lf.lfFaceName),
           ARIALU, wcslen(ARIALU) * sizeof(wchar_t));
  EnumFontFamiliesEx(hDC, &lf,
                     (FONTENUMPROC)&EnumFontFamiliesExProc,
                     (LPARAM)(&bFound), 0);

  if (bFound) {
    m_iFont = ARIAL_FONT;
    goto exit;
  }

  // Lastly check for Lucida Sans Unicode
  SecureZeroMemory(lf.lfFaceName, sizeof(lf.lfFaceName));
  memcpy_s(lf.lfFaceName, sizeof(lf.lfFaceName),
           LUCIDAUS, wcslen(LUCIDAUS) * sizeof(wchar_t));
  EnumFontFamiliesEx(hDC, &lf,
                     (FONTENUMPROC)&EnumFontFamiliesExProc,
                     (LPARAM)(&bFound), 0);

  if (bFound) {
    m_iFont = LUCIDA_FONT;
    goto exit;
  }

  OutputDebugString(L"CVKeyBoardDlg::IsOSKAvailable - No Unicode font installed. OSK not available.\n");
  if (!warnedAlready) {
    warnedAlready = true;
    stringT sText;
    LoadAString(sText, IDS_OSK_NO_UNICODE_FONT);
    MessageBox(NULL, sText.c_str(), NULL, MB_ICONERROR);
  }

exit:
  ::ReleaseDC(NULL, hDC);
  return bFound;
}

stringT get_window_text(HWND hWnd)
{
  const int len = GetWindowTextLength(hWnd) + 1;
  stringT s(len, 0);

  if (len > 1)
  {
    GetWindowText(hWnd, &s[0], len);
    s.pop_back();  // Remove trailing NULL [C++11 feature]
  }
  else
    s.clear();

  return s;
}

//-----------------------------------------------------------------------------
CVKeyBoardDlg::CVKeyBoardDlg(HWND hParent, HWND hMasterPhrase, LPCWSTR wcKLID)
  : m_hParent(hParent), m_hMasterPhrase(hMasterPhrase),
    m_PassphraseFont(NULL),
    m_phrase(L""), m_phrasecount(0), m_State(0), m_SaveState(0),
    m_bShift(false), m_bLCtrl(false), m_bRCtrl(false),
    m_bAltGr(false), m_bAltNum(false),
    m_bCapsLock(false), m_bRandom(false),
    m_bLCtrlChars(false), m_bAltGrChars(false), m_bRCtrlChars(false),
    m_bDeadKeyActive(false), m_iKeyboard(0), m_Kana(0), m_Hiragana(0), m_Size(0),
    m_bSaveKLID(BST_CHECKED)
{
  // Verify all is OK
  ASSERT(_countof(defscancodes101) == NUM_KEYS);
  ASSERT(_countof(defscancodes102) == NUM_KEYS);
  ASSERT(_countof(defscancodes106) == NUM_KEYS);
  ASSERT(_countof(pXdefnumbers) == NUM_DIGITS);

  // Get us
  m_hInstance = GetModuleHandle(NULL);

  m_bUseSecureDesktop = PWSprefs::GetInstance()->GetPref(PWSprefs::UseSecureDesktop);
  m_iUserTimeLimit = PWSprefs::GetInstance()->GetPref(PWSprefs::SecureDesktopTimeout);

  // Initialise numbers
  for (int i = 0; i < NUM_DIGITS; i++) {
    m_pnumbers[i] = _wcsdup(pXdefnumbers[i]);
  }

  // Set background colour for for dialog as white
  m_hBkBrush = CreateSolidBrush(RGB(255, 255, 255));

  // dll is guaranteed to be loadable, right version and in general 100% kosher
  // by IsOSKAvailable(). Caller is responsible to call that, though...
#if defined(_DEBUG) || defined(DEBUG)
  wchar_t *dll_name = L"pws_osk_D.dll";
#else
  wchar_t *dll_name = L"pws_osk.dll";
#endif
  m_OSK_module = HMODULE(pws_os::LoadLibrary(dll_name, pws_os::LOAD_LIBRARY_APP));

  ASSERT(m_OSK_module != NULL);
  m_pGetKBData = LP_OSK_GetKeyboardData(pws_os::GetFunction(m_OSK_module,
                                                            "OSK_GetKeyboardData"));
  m_pListKBs   = LP_OSK_ListKeyboards(pws_os::GetFunction(m_OSK_module,
                                                          "OSK_ListKeyboards"));

  m_uiKLID = 0;
  if (wcKLID != NULL) {
    const wchar_t *wc_hex = L"0123456789ABCDEFabcdef";
   stringT sKLID(wcKLID);
    size_t non_hex = sKLID.find_first_not_of(wc_hex);

    // Make sure it is 8 hex characters and convert to a UINT
    if (sKLID.length() == 8 && non_hex ==stringT::npos) {
     stringT s(L"0x");
      s += sKLID;
      std::wistringstream iss(s);
      iss >> std::setbase(0) >> m_uiKLID;
    }
  }
}

CVKeyBoardDlg::~CVKeyBoardDlg()
{
  // Free the number values
  for (int i = 0; i < NUM_DIGITS; i++) {
    free(m_pnumbers[i]);
    m_pnumbers[i] = NULL;
  }

  // Delete the passphrase font
  if (m_PassphraseFont != NULL) {
    DeleteObject(m_PassphraseFont);
    m_PassphraseFont = NULL;
  }

  pws_os::FreeLibrary(m_OSK_module);
}

INT_PTR CVKeyBoardDlg::VKDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

  /**
      NOTE: Normally return code TRUE meaning it has processed this message and FALSE meaning it did not.

      However - MS Dpcumentation is conflicting!

      The following messages have different rules:
          WM_CHARTOITEM
          WM_COMPAREITEM
          WM_CTLCOLORBTN
          WM_CTLCOLORDLG
          WM_CTLCOLOREDIT
          WM_CTLCOLORLISTBOX
          WM_CTLCOLORSCROLLBAR
          WM_CTLCOLORSTATIC
          WM_INITDIALOG
          WM_QUERYDRAGICON
          WM_VKEYTOITEM
  **/

  static CVKeyBoardDlg *self;

  if (uMsg != WM_INITDIALOG && self == NULL)
    return FALSE;

  switch (uMsg) {
  case WM_INITDIALOG:
  {
    self = (CVKeyBoardDlg *)lParam;
    self->m_hwndDlg = hwndDlg;
    self->OnInitDialog();
    return TRUE; // Processed - special case
  }
  case WM_COMMAND:
  {
    const int iControlID = LOWORD(wParam);
    const int iNotificationCode = HIWORD(wParam);
    // lParam == handle to the control window
    switch (iNotificationCode) {
    case BN_CLICKED:
    {
      // Reset timer start time
      iStartTime = GetTickCount();

      switch (iControlID) {
      case IDC_VKCANCEL:
      {
        // If pressed when a Dead Key is active, just cancel this
        if (self->m_bDeadKeyActive)
        {
          self->SetDeadKeyEnvironment(false);
          self->SetButtons();
          return TRUE;
        }

        // Cancel dialog - modeless - just hide & disable
        ShowWindow(hwndDlg, SW_HIDE);

        return TRUE;
      }
      case IDC_VK101: { self->OnChangeKeyboardType(); return TRUE; }
      case IDC_VK102: { self->OnChangeKeyboardType(); return TRUE; }
      case IDC_VKINSERT: { self->OnInsert(); return TRUE; }
      case IDC_VKCLEARBUFFER: { self->OnClearBuffer(); return TRUE; }
      case IDC_VKBACKSPACE: { self->OnBackSpace(); return TRUE; }
      case IDC_VKBBTN_LSHIFT: { self->OnShift(); return TRUE; }
      case IDC_VKBBTN_RSHIFT: { self->OnShift(); return TRUE; }
      case IDC_VKBBTN_LCTRL: { self->OnLCtrl(); return TRUE; }
      case IDC_VKBBTN_RCTRL: { self->OnRCtrl(); return TRUE; }
      case IDC_VKBBTN_RHCTRL: { self->OnRHCtrl(); return TRUE; }
      case IDC_VKBBTN_ALTGR: { self->OnAltGr(); return TRUE; }
      case IDC_VKBBTN_ALTNUM: { self->OnAltNum(); return TRUE; }
      case IDC_VKBBTN_CAPSLOCK: { self->OnCapsLock(); return TRUE; }
      case IDC_VKBBTN_SPACEBAR: { self->OnSpaceBar(); return TRUE; }
      case IDC_VKBBTN_SMALLSPACEBAR: { self->OnSpaceBar(); return TRUE; }
      case IDC_VKBBTN_SIZE: { self->OnKeySize(); return TRUE; }
      case IDC_VKBBTN_HIRAGANA: { self->OnHiragana(); return TRUE; }
      case IDC_VKRANDOMIZE: { self->OnRandomize(); return TRUE; }
      case IDC_SAVEKLID: { self->OnSaveKLID(); return TRUE; }
      default:
      {
        if (iControlID >= IDC_VKBBTN_N0 && iControlID <= IDC_VKBBTN_N9)
        {
          self->OnNumerics(iControlID);
          return TRUE;
        }
        if (iControlID >= IDC_VKBBTN_KBD01 && iControlID <= IDC_VKBBTN_KBD51)
        {
          self->OnKeys(iControlID);
          return TRUE;
        }
      }
      }
    }  // BN_CLICKED
    case  CBN_SELCHANGE:
    {
      if (iControlID == IDC_VKEYBOARDS) {
        self->OnChangeKeyboard();
        return TRUE;
      }
    }  // CBN_SELCHANGE
    }  // switch (iNotificationCode)
  } // WM_COMMAND

  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLORDLG:
  {
    if (!IsWindowEnabled(hwndDlg))
      return NULL;

    // Black text on white background
    SetTextColor((HDC)wParam, RGB(0, 0, 0));
    SetBkColor((HDC)wParam, RGB(255, 255, 255));
    return (INT_PTR)(self->m_hBkBrush);
  }
  case WM_LBUTTONDOWN:
  {
    PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
    return (INT_PTR)TRUE;
  }
  case WM_DRAWITEM:
  {
    const int iControlID = LOWORD(wParam);
    // Need to get the handle for this control and then the corresponding CXKBButton
    Iter_Map_ID_XButton id = self->m_Map_ID2XButton.find(iControlID);
    if (id != self->m_Map_ID2XButton.end())
    {
      id->second->DrawItem((LPDRAWITEMSTRUCT)lParam);
    }
    return (INT_PTR)TRUE;
  }
  } // switch (uMsg)

  return FALSE;  // Not processed
}

BOOL CVKeyBoardDlg::OnInitDialog()
{
  m_hwndVKStaticTimer = GetDlgItem(m_hwndDlg, IDC_STATIC_TIMER);
  m_hwndVKStaticTimerText = GetDlgItem(m_hwndDlg, IDC_STATIC_TIMERTEXT);
  m_hwndVKStaticSeconds = GetDlgItem(m_hwndDlg, IDC_STATIC_SECONDS);

  // Set up buttons button
  for (int i = 0; i < NUM_DIGITS; i++) {
    m_vkbb_Numbers[i].m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_N0 + i);
    m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_N0 + i, &m_vkbb_Numbers[i]));
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    m_vkbb_Keys[i].m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_KBD01 + i);
    m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_KBD01 + i, &m_vkbb_Keys[i]));
  }

  m_vkbb_Randomize.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKRANDOMIZE);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKRANDOMIZE, &m_vkbb_Randomize));

  m_vkbb_InsertClose.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_N0);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_KBD01, &m_vkbb_InsertClose));

  m_vkbb_Insert.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKINSERT);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKINSERT, &m_vkbb_Insert));

  m_vkbb_Cancel.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKCANCEL);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKCANCEL, &m_vkbb_Cancel));

  m_vkbb_ClearBuffer.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKCLEARBUFFER);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKCLEARBUFFER, &m_vkbb_ClearBuffer));

  m_vkbb_BackSpace.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBACKSPACE);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBACKSPACE, &m_vkbb_BackSpace));

  m_vkbb_SpaceBar.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_SPACEBAR);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_SPACEBAR, &m_vkbb_SpaceBar));

  m_vkbb_SmallSpaceBar.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_SMALLSPACEBAR);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_SMALLSPACEBAR, &m_vkbb_SmallSpaceBar));

  m_vkbb_LShift.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_LSHIFT);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_LSHIFT, &m_vkbb_LShift));

  m_vkbb_RShift.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_RSHIFT);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_RSHIFT, &m_vkbb_RShift));

  m_vkbb_LCtrl.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_LCTRL);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_LCTRL, &m_vkbb_LCtrl));

  m_vkbb_RCtrl.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_RCTRL);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_RCTRL, &m_vkbb_RCtrl));

  m_vkbb_RHCtrl.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_RHCTRL);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_RHCTRL, &m_vkbb_RHCtrl));

  m_vkbb_Alt.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_ALT);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_ALT, &m_vkbb_Alt));

  m_vkbb_AltGr.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_ALTGR);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_ALTGR, &m_vkbb_AltGr));

  m_vkbb_AltNum.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_ALTNUM);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_ALTNUM, &m_vkbb_AltNum));

  m_vkbb_CapsLock.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_CAPSLOCK);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_CAPSLOCK, &m_vkbb_CapsLock));

  m_vkbb_Size.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_SIZE);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_SIZE, &m_vkbb_Size));

  m_vkbb_Hiragana.m_hWnd = GetDlgItem(m_hwndDlg, IDC_VKBBTN_HIRAGANA);
  m_Map_ID2XButton.insert(pair<UINT, CVKBButton *>(IDC_VKBBTN_HIRAGANA, &m_vkbb_Hiragana));

  // Remove flat style from 'real' buttons
  m_vkbb_Randomize.SetFlatState(false);
  m_vkbb_InsertClose.SetFlatState(false);
  m_vkbb_Insert.SetFlatState(false);
  m_vkbb_Cancel.SetFlatState(false);
  m_vkbb_ClearBuffer.SetFlatState(false);
  m_vkbb_LShift.SetFlatState(false);
  m_vkbb_RShift.SetFlatState(false);
  m_vkbb_LCtrl.SetFlatState(false);
  m_vkbb_RCtrl.SetFlatState(false);
  m_vkbb_Alt.SetFlatState(false);
  m_vkbb_AltGr.SetFlatState(false);
  m_vkbb_AltNum.SetFlatState(false);
  m_vkbb_CapsLock.SetFlatState(false);
  m_vkbb_BackSpace.SetFlatState(false);

  // Make Japanese button push style but not to change colour when pushed
  m_vkbb_Size.SetFlatState(false);
  m_vkbb_Size.ChangePushColour(false);
  m_vkbb_Hiragana.SetFlatState(false);
  m_vkbb_Hiragana.ChangePushColour(false);

  // Alt, Left & Right-Half Control always disabled, as is the 106-key radio button
  EnableWindow(m_vkbb_Alt.m_hWnd, FALSE);
  EnableWindow(m_vkbb_LCtrl.m_hWnd, FALSE);
  EnableWindow(m_vkbb_RHCtrl.m_hWnd, FALSE);
  EnableWindow(GetDlgItem(m_hParent, IDC_VK106), FALSE);

  // Initially nothing to reset
  EnableWindow(m_vkbb_Insert.m_hWnd, FALSE);
  EnableWindow(m_vkbb_ClearBuffer.m_hWnd, FALSE);

  // Get window handle for keyboard combobox
  m_hcbxKeyBoards = GetDlgItem(m_hwndDlg, IDC_VKEYBOARDS);

  int nKeyboards = SendMessage(m_hcbxKeyBoards, CB_GETCOUNT, NULL, NULL);

  if (nKeyboards == 0) {
    // Get current Keyboard layout name
    wchar_t wcKLID[KL_NAMELENGTH + 1];
    VERIFY(GetKeyboardLayoutName(wcKLID));

    // Convert from hex string to integer
   stringT s(L"0x");
    s += wcKLID;
    std::wistringstream iss(s);
    iss >> std::setbase(0) >> m_uiPhysKLID;

    st_Keyboard_Layout kbl;
    kbl.uiKLID = m_uiPhysKLID;
    kbl.uiCtrlID = IDS_VKB_PHYSICAL;

    m_KBL.push_back(kbl);

    // Get all Keyboard layouts installed
    GetAllKeyboardsAvailable();

    KBL_citer kbl_iter;
    for (kbl_iter = m_KBL.begin(); kbl_iter != m_KBL.end(); kbl_iter++) {
      const st_Keyboard_Layout &st_kbl = *kbl_iter;
      stringT stemp;
      LoadAString(stemp, st_kbl.uiCtrlID);
      int iItem = SendMessage(m_hcbxKeyBoards, CB_ADDSTRING, NULL, (LPARAM)(stemp.c_str()));
      SendMessage(m_hcbxKeyBoards, CB_SETITEMDATA, iItem, (DWORD)st_kbl.uiKLID);
    }
  }

  int cbx_index(0);
  if (m_uiKLID != 0) {
    // Select last used - but first find it, as ComboBox is sorted by name
    for (int i = 0; i < nKeyboards; i++) {
      if ((UINT)SendMessage(m_hcbxKeyBoards, CB_SETITEMDATA, i, NULL) == m_uiKLID) {
        cbx_index = i;
        break;
      }
    }
  }
  else {
    m_uiKLID = m_uiPhysKLID;
  }

  SendMessage(m_hcbxKeyBoards, CB_SETCURSEL, cbx_index, NULL);

  if (m_uiKLID == JAPANESE_KBD) {
    SetJapaneseKeyboard();
  }
  else if (m_uiKLID == KOREAN_KBD) {
    SetKoreanKeyboard();
  }
  else {
    SetStandardKeyboard();
  }

  // Set to save keyboard
  SendMessage(GetDlgItem(m_hParent, IDC_SAVEKLID), BM_SETCHECK, BST_CHECKED, NULL);

  // Set keyboard type
  CheckRadioButton(m_hwndDlg, IDC_VK101, IDC_VK106, IDC_VK101);

  // Set number of characters in the buffer
  SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), L"0");

  HWND hwndStaticTimer = GetDlgItem(m_hwndDlg, IDC_STATIC_TIMER);

  if (m_bUseSecureDesktop) {
    // Set User Time Limit in Winodw
    int iMinutes = m_iUserTimeLimit / 60;
    int iSeconds = m_iUserTimeLimit - (60 * iMinutes);
    stringT sTime;
    Format(sTime, _T("%02d:%02d"), iMinutes, iSeconds);
    SetWindowText(hwndStaticTimer, sTime.c_str());
  }
  else
  {
    // Hide it if not using Secure Desktop
    ShowWindow(hwndStaticTimer, SW_HIDE);
  }

  // Set the tooltip
  // Note naming convention: string IDS_VKxxx corresponds to control IDC_xxx

  // Create the tooltip. g_hInst is the global instance handle.
  m_hwndTooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
    WS_POPUP | WS_EX_TOOLWINDOW | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
    CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT,
    m_hwndDlg, NULL,
    m_hInstance, NULL);

  if (!m_hwndTooltip)
    ASSERT(0);

  SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);

  //int iTime = SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_AUTOPOP, NULL);
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 1000);       // Default  500 ms
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 5000);       // Default 5000 ms
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_RESHOW,  1000);       // Default  100 ms

  AddTooltip(IDC_VKCLEARBUFFER, IDS_VKCLEARBUFFER);
  AddTooltip(IDC_VKBBTN_CAPSLOCK, IDS_VKSTATIC_CAPSLOCK);
  AddTooltip(IDC_VKBBTN_LSHIFT, IDS_VKSTATIC_SHIFT);
  AddTooltip(IDC_VKBBTN_RSHIFT, IDS_VKSTATIC_SHIFT);
  AddTooltip(IDC_VKBBTN_LCTRL, IDS_VKLCTRL, IDS_VKSTATIC_SPECIAL);
  AddTooltip(IDC_VKBBTN_RCTRL, IDS_VKRCTRL, IDS_VKSTATIC_SPECIAL);
  AddTooltip(IDC_VKBBTN_ALTGR, IDS_VKALTGR, IDS_VKSTATIC_SPECIAL);
  AddTooltip(IDC_VKBBTN_ALT, IDS_VKSTATIC_ALT);
  AddTooltip(IDC_VKBBTN_ALTNUM, IDS_VKSTATIC_ALTNUM);
  AddTooltip(IDC_VKRANDOMIZE, IDS_VKSTATIC_RANDOMIZE);

  // If not using the user specified font, show the warning.
  if (m_iFont != USER_FONT && m_bUserSpecifiedFont) {
   stringT cs_VKeyboardFont = L"";
    ShowWindow(GetDlgItem(m_hParent, IDC_INFO), SW_SHOW);
    EnableWindow(GetDlgItem(m_hParent, IDC_INFO), TRUE);
    wchar_t * pszFont(NULL);
    switch (m_iFont) {
    case ARIALMS_FONT:
      pszFont = ARIALUMS;
      break;
    case ARIAL_FONT:
      pszFont = ARIALU;
      break;
    case LUCIDA_FONT:
      pszFont = LUCIDAUS;
      break;
    default:
      ASSERT(0);
    }
    if (pszFont != NULL) {
      stringT sTemp;
      Format(sTemp, IDS_USRFONT, cs_VKeyboardFont.c_str(), pszFont);
      AddTooltip(IDC_INFO, sTemp);
    }
  }
  else
  {
    if (m_iFont == LUCIDA_FONT) {
      ShowWindow(GetDlgItem(m_hParent, IDC_INFO), SW_SHOW);
      EnableWindow(GetDlgItem(m_hParent, IDC_INFO), TRUE);
      AddTooltip(IDC_INFO, IDS_OSKFONT);
    }
  }
  // Activate tooltips
  SendMessage(m_hwndTooltip, TTM_ACTIVATE, TRUE, NULL);

  // Set up characters
  ProcessKeyboard(m_uiKLID);

  // Show them
  SetButtons();

  // Apply Uincode font to all that need it
  for (int i = 0; i < NUM_DIGITS; i++) {
    ApplyUnicodeFont(GetDlgItem(m_hwndDlg, IDC_VKBBTN_N0 + i));
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    ApplyUnicodeFont(GetDlgItem(m_hwndDlg, IDC_VKBBTN_KBD01 + i));
  }

  // Make sure we can see the Japanese charatcers - if needed
  ApplyUnicodeFont(GetDlgItem(m_hwndDlg, IDC_VKBBTN_HIRAGANA));
  ApplyUnicodeFont(GetDlgItem(m_hwndDlg, IDC_VKBBTN_SIZE));

  if (m_phrasecount == 1) {
    EnableWindow(m_vkbb_Insert.m_hWnd, TRUE);
    EnableWindow(m_vkbb_ClearBuffer.m_hWnd, TRUE);
  }
  return TRUE;
}

void CVKeyBoardDlg::OnInsert()
{
  if (m_bAltNum)
    OnAltNum();

  // Return OK - modeless - just hide
  ShowWindow(m_hwndDlg, SW_HIDE);

  // Send data to caller
  SendMessage(m_hMasterPhrase, PWS_MSG_INSERTBUFFER, 0, 0);

  // Clear buffer!
  // Must make the user type the phrase in twice to confirm what they have
  // typed rather than just using the same (possibly incorrect) value.
  OnClearBuffer();
}

void CVKeyBoardDlg::OnNumerics(/* HWND hwnd, */ UINT nID)
{
  if (m_bAltNum) {
    // Alt + Numeric pad
    m_altchar = (m_altchar * 10) + (nID - IDC_VKBBTN_N0);
  } else {
    stringT stxt;
    stxt = get_window_text(m_vkbb_Numbers[nID - IDC_VKBBTN_N0].m_hWnd);
    m_phrase += stxt.c_str();
    m_phrasecount++;
    if (m_phrasecount == 1) {
      EnableWindow(m_vkbb_Insert.m_hWnd, TRUE);
      EnableWindow(m_vkbb_ClearBuffer.m_hWnd, TRUE);
    }

    stringT stemp;
    Format(stemp, L"%d", m_phrasecount);
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), stemp.c_str());
  }
}

void CVKeyBoardDlg::OnKeys(UINT nID)
{
  Iter_Map_st_SC2CHAR iter_sc;
  bool bDeadKeyPressed(false);
  stringT stxt;

  // Get character
  stxt = get_window_text(m_vkbb_Keys[nID - IDC_VKBBTN_KBD01].m_hWnd);

  // We don't support double deadkeys.  So if already active,
  // don't bother checking for another.
  if (!m_bDeadKeyActive) {
    iter_sc = m_map_stSC2Char.find(m_scancodes[nID - IDC_VKBBTN_KBD01]);
    if (iter_sc != m_map_stSC2Char.end()) {
      if (state2index[m_State] < 0) {
        OutputDebugString(L"OnKeys; Unknown state!\n");
        ASSERT(0);
      }
      bDeadKeyPressed = iter_sc->second.bsDeadKey.test(state2index[m_State]);
    } else {
      OutputDebugString(L"OnKeys: Unknown scancode pressed!\n");
      ASSERT(0);
    }
  }

  if (bDeadKeyPressed) {
  // Get the DeadKey and setup the DeadKey environment and combination keys
    wchar_t wc_temp;
    wc_temp = stxt.c_str()[0];

    Iter_MMap_DK2SCSSCC iter_DK2SCSSCC = m_stKBImpl.pmmapDK2SCSSCC->find(wc_temp);
    if (iter_DK2SCSSCC == m_stKBImpl.pmmapDK2SCSSCC->end()) {
      OutputDebugString(L"OnKeys; Unknown deadkey pressed!\n");
      ASSERT(0);
    } else {
      m_wcDeadKey = wc_temp;
      SetDeadKeyButtons();
      SetDeadKeyEnvironment(true);
    }
  } else {
    // Not a DeadKey
    m_phrase += stxt.c_str();
    m_phrasecount++;
    if (m_phrasecount == 1) {
      EnableWindow(m_vkbb_Insert.m_hWnd, TRUE);
      EnableWindow(m_vkbb_ClearBuffer.m_hWnd, TRUE);
    }
    stringT stemp;
    Format(stemp, L"%d", m_phrasecount);
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), stemp.c_str());

    bool bNeedToSetButtons(false);
    if (m_bDeadKeyActive) {
      // Was a deadkey combination character - reset back to normal keys
      m_wcDeadKey = (wchar_t)0;
      SetDeadKeyEnvironment(false);
      bNeedToSetButtons = true;
    }
    // Turn off shift once character added
    if (m_bShift) {
      OnShift();
      bNeedToSetButtons = false;
    }
    // If we didn't set buttons because of Shift status but need to because of
    // Dead Key change - do it now
    if (bNeedToSetButtons)
      SetButtons();
  }
}

void CVKeyBoardDlg::OnSpaceBar()
{
  // SpaceBar key - if a DeadKey is active, this adds the original character
  stringT stxt;
  stxt = get_window_text(m_vkbb_SpaceBar.m_hWnd);
  m_phrase += stxt.c_str();
  m_phrasecount++;

  if (m_phrasecount == 1) {
    EnableWindow(m_vkbb_Insert.m_hWnd, TRUE);
    EnableWindow(m_vkbb_ClearBuffer.m_hWnd, TRUE);
  }

  if (m_bDeadKeyActive) {
    // Was a deadkey combination character - reset back to normal keys
    SetDeadKeyEnvironment(false);
    SetButtons();
  }

  stringT stemp;
  Format(stemp, L"%d", m_phrasecount);
  SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), stemp.c_str());
}

void CVKeyBoardDlg::OnAltNum()
{
  // Allow adding of characters via the Numeric Key Pad
  m_bAltNum = !m_bAltNum;

  m_vkbb_AltNum.SetPushedState(m_bAltNum);

  if (m_bAltNum) {
    m_altchar = 0;
  } else {
    if (m_altchar > 0 && m_altchar < 32768) {
      wchar_t c[2] = {0, 0};
      c[0] = (wchar_t)m_altchar;
      m_phrase += c;
      m_phrasecount++;
      if (m_phrasecount == 1) {
        EnableWindow(m_vkbb_Insert.m_hWnd, TRUE);
        EnableWindow(m_vkbb_ClearBuffer.m_hWnd, TRUE);
      }
      stringT stemp;
      Format(stemp, L"%d", m_phrasecount);
      SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), stemp.c_str());
    }
  }

  // Can't have any other special keys active once AltNum pressed
  // Save the state for when the user has finished
  if (m_bAltNum) {
    m_SaveState = m_State;

    m_bSaveShift = m_bShift;
    m_bSaveLCtrl = m_bLCtrl;
    m_bSaveRCtrl = m_bRCtrl;
    m_bSaveAltGr = m_bAltGr;
    m_bSaveCapsLock = m_bCapsLock;

    m_bShift = m_bLCtrl = m_bRCtrl = m_bAltGr = m_bCapsLock = false;

    m_State |= VST_MENU;
    m_State &= ~(VST_SHIFT | VST_LCTRL | VST_ALTGR | VST_RCTRL | VST_CAPSLOCK);
  } else {
    m_State = m_SaveState;

    m_bShift = m_bSaveShift;
    m_bLCtrl = m_bSaveLCtrl;
    m_bRCtrl = m_bSaveRCtrl;
    m_bAltGr = m_bSaveAltGr;
    m_bCapsLock = m_bSaveCapsLock;
  }

  // Enable the Numeric Key Pad
  for (int i = 0; i < NUM_KEYS; i++) {
    EnableWindow(m_vkbb_Keys[i].m_hWnd, m_bAltNum ? FALSE : TRUE);
  }

  // Set Shift/Caps Lock buttons availability
  BOOL bEnableS, bEnableC, bEnable;
  if (m_bAltNum) {
    bEnableS = FALSE;
    bEnableC = FALSE;
  } else {
    bEnableS = m_bAllow_bS ? TRUE : FALSE;
    bEnableC = m_bAllow_bC ? TRUE : FALSE;
    if (m_bLCtrl) {
      bEnableS = m_bAllow_lS ? TRUE : FALSE;
      bEnableC = m_bAllow_lC ? TRUE : FALSE;
    }
    if (m_bAltGr) {
      bEnableS = m_bAllow_gS ? TRUE : FALSE;
      bEnableC = m_bAllow_gC ? TRUE : FALSE;
    }
    if (m_bRCtrl) {
      bEnableS = m_bAllow_rS ? TRUE : FALSE;
      bEnableC = m_bAllow_rC ? TRUE : FALSE;
    }
  }
  EnableWindow(m_vkbb_LShift.m_hWnd, bEnableS);
  EnableWindow(m_vkbb_RShift.m_hWnd, bEnableS);
  EnableWindow(m_vkbb_CapsLock.m_hWnd, bEnableC);

  bEnable = m_bAltNum ? FALSE : TRUE;

  // Don't touch the Left/Right Control keys if Japanese keyboard
  if (m_bLCtrlChars && m_uiKLID != JAPANESE_KBD)
    EnableWindow(m_vkbb_LCtrl.m_hWnd, bEnable);
  if (m_bRCtrlChars && m_uiKLID != JAPANESE_KBD)
    EnableWindow(m_vkbb_RCtrl.m_hWnd, bEnable);

  if (m_bAltGrChars)
    EnableWindow(m_vkbb_AltGr.m_hWnd, bEnable);

  // Don't allow changing keyboards in AltNum mode (don't worry about 106-keyboard)
  // And other keys active in the Japanese keyboard
  if (m_uiKLID != JAPANESE_KBD) {
    EnableWindow(GetDlgItem(m_hParent, IDC_VK101), bEnable);
    EnableWindow(GetDlgItem(m_hParent, IDC_VK102), bEnable);
  } else {
    if (m_Kana == JAPANESE) {
      EnableWindow(m_vkbb_SmallSpaceBar.m_hWnd, bEnable);
      EnableWindow(m_vkbb_Hiragana.m_hWnd, bEnable);
      if (m_Hiragana == KATAKANA)
        EnableWindow(m_vkbb_Size.m_hWnd, bEnable);
    }
  }

  // Allow/deny other controls
  EnableWindow(m_hcbxKeyBoards, bEnable);
  EnableWindow(m_vkbb_Randomize.m_hWnd, bEnable);
  if (m_uiKLID != JAPANESE_KBD || (m_uiKLID == JAPANESE_KBD && m_Kana == ENGLISH))
    EnableWindow(m_vkbb_SpaceBar.m_hWnd, bEnable);
}

void CVKeyBoardDlg::OnBackSpace()
{
  if (m_phrasecount > 0) {
    m_phrase.erase(m_phrasecount - 1, 1);
    m_phrasecount--;

    stringT stemp;
    Format(stemp, L"%d", m_phrasecount);
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), stemp.c_str());
  }
}

void CVKeyBoardDlg::OnShift()
{
  m_bShift = !m_bShift;

  m_vkbb_LShift.SetPushedState(m_bShift);
  m_vkbb_RShift.SetPushedState(m_bShift);
  InvalidateRect(m_vkbb_LShift.m_hWnd, NULL, FALSE);
  InvalidateRect(m_vkbb_RShift.m_hWnd, NULL, FALSE);

  if (m_bShift)
    m_State |= VST_SHIFT;
  else
    m_State &= ~VST_SHIFT;

  SetButtons();
}

void CVKeyBoardDlg::OnAltGr()
{
  m_bAltGr = !m_bAltGr;

  m_vkbb_AltGr.SetPushedState(m_bAltGr);

  // Korean & Japanese
  //CString cs_ToolTip;
  switch (m_uiKLID) {
    case KOREAN_KBD:
      m_State = 0;
      SetWindowText(m_vkbb_AltGr.m_hWnd, m_bAltGr ? L"Kor" : L"Eng");
      UpdateTooltipText(IDC_VKBBTN_ALTGR, m_bAltGr ? IDS_VK_SW_ENGLISH : IDS_VK_SW_KOREAN);
      break;
    case JAPANESE_KBD:
      m_State = 0;
      SetWindowText(m_vkbb_AltGr.m_hWnd, m_bAltGr ? L"Kana" : L"Eng");
      m_Kana = m_bAltGr ? JAPANESE : ENGLISH;
      SetSpecialKeys();

      UpdateTooltipText(IDC_VKBBTN_ALTGR, m_bAltGr ? IDS_VK_SW_ENGLISH : IDS_VK_SW_KANA);
      if (m_bAltGr) {
        AddTooltip(IDC_VKBBTN_HIRAGANA, IDS_VK_SW_KATAKANA);
      } else {
        DeleteTooltip(IDC_VKBBTN_HIRAGANA);
      }
      break;
  }

  if (m_bAltGr)
    m_State |= VST_ALTGR;
  else
    m_State &= ~VST_ALTGR;

  if (m_bRCtrlChars)
    EnableWindow(m_vkbb_RCtrl.m_hWnd, m_bAltGr ? FALSE : TRUE);

  if (m_bAltGr) {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_gS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_gS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_gC ? TRUE : FALSE);
  } else {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_bC ? TRUE : FALSE);
  }

  SetButtons();
}

void CVKeyBoardDlg::OnCapsLock()
{
  // Note: In Korean keyboard - can't have CapsLock without Shift
  m_bCapsLock = !m_bCapsLock;

  m_vkbb_CapsLock.SetPushedState(m_bCapsLock);

  if (m_bCapsLock)
    m_State |= VST_CAPSLOCK;
  else
    m_State &= ~VST_CAPSLOCK;

  SetButtons();
}

void CVKeyBoardDlg::OnLCtrl()
{
  m_bLCtrl = !m_bLCtrl;

  m_vkbb_LCtrl.SetPushedState(m_bLCtrl);

  if (m_bLCtrl)
    m_State |= VST_LCTRL;
  else
    m_State &= ~VST_LCTRL;

  if (m_bLCtrl) {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_lS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_lS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_lC ? TRUE : FALSE);
  } else {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_bC ? TRUE : FALSE);
  }

  SetButtons();
}

void CVKeyBoardDlg::OnRCtrl()
{
  // Handle normal size Right Control button
  DoRCtrl(true);
}

void CVKeyBoardDlg::OnRHCtrl()
{
  // Handle half-size Right Control button
  DoRCtrl(false);
}

void CVKeyBoardDlg::DoRCtrl(const bool bDoFull)
{
  // Handle both normal size and half-size Right Control button
  m_bRCtrl = !m_bRCtrl;

  if (bDoFull)
    m_vkbb_RCtrl.SetPushedState(m_bRCtrl);
  else
    m_vkbb_RHCtrl.SetPushedState(m_bRCtrl);

  if (m_bRCtrl)
    m_State |= VST_RCTRL;
  else
    m_State &= ~VST_RCTRL;

  EnableWindow(m_vkbb_AltNum.m_hWnd, m_bRCtrl ? FALSE : TRUE);
  if (m_bAltGrChars)
    EnableWindow(m_vkbb_AltGr.m_hWnd, m_bRCtrl ? FALSE : TRUE);

  if (m_bLCtrlChars)
    EnableWindow(m_vkbb_LCtrl.m_hWnd, m_bRCtrl ? FALSE : TRUE);

  if (m_bRCtrl) {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_rS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_rS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_rC ? TRUE : FALSE);
  } else {
    EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
    EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_bC ? TRUE : FALSE);
  }

  SetButtons();
}

void CVKeyBoardDlg::OnKeySize()
{
  // Switch between Half-width & Full-width - Katakana only
  // 0 == Half-width; 1 = Full-width
  m_Size = 1 - m_Size;

  stringT sSize;
  sSize = m_Size == HALF ? wcHalfWidth : wcFullWidth;
  SetWindowText(m_vkbb_Size.m_hWnd, sSize.c_str());

  UpdateTooltipText(IDC_VKBBTN_SIZE, m_Size == HALF ? IDS_VK_SW_FULLWIDTH : IDS_VK_SW_HALFWIDTH);
  SendMessage(m_hwndTooltip, TTM_UPDATE, 0, 0);

  SetButtons();

  UpdateWindow(m_vkbb_Size.m_hWnd);
}

void CVKeyBoardDlg::OnHiragana()
{
  // Switch between Hiragana & Katakana
  // 0 == Hiragana; 1 = Katakana
  m_Hiragana = 1 - m_Hiragana;

  stringT s_HK = m_Hiragana == HIRAGANA ? wcHiragana : wcKatakana;
  SetWindowText(m_vkbb_Hiragana.m_hWnd, s_HK.c_str());

  UpdateTooltipText(IDC_VKBBTN_HIRAGANA, m_Hiragana == HIRAGANA ? IDS_VK_SW_KATAKANA : IDS_VK_SW_HIRAGANA);

  if (m_Hiragana == HIRAGANA) {
    DeleteTooltip(IDC_VKBBTN_SIZE);
  } else {
    m_Size = 0;
    AddTooltip(IDC_VKBBTN_SIZE, IDS_VK_SW_FULLWIDTH);
  }

  SendMessage(m_hwndTooltip, TTM_UPDATE, 0, 0);

  SetSpecialKeys();

  SetButtons();

  UpdateWindow(m_vkbb_Hiragana.m_hWnd);
}

void CVKeyBoardDlg::OnRandomize()
{
  m_bRandom = !m_bRandom;

  m_vkbb_Randomize.SetPushedState(m_bRandom);

  // Reset numbers and scan codes
  ResetKeys();

  if (m_bRandom) {
    // Now 'randomise' them!
    PWSrand *pwsr = PWSrand::GetInstance();
    int ir;
    BYTE itemp;
    wchar_t *pwctemp;

    for (int i = 0; i < NUM_KEYS; i++) {
      ir = pwsr->RangeRand(NUM_KEYS - 1);
      if (m_scancodes[i] == 0 || m_scancodes[ir] == 0)
        continue;

      itemp = m_scancodes[i];
      m_scancodes[i] = m_scancodes[ir];
      m_scancodes[ir] = itemp;
    }

    for (int i = 0; i < NUM_DIGITS; i++) {
      ir = pwsr->RangeRand(NUM_DIGITS - 1);
      pwctemp = m_pnumbers[i];
      m_pnumbers[i] = m_pnumbers[ir];
      m_pnumbers[ir] = pwctemp;
    }
  }

  for (int i = 0; i < NUM_DIGITS; i++) {
    SetWindowText(m_vkbb_Numbers[i].m_hWnd, m_pnumbers[i]);
  }

  // Reset table of valid scancodes - order IS VERY important!
  m_vsc.clear();
  for (int i = 0; i < NUM_KEYS; i++) {
    m_vsc.push_back(m_scancodes[i]);
  }
  m_vsc.push_back(0x39); // Add on the end the Spacebar

  SetButtons();
}

void CVKeyBoardDlg::SetButtons()
{
  if (m_bDeadKeyActive)
    SetDeadKeyButtons();
  else
    SetNormalButtons();
}

void CVKeyBoardDlg::SetNormalButtons()
{
  // Set Normal Buttons
  stringT sDeadkey;
  LoadAString(sDeadkey, IDS_VKDEADKEY);

  if (m_bAltNum) {
    // Normal keys disbled if using AltNum
    for (int i = 0; i < NUM_KEYS; i++) {
      SetWindowText(m_vkbb_Keys[i].m_hWnd, L"");
      EnableWindow(m_vkbb_Keys[i].m_hWnd, FALSE);
      m_vkbb_Keys[i].SetDeadKeyState(false);
    }
  } else {
    // Normal keys
    Iter_Map_st_SC2CHAR iter_sc;
    CIter_Map_SCSS2MC citer_scss;
    stringT sTemp;
    wchar_t wc_temp;
    int index;
    bool bDeadKey;

    if (m_uiKLID == JAPANESE_KBD) {
      if (m_Kana == ENGLISH) {
        index = 0;       // Base
      } else {
        if (m_Hiragana == HIRAGANA) {
          index = 4;     // Left Control
        } else {
          // Katakana
          if (m_Size == FULL) {
            index = 8;   // AltGr
          } else {
            index = 12;  // Right Control
          }
        }
      }
      index = index + (m_bShift ? 2 : 0) + (m_bCapsLock ? 1 : 0);
    } else
      index = state2index[m_State];

    if (state2index[m_State] < 0) {
      OutputDebugString(L"SetButtons: Unknown state! (1)\n");
      ASSERT(0);
    }

    // Now put the character on the keys
    for (int i = 0; i < NUM_KEYS; i++) {
      bDeadKey = false;
      sTemp.clear();
      if (m_scancodes[i] == 0 ||
          m_map_stSC2Char.find(m_scancodes[i]) == m_map_stSC2Char.end()) {
        // Zero scancode or not in our map to a character
        //     == unused key - disable/don't show
        SetWindowText(m_vkbb_Keys[i].m_hWnd, sTemp.c_str());
        EnableWindow(m_vkbb_Keys[i].m_hWnd, FALSE);
        ShowWindow(m_vkbb_Keys[i].m_hWnd, SW_HIDE);
      } else {
        iter_sc = m_map_stSC2Char.find(m_scancodes[i]);
        if (iter_sc != m_map_stSC2Char.end()) {
          if (index < 0) {
            OutputDebugString(L"SetButtons: Unknown state! (2)\n");
            ASSERT(0);
          } else {
            // Get scancode + shiftstate value
            unsigned short int uiSCSS = m_scancodes[i] * 256 + m_State;

            // Get the wchar_t character
            wc_temp = iter_sc->second.wcChar[index];
            sTemp = wc_temp;

            // If negative, it MAY be a multi-character value - although some Asian languages do
            // use Unicode values greater than 0x7FFF.
            // We only have 2, 3 & 4 multi-character sequences.  0xFFF8-F are reserved in
            // the Unicode standard and so we are covered.
            switch ((short int)wc_temp) {
              case -2:
                citer_scss = m_stKBImpl.pmapSCSS2MC2->find(uiSCSS);
                if (citer_scss != m_stKBImpl.pmapSCSS2MC2->end()) {
                  wchar_t pctemp2[3] = {0, 0, 0};
                  memcpy((void *)&pctemp2[0], (void *)&m_stKBImpl.wcMC2[citer_scss->second * 2 * sizeof(wchar_t)], 2 * sizeof(wchar_t));
                  sTemp = pctemp2;
                } else
                  sTemp = wc_temp;
                break;
              case -3:
                citer_scss = m_stKBImpl.pmapSCSS2MC3->find(uiSCSS);
                if (citer_scss != m_stKBImpl.pmapSCSS2MC3->end()) {
                  wchar_t pctemp3[4] = {0, 0, 0, 0};
                  memcpy((void *)&pctemp3[0], (void *)&m_stKBImpl.wcMC3[citer_scss->second * 3 * sizeof(wchar_t)], 3 * sizeof(wchar_t));
                  sTemp = pctemp3;
                } else
                  sTemp = wc_temp;
                break;
              case -4:
                citer_scss = m_stKBImpl.pmapSCSS2MC4->find(uiSCSS);
                if (citer_scss != m_stKBImpl.pmapSCSS2MC4->end()) {
                  wchar_t pctemp4[5] = {0, 0, 0, 0, 0};
                  memcpy((void *)&pctemp4[0], (void *)&m_stKBImpl.wcMC4[citer_scss->second * 4 * sizeof(wchar_t)], 4 * sizeof(wchar_t));
                  sTemp = pctemp4;
                } else
                  sTemp = wc_temp;
                break;
            }
            bDeadKey = iter_sc->second.bsDeadKey.test(index);
          }
          // Now set character on key
          SetWindowText(m_vkbb_Keys[i].m_hWnd, sTemp.c_str());
          EnableWindow(m_vkbb_Keys[i].m_hWnd, sTemp.empty() ? FALSE : TRUE);
          ShowWindow(m_vkbb_Keys[i].m_hWnd, SW_SHOW);
          m_vkbb_Keys[i].SetDeadKeyState(bDeadKey);
          // If DeadKey - add a ToolTip, if not - remove it
          if (bDeadKey) {
            AddTooltip(IDC_VKBBTN_KBD01 + i, IDS_VKDEADKEY);
          } else {
            DeleteTooltip(IDC_VKBBTN_KBD01 + i);
          }
        }
      }
    }

    // Deal with space bar
    bDeadKey = false;
    iter_sc = m_map_stSC2Char.find(0x39);
    if (iter_sc == m_map_stSC2Char.end()) {
      sTemp.clear();
    } else {
      if (index < 0) {
        wc_temp = (wchar_t)0;
        OutputDebugString(L"SetButtons: Unknown state! (3)\n");
        ASSERT(0);
      } else {
        wc_temp = iter_sc->second.wcChar[index];
        bDeadKey = iter_sc->second.bsDeadKey.test(index);
      }
      if (wc_temp == 0)
        sTemp.clear();
      else
        sTemp = wc_temp;
    }

    if (IsWindowEnabled(m_vkbb_SpaceBar.m_hWnd)) {
      SetWindowText(m_vkbb_SpaceBar.m_hWnd, sTemp.c_str());
      EnableWindow(m_vkbb_SpaceBar.m_hWnd, sTemp.empty() ? FALSE : TRUE);
      m_vkbb_SpaceBar.SetDeadKeyState(bDeadKey);
    } else {
      SetWindowText(m_vkbb_SmallSpaceBar.m_hWnd, sTemp.c_str());
      EnableWindow(m_vkbb_SmallSpaceBar.m_hWnd, sTemp.empty() ? FALSE : TRUE);
      m_vkbb_SmallSpaceBar.SetDeadKeyState(bDeadKey);
    }
  }

  InvalidateRect(m_hwndDlg, NULL, FALSE);
}

void CVKeyBoardDlg::SetDeadKeyButtons()
{
  ASSERT(m_wcDeadKey != (wchar_t)0);

  // Clear out buttons
  for (int i = 0; i < NUM_KEYS; i++) {
    SetWindowText(m_vkbb_Keys[i].m_hWnd, L"");
    EnableWindow(m_vkbb_Keys[i].m_hWnd, FALSE);
    m_vkbb_Keys[i].SetDeadKeyState(false);
  }

  // And the Space Bar which allows user to type in the DeadKey
  // in its own right
  SetWindowText(m_vkbb_SpaceBar.m_hWnd, L"");
  EnableWindow(m_vkbb_SpaceBar.m_hWnd, FALSE);
  m_vkbb_SpaceBar.SetDeadKeyState(false);

  // Now put back associated combination values
  stringT sTemp;
  Iter_MMap_DK2SCSSCC iter_mm, iter_low, iter_up;

  iter_low = m_stKBImpl.pmmapDK2SCSSCC->lower_bound(m_wcDeadKey);
  iter_up = m_stKBImpl.pmmapDK2SCSSCC->upper_bound(m_wcDeadKey);

  // Only 2 states are saved for Dead Keys - shifted or not.
  // Shifted   == Shift OR CapsLock
  // UnShifted == Base OR (Shift AND CapsLock)
  BYTE bShift = (((m_State & (VST_SHIFT | VST_CAPSLOCK)) == VST_SHIFT) ||
    ((m_State & (VST_SHIFT | VST_CAPSLOCK)) == VST_CAPSLOCK)) ? (BYTE)VST_SHIFT : (BYTE)VST_BASE;

  for (iter_mm = iter_low; iter_mm != iter_up; iter_mm++) {
    // Check we have this scancode + shiftstate. If OK, use index to update key text
    if (iter_mm->second.SS != bShift)
      continue;

    std::vector<BYTE>::iterator iter_sc = std::find(m_vsc.begin(), m_vsc.end(),
                                                    iter_mm->second.SC);
    if (iter_sc == m_vsc.end())
      continue;

    std::vector<BYTE>::size_type index = iter_sc - m_vsc.begin();
    sTemp = iter_mm->second.wcCC;

    if (m_vsc[index] == 0x39) {
      // Space Bar
      SetWindowText(m_vkbb_SpaceBar.m_hWnd, sTemp.c_str());
      EnableWindow(m_vkbb_SpaceBar.m_hWnd, TRUE);
    } else {
      // Other character
      SetWindowText(m_vkbb_Keys[index].m_hWnd, sTemp.c_str());
      EnableWindow(m_vkbb_Keys[index].m_hWnd, TRUE);
    }
  }

  InvalidateRect(m_hwndDlg, NULL, FALSE);
}

void CVKeyBoardDlg::OnClearBuffer()
{
  // Clear the character buffer
  m_phrasecount = 0;
  m_phrase = L"";

  SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), L"0");

  EnableWindow(m_vkbb_Insert.m_hWnd, FALSE);
  EnableWindow(m_vkbb_ClearBuffer.m_hWnd, FALSE);
}

void CVKeyBoardDlg::OnChangeKeyboard()
{
  int isel = SendMessage(m_hcbxKeyBoards, CB_GETCURSEL, NULL, NULL);

  if (isel == CB_ERR)
    return;

  // Get the requested layout
  UINT uiKLID = (UINT)SendMessage(m_hcbxKeyBoards, CB_GETITEMDATA, isel, NULL);
  if (uiKLID == m_uiKLID)
    return;

  // Remove old tooltips
  if (m_uiKLID == JAPANESE_KBD) {
    if (IsWindowEnabled(m_vkbb_Size.m_hWnd)) {
      DeleteTooltip(IDC_VKBBTN_SIZE);
    }
    if (IsWindowEnabled(m_vkbb_Hiragana.m_hWnd)) {
      DeleteTooltip(IDC_VKBBTN_HIRAGANA);
    }
  }

  // Now make it the current keyboard
  m_uiKLID = uiKLID;
  UINT uiToolString, uiSpecial(0);
  if (m_uiKLID == JAPANESE_KBD) {
    uiToolString = IDS_VK_SW_KANA;
  } else if (m_uiKLID == KOREAN_KBD) {
    uiToolString = IDS_VK_SW_KOREAN;
  } else {
    uiToolString = IDS_VKALTGR;
    uiSpecial = IDS_VKSTATIC_SPECIAL;
  }

  UpdateTooltipText(IDC_VKBBTN_ALTGR, uiToolString, uiSpecial);

  // Set up characters
  ProcessKeyboard(m_uiKLID);

  // Set up Buttons
  SetButtons();
}

void CVKeyBoardDlg::OnChangeKeyboardType()
{
  // Set up characters
  ProcessKeyboard(m_uiKLID, false);

  // Set up Buttons
  SetButtons();

  // Lose focus
  SetFocus(m_vkbb_Cancel.m_hWnd);
}

void CVKeyBoardDlg::GetAllKeyboardsAvailable()
{
  UINT uiKLID(0);  // Needed to start the iteration of keyboards
  UINT uiCtrlID(0);
  st_Keyboard_Layout kbl;

  // Call DLL function first with a zero Keyboard layout ID.
  // Continue until the returned ID is zero meaning end of list reached.
  while (1) {
    m_pListKBs(uiKLID, uiCtrlID);
    if (uiKLID == 0)
      break;

    // Add to the main array
    kbl.uiCtrlID = uiCtrlID;
    kbl.uiKLID = uiKLID;
    m_KBL.push_back(kbl);

    if (kbl.uiKLID == m_uiPhysKLID) {
      LoadAString(m_selectedkb, kbl.uiCtrlID);
      SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_CURRENTKBD), m_selectedkb.c_str());
    }
  };
}

void CVKeyBoardDlg::ProcessKeyboard(const UINT uiKLID, const bool bSetType)
{
  BOOL brc = m_pGetKBData(uiKLID, m_stKBImpl);
  ASSERT(brc);

  // Reset numbers and scan codes
  m_bRandom = false;
  m_vkbb_Randomize.SetPushedState(false);

  m_bAltGrChars = m_stKBImpl.stVKBD.bsValidSpecials.test(g);

  // Check to see if the special keys are needed for this keyboard.
  // Note: Japanese uses them all - but they mean different things and so
  // we must disable them here
  if (m_uiKLID == JAPANESE_KBD) {
    m_bLCtrlChars = false;
    m_bRCtrlChars = false;
  } else {
    m_bLCtrlChars = m_stKBImpl.stVKBD.bsValidSpecials.test(l);
    m_bRCtrlChars = m_stKBImpl.stVKBD.bsValidSpecials.test(r);
  }

  EnableWindow(m_vkbb_LCtrl.m_hWnd, m_bLCtrlChars ? TRUE : FALSE);
  EnableWindow(m_vkbb_AltGr.m_hWnd, m_bAltGrChars ? TRUE : FALSE);
  EnableWindow(m_vkbb_RCtrl.m_hWnd, m_bRCtrlChars ? TRUE : FALSE);

  // Determine if the shift/Caps Lock keys are meaningful here
  m_bAllow_bC = m_stKBImpl.stVKBD.bsValidSpecials.test(bC) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(sbC);
  m_bAllow_bS = m_stKBImpl.stVKBD.bsValidSpecials.test(sb) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(sbC);
  m_bAllow_lC = m_stKBImpl.stVKBD.bsValidSpecials.test(lC) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(slC);
  m_bAllow_lS = m_stKBImpl.stVKBD.bsValidSpecials.test(sl) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(slC);
  m_bAllow_gC = m_stKBImpl.stVKBD.bsValidSpecials.test(gC) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(sgC);
  m_bAllow_gS = m_stKBImpl.stVKBD.bsValidSpecials.test(sg) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(sgC);
  m_bAllow_rC = m_stKBImpl.stVKBD.bsValidSpecials.test(rC) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(srC);
  m_bAllow_rS = m_stKBImpl.stVKBD.bsValidSpecials.test(sr) ||
                m_stKBImpl.stVKBD.bsValidSpecials.test(srC);

  EnableWindow(m_vkbb_LShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
  EnableWindow(m_vkbb_RShift.m_hWnd, m_bAllow_bS ? TRUE : FALSE);
  EnableWindow(m_vkbb_CapsLock.m_hWnd, m_bAllow_bC ? TRUE : FALSE);

  // Set keyboard type radio button
  if (bSetType) {
    UINT uiButton;
    // If in our vector, then it is a 101 keyboard
    if (std::find(vk101.begin(), vk101.end(), uiKLID) != vk101.end()) {
      m_iKeyboard = 0;  // Set 101 keyboard
      uiButton = IDC_VK101;
    } else if (m_uiKLID == JAPANESE_KBD) {
      m_iKeyboard = 2;  // 106 keyboard
      uiButton = IDC_VK106;
    } else {
      m_iKeyboard = 1;  // 102 keyboard
      uiButton = IDC_VK102;
    }
    CheckRadioButton(m_hwndDlg, IDC_VK101, IDC_VK106, uiButton);

  }

  // Need to do this prior to call to ResetKeys
  bool bJapanese = (m_uiKLID == JAPANESE_KBD);
  EnableWindow(GetDlgItem(m_hParent, IDC_VK101), bJapanese ? FALSE : TRUE);
  ShowWindow(GetDlgItem(m_hParent, IDC_VK101), bJapanese ? SW_HIDE : SW_SHOW);
  EnableWindow(GetDlgItem(m_hParent, IDC_VK102), bJapanese ? FALSE : TRUE);
  ShowWindow(GetDlgItem(m_hParent, IDC_VK102), bJapanese ? SW_HIDE : SW_SHOW);
  ShowWindow(GetDlgItem(m_hParent, IDC_VK106), bJapanese ? SW_SHOW : SW_HIDE);

  // Reset Keys
  ResetKeys();

  // Set up scancodes
  m_map_stSC2Char.clear();
  for (int i = 0; i < m_stKBImpl.stVKBD.numScanCodes; i++) {
    if (std::find(m_vsc.begin(), m_vsc.end(), (int)m_stKBImpl.stVKBD.stSC2CHAR[i].SC) ==
                  m_vsc.end())
      continue;

    m_map_stSC2Char.insert(std::make_pair(m_stKBImpl.stVKBD.stSC2CHAR[i].SC,
                                           m_stKBImpl.stVKBD.stSC2CHAR[i]));
  }

  // Setup Keyboard
  switch (m_uiKLID) {
    case JAPANESE_KBD:
      // Japanese
      m_Size = m_Hiragana = m_Kana = 0;
      SetJapaneseKeyboard();
      break;
    case KOREAN_KBD:
      // Korean
      SetKoreanKeyboard();
      break;
    default:
      // All others
      SetStandardKeyboard();
  }

  // Reset all the push buttons
  m_bAltNum = m_bAltGr = m_bCapsLock = m_bRandom = m_bShift = m_bLCtrl = m_bRCtrl = false;
  m_vkbb_LCtrl.SetPushedState(false);
  m_vkbb_RCtrl.SetPushedState(false);
  m_vkbb_RHCtrl.SetPushedState(false);
  m_vkbb_LShift.SetPushedState(false);
  m_vkbb_RShift.SetPushedState(false);
  m_vkbb_Alt.SetPushedState(false);
  m_vkbb_AltGr.SetPushedState(false);
  m_vkbb_AltNum.SetPushedState(false);
  m_vkbb_CapsLock.SetPushedState(false);

  m_State = 0;
}

void CVKeyBoardDlg::ResetKeyboard()
{
  m_phrasecount = 0;
  m_phrase = L"";
  SetDeadKeyEnvironment(false);
  m_wcDeadKey = (wchar_t)0;
  SetButtons();

  SetWindowText(GetDlgItem(m_hwndDlg, IDC_VKSTATIC_COUNT), L"0");

  EnableWindow(m_vkbb_Insert.m_hWnd, FALSE);
  EnableWindow(m_vkbb_ClearBuffer.m_hWnd, FALSE);
}

void CVKeyBoardDlg::ResetKeys()
{
  // Put back the default scancodes on each key
  BYTE *psc;
  switch (m_iKeyboard) {
    default:
    case 0:
      psc = (BYTE *)&defscancodes101[0];
      break;
    case 1:
      psc = (BYTE *)&defscancodes102[0];
      break;
    case 2:
      psc = (BYTE *)&defscancodes106[0];
      break;
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    m_scancodes[i] = *psc;
    psc++;
  }

  // Reset the NumPad numbers
  for (int i = 0; i < NUM_DIGITS; i++) {
    free(m_pnumbers[i]);
    m_pnumbers[i] = _wcsdup(pXdefnumbers[i]);
  }

  // Make normal
  SetDeadKeyEnvironment(false);
  m_wcDeadKey = (wchar_t)0;

  // Reset table of valid scancodes - order IS important!
  m_vsc.clear();
  for (int i = 0; i < NUM_KEYS; i++) {
    m_vsc.push_back(m_scancodes[i]);
  }
  m_vsc.push_back(0x39); // Add on the end the Spacebar
}

void CVKeyBoardDlg::SetDeadKeyEnvironment(const bool bState)
{
  m_bDeadKeyActive = bState;

  BOOL bEnable = m_bDeadKeyActive ? FALSE : TRUE;

  // If Dead Key active - need to disable some buttons:
  for (int i = 0; i < NUM_DIGITS; i++) {
    EnableWindow(m_vkbb_Numbers[i].m_hWnd, bEnable);
  }

  // Save current state and reset it until the user presses the DeadKey combination character
  if (m_bDeadKeyActive) {
    m_SaveState = m_State;

    m_bSaveShift = m_bShift;
    m_bSaveLCtrl = m_bLCtrl;
    m_bSaveRCtrl = m_bRCtrl;
    m_bSaveAltGr = m_bAltGr;
    m_bSaveCapsLock = m_bCapsLock;

    m_bShift = m_bLCtrl = m_bRCtrl = m_bAltGr = m_bCapsLock = false;

    m_State |= VST_MENU;
    m_State &= ~(VST_SHIFT | VST_LCTRL | VST_ALTGR | VST_RCTRL | VST_CAPSLOCK);
  } else {
    m_State = m_SaveState;

    m_bShift = m_bSaveShift;
    m_bLCtrl = m_bSaveLCtrl;
    m_bRCtrl = m_bSaveRCtrl;
    m_bAltGr = m_bSaveAltGr;
    m_bCapsLock = m_bSaveCapsLock;
  }

  EnableWindow(m_hcbxKeyBoards, bEnable);
  EnableWindow(m_vkbb_Randomize.m_hWnd, bEnable);
  EnableWindow(m_vkbb_Insert.m_hWnd, bEnable);
  EnableWindow(m_vkbb_ClearBuffer.m_hWnd, bEnable);

  if (m_bLCtrlChars)
    EnableWindow(m_vkbb_LCtrl.m_hWnd, m_bLCtrl ? TRUE : FALSE);
  if (m_bRCtrlChars)
    EnableWindow(m_vkbb_RCtrl.m_hWnd, m_bRCtrl ? TRUE : FALSE);
  if (m_bAltGrChars)
    EnableWindow(m_vkbb_AltGr.m_hWnd, m_bAltGr ? TRUE : FALSE);

  EnableWindow(m_vkbb_AltNum.m_hWnd, bEnable);
  EnableWindow(GetDlgItem(m_hParent, IDC_VK101), bEnable);
  EnableWindow(GetDlgItem(m_hParent, IDC_VK102), bEnable);
}

void CVKeyBoardDlg::SetJapaneseKeyboard()
{
  bool bHiragana;
  stringT sKana, sHK, sSize;

  if (m_Kana == ENGLISH) {
    bHiragana = false;
    sKana = L"Eng";
    sHK = L"";
    sSize = L"";
  } else {
    bHiragana = (m_Hiragana == HIRAGANA);
    sKana = L"Kana";
    sHK = bHiragana ? wcHiragana : wcKatakana;
    sSize = m_Size == HALF ? wcHalfWidth : wcFullWidth;
  }

  SetWindowText(m_vkbb_Hiragana.m_hWnd, sHK.c_str());
  SetWindowText(m_vkbb_Size.m_hWnd, sSize.c_str());

  // Set Japanese (Eng to start with) label and stop it showing pushed state
  SetWindowText(m_vkbb_AltGr.m_hWnd, sKana.c_str());
  EnableWindow(m_vkbb_AltGr.m_hWnd, TRUE);
  m_vkbb_AltGr.ChangePushColour(false);

  SetSpecialKeys();
}

void CVKeyBoardDlg::SetKoreanKeyboard()
{
  // Set Korean (Eng to start with) label and stop it showing pushed state
  SetWindowText(m_vkbb_AltGr.m_hWnd, L"Eng");
  EnableWindow(m_vkbb_AltGr.m_hWnd, TRUE);
  m_vkbb_AltGr.ChangePushColour(false);

  SetSpecialKeys();
}

void CVKeyBoardDlg::SetStandardKeyboard()
{
  // Put back standard label and make it show pushed state
  SetWindowText(m_vkbb_AltGr.m_hWnd, L"Alt Gr");
  m_vkbb_AltGr.ChangePushColour(true);

  SetSpecialKeys();
}

void CVKeyBoardDlg::SetSpecialKeys()
{
  /*
    Japanese Keyboard:
    1. English = Normal Space Bar, Full R Ctrl (should have Size - but not needed)
    2. Kana + Hiragana = Small Space Bar, Hiragana, Full R Ctrl
    3. Kana + Katakana = Small Space Bar, Katakana, Size, Half R Ctrl
  */
  bool bEnableSpaceBar(true), bShowRCtrl(true), bEnableHK(false);
  if (m_uiKLID == JAPANESE_KBD && m_Kana == JAPANESE) {
    bEnableSpaceBar = false;
    bShowRCtrl = m_Hiragana == HIRAGANA;
    bEnableHK = true;
  }
  if (m_uiKLID == JAPANESE_KBD) {
    EnableWindow(m_vkbb_LCtrl.m_hWnd, FALSE);
    EnableWindow(m_vkbb_RCtrl.m_hWnd, FALSE);
  }

  EnableWindow(m_vkbb_SpaceBar.m_hWnd, bEnableSpaceBar ? TRUE : FALSE);
  ShowWindow(m_vkbb_SpaceBar.m_hWnd, bEnableSpaceBar ? SW_SHOW : SW_HIDE);

  EnableWindow(m_vkbb_SmallSpaceBar.m_hWnd, bEnableSpaceBar ? FALSE : TRUE);
  ShowWindow(m_vkbb_SmallSpaceBar.m_hWnd, bEnableSpaceBar ? SW_HIDE : SW_SHOW);

  ShowWindow(m_vkbb_RCtrl.m_hWnd, bShowRCtrl ? SW_SHOW : SW_HIDE);
  ShowWindow(m_vkbb_RHCtrl.m_hWnd, bShowRCtrl ? SW_HIDE : SW_SHOW);

  EnableWindow(m_vkbb_Size.m_hWnd, bShowRCtrl ? FALSE : TRUE);
  ShowWindow(m_vkbb_Size.m_hWnd, bShowRCtrl ? SW_HIDE : SW_SHOW);
  SetWindowText(m_vkbb_Size.m_hWnd, wcHalfWidth);

  EnableWindow(m_vkbb_Hiragana.m_hWnd, bEnableHK ? TRUE : FALSE);
  ShowWindow(m_vkbb_Hiragana.m_hWnd, bEnableHK ? SW_SHOW : SW_HIDE);
  SetWindowText(m_vkbb_Hiragana.m_hWnd, wcHiragana);

  InvalidateRect(m_hwndDlg, NULL, FALSE);
}

void CVKeyBoardDlg::ApplyUnicodeFont(HWND hDlgItem)
{
  ASSERT(hDlgItem != NULL);
  if (hDlgItem == NULL)
    return;

  if (m_PassphraseFont == NULL) {
    wchar_t *pszFont(NULL);
    stringT sVKeyboardFont = L"";
    switch (m_iFont) {
      case USER_FONT:
        pszFont = (wchar_t *)sVKeyboardFont.c_str();
        break;
      case ARIALMS_FONT:
        pszFont = ARIALUMS;
        break;
      case ARIAL_FONT:
        pszFont = ARIALU;
        break;
      case LUCIDA_FONT:
        pszFont = LUCIDAUS;
        break;
      default:
        ASSERT(0);
    }

    // Note these font names are less than the max. permitted length
    // (LF_FACESIZE = 31 + null). No need to check length before copy.

    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    LOGFONT lf;
    SecureZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -16;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    wcsncpy_s(lf.lfFaceName, LF_FACESIZE, pszFont, wcslen(pszFont));

    m_PassphraseFont = CreateFontIndirect(&lf);
  }

  SendMessage(hDlgItem, WM_SETFONT, (WPARAM)m_PassphraseFont, TRUE);
}

void CVKeyBoardDlg::OnSaveKLID()
{
  m_bSaveKLID = SendMessage(GetDlgItem(m_hParent, IDC_SAVEKLID), BM_GETCHECK, NULL, NULL);
}

// Modified from MSDN: http://msdn.microsoft.com/en-us/library/bb760252(v=vs.85).aspx

BOOL CVKeyBoardDlg::AddTooltip(UINT uiControlID,stringT sText)
{
  if (!uiControlID || sText.empty())
    return FALSE;

  // Get the window of the tool.
  HWND hwndTool = GetDlgItem(m_hwndDlg, uiControlID);

  // Associate the tooltip with the tool.
  TOOLINFO ti = { 0 };
  ti.cbSize = sizeof(ti);
  ti.hwnd = m_hwndDlg;
  ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_CENTERTIP | TTF_TRANSPARENT;
  ti.uId = (UINT_PTR)hwndTool;
  ti.lpszText = (LPWSTR)sText.c_str();

  BOOL brc = SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);

  return brc;
}

BOOL CVKeyBoardDlg::AddTooltip(UINT uiControlID, UINT uiToolString, UINT uiFormat)
{
  if (!uiControlID || !uiToolString)
    return FALSE;

  stringT sText;
  LoadAString(sText, uiToolString);
  if (sText.empty())
    return FALSE;

  if (uiFormat != NULL)
  {
    Format(sText, uiFormat, sText.c_str());
  }

  return AddTooltip(uiControlID, sText);
}

BOOL CVKeyBoardDlg::DeleteTooltip(UINT uiControlID)
{
  if (!uiControlID)
    return FALSE;

  // Get the window of the tool.
  HWND hwndTool = GetDlgItem(m_hwndDlg, uiControlID);

  // Delete the tooltip.
  TOOLINFO ti = { 0 };
  ti.cbSize = sizeof(ti);
  ti.hwnd = m_hwndDlg;
  ti.uId = (UINT_PTR)hwndTool;

  BOOL brc = SendMessage(m_hwndTooltip, TTM_DELTOOL, 0, (LPARAM)&ti);

  return brc;
}

BOOL CVKeyBoardDlg::UpdateTooltipText(UINT uiControlID,stringT sText)
{
  if (!uiControlID || sText.empty())
    return FALSE;

  // Get the window of the tool.
  HWND hwndTool = GetDlgItem(m_hwndDlg, uiControlID);

  // Delete the tooltip.
  TOOLINFO ti = { 0 };
  ti.cbSize = sizeof(ti);
  ti.hinst = m_hInstance;
  ti.hwnd = m_hwndDlg;
  ti.uId = (UINT_PTR)hwndTool;
  ti.lpszText = (LPWSTR)sText.c_str();

  BOOL brc = SendMessage(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

  return brc;
}

BOOL CVKeyBoardDlg::UpdateTooltipText(UINT uiControlID, UINT uiToolString, UINT uiFormat)
{
  if (!uiControlID || !uiToolString)
    return FALSE;

  stringT sText;
  LoadAString(sText, uiToolString);
  if (sText.empty())
    return FALSE;

  if (uiFormat != NULL)
  {
    Format(sText, uiFormat, sText.c_str());
  }

  return UpdateTooltipText(uiControlID, sText);
}
