/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasswordPolicyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPasswordPolicyDlg dialog
#include "PWDialog.h"
#include "ControlExtns.h"
#include "TBMStatic.h"

#include "core/coredefs.h"
#include "core/PWPolicy.h"

class CPasswordPolicyDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CPasswordPolicyDlg)

public:
  // Construction
  CPasswordPolicyDlg(UINT uicaller, CWnd *pParent, bool bLongPPs, bool bReadOnly,
                     PWPolicy &st_pp);
  ~CPasswordPolicyDlg();

  void SetPolicyData(CString &cs_policyname, PSWDPolicyMap &MapPSWDPLC);
  void GetPolicyData(PWPolicy &st_pp, CString &cs_policyname, PSWDPolicyMap &MapPSWDPLC)
  {st_pp = m_st_default_pp; cs_policyname = m_policyname; MapPSWDPLC = m_MapPSWDPLC;}
 
protected:
  // Dialog Data
  //{{AFX_DATA(CPasswordPolicyDlg)
  enum { IDD = IDD_PASSWORDPOLICY, IDD_SHORT = IDD_PASSWORDPOLICY_SHORT };

  enum { EVPR_NONE = 0, EVPR_EV = 1, EVPR_PR = 2 };
 
  CSymbolEdit m_SymbolsEdit;
  CEdit m_PolicyNameEdit;
  CComboBox m_cbxPolicyNames;
  CStaticExtn m_stcMessage;

  UINT m_uicaller;
  BOOL m_PWUseLowercase, m_oldPWUseLowercase;
  BOOL m_PWUseUppercase, m_oldPWUseUppercase;
  BOOL m_PWUseDigits, m_oldPWUseDigits;
  BOOL m_PWUseSymbols, m_oldPWUseSymbols;
  BOOL m_PWUseHexdigits, m_oldPWUseHexdigits;
  BOOL m_PWEasyVision, m_oldPWEasyVision;
  BOOL m_PWMakePronounceable, m_oldPWMakePronounceable;
  BOOL m_UseNamedPolicy, m_oldUseNamedPolicy;
  int m_PWDefaultLength, m_oldPWDefaultLength;
  int m_PWDigitMinLength, m_oldPWDigitMinLength;
  int m_PWLowerMinLength, m_oldPWLowerMinLength;
  int m_PWSymbolMinLength, m_oldPWSymbolMinLength;
  int m_PWUpperMinLength, m_oldPWUpperMinLength;
  //}}AFX_DATA

  PWPolicy m_default_pwp;
  CSecString m_password;
  CSecEditExtn m_ex_password;

  CTBMStatic m_Help1;
  
  CString m_Symbols, m_oldSymbols;
  CString m_policyname, m_oldpolicyname;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CPasswordPolicyDlg)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CPasswordPolicyDlg)
  afx_msg void OnOK();
  afx_msg void OnCancel();
  afx_msg void OnHelp();
  afx_msg void OnUseHexdigits();
  afx_msg void OnUseLowerCase();
  afx_msg void OnUseUpperCase();
  afx_msg void OnUseDigits();
  afx_msg void OnUseSymbols();
  afx_msg void OnEasyVision();
  afx_msg void OnMakePronounceable();
  afx_msg void OnGeneratePassword();
  afx_msg void OnCopyPassword();
  afx_msg void OnENChangePassword();
  afx_msg void OnENChangePolicyName();
  afx_msg void OnNamesComboChanged();
  afx_msg void OnUseNamedPolicy();
  afx_msg void OnENOwnSymbols();
  afx_msg void OnSymbolReset();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  BOOL Validate();
  bool UpdatePasswordPolicy();
  void SetSpecificPolicyControls(const BOOL bEnable);
  void UnselectNamedPolicy();
  
  void do_hex(const bool bNonHex); // bNonHex == true enable non-hex
  void do_easyorpronounceable(const int iSet); // iSet == 0 none, 1 - EasyVision, 2 - Pronounceable
  void do_reset_symbols(bool restore_defaults);
  
  enum UseX {USELOWER = 0, USEUPPER = 1, USEDIGITS = 2, USESYM = 3};
  void do_useX(UseX x); // used by OnUse{LowerCase,UpperCase,Digits,Symbols}

  // This must correspond to the order in the following UINT arrays
  enum {SAVE_LOWERCASE = 0, SAVE_UPPERCASE, SAVE_DIGITS, SAVE_SYMBOLS, SAVE_EASYVISION, SAVE_PRONOUNCEABLE};

  // number of checkboxes & lengths disabled when hex chosen
  enum {N_NOHEX = 6, N_HEX_LENGTHS = 4};
  static const UINT nonHex[N_NOHEX]; // IDs of said checkboxes
  BOOL *m_pnonHex[N_NOHEX]; // Addresses of said checkbox variables
  static const UINT LenTxts[N_HEX_LENGTHS * 2]; // IDs of text associated with length
  static const UINT nonHexLengths[N_HEX_LENGTHS]; // IDs of said lengths
  static const UINT nonHexLengthSpins[N_HEX_LENGTHS]; // IDs of said lengths' spinboxes
  int m_save[N_NOHEX]; // save cb's state when disabling hex
  UINT m_savelen[N_HEX_LENGTHS];

  std::wstring m_PolicyName;
  PWPolicy m_st_default_pp;
  PSWDPolicyMap m_MapPSWDPLC;
  PSWDPolicyMapIter m_iter;

  bool m_bReadOnly;
  CButton *m_pCopyBtn;

  bool m_bLongPPs;
  CBitmap m_CopyPswdBitmap, m_DisabledCopyPswdBitmap;
  BOOL m_bImageLoaded, m_bDisabledImageLoaded;
  bool m_bCopyPasswordEnabled;
};
