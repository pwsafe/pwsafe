/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_PasswordPolicy.h : header file
//

#pragma once

#include "AddEdit_PropertyPage.h"
#include "ControlExtns.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_PasswordPolicy dialog

class CAddEdit_PasswordPolicy : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_PasswordPolicy)

  CAddEdit_PasswordPolicy(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_PasswordPolicy();

  // Dialog Data
  //{{AFX_DATA(CAddEdit_PasswordPolicy)
  enum { IDD = IDD_ADDEDIT_PASSWORDPOLICY,
         IDD_SHORT = IDD_ADDEDIT_PASSWORDPOLICY_SHORT };

  CSymbolEdit m_symbols;
  CComboBoxExtn m_cbxPolicyNames;

  BOOL m_pwuselowercase;
  BOOL m_pwuseuppercase;
  BOOL m_pwusedigits;
  BOOL m_pwusesymbols;
  BOOL m_pwusehexdigits;
  BOOL m_pweasyvision;
  BOOL m_pwmakepronounceable;

  int m_pwdefaultlength;
  int m_pwdigitminlength;
  int m_pwlowerminlength;
  int m_pwsymbolminlength;
  int m_pwupperminlength;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_PasswordPolicy)
protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_PasswordPolicy)
  virtual BOOL OnInitDialog();
  afx_msg void OnHelp();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM );

  afx_msg void OnChanged();

  afx_msg void OnUseHexdigits();
  afx_msg void OnUseLowerCase();
  afx_msg void OnUseUpperCase();
  afx_msg void OnUseDigits();
  afx_msg void OnUseSymbols();
  afx_msg void OnEasyVision();
  afx_msg void OnMakePronounceable();
  afx_msg void OnSelectNamedPolicy();
  afx_msg void OnSetSpecificPWPolicy();
  afx_msg void OnOwnSymbolsChanged();
  afx_msg void OnNamesComboChanged();
  afx_msg void OnSymbolReset();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetPolicyControls();
  void SetPolicyFromVariables();
  void SetVariablesFromPolicy();
  void DisablePolicy();
  bool ValidatePolicy(CWnd *&pFocus); // is policy self-consistent?

  void do_hex(const bool bHex);                 // bHex == true enable hex
  void do_easyorpronounceable(const bool bSet); // bSet == true enable one of these options
  enum UseX {USELOWER = 0, USEUPPER = 1, USEDIGITS = 2, USESYM = 3};
  void do_useX(UseX x); // used by OnUse{LowerCase,UpperCase,Digits,Symbols}
  // number of checkboxes & lengths disabled when hex chosen
  enum {N_NOHEX = 6, N_HEX_LENGTHS = 4};
  static const UINT nonHex[N_NOHEX];                  // IDs of said checkboxes
  static const UINT LenTxts[N_HEX_LENGTHS * 2];       // IDs of text associated with length
  static const UINT nonHexLengths[N_HEX_LENGTHS];     // IDs of said lengths
  static const UINT nonHexLengthSpins[N_HEX_LENGTHS]; // IDs of said lengths' spinboxes

  // 2nd idex: 0 = pronounceable; 1 = hex
  BOOL m_save_enabled[N_HEX_LENGTHS][2];   // Save when disabling hex/pronounceable
  BOOL m_save_visible[N_HEX_LENGTHS];   // Save when disabling hex/pronounceable
  bool m_bInitdone;

  int m_policy_radibtn;  // Can't use M_policy() anymore

};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
