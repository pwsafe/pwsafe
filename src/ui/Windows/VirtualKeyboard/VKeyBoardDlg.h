/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// It is Unicode ONLY.

#pragma once

// VKeyBoardDlg.h : header file
//-----------------------------------------------------------------------------

#define NUM_KEYS (IDC_VKBBTN_KBD51 - IDC_VKBBTN_KBD01 + 1)
#define NUM_DIGITS (IDC_VKBBTN_N9 - IDC_VKBBTN_N0 + 1)

#include "../ThisMfcApp.h"
#include "../PWDialog.h"
#include "../resource.h"
#include "../SecString.h"
#include "VKresource.h"
#include "VKBButton.h"
#include "../../../os/windows/pws_osk/pws_osk.h"
#include <vector>
#include <map>

typedef OSK_API void (* LP_OSK_ListKeyboards) (UINT &uiKLID, UINT &uiCtrlID);
typedef OSK_API BOOL (* LP_OSK_GetKeyboardData) (UINT uiKLID, st_KBImpl &stKBImpl);
typedef OSK_API int  (* LP_OSK_GetVersion) ();

enum {USER_FONT, ARIALMS_FONT, ARIAL_FONT, LUCIDA_FONT};

enum eJapanese {ENGLISH = 0, JAPANESE};    // Used for m_Kana
enum eHK       {HIRAGANA = 0, KATAKANA};   // Used for m_Hiragana
enum eSize     {HALF = 0, FULL};           // Used for m_Size

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

typedef std::vector<st_Keyboard_Layout> vKeyboard_Layouts;
typedef vKeyboard_Layouts::const_iterator KBL_citer;

typedef std::map<BYTE, const st_SC2CHAR> Map_st_SC2Char;
typedef Map_st_SC2Char::iterator Iter_Map_st_SC2CHAR;

class CVKeyBoardDlg : public CPWDialog
{
public:
  static bool IsOSKAvailable(); // true iff dll available, right version, etc.

  static wchar_t *ARIALUMS;
  static wchar_t *ARIALU;
  static wchar_t *LUCIDAUS;

  CVKeyBoardDlg(CWnd *pParent, LPCWSTR wcKLID = NULL);
  ~CVKeyBoardDlg();

  enum { IDD = IDD_VKEYBOARD };

  const CSecString &GetPassphrase() const {return m_phrase;}
  const UINT &GetKLID() const {return m_uiKLID;}
  const bool SaveKLID() const {return m_bSaveKLID == BST_CHECKED;}
  const bool PlaySound() const { return m_bPlaySound == BST_CHECKED; }

  void ResetKeyboard();

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  int m_phrasecount;
  CComboBox m_cbxKeyBoards;

  //{{AFX_MSG(CVKeyBoardDlg)
  afx_msg void OnPostNcDestroy();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
  afx_msg void OnCancel();
  afx_msg void OnInsert();
  afx_msg void OnRandomize();
  afx_msg void OnClearBuffer();
  afx_msg void OnBackSpace();
  afx_msg void OnShift();
  afx_msg void OnLCtrl();
  afx_msg void OnRCtrl();
  afx_msg void OnRHCtrl();
  afx_msg void OnAltNum();
  afx_msg void OnAltGr();
  afx_msg void OnCapsLock();
  afx_msg void OnSpaceBar();
  afx_msg void OnKeySize();
  afx_msg void OnHiragana();
  afx_msg void OnNumerics(UINT nID);
  afx_msg void OnKeys(UINT nID);
  afx_msg void OnChangeKeyboard();
  afx_msg void OnChangeKeyboardType();
  afx_msg void OnSaveKLID();
  afx_msg void OnKeyPressPlaySound();
  afx_msg void OnShowPassphrase();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  int m_iKeyboard;
  CToolTipCtrl *m_pToolTipCtrl;

private:
  void GetAllKeyboardsAvailable();
  void ProcessKeyboard(const UINT uiKLID, const bool bSetType = true);
  void ResetKeys();
  void SetDeadKeyEnvironment(const bool bState);
  void SetButtons();
  void SetNormalButtons();
  void SetSpecialButtons();
  void SetDeadKeyButtons();
  void SetJapaneseKeyboard();
  void SetKoreanKeyboard();
  void SetStandardKeyboard();
  void SetJapaneseKeys();
  void ApplyUnicodeFont(CWnd* pDlgItem);
  void DoRCtrl(const bool bDoFull);

  CFont *m_pPassphraseFont;

  CVKBButton m_vkbb_Alt, m_vkbb_AltGr, m_vkbb_CapsLock, m_vkbb_AltNum, m_vkbb_BackSpace;
  CVKBButton m_vkbb_LShift, m_vkbb_LCtrl, m_vkbb_RShift, m_vkbb_RCtrl, m_vkbb_RHCtrl;
  CVKBButton m_vkbb_Randomize;
  CVKBButton m_vkbb_Insert, m_vkbb_Cancel, m_vkbb_ClearBuffer;

  CVKBButton m_vkbb_SpaceBar;
  CVKBButton m_vkbb_Numbers[NUM_DIGITS];
  CVKBButton m_vkbb_Keys[NUM_KEYS];

  // Japanese
  CVKBButton m_vkbb_SmallSpaceBar, m_vkbb_Size, m_vkbb_Hiragana;

  wchar_t m_wcDeadKey;
  wchar_t *m_pnumbers[NUM_DIGITS];
  BYTE m_scancodes[NUM_KEYS];

  CString m_selectedkb;

  st_VKBD *m_pstvkbd;
  Map_st_SC2Char m_map_stSC2Char;
  std::vector<BYTE> m_vsc;

  CSecString m_phrase;
#ifdef _DEBUG
  // Don't even define this in the Release build - Used for testing only!
  CSecString m_displayedphrase;
#endif
  int m_altchar;
  int m_Size, m_Hiragana, m_Kana;
  bool m_bAltNum, m_bAltGr, m_bCapsLock, m_bRandom;
  bool m_bShift, m_bLCtrl, m_bRCtrl;
  bool m_bSaveShift, m_bSaveLCtrl, m_bSaveRCtrl, m_bSaveAltGr, m_bSaveCapsLock;
  bool m_bLCtrlChars, m_bAltGrChars, m_bRCtrlChars;
  bool m_bAllow_bC, m_bAllow_bS, m_bAllow_lC, m_bAllow_lS;
  bool m_bAllow_gC, m_bAllow_gS, m_bAllow_rC, m_bAllow_rS;
  bool m_bDeadKeyActive, m_bDeadKeySaved;

  static int m_iFont;
  static bool m_bUserSpecifiedFont;

  UINT m_uiKLID, m_uiPhysKLID;
  BOOL m_bSaveKLID, m_bPlaySound, m_bShowPassphrase;
  vKeyboard_Layouts m_KBL;
  BYTE m_State, m_SaveState;
  CBrush m_pBkBrush;

  HINSTANCE m_OSK_module;
  LP_OSK_GetKeyboardData m_pGetKBData;
  LP_OSK_ListKeyboards m_pListKBs;
  st_KBImpl m_stKBImpl;
  CWnd *m_pParent;
  UINT m_uiMouseDblClkTime;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
