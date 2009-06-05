/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_PasswordPolicy.h : header file
//

#pragma once

#include "AddEdit_PropertyPage.h"
#include "corelib/ItemData.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_PasswordPolicy dialog

class DboxMain;

class CAddEdit_PasswordPolicy : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_PasswordPolicy)

  CAddEdit_PasswordPolicy(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_PasswordPolicy();

  const TCHAR *GetHelpName() const {return _T("TO_DO!");}

  // Dialog Data
  //{{AFX_DATA(CAddEdit_PasswordPolicy)
  enum { IDD = IDD_ADDEDIT_PASSWORDPOLICY };

  BOOL m_pwuselowercase;
  BOOL m_pwuseuppercase;
  BOOL m_pwusedigits;
  BOOL m_pwusesymbols;
  BOOL m_pwusehexdigits;
  BOOL m_pweasyvision;
  BOOL m_pwmakepronounceable;
  size_t m_pwdefaultlength;
  size_t m_pwdigitminlength;
  size_t m_pwlowerminlength;
  size_t m_pwsymbolminlength;
  size_t m_pwupperminlength;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_PasswordPolicy)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_PasswordPolicy)
  virtual BOOL OnInitDialog();
  afx_msg void OnUseHexdigits();
  afx_msg void OnUseLowerCase();
  afx_msg void OnUseUpperCase();
  afx_msg void OnUseDigits();
  afx_msg void OnUseSymbols();
  afx_msg void OnEasyVision();
  afx_msg void OnMakePronounceable();
  afx_msg BOOL OnKillActive();
  afx_msg void OnSetDefaultPWPolicy();
  afx_msg void OnSetSpecificPWPolicy();
  afx_msg void OnResetPolicy();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM );
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetPolicyControls();
  void SetPolicyFromVariables();
  void SetVariablesFromPolicy();

  void do_hex(const bool bHex);                 // bHex == true enable hex
  void do_easyorpronounceable(const bool bSet); // bSet == true enable one of these options

  // number of checkboxes & lengths disabled when hex chosen
  enum {N_NOHEX = 6, N_HEX_LENGTHS = 4};
  static const UINT nonHex[N_NOHEX];                  // IDs of said checkboxes
  static const UINT LenTxts[N_HEX_LENGTHS * 2];       // IDs of text associated with length
  static const UINT nonHexLengths[N_HEX_LENGTHS];     // IDs of said lengths
  static const UINT nonHexLengthSpins[N_HEX_LENGTHS]; // IDs of said lengths' spinboxes
  BOOL m_savebool[N_NOHEX];                           // Save cb's state when disabling hex
  UINT m_savelen[N_HEX_LENGTHS];                      // Save lengths when disabling box
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
