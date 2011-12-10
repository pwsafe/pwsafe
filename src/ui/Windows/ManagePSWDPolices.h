/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "DDStatic.h"

#include "core/StringX.h"
#include "core/coredefs.h"

#include "resource.h"

// CManagePSWDPolices dialog

class DboxMain;

class CManagePSWDPolices : public CPWDialog
{
public:
  CManagePSWDPolices(CWnd* pParent = NULL, const bool bLongPPs = true);
  virtual ~CManagePSWDPolices();

  // Dialog Data
  enum { IDD = IDD_MANAGEPASSWORDPOLICIES };
  
  PSWDPolicyMap &GetPasswordPolicies(st_PSWDPolicy &st_default_pp)
  {st_default_pp = m_st_default_pp; return m_MapPSWDPLC;}
  void GetDefaultPasswordPolicies(st_PSWDPolicy &st_default_pp)
  {st_default_pp = m_st_default_pp;}

  bool IsChanged() {return m_bChanged;}

protected:
  CListCtrl m_PolicyNames;
  CListCtrl m_PolicyDetails;
  CListCtrl m_PolicyEntries;

  CSecEditExtn m_ex_password;
  CSecString m_password;
  CStatic m_CopyPswdStatic;

  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg void OnHelp();
  afx_msg void OnCancel();
  afx_msg void OnNew();
  afx_msg void OnEdit();
  afx_msg void OnList();
  afx_msg void OnDelete();
  afx_msg void OnGeneratePassword();
  afx_msg void OnCopyPassword();
  afx_msg void OnPolicySelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnEntryDoubleClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnNameClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnEntryClick(NMHDR *pNotifyStruct, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

private:
  void UpdateNames();
  void UpdateDetails();
  void UpdateEntryList();
  static int CALLBACK SortNames(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  static int CALLBACK SortEntries(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  
  DboxMain *m_pDbx;
  CToolTipCtrl *m_pToolTipCtrl;

  PSWDPolicyMap m_MapPSWDPLC;
  st_PSWDPolicy m_st_default_pp;

  GTUSet m_setGTU;

  CBitmap m_CopyPswdBitmap;

  int m_iSortNamesIndex, m_iSortEntriesIndex;
  bool m_bSortNamesAscending, m_bSortEntriesAscending;

  int m_iSelectedItem;
  bool m_bChanged, m_bViewPolicy, m_bLongPPs;
};
