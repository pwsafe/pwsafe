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

// Structure for maintaining history of changes for Undo/Redo
// Change flags
enum  CPP_FLAGS {CPP_INVALID = 0, CPP_ADD = 1, CPP_DELETE = 2, CPP_MODIFIED = 4};

struct st_PSWDPolicyChange {
  StringX name;
  st_PSWDPolicy st_pp_save;
  st_PSWDPolicy st_pp_new;
  CPP_FLAGS flags;

  st_PSWDPolicyChange()
  : name(_T("")), flags(CPP_INVALID)
  {
    st_pp_save.Empty();
    st_pp_new.Empty();
  }

  st_PSWDPolicyChange(const StringX &in_name, CPP_FLAGS in_flags,
          const st_PSWDPolicy &in_st_pp_original,
          const st_PSWDPolicy &in_st_pp_new)
  : name(in_name), flags(in_flags), st_pp_save(in_st_pp_original),
  st_pp_new(in_st_pp_new)
  {}

  st_PSWDPolicyChange(const st_PSWDPolicyChange &that)
    : name(that.name), flags(that.flags), st_pp_save(that.st_pp_save),
    st_pp_new(that.st_pp_new)
  {}

  st_PSWDPolicyChange &operator=(const st_PSWDPolicyChange &that)
  {
    if (this != &that) {
      name = that.name;
      flags = that.flags;
      st_pp_save = that.st_pp_save;
      st_pp_new = that.st_pp_new;
    }
    return *this;
  }

  bool operator==(const st_PSWDPolicyChange &that) const
  {
    if (this != &that) {
      if (name           != that.name           ||
          st_pp_save != that.st_pp_save ||
          st_pp_new      != that.st_pp_new)
        return false;
    }
    return true;
  }

  bool operator!=(const st_PSWDPolicyChange &that) const
  { return !(*this == that);}

  void Empty()
  { 
    name.clear();
    flags = CPP_INVALID;
    st_pp_save.Empty();
    st_pp_new.Empty();
  }
};

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
  
  // Hisory of current changes for Undo/Redo and index to current change
  // that can be undone. Note: if this is less that the size of the vector
  // of saved changes, then there are changes that can be redone.
  std::vector<st_PSWDPolicyChange> m_vchanges;
  int m_iundo_pos;

  DboxMain *m_pDbx;
  CToolTipCtrl *m_pToolTipCtrl;

  PSWDPolicyMap m_MapPSWDPLC;
  st_PSWDPolicy m_st_default_pp;

  GTUSet m_setGTU;

  CBitmap m_CopyPswdBitmap;

  stringT m_std_symbols, m_easyvision_symbols, m_pronounceable_symbols;

  int m_iSortNamesIndex, m_iSortEntriesIndex;
  bool m_bSortNamesAscending, m_bSortEntriesAscending;

  int m_iSelectedItem;
  bool m_bChanged, m_bViewPolicy, m_bLongPPs, m_bReadOnly;
  
  bool m_bUndoShortcut, m_bRedoShortcut;
  unsigned char m_cUndoVirtKey, m_cUndoModifier, m_cRedoVirtKey, m_cRedoModifier;
};
