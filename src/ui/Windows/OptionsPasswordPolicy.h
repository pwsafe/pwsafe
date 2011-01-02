/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once
#include "afxwin.h"

// OptionsPasswordPolicy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy dialog
#include "Options_PropertyPage.h"
#include "core/PWPolicy.h"
#include "ControlExtns.h"

class DboxMain;

class COptionsPasswordPolicy : public COptions_PropertyPage
{
  DECLARE_DYNCREATE(COptionsPasswordPolicy)

  // Construction
public:
  COptionsPasswordPolicy(bool bFromOptions = true);
  ~COptionsPasswordPolicy();
  DboxMain *m_pDbx;

  // Dialog Data
  //{{AFX_DATA(COptionsPasswordPolicy)
  enum { IDD = IDD_PS_PASSWORDPOLICY };
  UINT m_pwdefaultlength;
  BOOL m_pwuselowercase;
  BOOL m_pwuseuppercase;
  BOOL m_pwusedigits;
  BOOL m_pwusesymbols;
  BOOL m_pweasyvision;
  BOOL m_pwusehexdigits;
  BOOL m_pwmakepronounceable;
  UINT m_pwdigitminlength;
  UINT m_pwlowerminlength;
  UINT m_pwsymbolminlength;
  UINT m_pwupperminlength;
  //}}AFX_DATA

  UINT m_savepwdefaultlength;
  BOOL m_savepwuselowercase;
  BOOL m_savepwuseuppercase;
  BOOL m_savepwusedigits;
  BOOL m_savepwusesymbols;
  BOOL m_savepweasyvision;
  BOOL m_savepwusehexdigits;
  BOOL m_savepwmakepronounceable;
  UINT m_savepwdigitminlength;
  UINT m_savepwlowerminlength;
  UINT m_savepwsymbolminlength;
  UINT m_savepwupperminlength;

  PWPolicy m_default_pwp;
  CSecString m_password;
  CSecEditExtn m_ex_password;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsPasswordPolicy)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsPasswordPolicy)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnUsehexdigits();
  afx_msg void OnUselowercase();
  afx_msg void OnUseuppercase();
  afx_msg void OnUsedigits();
  afx_msg void OnUsesymbols();
  afx_msg void OnEasyVision();
  afx_msg void OnMakePronounceable();
  afx_msg BOOL OnKillActive();
  afx_msg void OnRandom();
  afx_msg void OnCopyPassword();
  afx_msg void OnENChangePassword();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  BOOL Validate();

  void do_nohex(const bool bNonHex); // bNonHex == true enable non-hex
  void do_easyorpronounceable(const bool bSet); // bSet == true enable one of these options
  // number of checkboxes & lengths disabled when hex chosen
  enum {N_NOHEX = 6, N_HEX_LENGTHS = 4};
  static const UINT nonHex[N_NOHEX]; // IDs of said checkboxes
  static const UINT LenTxts[N_HEX_LENGTHS*2]; // IDs of text associated with length
  static const UINT nonHexLengths[N_HEX_LENGTHS]; // IDs of said lengths
  static const UINT nonHexLengthSpins[N_HEX_LENGTHS]; // IDs of said lengths' spinboxes
  int m_save[N_NOHEX]; // save cb's state when disabling hex
  UINT m_savelen[N_HEX_LENGTHS];

  bool m_bFromOptions;  // True if called by Options, false if called from GeneratePassword
};
