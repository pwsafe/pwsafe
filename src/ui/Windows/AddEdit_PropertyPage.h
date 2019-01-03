/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertyPage.h"
#include "SecString.h"
#include "ControlExtns.h"

#include "core/ItemData.h"
#include "core/ItemAtt.h"
#include "core/PWSprefs.h"
#include "core/PWHistory.h"

class CAddEdit_PropertySheet;

class PWScore;
class CItemData;

struct st_AE_master_data {
  bool bLongPPs;   // Long or Wide PropertyPages
  UINT uicaller;   // Add, Edit or View

  PWScore *pcore;
  CItemData *pci_original;
  CItemData *pci;  // The entry being edited

  StringX currentDB;

  // Basic related stuff
  size_t entrysize;
  CSecString defusername;
  CSecString group;
  CSecString title;
  CSecString username;
  CSecString realpassword;
  CSecString lastpassword;
  CSecString oldRealPassword;
  CSecString notes;
  CSecString originalnotesTRC;
  CSecString URL;
  CSecString email;

  CSecString base;
  std::vector<StringX> vsxdependents;
  pws_os::CUUID entry_uuid;
  pws_os::CUUID original_base_uuid;
  pws_os::CUUID base_uuid;
  int ibasedata;
  enum CItemData::EntryType original_entrytype;

  // Additional related stuff
  CSecString autotype;
  CSecString runcommand;
  short DCA, oldDCA, ShiftDCA, oldShiftDCA;
  // Preferences min/max values
  short prefminPWHNumber;
  short prefmaxPWHNumber;

  // Date & Time related stuff
  CSecString locCTime;
  CSecString locPMTime, locATime, locXTime, locRMTime;
  CSecString oldlocXTime;
  time_t tttXTime;
  time_t tttCPMTime;  // Password creation or last changed datetime
  int XTimeInt, oldXTimeInt;

  // Password History related stuff
  PWHistList pwhistlist;
  CSecString PWHistory;
  size_t NumPWHistory, oldNumPWHistory;
  size_t MaxPWHistory, oldMaxPWHistory;
  BOOL SavePWHistory, oldSavePWHistory;

  // Password Policy
  PWPolicy pwp, oldpwp, default_pwp;
  int ipolicy, oldipolicy, iownsymbols, ioldownsymbols;
  CSecString symbols;
  CSecString default_symbols;
  CSecString oldsymbols;
  CSecString policyname;
  CSecString oldpolicyname;
  // Preferences min/max values
  int prefminPWLength;
  int prefmaxPWLength;

  // Keyboard shortcut
  int KBShortcut, oldKBShortcut;

  // Attachment
  CItemAtt attachment;
  CItemAtt oldattachment;
  
  // Attributes
  unsigned char ucprotected;
};

class CAddEdit_PropertyPage : public CPWPropertyPage
{
public:
  CAddEdit_PropertyPage(CWnd *pParent, UINT nID,
                        st_AE_master_data *pAEMD);
  CAddEdit_PropertyPage(CWnd *pParent, UINT nID, UINT nID_Short,
                        st_AE_master_data *pAEMD);
  virtual ~CAddEdit_PropertyPage() {}

  virtual BOOL OnQueryCancel();

  static COLORREF crefGreen, crefWhite;

  DECLARE_DYNAMIC(CAddEdit_PropertyPage)

  // inline functions to make code look 'nicer'
  UINT &M_uicaller() {return m_AEMD.uicaller;}

  PWScore * &M_pcore() {return m_AEMD.pcore;}
  CItemData * &M_pci() {return m_AEMD.pci;}

  StringX &M_currentDB() {return m_AEMD.currentDB;}

  // Basic related stuff
  size_t &M_entrysize() {return m_AEMD.entrysize;}
  CSecString &M_defusername() {return m_AEMD.defusername;}
  CSecString &M_group() {return m_AEMD.group;}
  CSecString &M_title() {return m_AEMD.title;}
  CSecString &M_username() {return m_AEMD.username;}
  CSecString &M_realpassword() {return m_AEMD.realpassword;}
  CSecString &M_lastpassword() { return m_AEMD.lastpassword; }
  CSecString &M_oldRealPassword() {return m_AEMD.oldRealPassword;}
  CSecString &M_notes() {return m_AEMD.notes;}
  CSecString &M_originalnotesTRC() {return m_AEMD.originalnotesTRC;}
  CSecString &M_URL() {return m_AEMD.URL;}
  CSecString &M_email() {return m_AEMD.email;}
  CSecString &M_symbols() {return m_AEMD.symbols;}
  CSecString &M_oldsymbols() {return m_AEMD.oldsymbols;}
  CSecString &M_default_symbols() {return m_AEMD.default_symbols;}

  CSecString &M_base() {return m_AEMD.base;}
  std::vector<StringX> &M_vsxdependents() {return m_AEMD.vsxdependents;}
  pws_os::CUUID &M_entry_uuid() {return m_AEMD.entry_uuid;}
  pws_os::CUUID &M_base_uuid() {return m_AEMD.base_uuid;}
  pws_os::CUUID &M_original_base_uuid() { return m_AEMD.original_base_uuid; }
  int &M_ibasedata() {return m_AEMD.ibasedata;}
  CItemData::EntryType &M_original_entrytype() {return m_AEMD.original_entrytype;}

  // Additional related stuff
  CSecString &M_autotype() {return m_AEMD.autotype;}
  CSecString &M_runcommand() {return m_AEMD.runcommand;}
  short &M_DCA() {return m_AEMD.DCA;}
  short &M_oldDCA() {return m_AEMD.oldDCA;}
  short &M_ShiftDCA() {return m_AEMD.ShiftDCA;}
  short &M_oldShiftDCA() {return m_AEMD.oldShiftDCA;}
  // Preferences min/max values
  short &M_prefminPWHNumber() { return m_AEMD.prefminPWHNumber; }
  short &M_prefmaxPWHNumber() { return m_AEMD.prefmaxPWHNumber; }
  
  // Date & Time related stuff
  CSecString &M_locCTime() {return m_AEMD.locCTime;}
  CSecString &M_locPMTime() {return m_AEMD.locPMTime;}
  CSecString &M_locATime() {return m_AEMD.locATime;}
  CSecString &M_locXTime() {return m_AEMD.locXTime;}
  CSecString &M_locRMTime() {return m_AEMD.locRMTime;}
  CSecString &M_oldlocXTime() {return m_AEMD.oldlocXTime;}
  time_t &M_tttXTime() {return m_AEMD.tttXTime;}
  time_t &M_tttCPMTime() {return m_AEMD.tttCPMTime;}
  int &M_XTimeInt() {return m_AEMD.XTimeInt;}
  int &M_oldXTimeInt() {return m_AEMD.oldXTimeInt;}

  // Password History related stuff
  PWHistList &M_pwhistlist() {return m_AEMD.pwhistlist;}
  CSecString &M_PWHistory() {return m_AEMD.PWHistory;}
  size_t &M_NumPWHistory() {return m_AEMD.NumPWHistory;}
  size_t &M_oldNumPWHistory() {return m_AEMD.oldNumPWHistory;}
  size_t &M_MaxPWHistory() {return m_AEMD.MaxPWHistory;}
  size_t &M_oldMaxPWHistory() {return m_AEMD.oldMaxPWHistory;}
  BOOL &M_SavePWHistory() {return m_AEMD.SavePWHistory;}
  BOOL &M_oldSavePWHistory() {return m_AEMD.oldSavePWHistory;}
  // Preferences min/max values
  int &M_prefminPWLength() { return m_AEMD.prefminPWLength; }
  int &M_prefmaxPWLength() { return m_AEMD.prefmaxPWLength; }

  // Password Policy
  PWPolicy &M_pwp() {return m_AEMD.pwp;}
  PWPolicy &M_oldpwp() {return m_AEMD.oldpwp;}
  PWPolicy &M_default_pwp() {return m_AEMD.default_pwp;}
  int &M_ipolicy() {return m_AEMD.ipolicy;}
  int &M_oldipolicy() {return m_AEMD.oldipolicy;}
  int &M_iownsymbols() {return m_AEMD.iownsymbols;}
  int &M_ioldownsymbols() {return m_AEMD.ioldownsymbols;}
  CSecString &M_policyname() {return m_AEMD.policyname;}
  CSecString &M_oldpolicyname() {return m_AEMD.oldpolicyname;}

  // Keyboard shortcut
  int &M_KBShortcut() {return m_AEMD.KBShortcut;}
  int &M_oldKBShortcut() {return m_AEMD.oldKBShortcut;}
  
  // Attachment
  CItemAtt &M_attachment() {return m_AEMD.attachment;}
  CItemAtt &M_oldattachment() {return m_AEMD.oldattachment;}
  
  // Attributes
  unsigned char &M_protected() {return m_AEMD.ucprotected;}

protected:
  st_AE_master_data &m_AEMD;
  CAddEdit_PropertySheet *m_ae_psh;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
