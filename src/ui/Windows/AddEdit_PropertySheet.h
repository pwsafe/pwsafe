/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class PWScore;
class DboxMain;
class ItemData;

#include "PWPropertySheet.h"
#include "AddEdit_PropertyPage.h"
#include "AddEdit_Basic.h"
#include "AddEdit_Additional.h"
#include "AddEdit_DateTimes.h"
#include "AddEdit_PasswordPolicy.h"
#include "SecString.h"

class CAddEdit_PropertySheet : public CPWPropertySheet
{
public:
  CAddEdit_PropertySheet(UINT nID, CWnd* pDbx, PWScore *pcore, 
                         CItemData *pci_original, CItemData *pci,
                         const StringX currentDB = L"");
  ~CAddEdit_PropertySheet();

  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_DYNAMIC(CAddEdit_PropertySheet)

  // Get/Set routines needed by DboxMain Add & Edit
  CSecString &GetGroup() {return m_AEMD.group;}
  void SetGroup(StringX group) {m_AEMD.group = CSecString(group);}

  CSecString &GetUsername() {return m_AEMD.username;}
  void SetUsername(StringX username) {m_AEMD.username = CSecString(username);}
  void SetDefUsername(StringX defusername) {m_AEMD.defusername = CSecString(defusername);}

  CItemData::EntryType &GetOriginalEntrytype() {return m_AEMD.original_entrytype;}
  void SetOriginalEntrytype(enum CItemData::EntryType original_entrytype)
  {m_AEMD.original_entrytype = original_entrytype;}

  CSecString &GetBase() {return m_AEMD.base;}

  int &GetIBasedata() {return m_AEMD.ibasedata;}
  uuid_array_t &GetBaseUUID() {return m_AEMD.base_uuid;}

  CItemData *GetOriginalCI() {return m_AEMD.pci_original;}
  CItemData *GetNewCI() {return m_AEMD.pci;}
  PWScore *GetCore() {return m_AEMD.pcore;}

  bool IsEntryModified() {return m_bIsModified;}

  void SetChanged(const bool bChanged);
  bool IsChanged() {return m_bChanged;}
  void SetNotesChanged(const bool bChanged) {m_bNotesChanged = bChanged;}
  bool IsNotesChanged() {return m_bNotesChanged;}

protected:
  st_AE_master_data m_AEMD;

private:
  void SetupInitialValues();

  CAddEdit_Basic           *m_pp_basic;
  CAddEdit_Additional      *m_pp_additional;
  CAddEdit_DateTimes       *m_pp_datetimes;
  CAddEdit_PasswordPolicy  *m_pp_pwpolicy;

  bool m_bIsModified, m_bChanged, m_bNotesChanged;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
