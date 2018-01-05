/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class PWScore;
class ItemData;

#include "PWPropertySheet.h"
#include "AddEdit_PropertyPage.h"
#include "AddEdit_Basic.h"
#include "AddEdit_Additional.h"
#include "AddEdit_DateTimes.h"
#include "AddEdit_PasswordPolicy.h"
#include "AddEdit_Attachment.h"
#include "SecString.h"

class CAddEdit_PropertySheet : public CPWPropertySheet
{
public:
  CAddEdit_PropertySheet(UINT nID, CWnd* pDbx, PWScore *pcore, 
                         CItemData *pci_original, CItemData *pci,
                         const bool bLongPPs,
                         const StringX currentDB = L"");
  ~CAddEdit_PropertySheet();

  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  //{{AFX_MSG(CAddEdit_PropertySheet)
  //}}AFX_MSG

  DECLARE_DYNAMIC(CAddEdit_PropertySheet)

  // Get/Set routines needed by DboxMain Add & Edit
  CSecString &GetGroup() {return m_AEMD.group;}
  void SetGroup(StringX group) {m_AEMD.group = CSecString(group);}

  CSecString &GetUsername() {return m_AEMD.username;}
  void SetUsername(StringX username) {m_AEMD.username = CSecString(username);}
  void SetDefUsername(StringX defusername) {m_AEMD.defusername = CSecString(defusername);}

  CItemData::EntryType &GetOriginalEntrytype() {return m_AEMD.original_entrytype;}

  const CSecString &GetBase() const {return m_AEMD.base;}

  const int GetIBasedata() const {return m_AEMD.ibasedata;}
  pws_os::CUUID &GetBaseUUID() {return m_AEMD.base_uuid;}
  pws_os::CUUID &GetOriginalkBaseUUID() { return m_AEMD.original_base_uuid; }

  const CItemData *GetOriginalCI() const {return m_AEMD.pci_original;}
  const CItemData *GetNewCI() const {return m_AEMD.pci;}
  const CItemAtt *GetAtt() const {return &m_AEMD.attachment;}
  PWScore *GetCore() const {return m_AEMD.pcore;}

  bool IsEntryModified() const {return m_bIsModified;}

  void SetChanged(const bool bChanged);
  bool IsChanged() const {return m_bChanged;}

  void SetNotesChanged(bool bNotesChanged) {m_bNotesChanged = bNotesChanged;}
  bool IsNotesChanged() const {return m_bNotesChanged;}

  void SetSymbolsChanged(bool bSymbolsChanged);
  bool IsSymbolsChanged() const {return m_bSymbolsChanged;}

protected:
  st_AE_master_data m_AEMD;

  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

  DECLARE_MESSAGE_MAP()

private:
  void SetupInitialValues();
  BOOL OnApply(const int &iCID);

  CAddEdit_Basic           *m_pp_basic;
  CAddEdit_Additional      *m_pp_additional;
  CAddEdit_DateTimes       *m_pp_datetimes;
  CAddEdit_PasswordPolicy  *m_pp_pwpolicy;
  CAddEdit_Attachment      *m_pp_attachment;

  bool m_bIsModified, m_bChanged, m_bNotesChanged, m_bSymbolsChanged;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
