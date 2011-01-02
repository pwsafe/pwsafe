/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "SecString.h"
#include "core/PwsPlatform.h"
#include "PWDialog.h"
#include "AdvancedDlg.h"

//-----------------------------------------------------------------------------
/**
* This class is a bit schizophrenic - it has multiple personalities,
* displaying the one specified by the "index" parameter
*
* An OO purist would have a fit (mumble subclasses mumble polymorphism
* mumble sigh)...
*/

class DboxMain;
class CVKeyBoardDlg;

class CPasskeyEntry : public CPWDialog
{
  // Construction
public:
  CPasskeyEntry(CWnd* pParent,
                const CString& a_filespec, int index, /* GCP_NORMAL */
                bool bReadOnly, bool bForceReadOnly, bool bHideReadOnly,
                CAdvancedDlg::Type adv_type, st_SaveAdvValues *pst_SADV);

  ~CPasskeyEntry();

  int GetStatus() const {return m_status;}
  bool IsReadOnly() const {return m_PKE_ReadOnly == TRUE;}
  const CSecString &GetPasskey() const {return m_passkey;}
  const CString &GetFileName() const {return m_filespec;}
  CString m_appversion;

  CItemData::FieldBits m_bsFields;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;
  int m_treatwhitespaceasempty;
  BOOL m_bAdvanced;

protected:
  // Dialog Data
  enum { IDD_BASIC = IDD_PASSKEYENTRY };
  enum { IDD_WEXIT = IDD_PASSKEYENTRY_WITHEXIT };
  enum { IDD = IDD_PASSKEYENTRY_FIRST };

  //{{AFX_DATA(CPasskeyEntry)
#if !defined(POCKET_PC)
  CSysColStatic m_ctlLogo;
  CSysColStatic m_ctlLogoText;
  CButton m_ctlOK;
#endif
  CSecEditExtn *m_pctlPasskey;
  CSecString m_passkey;
  BOOL m_PKE_ReadOnly;
  bool m_bForceReadOnly;
  bool m_bHideReadOnly;

  //}}AFX_DATA
  CString m_message;
  CComboBoxExtn m_MRU_combo;
  CString m_filespec, m_orig_filespec;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPasskeyEntry)
  virtual void DoDataExchange(CDataExchange* pDX);
  //}}AFX_VIRTUAL

  int m_tries;
  int m_status;
  int m_index;
  CAdvancedDlg::Type m_adv_type;

  static int dialog_lookup[5];

  HICON m_hIcon;

  // Generated message map functions
  //{{AFX_MSG(CPasskeyEntry)
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnExit();
  afx_msg void OnAdvanced();
#if defined(POCKET_PC)
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
#endif
  afx_msg void OnCreateDb();
  afx_msg void OnComboEditChange();
  afx_msg void OnComboSelChange();
  afx_msg void OnOpenFileBrowser();
  afx_msg void OnVirtualKeyboard();
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;
  CAdvancedDlg *m_pAdv;

  void SetHeight(const int num);
  void UpdateRO();
  void ProcessPhrase();
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  st_SaveAdvValues *m_pst_SADV;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
