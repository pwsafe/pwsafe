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
public:
  DECLARE_DYNAMIC(COptionsPasswordPolicy)

  // Construction
  COptionsPasswordPolicy(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsPasswordPolicy();

  void SetPolicyData(st_PSWDPolicy &st_pp, CString &policyname,
                     PSWDPolicyMap &MapPSWDPLC);
  void GetPolicyData(CString &cs_policyname, PSWDPolicyMap &MapPSWDPLC)
  {cs_policyname = m_policyname; MapPSWDPLC = m_MapPSWDPLC;}
 
protected:
  // Dialog Data
  //{{AFX_DATA(COptionsPasswordPolicy)
  enum { IDD = IDD_PS_PASSWORDPOLICY, IDD_SHORT = IDD_PS_PASSWORDPOLICY_SHORT };
 
  CSymbolEdit m_SymbolsEdit;
  CEdit m_PolicyNameEdit;
  CComboBox m_cbxPolicyNames;

  BOOL m_PWUseLowercase;
  BOOL m_PWUseUppercase;
  BOOL m_PWUseDigits;
  BOOL m_PWUseSymbols;
  BOOL m_PWUseHexdigits;
  BOOL m_PWEasyVision;
  BOOL m_PWMakePronounceable;
  BOOL m_UseNamedPolicy;
  int m_PWDefaultLength;
  int m_PWDigitMinLength;
  int m_PWLowerMinLength;
  int m_PWSymbolMinLength;
  int m_PWUpperMinLength;
  //}}AFX_DATA

  PWPolicy m_default_pwp;
  CSecString m_password;
  CSecEditExtn m_ex_password;
  
  int m_UseOwnSymbols;
  CString m_Symbols;
  CString m_policyname;
  CString m_oldpolicyname;

  CButtonExtn m_chkbox[7];
  CButtonExtn m_radiobtn[2];

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsPasswordPolicy)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsPasswordPolicy)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnUseHexdigits();
  afx_msg void OnUseLowerCase();
  afx_msg void OnUseUpperCase();
  afx_msg void OnUseDigits();
  afx_msg void OnUseSymbols();
  afx_msg void OnEasyVision();
  afx_msg void OnMakePronounceable();
  afx_msg BOOL OnKillActive();
  afx_msg void OnGeneratePassword();
  afx_msg void OnCopyPassword();
  afx_msg void OnENChangePassword();
  afx_msg void OnSymbols();
  afx_msg void OnENChangePolicyName();
  afx_msg void OnNamesComboChanged();
  afx_msg void OnUseNamedPolicy();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  BOOL Validate();
  bool VerifyPolicynameUnique(CString policyname)
  {return (m_MapPSWDPLC.find((LPCWSTR)policyname) == m_MapPSWDPLC.end());}
  void UpdatePasswordPolicy();

  void do_hex(const bool bNonHex); // bNonHex == true enable non-hex
  void do_easyorpronounceable(const bool bSet); // bSet == true enable one of these options

  // This must correspond to the order in the following UINT arrays
  enum {SAVE_LOWERCASE = 0, SAVE_UPPERCASE, SAVE_DIGITS, SAVE_SYMBOLS, SAVE_EASYVISION, SAVE_PRONOUNCEABLE};

  // number of checkboxes & lengths disabled when hex chosen
  enum {N_NOHEX = 6, N_HEX_LENGTHS = 4};
  static const UINT nonHex[N_NOHEX]; // IDs of said checkboxes
  BOOL *pnonHex[N_NOHEX]; // Addresses of said checkbox variables
  static const UINT LenTxts[N_HEX_LENGTHS * 2]; // IDs of text associated with length
  static const UINT nonHexLengths[N_HEX_LENGTHS]; // IDs of said lengths
  static const UINT nonHexLengthSpins[N_HEX_LENGTHS]; // IDs of said lengths' spinboxes
  int m_save[N_NOHEX]; // save cb's state when disabling hex
  UINT m_savelen[N_HEX_LENGTHS];

  stringT m_PolicyName;
  st_PSWDPolicy m_default_st_pp;
  PSWDPolicyMap m_MapPSWDPLC;
  PSWDPolicyMapIter m_iter;
};
