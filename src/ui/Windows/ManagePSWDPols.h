/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "DDStatic.h"
#include "PWTouch.h"

#include "core/StringX.h"
#include "core/coredefs.h"
#include "core/PWPolicy.h"

#include "resource.h"

// PolicyNames CListCtrl
class CPNListCtrlX : public CListCtrl {
};

/**
* typedef to hide the fact that CPNListCtrl is really a mixin.
*/

typedef CPWTouch< CPNListCtrlX > CPNListCtrl;

// CManagePSWDPols dialog

class CManagePSWDPols : public CPWDialog
{
public:
  CManagePSWDPols(CWnd* pParent = NULL, const bool bLongPPs = true);
  virtual ~CManagePSWDPols();

  // Dialog Data
  enum { IDD = IDD_MANAGEPASSWORDPOLICIES };
  
  PSWDPolicyMap &GetPasswordPolicies(PWPolicy &st_default_pp)
  {st_default_pp = m_st_default_pp; return m_MapPSWDPLC;}
  void GetDefaultPasswordPolicies(PWPolicy &st_default_pp) const
  {st_default_pp = m_st_default_pp;}

  bool IsChanged() const {return m_bChanged;}

protected:
  CPNListCtrl m_PolicyNames;
  CListCtrl m_PolicyDetails;
  CListCtrl m_PolicyEntries;

  CSecEditExtn m_ex_password;
  CSecString m_password;

  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  afx_msg void OnHelp();
  afx_msg void OnCancel();
  afx_msg void OnOK();
  afx_msg void OnNew();
  afx_msg void OnEdit();
  afx_msg void OnList();
  afx_msg void OnDelete();
  afx_msg void OnGeneratePassword();
  afx_msg void OnCopyPassword();
  afx_msg void OnUndo();
  afx_msg void OnRedo();
  afx_msg void OnPolicySelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnPolicyRightClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnEntryDoubleClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnNameClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnEntryClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnListEntries();

  DECLARE_MESSAGE_MAP()

private:
  void UpdateNames();
  void UpdateDetails();
  void UpdateEntryList();

  static int CALLBACK SortNames(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  static int CALLBACK SortEntries(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  
  // History of current changes for Undo/Redo and index to current change
  // that can be undone. Note: if this is less that the size of the vector
  // of saved changes, then there are changes that can be redone.
  std::vector<st_PSWDPolicyChange> m_vchanges;
  int m_iundo_pos;

  std::vector<st_GroupTitleUser> m_ventries;
  PSWDPolicyMap m_MapPSWDPLC;
  PWPolicy m_st_default_pp;

  GTUSet m_setGTU;

  CBitmap m_CopyPswdBitmap, m_DisabledCopyPswdBitmap;
  CButton *m_pCopyBtn;

  bool m_bCopyPasswordEnabled;
  BOOL m_bImageLoaded, m_bDisabledImageLoaded;
  
  int m_iSortNamesIndex, m_iSortEntriesIndex;
  bool m_bSortNamesAscending, m_bSortEntriesAscending;

  int m_iSelectedItem;
  bool m_bChanged, m_bViewPolicy, m_bLongPPs, m_bReadOnly;
  
  bool m_bUndoShortcut, m_bRedoShortcut;
  unsigned short int m_siUndoVirtKey, m_siRedoVirtKey;
  unsigned char m_cUndoModifier, m_cRedoModifier;
};
