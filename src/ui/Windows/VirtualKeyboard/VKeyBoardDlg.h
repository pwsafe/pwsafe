/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// It is Unicode ONLY.

#pragma once

// VKeyBoardDlg.h : header file
//-----------------------------------------------------------------------------

/*

  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!

*/

#include "../resource.h"

#include "VKdefinitions.h"
#include "VKresource.h"
#include "VKBButton.h"

#include "../../../core/StringX.h"

class CVKeyBoardDlg
{
public:
  static bool IsOSKAvailable(); // true iff dll available, right version, etc.
  static INT_PTR CALLBACK VKDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

  static wchar_t *ARIALUMS;
  static wchar_t *ARIALU;
  static wchar_t *LUCIDAUS;

  CVKeyBoardDlg(HWND hParent, HWND hMasterPhrase, LPCWSTR wcKLID = NULL);
  ~CVKeyBoardDlg();

  enum { IDD = IDD_SDVKEYBOARD };

  const StringX &GetPassphrase() const {return m_phrase;}
  const UINT &GetKLID() const {return m_uiKLID;}
  const bool SaveKLID() const {return m_bSaveKLID == BST_CHECKED;}

  void ResetKeyboard();

protected:
  friend class CSDThread;

  BOOL OnInitDialog();

  int m_phrasecount;

  void OnCancel();
  void OnInsert();
  void OnRandomize();
  void OnClearBuffer();
  void OnBackSpace();
  void OnShift();
  void OnLCtrl();
  void OnRCtrl();
  void OnRHCtrl();
  void OnAltNum();
  void OnAltGr();
  void OnCapsLock();
  void OnSpaceBar();
  void OnKeySize();
  void OnHiragana();
  void OnNumerics(UINT nID);
  void OnKeys(UINT nID);
  void OnChangeKeyboard();
  void OnChangeKeyboardType();
  void OnSaveKLID();

  int m_iKeyboard;

  void GetAllKeyboardsAvailable();
  void ProcessKeyboard(const UINT uiKLID, const bool bSetType = true);
  void ResetKeys();
  void SetDeadKeyEnvironment(const bool bState);
  void SetButtons();
  void SetNormalButtons();
  void SetDeadKeyButtons();
  void SetJapaneseKeyboard();
  void SetKoreanKeyboard();
  void SetStandardKeyboard();
  void SetSpecialKeys();
  void ApplyUnicodeFont(HWND hDlgItem);
  void DoRCtrl(const bool bDoFull);

  BOOL AddTooltip(UINT uiControlID, UINT uiToolString, UINT uiFormat = NULL);
  BOOL AddTooltip(UINT uiControlID, stringT sText);
  BOOL DeleteTooltip(UINT uiControlID);
  BOOL UpdateTooltipText(UINT uiControlID, UINT uiToolString, UINT uiFormat = NULL);
  BOOL UpdateTooltipText(UINT uiControlID, stringT sText);

  HFONT m_PassphraseFont;

  CVKBButton m_vkbb_Alt, m_vkbb_AltGr, m_vkbb_CapsLock, m_vkbb_AltNum, m_vkbb_BackSpace;
  CVKBButton m_vkbb_LShift, m_vkbb_LCtrl, m_vkbb_RShift, m_vkbb_RCtrl, m_vkbb_RHCtrl;
  CVKBButton m_vkbb_Randomize;
  CVKBButton m_vkbb_InsertClose, m_vkbb_Insert, m_vkbb_Cancel, m_vkbb_ClearBuffer;

  CVKBButton m_vkbb_SpaceBar;
  CVKBButton m_vkbb_Numbers[NUM_DIGITS];
  CVKBButton m_vkbb_Keys[NUM_KEYS];

  // Japanese
  CVKBButton m_vkbb_SmallSpaceBar, m_vkbb_Size, m_vkbb_Hiragana;

  Map_ID2XButton m_Map_ID2XButton;

  wchar_t m_wcDeadKey;
  wchar_t *m_pnumbers[NUM_DIGITS];
  BYTE m_scancodes[NUM_KEYS];

  StringX m_selectedkb;

  st_VKBD *m_pstvkbd;
  Map_st_SC2Char m_map_stSC2Char;
  std::vector<BYTE> m_vsc;

  StringX m_phrase;
  int m_altchar;
  int m_Size, m_Hiragana, m_Kana;
  bool m_bAltNum, m_bAltGr, m_bCapsLock, m_bRandom;
  bool m_bShift, m_bLCtrl, m_bRCtrl;
  bool m_bSaveShift, m_bSaveLCtrl, m_bSaveRCtrl, m_bSaveAltGr, m_bSaveCapsLock;
  bool m_bLCtrlChars, m_bAltGrChars, m_bRCtrlChars, m_bDeadKeyActive;
  bool m_bAllow_bC, m_bAllow_bS, m_bAllow_lC, m_bAllow_lS;
  bool m_bAllow_gC, m_bAllow_gS, m_bAllow_rC, m_bAllow_rS;

  static int m_iFont;
  static bool m_bUserSpecifiedFont;

  UINT m_uiKLID, m_uiPhysKLID;
  BOOL m_bSaveKLID;
  vKeyboard_Layouts m_KBL;
  BYTE m_State, m_SaveState;
  HBRUSH m_hBkBrush;

  HMODULE m_OSK_module, m_hInstance;
  LP_OSK_GetKeyboardData m_pGetKBData;
  LP_OSK_ListKeyboards m_pListKBs;
  st_KBImpl m_stKBImpl;
  HWND m_hParent, m_hMasterPhrase, m_hwndDlg, m_hcbxKeyBoards, m_hwndTooltip;
  HWND m_hwndVKStaticTimer, m_hwndVKStaticTimerText, m_hwndVKStaticSeconds;
  bool m_bUseSecureDesktop;
  int m_iUserTimeLimit;
  };
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
