/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"
#include "ControlExtns.h"
#include "corelib/MyString.h"
#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

//-----------------------------------------------------------------------------
/**
* This class is a bit schizophrenic - it has multiple personalities,
* displaying the one specified by the "index" parameter
*
* An OO purist would have a fit (mumble subclasses mumble polymorphism
* mumble sigh)...
*/

class CPasskeyEntry
  : public CPWDialog
{
  // Construction
public:
  CPasskeyEntry(CWnd* pParent,
    const CString& a_filespec, int index = 1 /* GCP_NORMAL */,
    bool bReadOnly = false,
    bool bForceReadOnly = false,
    int adv_type = -1); 

  ~CPasskeyEntry();

  int GetStatus() const {return m_status;}
  bool IsReadOnly() const {return m_PKE_ReadOnly == TRUE;}
  const CMyString &GetPasskey() const {return m_passkey;}
  const CString &GetFileName() const {return m_filespec;}
  CString m_appversion;

  bool m_bAdvanced;
  CItemData::FieldBits m_bsFields;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;
  int m_treatwhitespaceasempty;

private:
  // Dialog Data
  enum { IDD_BASIC = IDD_PASSKEYENTRY };
  enum { IDD_WEXIT = IDD_PASSKEYENTRY_WITHEXIT };
  //{{AFX_DATA(CPasskeyEntry)
  enum { IDD = IDD_PASSKEYENTRY_FIRST };
#if !defined(POCKET_PC)
  CSysColStatic m_ctlLogo;
  CSysColStatic m_ctlLogoText;
  CButton m_ctlOK;
#endif
  CSecEditExtn m_ctlPasskey;
  CMyString m_passkey;
  BOOL m_PKE_ReadOnly;
  bool m_bForceReadOnly;
  //}}AFX_DATA
  CString m_message;
  CComboBoxExtn m_MRU_combo;
  CString m_filespec, m_orig_filespec;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPasskeyEntry)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  int m_tries;
  int m_status;
  int m_index;
  int m_adv_type;

  static int dialog_lookup[5];

  HICON m_hIcon;

  // Generated message map functions
  //{{AFX_MSG(CPasskeyEntry)
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnExitAdvanced();
#if defined(POCKET_PC)
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
#endif
  //}}AFX_MSG
  afx_msg void OnCreateDb();

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnComboEditChange();
  afx_msg void OnComboSelChange();
  afx_msg void OnOpenFileBrowser();

private:
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
