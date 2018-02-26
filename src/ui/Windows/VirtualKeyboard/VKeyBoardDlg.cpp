/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// It is Unicode ONLY.

/// \file VKeyBoard.cpp
//-----------------------------------------------------------------------------

#include "../stdafx.h"

#include "VKeyBoardDlg.h"
#include "VKShiftState.h"
#include "VKresource.h"
#include "VKresource3.h"

#include "../Windowsdefs.h"
#include "../PasswordSafe.h" // for app extern declaration
#include "../ThisMfcApp.h" // for NoSysEnvWarnings()
#include "../GeneralMsgBox.h"

#include "../../../os/dir.h"
#include "../../../os/lib.h"
#include "../../../os/windows/pws_osk/pws_osk.h"
#include "../../../core/PWSrand.h"
#include "../../../core/PWSprefs.h"

#include <sstream>
#include <iomanip>  // For setbase and setw
#include <algorithm>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

const wchar_t *pdefnumbers[] = {
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
  HINSTANCE OSK_module = HINSTANCE(pws_os::LoadLibrary(dll_name, pws_os::loadLibraryTypes::APP));

  if (OSK_module == NULL) {
    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Unable to load OSK DLL. OSK not available.\n");
    return false;
  } else {
    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - OSK DLL loaded OK.\n");

    LP_OSK_GetKeyboardData pGetKBData =
      LP_OSK_GetKeyboardData(pws_os::GetFunction(OSK_module, "OSK_GetKeyboardData"));
    LP_OSK_ListKeyboards pListKBs =
      LP_OSK_ListKeyboards(pws_os::GetFunction(OSK_module, "OSK_ListKeyboards"));
    LP_OSK_GetVersion pOSKVersion =
      LP_OSK_GetVersion(pws_os::GetFunction(OSK_module, "OSK_GetVersion"));

    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Found OSK_GetVersion: %s\n",
                  pOSKVersion != NULL ? L"OK" : L"FAILED");
    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Found OSK_ListKeyboards: %s\n",
                  pListKBs != NULL ? L"OK" : L"FAILED");
    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Found OSK_GetKeyboardData: %s\n",
                  pGetKBData != NULL ? L"OK" : L"FAILED");

    if (pListKBs == NULL || pGetKBData == NULL || pOSKVersion == NULL)
      pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Unable to get all required OSK functions. OSK not available.\n");
    else if (pOSKVersion() == VK_DLL_VERSION) {
      bVKAvailable = true;
    } else if (!warnedAlready && !app.NoSysEnvWarnings()) {
      CGeneralMsgBox gmb;
      warnedAlready = true;
      gmb.AfxMessageBox(IDS_OSK_VERSION_MISMATCH, MB_ICONERROR);
    }

    BOOL brc = pws_os::FreeLibrary(OSK_module);
    pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - Free OSK DLL: %s\n",
                  brc == TRUE ? L"OK" : L"FAILED");
  }

  if (!bVKAvailable)
    return false;

  // We have the DLL, now check Unicode font installed
  bool bFound(false);
  LOGFONT lf = {0, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0,
                L""};

  HDC hDC = ::GetDC(NULL);

  // First check user's font (if any)
  StringX cs_VKeyboardFont = PWSprefs::GetInstance()->
                                 GetPref(PWSprefs::VKeyboardFontName);
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

  pws_os::Trace(L"CVKeyBoardDlg::IsOSKAvailable - No Unicode font installed. OSK not available.\n");
  if (!warnedAlready && !app.NoSysEnvWarnings()) {
    CGeneralMsgBox gmb;
    warnedAlready = true;
    gmb.AfxMessageBox(IDS_OSK_NO_UNICODE_FONT, MB_ICONERROR);
  }

exit:
  ::ReleaseDC(NULL, hDC);
  return bFound;
}

//-----------------------------------------------------------------------------
CVKeyBoardDlg::CVKeyBoardDlg(CWnd* pParent, LPCWSTR wcKLID)
  : CPWDialog(CVKeyBoardDlg::IDD, pParent), m_pParent(pParent),
    m_pToolTipCtrl(NULL), m_pPassphraseFont(NULL),
    m_phrase(L""), m_phrasecount(0), m_State(0), m_SaveState(0),
    m_bShift(false), m_bLCtrl(false), m_bRCtrl(false),
    m_bAltGr(false), m_bAltNum(false),
    m_bCapsLock(false), m_bRandom(false),
    m_bLCtrlChars(false), m_bAltGrChars(false), m_bRCtrlChars(false),
    m_bDeadKeyActive(false), m_bDeadKeySaved(false),
    m_iKeyboard(0), m_Kana(0), m_Hiragana(0), m_Size(0),
    m_uiMouseDblClkTime(0), m_bSaveKLID(BST_CHECKED), m_bPlaySound(BST_UNCHECKED),
    m_bShowPassphrase(BST_UNCHECKED)
{
  // Verify all is OK
  ASSERT(_countof(defscancodes101) == NUM_KEYS);
  ASSERT(_countof(defscancodes102) == NUM_KEYS);
  ASSERT(_countof(defscancodes106) == NUM_KEYS);
  ASSERT(_countof(pdefnumbers) == NUM_DIGITS);

  // Initialise numbers
  for (int i = 0; i < NUM_DIGITS; i++)
    m_pnumbers[i] = NULL;

  // Set background colour for for dialog as white
  m_pBkBrush.CreateSolidBrush(RGB(255, 255, 255));

  // dll is guaranteed to be loadable, right version and in general 100% kosher
  // by IsOSKAvailable(). Caller is responsible to call that, though...
#if defined(_DEBUG) || defined(DEBUG)
  wchar_t *dll_name = L"pws_osk_D.dll";
#else
  wchar_t *dll_name = L"pws_osk.dll";
#endif
  m_OSK_module = HMODULE(pws_os::LoadLibrary(dll_name, pws_os::loadLibraryTypes::APP));

  ASSERT(m_OSK_module != NULL);
  m_pGetKBData = LP_OSK_GetKeyboardData(pws_os::GetFunction(m_OSK_module,
                                                            "OSK_GetKeyboardData"));
  m_pListKBs   = LP_OSK_ListKeyboards(pws_os::GetFunction(m_OSK_module,
                                                          "OSK_ListKeyboards"));

  m_uiKLID = 0;
  if (wcKLID != NULL) {
    const wchar_t *wc_hex = L"0123456789ABCDEFabcdef";
    std::wstring sKLID(wcKLID);
    size_t non_hex = sKLID.find_first_not_of(wc_hex);

    // Make sure it is 8 hex characters and convert to a UINT
    if (sKLID.length() == 8 && non_hex == std::wstring::npos) {
      std::wstring s(L"0x");
      s += sKLID;
      std::wistringstream iss(s);
      iss >> std::setbase(0) >> m_uiKLID;
    }
  }
  // Get current SYSTEM-WIDE mouse double click time interval
  m_uiMouseDblClkTime = GetDoubleClickTime();

#ifdef _DEBUG
  m_displayedphrase = L"";  // Used for testing only!
#endif
}

CVKeyBoardDlg::~CVKeyBoardDlg()
{
  // Free the number values
  for (int i = 0; i < NUM_DIGITS; i++) {
    free(m_pnumbers[i]);
    m_pnumbers[i] = NULL;
  }

  // Delete the passphrase font
  if (m_pPassphraseFont != NULL) {
    m_pPassphraseFont->DeleteObject();
    delete m_pPassphraseFont;
    m_pPassphraseFont = NULL;
  }

  delete m_pToolTipCtrl;

  pws_os::FreeLibrary(m_OSK_module);

  // Reset double click mouse interval
  VERIFY(SetDoubleClickTime(m_uiMouseDblClkTime));
}

void CVKeyBoardDlg::OnPostNcDestroy()
{
  delete this;
}

void CVKeyBoardDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CVKeyBoardDlg)
  DDX_Text(pDX, IDC_VKSTATIC_COUNT, m_phrasecount);
  DDX_Text(pDX, IDC_VKSTATIC_CURRENTKBD, m_selectedkb);

  DDX_Control(pDX, IDC_VKEYBOARDS, m_cbxKeyBoards);
  DDX_Radio(pDX, IDC_VK101, m_iKeyboard); // only first!
  DDX_Control(pDX, IDC_VKRANDOMIZE, m_vkbb_Randomize);
  DDX_Control(pDX, IDC_VKCANCEL, m_vkbb_Cancel);
  DDX_Control(pDX, IDC_VKINSERT, m_vkbb_Insert);
  DDX_Control(pDX, IDC_VKCLEARBUFFER, m_vkbb_ClearBuffer);
  DDX_Control(pDX, IDC_VKBACKSPACE, m_vkbb_BackSpace);
  DDX_Control(pDX, IDC_VKBBTN_LSHIFT, m_vkbb_LShift);
  DDX_Control(pDX, IDC_VKBBTN_RSHIFT, m_vkbb_RShift);
  DDX_Control(pDX, IDC_VKBBTN_LCTRL, m_vkbb_LCtrl);
  DDX_Control(pDX, IDC_VKBBTN_RCTRL, m_vkbb_RCtrl);
  DDX_Control(pDX, IDC_VKBBTN_RHCTRL, m_vkbb_RHCtrl);
  DDX_Control(pDX, IDC_VKBBTN_ALT, m_vkbb_Alt); // NOTE: This key has no function
  DDX_Control(pDX, IDC_VKBBTN_ALTGR, m_vkbb_AltGr);
  DDX_Control(pDX, IDC_VKBBTN_ALTNUM, m_vkbb_AltNum);
  DDX_Control(pDX, IDC_VKBBTN_CAPSLOCK, m_vkbb_CapsLock);

  DDX_Control(pDX, IDC_VKBBTN_SPACEBAR, m_vkbb_SpaceBar);

  // 106 keyboard
  DDX_Control(pDX, IDC_VKBBTN_SMALLSPACEBAR, m_vkbb_SmallSpaceBar);
  DDX_Control(pDX, IDC_VKBBTN_SIZE, m_vkbb_Size);
  DDX_Control(pDX, IDC_VKBBTN_HIRAGANA, m_vkbb_Hiragana);

  // Save last used keyboard
  DDX_Check(pDX, IDC_SAVEKLID, m_bSaveKLID);

  // Play sound on key press
  DDX_Check(pDX, IDC_KEYPRESS_PLAYSOUND, m_bPlaySound);

#ifdef _DEBUG
  // Show passphrase IDC_SHOWBUFFER - Used for testing only!
  DDX_Check(pDX, IDC_SHOWBUFFER, m_bShowPassphrase);
  DDX_Text(pDX, IDC_STATIC_VKPASSPHRASE, m_displayedphrase);
#endif
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVKeyBoardDlg, CPWDialog)
  //{{AFX_MSG_MAP(CVKeyBoardDlg)
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDOWN()
  ON_WM_ACTIVATE()
  ON_CBN_SELCHANGE(IDC_VKEYBOARDS, OnChangeKeyboard)
  ON_BN_CLICKED(IDC_VK101, OnChangeKeyboardType)
  ON_BN_CLICKED(IDC_VK102, OnChangeKeyboardType)
  ON_BN_CLICKED(IDC_VKCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_VKINSERT, OnInsert)
  ON_BN_CLICKED(IDC_VKCLEARBUFFER, OnClearBuffer)
  ON_BN_CLICKED(IDC_VKRANDOMIZE, OnRandomize)
  ON_BN_CLICKED(IDC_VKBACKSPACE, OnBackSpace)
  ON_BN_CLICKED(IDC_VKBBTN_LSHIFT, OnShift)
  ON_BN_CLICKED(IDC_VKBBTN_RSHIFT, OnShift)
  ON_BN_CLICKED(IDC_VKBBTN_LCTRL, OnLCtrl)
  ON_BN_CLICKED(IDC_VKBBTN_RCTRL, OnRCtrl)
  ON_BN_CLICKED(IDC_VKBBTN_RHCTRL, OnRHCtrl)
  ON_BN_CLICKED(IDC_VKBBTN_ALTGR, OnAltGr)
  ON_BN_CLICKED(IDC_VKBBTN_ALTNUM, OnAltNum)
  ON_BN_CLICKED(IDC_VKBBTN_CAPSLOCK, OnCapsLock)
  ON_BN_CLICKED(IDC_VKBBTN_SPACEBAR, OnSpaceBar)
  ON_BN_CLICKED(IDC_VKBBTN_SMALLSPACEBAR, OnSpaceBar)
  ON_BN_CLICKED(IDC_VKBBTN_SIZE, OnKeySize)
  ON_BN_CLICKED(IDC_VKBBTN_HIRAGANA, OnHiragana)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_VKBBTN_N0, IDC_VKBBTN_N9, OnNumerics)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_VKBBTN_KBD01, IDC_VKBBTN_KBD51, OnKeys)
  ON_BN_CLICKED(IDC_SAVEKLID, OnSaveKLID)
  ON_BN_CLICKED(IDC_KEYPRESS_PLAYSOUND, OnKeyPressPlaySound)
#ifdef _DEBUG
  ON_BN_CLICKED(IDC_SHOWBUFFER, OnShowPassphrase)  // Used for testing only!
#endif
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CVKeyBoardDlg::OnActivate(UINT nState, CWnd* , BOOL )
{
  if (nState == WA_INACTIVE) {
    VERIFY(SetDoubleClickTime(m_uiMouseDblClkTime));
  } else {
    VERIFY(SetDoubleClickTime(1));
  }
}

BOOL CVKeyBoardDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Set user's preference re\: sound
  m_bPlaySound = PWSprefs::GetInstance()->GetPref(PWSprefs::VKPlaySound) ? BST_CHECKED : BST_UNCHECKED;

#ifdef _DEBUG
  // allow developer to view passphrase
  GetDlgItem(IDC_SHOWBUFFER)->ShowWindow(SW_SHOW);
  GetDlgItem(IDC_SHOWBUFFER)->EnableWindow(TRUE);
  // Hide passphrase initially
  GetDlgItem(IDC_STATIC_VKPASSPHRASE)->ShowWindow(SW_HIDE);
#else
  // Hide & disable option to show passphrase in Release build
  GetDlgItem(IDC_SHOWBUFFER)->ShowWindow(SW_HIDE);
  GetDlgItem(IDC_SHOWBUFFER)->EnableWindow(FALSE);
  GetDlgItem(IDC_STATIC_VKPASSPHRASE)->ShowWindow(SW_HIDE);
#endif

  // Subclass the buttons - default is a 'flat' button
  for (int i = 0; i < NUM_DIGITS; i++) {
    m_vkbb_Numbers[i].SubclassDlgItem(IDC_VKBBTN_N0 + i, this);
    m_vkbb_Numbers[i].ModifyStyle(0, BS_CENTER | BS_VCENTER);
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    m_vkbb_Keys[i].SubclassDlgItem(IDC_VKBBTN_KBD01 + i, this);
    m_vkbb_Keys[i].ModifyStyle(0, BS_CENTER | BS_VCENTER);
  }

  // Remove flat style from 'real' buttons
  m_vkbb_Randomize.SetFlatState(false);
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

  // Make Japanese button push style but not to change colour when pushed
  m_vkbb_Size.SetFlatState(false);
  m_vkbb_Size.ChangePushColour(false);
  m_vkbb_Hiragana.SetFlatState(false);
  m_vkbb_Hiragana.ChangePushColour(false);

  // Alt, Left & Right-Half Control always disabled, as is the 106-key radio button
  m_vkbb_Alt.EnableWindow(FALSE);
  m_vkbb_LCtrl.EnableWindow(FALSE);
  m_vkbb_RHCtrl.EnableWindow(FALSE);
  GetDlgItem(IDC_VK106)->EnableWindow(FALSE);

  // Initially nothing to reset
  m_vkbb_Insert.EnableWindow(FALSE);
  m_vkbb_ClearBuffer.EnableWindow(FALSE);

  if (m_cbxKeyBoards.GetCount() == 0) {
    // Get current Keyboard layout name
    wchar_t wcKLID[KL_NAMELENGTH  + 1];
    VERIFY(GetKeyboardLayoutName(wcKLID));

    // Convert from hex string to integer
    std::wstring s(L"0x");
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
      CString cs_temp(MAKEINTRESOURCE(st_kbl.uiCtrlID));
      int iItem = m_cbxKeyBoards.AddString(cs_temp);
      m_cbxKeyBoards.SetItemData(iItem, (DWORD)st_kbl.uiKLID);
    }
  }

  int cbx_index(0);
  if (m_uiKLID != 0) {
    // Select last used - but first find it, as ComboBox is sorted by name
    for (int i = 0; i < m_cbxKeyBoards.GetCount(); i++) {
      if ((UINT)m_cbxKeyBoards.GetItemData(i) == m_uiKLID) {
        cbx_index = i;
        break;
      }
    }
  } else {
    m_uiKLID = m_uiPhysKLID;
  }

  m_cbxKeyBoards.SetCurSel(cbx_index);

  if (m_uiKLID == JAPANESE_KBD) {
    SetJapaneseKeyboard();
  } else if (m_uiKLID == KOREAN_KBD) {
    SetKoreanKeyboard();
  } else {
    SetStandardKeyboard();
  }

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create Advanced Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return TRUE;
  }

  // Tooltips
  EnableToolTips();

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);
  // Quadruple the time to allow reading by user
  int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

  // Set the tooltip
  // Note naming convention: string IDS_VKxxx corresponds to control IDC_xxx
  CString cs_ToolTip, cs_temp;
  cs_ToolTip.LoadString(IDS_VKCLEARBUFFER);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKCLEARBUFFER), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_VKSTATIC_CAPSLOCK);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_CAPSLOCK), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_VKSTATIC_SHIFT);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_LSHIFT), cs_ToolTip);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_RSHIFT), cs_ToolTip);
  cs_temp.LoadString(IDS_VKLCTRL);
  cs_ToolTip.Format(IDS_VKSTATIC_SPECIAL, static_cast<LPCWSTR>(cs_temp));
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_LCTRL), cs_ToolTip);
  cs_temp.LoadString(IDS_VKRCTRL);
  cs_ToolTip.Format(IDS_VKSTATIC_SPECIAL, static_cast<LPCWSTR>(cs_temp));
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_RCTRL), cs_ToolTip);
  cs_temp.LoadString(IDS_VKALTGR);
  cs_ToolTip.Format(IDS_VKSTATIC_SPECIAL, static_cast<LPCWSTR>(cs_temp));
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_ALTGR), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_VKSTATIC_ALT);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_ALT), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_VKSTATIC_ALTNUM);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKBBTN_ALTNUM), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_VKSTATIC_RANDOMIZE);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_VKRANDOMIZE), cs_ToolTip);

  // If not using the user specified font, show the warning.
  if (m_iFont != USER_FONT && m_bUserSpecifiedFont) {
    StringX cs_VKeyboardFont = PWSprefs::GetInstance()->
                                 GetPref(PWSprefs::VKeyboardFontName);
    GetDlgItem(IDC_INFO)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_INFO)->EnableWindow(TRUE);
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
      cs_ToolTip.Format(IDS_USRFONT, static_cast<LPCWSTR>(cs_VKeyboardFont.c_str()), pszFont);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_INFO), cs_ToolTip);
    }
  } else
  if (m_iFont == LUCIDA_FONT) {
    GetDlgItem(IDC_INFO)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_INFO)->EnableWindow(TRUE);
    cs_ToolTip.LoadString(IDS_OSKFONT);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_INFO), cs_ToolTip);
  }

  // Set up characters
  ProcessKeyboard(m_uiKLID);

  // Show them
  SetButtons();

  // Apply Uincode font to all that need it
  for (int i = 0; i < NUM_DIGITS; i++) {
    ApplyUnicodeFont(GetDlgItem(IDC_VKBBTN_N0 + i));
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    ApplyUnicodeFont(GetDlgItem(IDC_VKBBTN_KBD01 + i));
  }

  // Make sure we can see the Japanese charatcers - if needed
  ApplyUnicodeFont(GetDlgItem(IDC_VKBBTN_HIRAGANA));
  ApplyUnicodeFont(GetDlgItem(IDC_VKBBTN_SIZE));

#ifdef _DEBUG
  // And the passphrase if the user selects to view it
  ApplyUnicodeFont(GetDlgItem(IDC_STATIC_VKPASSPHRASE));
#endif

  if (m_phrasecount > 0) {
    m_vkbb_Insert.EnableWindow(TRUE);
    m_vkbb_ClearBuffer.EnableWindow(TRUE);
  } else {
    m_vkbb_BackSpace.EnableWindow(FALSE);
  }

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CVKeyBoardDlg::OnInsert()
{
  if (!UpdateData(TRUE))
    return;

  if (m_bAltNum)
    OnAltNum();

  UpdateData(FALSE);

  // Return OK - modeless type
  ShowWindow(SW_HIDE);

  // Send data to caller
  m_pParent->SendMessage(PWS_MSG_INSERTBUFFER,0, 0);

  // Clear buffer!
  // Must make the user type the phrase in twice to confirm what they have
  // typed rather than just using the same (possibly incorrect) value.
  OnClearBuffer();
}

void CVKeyBoardDlg::OnCancel()
{
  // If pressed when a Dead Key is active, just cancel this
  if (m_bDeadKeyActive) {
    SetDeadKeyEnvironment(false);
    SetButtons();
    return;
  }

  // Cancel dialog - modeless - just hide
  ShowWindow(SW_HIDE);
}

void CVKeyBoardDlg::OnNumerics(UINT nID)
{
  if (m_bAltNum) {
    // Alt + Numeric pad
    m_altchar = (m_altchar * 10) + (nID - IDC_VKBBTN_N0);
  } else {
    CString cs_wtxt;
    m_vkbb_Numbers[nID - IDC_VKBBTN_N0].GetWindowText(cs_wtxt);
    m_phrase += CSecString(cs_wtxt);
#ifdef _DEBUG
    m_displayedphrase += CSecString(cs_wtxt);  // Used for testing only!
#endif
    m_phrasecount++;
    if (m_phrasecount == 1) {
      m_vkbb_Insert.EnableWindow(TRUE);
      m_vkbb_ClearBuffer.EnableWindow(TRUE);
    }
    m_vkbb_BackSpace.EnableWindow(TRUE);

    UpdateData(FALSE);
  }
}

void CVKeyBoardDlg::OnKeys(UINT nID)
{
  Iter_Map_st_SC2CHAR iter_sc;
  bool bDeadKeyPressed(false);
  CString cs_wtxt;

  // Get character
  m_vkbb_Keys[nID - IDC_VKBBTN_KBD01].GetWindowText(cs_wtxt);

  // We don't support double deadkeys.  So if already active,
  // don't bother checking for another.
  if (!m_bDeadKeyActive) {
    iter_sc = m_map_stSC2Char.find(m_scancodes[nID - IDC_VKBBTN_KBD01]);
    if (iter_sc != m_map_stSC2Char.end()) {
      if (state2index[m_State] < 0) {
        pws_os::Trace(L"OnKeys; Unknown state!");
        ASSERT(0);
      }
      bDeadKeyPressed = iter_sc->second.bsDeadKey.test(state2index[m_State]);
    } else {
      pws_os::Trace(L"OnKeys: Unknown scancode pressed!");
      ASSERT(0);
    }
  }

  if (bDeadKeyPressed) {
  // Get the DeadKey and setup the DeadKey environment and combination keys
    wchar_t wc_temp;
    LPCWSTR lpwtxt = cs_wtxt.GetBuffer(2);
    wc_temp = lpwtxt[0];
    cs_wtxt.ReleaseBuffer();

    Iter_MMap_DK2SCSSCC iter_DK2SCSSCC = m_stKBImpl.pmmapDK2SCSSCC->find(wc_temp);
    if (iter_DK2SCSSCC == m_stKBImpl.pmmapDK2SCSSCC->end()) {
      pws_os::Trace(L"OnKeys; Unknown deadkey pressed!");
      ASSERT(0);
    } else {
      m_wcDeadKey = wc_temp;
      SetDeadKeyButtons();
      SetDeadKeyEnvironment(true);
    }
  } else {
    // Not a DeadKey
    m_phrase += CSecString(cs_wtxt);
#ifdef _DEBUG
    m_displayedphrase += CSecString(cs_wtxt);  // Used for testing only!
#endif
    m_phrasecount++;
    if (m_phrasecount == 1) {
      m_vkbb_Insert.EnableWindow(TRUE);
      m_vkbb_ClearBuffer.EnableWindow(TRUE);
    }
    m_vkbb_BackSpace.EnableWindow(TRUE);

    bool bNeedToSetButtons(false);
    if (m_bDeadKeyActive) {
      // Was a deadkey combination character - reset back to normal keys
      m_wcDeadKey= (wchar_t)0;
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

  UpdateData(FALSE);
}

void CVKeyBoardDlg::OnSpaceBar()
{
  // SpaceBar key - if a DeadKey is active, this adds the original character
  CString cs_wtxt;
  m_vkbb_SpaceBar.GetWindowText(cs_wtxt);
  if (cs_wtxt.GetLength() > 1)
    cs_wtxt = L" ";
  m_phrase += CSecString(cs_wtxt);
#ifdef _DEBUG
  // Left seven eighths block for a space
  // (not complete block so that multiple spaces can be differentiated)
  m_displayedphrase += CSecString(L"\u2589");  // Used for testing only!
#endif
  m_phrasecount++;

  if (m_phrasecount == 1) {
    m_vkbb_Insert.EnableWindow(TRUE);
    m_vkbb_ClearBuffer.EnableWindow(TRUE);
  }
  m_vkbb_BackSpace.EnableWindow(TRUE);

  if (m_bDeadKeyActive) {
    // Was a deadkey combination character - reset back to normal keys
    SetDeadKeyEnvironment(false);
    SetButtons();
  }

  UpdateData(FALSE);
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
      m_phrase += CSecString(c);
#ifdef _DEBUG
      m_displayedphrase += CSecString(c);  // Used for testing only!
#endif
      m_phrasecount++;
      if (m_phrasecount == 1) {
        m_vkbb_Insert.EnableWindow(TRUE);
        m_vkbb_ClearBuffer.EnableWindow(TRUE);
      }
      m_vkbb_BackSpace.EnableWindow(TRUE);

      UpdateData(FALSE);
    }
  }

  // Can't have any other special keys active once AltNum pressed
  // Save the state for when the user has finished
  if (m_bAltNum) {
    m_SaveState = m_State;
    m_State &= ~(VST_MENU | VST_SHIFT | VST_LCTRL | VST_ALTGR | VST_RCTRL | VST_CAPSLOCK);

    m_bSaveShift = m_bShift;
    m_bSaveLCtrl = m_bLCtrl;
    m_bSaveRCtrl = m_bRCtrl;
    m_bSaveAltGr = m_bAltGr;
    m_bSaveCapsLock = m_bCapsLock;

    m_bShift = m_bLCtrl = m_bRCtrl = m_bAltGr = m_bCapsLock = false;
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
    m_vkbb_Keys[i].EnableWindow(m_bAltNum ? FALSE : TRUE);
  }

  // Set Shift/Caps Lock buttons availability
  BOOL bEnableS, bEnableC;
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
  m_vkbb_LShift.EnableWindow(bEnableS);
  m_vkbb_RShift.EnableWindow(bEnableS);
  m_vkbb_CapsLock.EnableWindow(bEnableC);

  const BOOL bEnable = m_bAltNum ? FALSE : TRUE;

  if (m_bAltNum) {
    // Disable all other special keys if AltNum enabled
    m_vkbb_LCtrl.EnableWindow(bEnable);
    m_vkbb_RCtrl.EnableWindow(bEnable);
    m_vkbb_AltGr.EnableWindow(bEnable);

    if (m_uiKLID == JAPANESE_KBD) {
      if (m_Kana == JAPANESE) {
        m_vkbb_SmallSpaceBar.EnableWindow(bEnable);
        m_vkbb_Hiragana.EnableWindow(bEnable);
        if (m_Hiragana == KATAKANA)
          m_vkbb_Size.EnableWindow(bEnable);
      }
    }
  } else {
    // Don't touch the Left/Right Control keys if Japanese keyboard
    m_vkbb_LCtrl.EnableWindow(m_bLCtrlChars ? TRUE : FALSE);
    m_vkbb_RCtrl.EnableWindow(m_bRCtrlChars ? TRUE : FALSE);
    if (m_uiKLID == JAPANESE_KBD) {
      m_vkbb_AltGr.EnableWindow(bEnable);
      if (m_Kana == JAPANESE) {
        m_vkbb_SmallSpaceBar.EnableWindow(bEnable);
        m_vkbb_Hiragana.EnableWindow(bEnable);
        if (m_Hiragana == KATAKANA)
          m_vkbb_Size.EnableWindow(bEnable);
      }
    } else {
      m_vkbb_AltGr.EnableWindow(m_bAltGrChars ? TRUE : FALSE);
    }
  }

  // Disable changing keyboards
  GetDlgItem(IDC_VK101)->EnableWindow(bEnable);
  GetDlgItem(IDC_VK102)->EnableWindow(bEnable);

  // Allow/deny other controls
  m_cbxKeyBoards.EnableWindow(bEnable);
  m_vkbb_SpaceBar.EnableWindow(bEnable);
  m_vkbb_BackSpace.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);
}

void CVKeyBoardDlg::OnBackSpace()
{
  if (m_phrasecount > 0) {
    m_phrase.Delete(m_phrasecount - 1, 1);
#ifdef _DEBUG
    m_displayedphrase.Delete(m_phrasecount - 1, 1);  // Used for testing only!
#endif
    m_phrasecount--;
    UpdateData(FALSE);
  }

  if (m_phrasecount == 0)
    m_vkbb_BackSpace.EnableWindow(FALSE);
}

void CVKeyBoardDlg::OnShift()
{
  m_bShift = !m_bShift;

  m_vkbb_LShift.SetPushedState(m_bShift);
  m_vkbb_RShift.SetPushedState(m_bShift);
  m_vkbb_LShift.Invalidate();
  m_vkbb_RShift.Invalidate();

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
  CString cs_ToolTip;
  switch (m_uiKLID) {
    case KOREAN_KBD:
      m_State = 0;
      m_vkbb_AltGr.SetWindowText(m_bAltGr ? L"Kor" : L"Eng");
      cs_ToolTip.LoadString(m_bAltGr ? IDS_VK_SW_ENGLISH : IDS_VK_SW_KOREAN);
      m_pToolTipCtrl->UpdateTipText(cs_ToolTip, (CWnd *)&m_vkbb_AltGr);
      break;
    case JAPANESE_KBD:
      m_State = 0;
      m_vkbb_AltGr.SetWindowText(m_bAltGr ? L"Kana" : L"Eng");
      m_Kana = m_bAltGr ? JAPANESE : ENGLISH;
      SetJapaneseKeys();
      cs_ToolTip.LoadString(m_bAltGr ? IDS_VK_SW_ENGLISH : IDS_VK_SW_KANA);
      m_pToolTipCtrl->UpdateTipText(cs_ToolTip, (CWnd *)&m_vkbb_AltGr);
      if (m_bAltGr) {
        cs_ToolTip.LoadString(IDS_VK_SW_KATAKANA);
        m_pToolTipCtrl->AddTool((CWnd *)&m_vkbb_Hiragana, cs_ToolTip);
      } else {
        m_pToolTipCtrl->DelTool((CWnd *)&m_vkbb_Hiragana);
      }
      break;
  }

  if (m_bAltGr)
    m_State |= VST_ALTGR;
  else
    m_State &= ~VST_ALTGR;

  const BOOL bEnable = m_bAltGr ? FALSE : TRUE;

  if (m_bAltGr) {
    m_vkbb_LShift.EnableWindow(m_bAllow_gS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_gS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_gC ? TRUE : FALSE);
    m_vkbb_LCtrl.EnableWindow(FALSE);
    m_vkbb_RCtrl.EnableWindow(FALSE);
  } else {
    m_vkbb_LShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_bC ? TRUE : FALSE);
    m_vkbb_LCtrl.EnableWindow(m_bLCtrlChars ? TRUE : FALSE);
    m_vkbb_RCtrl.EnableWindow(m_bRCtrlChars ? TRUE : FALSE);
  }

  SetButtons();

  m_vkbb_BackSpace.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);
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
    m_vkbb_LShift.EnableWindow(m_bAllow_lS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_lS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_lC ? TRUE : FALSE);
  } else {
    m_vkbb_LShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_bC ? TRUE : FALSE);
  }

  SetButtons();

  const BOOL bEnable = m_bLCtrl ? TRUE : FALSE;
  m_vkbb_BackSpace.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);
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

  m_vkbb_AltNum.EnableWindow(m_bRCtrl ? FALSE : TRUE);
  if (m_bAltGrChars)
    m_vkbb_AltGr.EnableWindow(m_bRCtrl ? FALSE : TRUE);

  if (m_bLCtrlChars)
    m_vkbb_LCtrl.EnableWindow(m_bRCtrl ? FALSE : TRUE);

  if (m_bRCtrl) {
    m_vkbb_LShift.EnableWindow(m_bAllow_rS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_rS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_rC ? TRUE : FALSE);
  } else {
    m_vkbb_LShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_RShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
    m_vkbb_CapsLock.EnableWindow(m_bAllow_bC ? TRUE : FALSE);
  }

  SetButtons();

  const BOOL bEnable = m_bRCtrl ? TRUE : FALSE;
  m_vkbb_BackSpace.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);
}

void CVKeyBoardDlg::OnKeySize()
{
  // Switch between Half-width & Full-width - Katakana only
  // 0 == Half-width; 1 = Full-width
  m_Size = 1 - m_Size;

  CString cs_Size;
  cs_Size = m_Size == HALF ? wcHalfWidth : wcFullWidth;
  m_vkbb_Size.SetWindowText(cs_Size);

  CString cs_ToolTip;
  UINT uiTT =  m_Size == HALF ? IDS_VK_SW_FULLWIDTH : IDS_VK_SW_HALFWIDTH;
  cs_ToolTip.LoadString(uiTT);
  m_pToolTipCtrl->UpdateTipText(cs_ToolTip, (CWnd *)&m_vkbb_Size);
  m_pToolTipCtrl->Update();

  SetButtons();

  m_vkbb_Size.UpdateWindow();
}

void CVKeyBoardDlg::OnHiragana()
{
  // Switch between Hiragana & Katakana
  // 0 == Hiragana; 1 = Katakana
  m_Hiragana = 1 - m_Hiragana;

  CString cs_HK = m_Hiragana == HIRAGANA ? wcHiragana : wcKatakana;
  m_vkbb_Hiragana.SetWindowText(cs_HK);

  CString cs_ToolTip;
  UINT uiTT =  m_Hiragana == HIRAGANA ? IDS_VK_SW_KATAKANA : IDS_VK_SW_HIRAGANA;
  cs_ToolTip.LoadString(uiTT);
  m_pToolTipCtrl->UpdateTipText(cs_ToolTip, (CWnd *)&m_vkbb_Hiragana);

  if (m_Hiragana == HIRAGANA) {
    m_pToolTipCtrl->DelTool((CWnd *)&m_vkbb_Size);
  } else {
    m_Size = 0;
    cs_ToolTip.LoadString(IDS_VK_SW_FULLWIDTH);
    m_pToolTipCtrl->AddTool((CWnd *)&m_vkbb_Size, cs_ToolTip);
  }

  m_pToolTipCtrl->Update();

  SetJapaneseKeys();

  SetButtons();

  m_vkbb_Hiragana.UpdateWindow();
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
    m_vkbb_Numbers[i].SetWindowText(m_pnumbers[i]);
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

  SetSpecialButtons();
}

void CVKeyBoardDlg::SetSpecialButtons()
{
  // Disable all other special keys - Alt, Ctrl, AltGr, Backspace etc.
  // (m_State & (VST_LCTRL | VST_ALTGR | VST_RCTRL)) != 0 ? FALSE : TRUE
  // m_bAltNum, m_bAltGr, m_bShift, m_bLCtrl, m_bRCtrl;
  BOOL bEnable;

  /*
    If AltNum on - only AltNum key allowed
    If AltGr  on - only AltGr  key allowed
  */

  bEnable = (m_bAltNum || m_bAltGr || m_bLCtrl || m_bRCtrl) ? FALSE : TRUE;

  m_vkbb_BackSpace.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);
  m_vkbb_SpaceBar.EnableWindow(bEnable);

  if (m_bAltNum) {
    bEnable = FALSE;
    m_vkbb_AltGr.EnableWindow(bEnable);
    m_vkbb_CapsLock.EnableWindow(bEnable);

    m_vkbb_LShift.EnableWindow(bEnable);
    m_vkbb_LCtrl.EnableWindow(bEnable);
    m_vkbb_RShift.EnableWindow(bEnable);
    m_vkbb_RCtrl.EnableWindow(bEnable);

    return;
  }

  if (m_bAltGr) {
    bEnable = FALSE;
    m_vkbb_AltNum.EnableWindow(bEnable);
    m_vkbb_CapsLock.EnableWindow(bEnable);

    m_vkbb_LShift.EnableWindow(bEnable);
    m_vkbb_LCtrl.EnableWindow(bEnable);
    m_vkbb_RShift.EnableWindow(bEnable);
    m_vkbb_RCtrl.EnableWindow(bEnable);

    return;
  }

  if (m_bLCtrl || m_bRCtrl) {
    bEnable = FALSE;
    m_vkbb_AltGr.EnableWindow(bEnable);
    m_vkbb_AltNum.EnableWindow(bEnable);

    return;
  }

  // Otherwise enable the buttons
  if (m_uiKLID == JAPANESE_KBD) {
    m_vkbb_LCtrl.EnableWindow(FALSE);
    m_vkbb_RCtrl.EnableWindow(FALSE);
    m_vkbb_RHCtrl.EnableWindow(FALSE);
  } else {
    m_vkbb_LCtrl.EnableWindow(m_bLCtrlChars ? TRUE : FALSE);
    m_vkbb_RCtrl.EnableWindow(m_bRCtrlChars ? TRUE : FALSE);
  }

  bEnable = TRUE;
  m_vkbb_AltNum.EnableWindow(bEnable);
  m_vkbb_AltGr.EnableWindow(m_bAltGrChars ? TRUE : FALSE);
  m_vkbb_CapsLock.EnableWindow(bEnable);
  m_vkbb_LShift.EnableWindow(bEnable);
  m_vkbb_RShift.EnableWindow(bEnable);
}

void CVKeyBoardDlg::SetNormalButtons()
{
  // Set Normal Buttons
  CString cs_ToolTip;
  cs_ToolTip.LoadString(IDS_VKDEADKEY);
  if (m_bAltNum) {
    // Normal keys disabled if using AltNum
    for (int i = 0; i < NUM_KEYS; i++) {
      m_vkbb_Keys[i].SetWindowText(L"");
      m_vkbb_Keys[i].EnableWindow(FALSE);
      m_vkbb_Keys[i].SetDeadKeyState(false);
    }

  } else {
    // Normal keys
    Iter_Map_st_SC2CHAR iter_sc;
    CIter_Map_SCSS2MC citer_scss;
    CString cs_temp;
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
      pws_os::Trace(L"SetButtons: Unknown state! (1)");
      ASSERT(0);
    }

    // Now put the character on the keys
    for (int i = 0; i < NUM_KEYS; i++) {
      bDeadKey = false;
      cs_temp.Empty();
      if (m_scancodes[i] == 0 ||
          m_map_stSC2Char.find(m_scancodes[i]) == m_map_stSC2Char.end()) {
        // Zero scancode or not in our map to a character
        //     == unused key - disable/don't show
        m_vkbb_Keys[i].SetWindowText(cs_temp);
        m_vkbb_Keys[i].EnableWindow(FALSE);
        m_vkbb_Keys[i].ShowWindow(SW_HIDE);
      } else {
        iter_sc = m_map_stSC2Char.find(m_scancodes[i]);
        if (iter_sc != m_map_stSC2Char.end()) {
          if (index < 0) {
            pws_os::Trace(L"SetButtons: Unknown state! (2)");
            ASSERT(0);
          } else {
            // Get scancode + shiftstate value
            unsigned short int uiSCSS = m_scancodes[i] * 256 + m_State;

            // Get the wchar_t character
            wc_temp = iter_sc->second.wcChar[index];
            cs_temp = wc_temp;

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
                  cs_temp = pctemp2;
                } else
                  cs_temp = wc_temp;
                break;
              case -3:
                citer_scss = m_stKBImpl.pmapSCSS2MC3->find(uiSCSS);
                if (citer_scss != m_stKBImpl.pmapSCSS2MC3->end()) {
                  wchar_t pctemp3[4] = {0, 0, 0, 0};
                  memcpy((void *)&pctemp3[0], (void *)&m_stKBImpl.wcMC3[citer_scss->second * 3 * sizeof(wchar_t)], 3 * sizeof(wchar_t));
                  cs_temp = pctemp3;
                } else
                  cs_temp = wc_temp;
                break;
              case -4:
                citer_scss = m_stKBImpl.pmapSCSS2MC4->find(uiSCSS);
                if (citer_scss != m_stKBImpl.pmapSCSS2MC4->end()) {
                  wchar_t pctemp4[5] = {0, 0, 0, 0, 0};
                  memcpy((void *)&pctemp4[0], (void *)&m_stKBImpl.wcMC4[citer_scss->second * 4 * sizeof(wchar_t)], 4 * sizeof(wchar_t));
                  cs_temp = pctemp4;
                } else
                  cs_temp = wc_temp;
                break;
            }
            bDeadKey = iter_sc->second.bsDeadKey.test(index);
          }
          // Now set character on key
          m_vkbb_Keys[i].SetWindowText(cs_temp);
          m_vkbb_Keys[i].EnableWindow(cs_temp.IsEmpty() ? FALSE : TRUE);
          m_vkbb_Keys[i].ShowWindow(SW_SHOW);
          m_vkbb_Keys[i].SetDeadKeyState(bDeadKey);
          // If DeadKey - add a ToolTip, if not - remove it
          if (bDeadKey) {
            m_pToolTipCtrl->AddTool((CWnd *)&m_vkbb_Keys[i], cs_ToolTip);
          } else {
            m_pToolTipCtrl->DelTool((CWnd *)&m_vkbb_Keys[i]);
          }
        }
      }
    }

    // Deal with space bar
    bDeadKey = false;
    iter_sc = m_map_stSC2Char.find(0x39);
    if (iter_sc == m_map_stSC2Char.end()) {
      cs_temp.Empty();
    } else {
      if (index < 0) {
        wc_temp = (wchar_t)0;
        pws_os::Trace(L"SetButtons: Unknown state! (3)");
        ASSERT(0);
      } else {
        wc_temp = iter_sc->second.wcChar[index];
        bDeadKey = iter_sc->second.bsDeadKey.test(index);
      }
      if (wc_temp == 0)
        cs_temp.Empty();
      else
        cs_temp = wc_temp;
    }

    if (m_vkbb_SpaceBar.IsWindowEnabled()) {
      m_vkbb_SpaceBar.EnableWindow(cs_temp.IsEmpty() ? FALSE : TRUE);
      // User can't see a blank - so tell them in words
      cs_temp.LoadString(IDS_VKBBTN_SPACEBAR);
      m_vkbb_SpaceBar.SetWindowText(cs_temp);
      m_vkbb_SpaceBar.SetDeadKeyState(bDeadKey);
    } else {
      m_vkbb_SmallSpaceBar.EnableWindow(cs_temp.IsEmpty() ? FALSE : TRUE);
      // User can't see a blank - so tell them in words
      cs_temp.LoadString(IDS_VKBBTN_SPACEBAR);
      m_vkbb_SmallSpaceBar.SetWindowText(cs_temp);
      m_vkbb_SmallSpaceBar.SetDeadKeyState(bDeadKey);
    }
  }

  UpdateData(FALSE);
  Invalidate();
}

void CVKeyBoardDlg::SetDeadKeyButtons()
{
  ASSERT(m_wcDeadKey != (wchar_t)0);

  // Clear out buttons
  for (int i = 0; i < NUM_KEYS; i++) {
    m_vkbb_Keys[i].SetWindowText(L"");
    m_vkbb_Keys[i].EnableWindow(FALSE);
    m_vkbb_Keys[i].SetDeadKeyState(false);
  }

  // And the Space Bar which allows user to type in the DeadKey
  // in its own right
  m_vkbb_SpaceBar.SetWindowText(L"");
  m_vkbb_SpaceBar.EnableWindow(FALSE);
  m_vkbb_SpaceBar.SetDeadKeyState(false);

  // Now put back associated combination values
  CString cs_temp;
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
    cs_temp = iter_mm->second.wcCC;

    if (m_vsc[index] == 0x39) {
      // Space Bar
      m_vkbb_SpaceBar.SetWindowText(cs_temp);
      m_vkbb_SpaceBar.EnableWindow(TRUE);
    } else {
      // Other character
      m_vkbb_Keys[index].SetWindowText(cs_temp);
      m_vkbb_Keys[index].EnableWindow(TRUE);
    }
  }

  UpdateData(FALSE);
  Invalidate();
}

void CVKeyBoardDlg::OnClearBuffer()
{
  // Clear the character buffer
  m_phrasecount = 0;
  m_phrase = L"";
#ifdef _DEBUG
  m_displayedphrase = L"";  // Used for testing only!
#endif

  m_vkbb_Insert.EnableWindow(FALSE);
  m_vkbb_ClearBuffer.EnableWindow(FALSE);
  m_vkbb_BackSpace.EnableWindow(FALSE);

  UpdateData(FALSE);
}

void CVKeyBoardDlg::OnChangeKeyboard()
{
  int isel = m_cbxKeyBoards.GetCurSel();

  if (isel == CB_ERR)
    return;

  // Get the requested layout
  UINT uiKLID = (UINT)m_cbxKeyBoards.GetItemData(isel);
  if (uiKLID == m_uiKLID)
    return;

  // Remove old tooltips
  if (m_uiKLID == JAPANESE_KBD) {
    if (m_vkbb_Size.IsWindowEnabled()) {
      m_pToolTipCtrl->DelTool((CWnd *)&m_vkbb_Size);
    }
    if (m_vkbb_Hiragana.IsWindowEnabled()) {
      m_pToolTipCtrl->DelTool((CWnd *)&m_vkbb_Hiragana);
    }
  }

  // Now make it the current keyboard
  m_uiKLID = uiKLID;
  CString cs_ToolTip, cs_temp;
  if (m_uiKLID == JAPANESE_KBD) {
    cs_ToolTip.LoadString(IDS_VK_SW_KANA);
  } else if (m_uiKLID == KOREAN_KBD) {
    cs_ToolTip.LoadString(IDS_VK_SW_KOREAN);
  } else {
    cs_temp.LoadString(IDS_VKALTGR);
    cs_ToolTip.Format(IDS_VKSTATIC_SPECIAL, static_cast<LPCWSTR>(cs_temp));
  }

  m_pToolTipCtrl->UpdateTipText(cs_ToolTip, (CWnd *)&m_vkbb_AltGr);

  // Set up characters
  ProcessKeyboard(m_uiKLID);

  // Set up Buttons
  SetButtons();
}

void CVKeyBoardDlg::OnChangeKeyboardType()
{
  UpdateData(TRUE);

  // Set up characters
  ProcessKeyboard(m_uiKLID, false);

  // Set up Buttons
  SetButtons();

  // Lose focus
  m_vkbb_Cancel.SetFocus();
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

    if (kbl.uiKLID == m_uiPhysKLID)
      m_selectedkb.LoadString(kbl.uiCtrlID);
  };
}

void CVKeyBoardDlg::ProcessKeyboard(const UINT uiKLID, const bool bSetType)
{
  VERIFY(m_pGetKBData(uiKLID, m_stKBImpl));

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

  m_vkbb_LCtrl.EnableWindow(m_bLCtrlChars ? TRUE : FALSE);
  m_vkbb_AltGr.EnableWindow(m_bAltGrChars ? TRUE : FALSE);
  m_vkbb_RCtrl.EnableWindow(m_bRCtrlChars ? TRUE : FALSE);

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

  m_vkbb_LShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
  m_vkbb_RShift.EnableWindow(m_bAllow_bS ? TRUE : FALSE);
  m_vkbb_CapsLock.EnableWindow(m_bAllow_bC ? TRUE : FALSE);

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
    ((CButton *)GetDlgItem(uiButton))->SetCheck(BST_CHECKED);
  }

  // Need to do this prior to call to ResetKeys
  bool bJapanese = (m_uiKLID == JAPANESE_KBD);
  GetDlgItem(IDC_VK101)->EnableWindow(bJapanese ? FALSE : TRUE);
  GetDlgItem(IDC_VK101)->ShowWindow(bJapanese ? SW_HIDE : SW_SHOW);
  GetDlgItem(IDC_VK102)->EnableWindow(bJapanese ? FALSE : TRUE);
  GetDlgItem(IDC_VK102)->ShowWindow(bJapanese ? SW_HIDE : SW_SHOW);
  GetDlgItem(IDC_VK106)->ShowWindow(bJapanese ? SW_SHOW : SW_HIDE);

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
  m_vkbb_AltGr.SetPushedState(false);
  m_vkbb_AltNum.SetPushedState(false);
  m_vkbb_CapsLock.SetPushedState(false);

  m_State = 0;
}

void CVKeyBoardDlg::ResetKeyboard()
{
  m_phrasecount = 0;
  m_phrase = L"";
#ifdef _DEBUG
  m_displayedphrase = L"";  // Used for testing only!
#endif
  SetDeadKeyEnvironment(false);
  m_wcDeadKey = (wchar_t)0;
  SetButtons();

  m_vkbb_Insert.EnableWindow(FALSE);
  m_vkbb_ClearBuffer.EnableWindow(FALSE);
  m_vkbb_BackSpace.EnableWindow(FALSE);

  UpdateData(FALSE);
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
    m_pnumbers[i] = _wcsdup(pdefnumbers[i]);
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
    m_vkbb_Numbers[i].EnableWindow(bEnable);
  }

  // Save current state and reset it until the user presses the DeadKey combination character
  if (m_bDeadKeyActive) {
    m_SaveState = m_State;

    m_bSaveShift = m_bShift;
    m_bSaveLCtrl = m_bLCtrl;
    m_bSaveRCtrl = m_bRCtrl;
    m_bSaveAltGr = m_bAltGr;
    m_bSaveCapsLock = m_bCapsLock;
    m_bDeadKeySaved = true;

    m_bShift = m_bLCtrl = m_bRCtrl = m_bAltGr = m_bCapsLock = false;

    m_State |= VST_MENU;
    m_State &= ~(VST_SHIFT | VST_LCTRL | VST_ALTGR | VST_RCTRL | VST_CAPSLOCK);
  } else {
    if (m_bDeadKeySaved) {
      m_State = m_SaveState;

      m_bShift = m_bSaveShift;
      m_bLCtrl = m_bSaveLCtrl;
      m_bRCtrl = m_bSaveRCtrl;
      m_bAltGr = m_bSaveAltGr;
      m_bCapsLock = m_bSaveCapsLock;
      m_bDeadKeySaved = false;
    }
  }

  m_cbxKeyBoards.EnableWindow(bEnable);
  m_vkbb_Randomize.EnableWindow(bEnable);
  m_vkbb_Insert.EnableWindow(bEnable);
  m_vkbb_ClearBuffer.EnableWindow(bEnable);

  if (m_bLCtrlChars)
    m_vkbb_LCtrl.EnableWindow(m_bLCtrl ? TRUE : FALSE);
  if (m_bRCtrlChars)
    m_vkbb_RCtrl.EnableWindow(m_bRCtrl ? TRUE : FALSE);
  if (m_bAltGrChars)
    m_vkbb_AltGr.EnableWindow(m_bAltGr ? TRUE : FALSE);

  m_vkbb_AltNum.EnableWindow(bEnable);
  GetDlgItem(IDC_VK101)->EnableWindow(bEnable);
  GetDlgItem(IDC_VK102)->EnableWindow(bEnable);
}

void CVKeyBoardDlg::SetJapaneseKeyboard()
{
  bool bHiragana;
  CString cs_Kana, cs_HK, cs_Size;

  if (m_Kana == ENGLISH) {
    bHiragana = false;
    cs_Kana = L"Eng";
    cs_HK = L"";
    cs_Size = L"";
  } else {
    bHiragana = (m_Hiragana == HIRAGANA);
    cs_Kana = L"Kana";
    cs_HK = bHiragana ? wcHiragana : wcKatakana;
    cs_Size = m_Size == HALF ? wcHalfWidth : wcFullWidth;
  }

  m_vkbb_Hiragana.SetWindowText(cs_HK);
  m_vkbb_Size.SetWindowText(cs_Size);

  // Set Japanese (Eng to start with) label and stop it showing pushed state
  m_vkbb_AltGr.SetWindowText(cs_Kana);
  m_vkbb_AltGr.EnableWindow(TRUE);
  m_vkbb_AltGr.ChangePushColour(false);

  SetJapaneseKeys();
}

void CVKeyBoardDlg::SetKoreanKeyboard()
{
  // Set Korean (Eng to start with) label and stop it showing pushed state
  m_vkbb_AltGr.SetWindowText(L"Eng");
  m_vkbb_AltGr.EnableWindow(TRUE);
  m_vkbb_AltGr.ChangePushColour(false);

  SetJapaneseKeys();
}

void CVKeyBoardDlg::SetStandardKeyboard()
{
  // Put back standard label and make it show pushed state
  m_vkbb_AltGr.SetWindowText(L"Alt Gr");
  m_vkbb_AltGr.ChangePushColour(true);

  SetJapaneseKeys();
}

void CVKeyBoardDlg::SetJapaneseKeys()
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
    m_vkbb_LCtrl.EnableWindow(FALSE);
    m_vkbb_RCtrl.EnableWindow(FALSE);
  }

  m_vkbb_SpaceBar.EnableWindow(bEnableSpaceBar ? TRUE : FALSE);
  m_vkbb_SpaceBar.ShowWindow(bEnableSpaceBar ? SW_SHOW : SW_HIDE);

  m_vkbb_SmallSpaceBar.EnableWindow(bEnableSpaceBar ? FALSE : TRUE);
  m_vkbb_SmallSpaceBar.ShowWindow(bEnableSpaceBar ? SW_HIDE : SW_SHOW);

  m_vkbb_RCtrl.ShowWindow(bShowRCtrl ? SW_SHOW : SW_HIDE);
  m_vkbb_RHCtrl.ShowWindow(bShowRCtrl ? SW_HIDE : SW_SHOW);

  m_vkbb_Size.EnableWindow(bShowRCtrl ? FALSE : TRUE);
  m_vkbb_Size.ShowWindow(bShowRCtrl ? SW_HIDE : SW_SHOW);
  m_vkbb_Size.SetWindowText(wcHalfWidth);

  m_vkbb_Hiragana.EnableWindow(bEnableHK  ? TRUE : FALSE);
  m_vkbb_Hiragana.ShowWindow(bEnableHK ? SW_SHOW : SW_HIDE);
  m_vkbb_Hiragana.SetWindowText(wcHiragana);

  Invalidate();
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL CVKeyBoardDlg::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST) {
    if (m_pToolTipCtrl != NULL) {
      // Change to allow tooltip on disabled controls
      MSG msg = *pMsg;
      msg.hwnd = (HWND)m_pToolTipCtrl->SendMessage(TTM_WINDOWFROMPOINT, 0,
                                                  (LPARAM)&msg.pt);
      CPoint pt = pMsg->pt;
      ::ScreenToClient(msg.hwnd, &pt);

      msg.lParam = MAKELONG(pt.x, pt.y);

      // Let the ToolTip process this message.
      m_pToolTipCtrl->RelayEvent(&msg);
    }
  }

  // Make a sound if user requested it
  if (pMsg->message == WM_LBUTTONDOWN) {
    if (m_bPlaySound == BST_CHECKED) {
      CWnd *pWnd = FromHandle(pMsg->hwnd);
      if (pWnd != NULL) {
        UINT nID = pWnd->GetDlgCtrlID();
        if (nID != 0) {
          Beep(1500, 100);
        } else {
          // Check that this was on a valid control rather than the dialog background
          // Also, no sound for passphrase (if visible in Debug build) or the NumPad GroupBox (nID == -1)
          POINT clpt(pMsg->pt);
          ScreenToClient(&clpt);
          HWND hwndChild = ::ChildWindowFromPoint(pMsg->hwnd, clpt);
          nID = ::GetDlgCtrlID(hwndChild);
          if (hwndChild != NULL && hwndChild != pMsg->hwnd && nID != IDC_STATIC_VKPASSPHRASE && nID != -1) {
            Beep(800, 100);
            Beep(500, 100);
          }
        }
      }
    }
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CVKeyBoardDlg::ApplyUnicodeFont(CWnd* pDlgItem)
{
  ASSERT(pDlgItem != NULL);
  if (pDlgItem == NULL)
    return;

  if (m_pPassphraseFont == NULL) {
    m_pPassphraseFont = new CFont;

    wchar_t * pszFont(NULL);
    StringX cs_VKeyboardFont = PWSprefs::GetInstance()->
                                 GetPref(PWSprefs::VKeyboardFontName);
    switch (m_iFont) {
      case USER_FONT:
        pszFont = (wchar_t *)cs_VKeyboardFont.c_str();
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
    // Get resolution
    HDC hDC = ::GetWindowDC(GetSafeHwnd());
    const int Ypixels = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(GetSafeHwnd(), hDC);

    int iFontSize = PWSprefs::GetInstance()->GetPref(PWSprefs::VKFontPtSz);
    if (iFontSize == 0) {
      // Use default
      iFontSize = MulDiv(16, 72, Ypixels) * 10;
      PWSprefs::GetInstance()->SetPref(PWSprefs::VKFontPtSz, iFontSize);
    }

    LOGFONT lf;
    SecureZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -MulDiv(iFontSize / 10, Ypixels, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    wcsncpy_s(lf.lfFaceName, LF_FACESIZE, pszFont, wcslen(pszFont));

    m_pPassphraseFont->CreateFontIndirect(&lf);
  }

  pDlgItem->SetFont(m_pPassphraseFont);
}

HBRUSH CVKeyBoardDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{
  if (!this->IsWindowEnabled())
    return (HBRUSH)NULL;

  HBRUSH hbr = CPWDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  UINT nID = pWnd->GetDlgCtrlID();

  switch (nCtlColor) {
    case CTLCOLOR_STATIC:
    case CTLCOLOR_DLG:
      // Black text on white background - except passphrase if visible
      pDC->SetTextColor(RGB(nID == IDC_STATIC_VKPASSPHRASE ? 255 : 0, 0, 0));
      pDC->SetBkColor(RGB(255, 255, 255));
      return (HBRUSH)(m_pBkBrush.GetSafeHandle());
    default:
      return hbr;
  }
}

void CVKeyBoardDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Allow dragging without the caption bar!
  CPWDialog::OnLButtonDown(nFlags, point);

  PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x,point.y));
}

void CVKeyBoardDlg::OnSaveKLID()
{
  m_bSaveKLID = ((CButton *)GetDlgItem(IDC_SAVEKLID))->GetCheck();
}

void CVKeyBoardDlg::OnKeyPressPlaySound()
{
  m_bPlaySound = ((CButton *)GetDlgItem(IDC_KEYPRESS_PLAYSOUND))->GetCheck();
}

void CVKeyBoardDlg::OnShowPassphrase()
{
#ifdef _DEBUG
  m_bShowPassphrase = ((CButton *)GetDlgItem(IDC_SHOWBUFFER))->GetCheck();

  GetDlgItem(IDC_STATIC_VKPASSPHRASE)->ShowWindow(m_bShowPassphrase == BST_CHECKED ? SW_SHOW : SW_HIDE);
#endif
}
