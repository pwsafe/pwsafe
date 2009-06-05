/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertyPage.h"
#include "SecString.h"
#include "ControlExtns.h"
#include "corelib/ItemData.h"
#include "corelib/PWSprefs.h"
#include "corelib/PWHistory.h"

class CAddEdit_PropertySheet;

class DboxMain;
class PWScore;
class CItemData;

struct st_AE_master_data {
  UINT uicaller;   // Add, Edit or View

  PWScore *pcore;
  DboxMain *pDbx;
  CItemData *pci;  // The entry being edited

  StringX currentDB;

  // Basic related stuff
  CSecString defusername;
  CSecString group;
  CSecString title;
  CSecString username;
  CSecString realpassword;
  CSecString oldRealPassword;
  CSecString realnotes;
  CSecString URL;

  CSecString base;
  CSecString dependents;
  uuid_array_t base_uuid;
  int num_dependents;
  int ibasedata;
  enum CItemData::EntryType original_entrytype;

  // Addtitional related stuff
  CSecString autotype;
  CSecString runcommand;
  short DCA, oldDCA;

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
  int ipolicy, oldipolicy;
};

class CAddEdit_PropertyPage : public CPWPropertyPage
{
public:
  CAddEdit_PropertyPage(CWnd *pParent, UINT nID, st_AE_master_data *pAEMD);
  virtual ~CAddEdit_PropertyPage() {}
  virtual BOOL OnQueryCancel();

  enum {PP_DATA_CHANGED = 0, PP_UPDATE_VARIABLES};

  static COLORREF crefGreen, crefWhite;

  DECLARE_DYNAMIC(CAddEdit_PropertyPage)


  // inline functions to make code look 'nicer'
  inline UINT &M_uicaller() {return m_AEMD.uicaller;}

  inline PWScore * &M_pcore() {return m_AEMD.pcore;}
  inline DboxMain * &M_pDbx() {return m_AEMD.pDbx;}
  inline CItemData * &M_pci() {return m_AEMD.pci;}

  inline StringX &M_currentDB() {return m_AEMD.currentDB;}

  // Basic related stuff
  inline CSecString &M_defusername() {return m_AEMD.defusername;}
  inline CSecString &M_group() {return m_AEMD.group;}
  inline CSecString &M_title() {return m_AEMD.title;}
  inline CSecString &M_username() {return m_AEMD.username;}
  inline CSecString &M_realpassword() {return m_AEMD.realpassword;}
  inline CSecString &M_oldRealPassword() {return m_AEMD.oldRealPassword;}
  inline CSecString &M_realnotes() {return m_AEMD.realnotes;}
  inline CSecString &M_URL() {return m_AEMD.URL;}

  inline CSecString &M_base() {return m_AEMD.base;}
  inline CSecString &M_dependents() {return m_AEMD.dependents;}
  inline uuid_array_t &M_base_uuid() {return m_AEMD.base_uuid;}
  inline int &M_num_dependents() {return m_AEMD.num_dependents;}
  inline int &M_ibasedata() {return m_AEMD.ibasedata;}
  inline enum CItemData::EntryType &M_original_entrytype() {return m_AEMD.original_entrytype;}

  // Addtitional related stuff
  inline CSecString &M_autotype() {return m_AEMD.autotype;}
  inline CSecString &M_runcommand() {return m_AEMD.runcommand;}
  inline short &M_DCA() {return m_AEMD.DCA;}
  inline short &M_oldDCA() {return m_AEMD.oldDCA;}

  // Date & Time related stuff
  inline CSecString &M_locCTime() {return m_AEMD.locCTime;}
  inline CSecString &M_locPMTime() {return m_AEMD.locPMTime;}
  inline CSecString &M_locATime() {return m_AEMD.locATime;}
  inline CSecString &M_locXTime() {return m_AEMD.locXTime;}
  inline CSecString &M_locRMTime() {return m_AEMD.locRMTime;}
  inline CSecString &M_oldlocXTime() {return m_AEMD.oldlocXTime;}
  inline time_t &M_tttXTime() {return m_AEMD.tttXTime;}
  inline time_t &M_tttCPMTime() {return m_AEMD.tttCPMTime;}
  inline int &M_XTimeInt() {return m_AEMD.XTimeInt;}
  inline int &M_oldXTimeInt() {return m_AEMD.oldXTimeInt;}

  // Password History related stuff
  inline PWHistList &M_pwhistlist() {return m_AEMD.pwhistlist;}
  inline CSecString &M_PWHistory() {return m_AEMD.PWHistory;}
  inline size_t &M_NumPWHistory() {return m_AEMD.NumPWHistory;}
  inline size_t &M_oldNumPWHistory() {return m_AEMD.oldNumPWHistory;}
  inline size_t &M_MaxPWHistory() {return m_AEMD.MaxPWHistory;}
  inline size_t &M_oldMaxPWHistory() {return m_AEMD.oldMaxPWHistory;}
  inline BOOL &M_SavePWHistory() {return m_AEMD.SavePWHistory;}
  inline BOOL &M_oldSavePWHistory() {return m_AEMD.oldSavePWHistory;}

  // Password Policy
  inline PWPolicy &M_pwp() {return m_AEMD.pwp;}
  inline PWPolicy &M_oldpwp() {return m_AEMD.oldpwp;}
  inline PWPolicy &M_default_pwp() {return m_AEMD.default_pwp;}
  inline int &M_ipolicy() {return m_AEMD.ipolicy;}
  inline int &M_oldipolicy() {return m_AEMD.oldipolicy;}

protected:
  st_AE_master_data &m_AEMD;
  CAddEdit_PropertySheet *m_ae_psh;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
