/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
  CAddEdit_PropertySheet(UINT nID, CWnd* pDbx, PWScore *pcore, CItemData *pci,
                         const StringX currentDB = L"");
  ~CAddEdit_PropertySheet();
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

  DECLARE_DYNAMIC(CAddEdit_PropertySheet)

  // Password Policy related stuff
  enum {DEFAULT_POLICY = 0, SPECIFIC_POLICY};

  // Get/Set routines needed by DboxMain Add & Edit
  CSecString &GetGroup() {return m_AEMD.group;}
  void SetGroup(StringX group) {m_AEMD.group = CSecString(group);}

  CSecString &GetUsername() {return m_AEMD.username;}
  void SetUsername(StringX username) {m_AEMD.username = CSecString(username);}
  void SetDefUsername(StringX defusername) {m_AEMD.defusername = CSecString(defusername);}

  void SetNumDependents(int &num_dependents) {m_AEMD.num_dependents = num_dependents;}
  void SetDependents(StringX dependents) {m_AEMD.dependents = CSecString(dependents);}

  enum CItemData::EntryType &GetOriginalEntrytype() {return m_AEMD.original_entrytype;}
  void SetOriginalEntrytype(enum CItemData::EntryType original_entrytype)
  {m_AEMD.original_entrytype = original_entrytype;}

  CSecString &GetBase() {return m_AEMD.base;}
  void SetBase(CSecString base) {m_AEMD.base = base;}

  int &GetIBasedata() {return m_AEMD.ibasedata;}
  uuid_array_t &GetBaseUUID() {return m_AEMD.base_uuid;}

protected:
  st_AE_master_data m_AEMD;

private:
  CAddEdit_Basic           *m_pp_basic;
  CAddEdit_Additional      *m_pp_additional;
  CAddEdit_DateTimes       *m_pp_datetimes;
  CAddEdit_PasswordPolicy  *m_pp_pwpolicy;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
