/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"
#include "AddEdit_PropertySheet.h"

#include "corelib/ItemData.h"
#include "corelib/PWSprefs.h"

IMPLEMENT_DYNAMIC(CAddEdit_PropertySheet, CPWPropertySheet)

CAddEdit_PropertySheet::CAddEdit_PropertySheet(UINT nID, CWnd* pParent,
                                               PWScore *pcore, CItemData *pci,
                                               const StringX currentDB)
  : CPWPropertySheet(nID, pParent)
{
  m_AEMD.uicaller = nID;

  ASSERT(pParent != NULL);
  ASSERT(pcore != NULL);
  ASSERT(pci != NULL);

  m_AEMD.pDbx = static_cast<DboxMain *>(pParent);
  m_AEMD.pcore = pcore;
  m_AEMD.pci = pci;

  m_AEMD.currentDB = currentDB;

  size_t num_err;

  PWSprefs *prefs = PWSprefs::GetInstance();

  m_AEMD.default_pwp.Empty();
  if (prefs->GetPref(PWSprefs::PWUseLowercase))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (prefs->GetPref(PWSprefs::PWUseUppercase))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (prefs->GetPref(PWSprefs::PWUseDigits))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (prefs->GetPref(PWSprefs::PWUseSymbols))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (prefs->GetPref(PWSprefs::PWUseHexDigits))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (prefs->GetPref(PWSprefs::PWUseEasyVision))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (prefs->GetPref(PWSprefs::PWMakePronounceable))
    m_AEMD.default_pwp.flags |= PWSprefs::PWPolicyMakePronounceable;

  m_AEMD.default_pwp.length = prefs->GetPref(PWSprefs::PWDefaultLength);
  m_AEMD.default_pwp.digitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
  m_AEMD.default_pwp.lowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
  m_AEMD.default_pwp.symbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
  m_AEMD.default_pwp.upperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);

  // Set up data used by all Property Pages, as appropriate
  if (m_AEMD.uicaller == IDS_ADDENTRY) {
    // Basic initialisation
    m_AEMD.group = L"";
    m_AEMD.title = L"";
    m_AEMD.username = L"";
    m_AEMD.realpassword = L"";
    m_AEMD.realnotes = L"";
    m_AEMD.URL = L"";

    // Entry type initialisation
    m_AEMD.original_entrytype = CItemData::ET_NORMAL;

    // Additional initialisation
    m_AEMD.autotype = L"";
    m_AEMD.runcommand = L"";
    m_AEMD.oldDCA = m_AEMD.DCA = -1;

    // Date & Time initialisation
    m_AEMD.locCTime.LoadString(IDS_NA);
    m_AEMD.locXTime = m_AEMD.locATime = m_AEMD.locRMTime = m_AEMD.locPMTime = 
          m_AEMD.oldlocXTime = m_AEMD.locCTime;
    m_AEMD.tttXTime = m_AEMD.tttCPMTime = (time_t)0;
    m_AEMD.oldXTimeInt = m_AEMD.XTimeInt = 0;

    // PWHistory initialisation
    m_AEMD.SavePWHistory = m_AEMD.oldSavePWHistory =
            PWSprefs::GetInstance()->GetPref(PWSprefs::SavePasswordHistory) ? TRUE : FALSE;
    m_AEMD.MaxPWHistory = m_AEMD.oldMaxPWHistory =
            PWSprefs::GetInstance()->GetPref(PWSprefs::NumPWHistoryDefault);
    m_AEMD.NumPWHistory = m_AEMD.oldNumPWHistory = 0;

    // PWPolicy fields
    m_AEMD.pwp = m_AEMD.oldpwp = m_AEMD.default_pwp;
    m_AEMD.ipolicy = m_AEMD.oldipolicy = DEFAULT_POLICY;
  } else {
    // Basic Data
    m_AEMD.group = m_AEMD.pci->GetGroup();
    m_AEMD.title = m_AEMD.pci->GetTitle();
    m_AEMD.username = m_AEMD.pci->GetUser();
    m_AEMD.realpassword = m_AEMD.oldRealPassword = m_AEMD.pci->GetPassword();
    m_AEMD.realnotes = m_AEMD.pci->GetNotes();
    m_AEMD.URL = m_AEMD.pci->GetURL();

    // Entry type initialisation
    m_AEMD.original_entrytype = m_AEMD.pci->GetEntryType();

    // Additional data
    m_AEMD.autotype = m_AEMD.pci->GetAutoType();
    m_AEMD.runcommand = m_AEMD.pci->GetRunCommand();
    m_AEMD.pci->GetDCA(m_AEMD.DCA);
    m_AEMD.oldDCA = m_AEMD.DCA;

    // Date Time fields
    m_AEMD.locCTime = m_AEMD.pci->GetCTimeL();
    if (m_AEMD.locCTime.IsEmpty())
      m_AEMD.locCTime.LoadString(IDS_NA);

    m_AEMD.locATime = m_AEMD.pci->GetATimeL();
    if (m_AEMD.locATime.IsEmpty())
      m_AEMD.locATime.LoadString(IDS_NA);

    m_AEMD.locRMTime = m_AEMD.pci->GetRMTimeL();
    if (m_AEMD.locRMTime.IsEmpty())
      m_AEMD.locRMTime.LoadString(IDS_NA);

    m_AEMD.locPMTime = m_AEMD.pci->GetPMTimeL();
    if (m_AEMD.locPMTime.IsEmpty())
      m_AEMD.locPMTime.LoadString(IDS_NA);

    if (!m_AEMD.locPMTime.IsEmpty())
      m_AEMD.pci->GetPMTime(m_AEMD.tttCPMTime);

    if ((long)m_AEMD.tttCPMTime == 0L) // if never changed - try creation date
      m_AEMD.pci->GetCTime(m_AEMD.tttCPMTime);

    m_AEMD.locXTime = m_AEMD.pci->GetXTimeL();
    if (m_AEMD.locXTime.IsEmpty()) {
      m_AEMD.locXTime.LoadString(IDS_NEVER);
      m_AEMD.tttXTime = 0;
    } else
      m_AEMD.pci->GetXTime(m_AEMD.tttXTime);

    m_AEMD.oldlocXTime = m_AEMD.locXTime;

    m_AEMD.pci->GetXTimeInt(m_AEMD.XTimeInt);
    m_AEMD.oldXTimeInt = m_AEMD.XTimeInt;

    // PWHistory fields
    m_AEMD.PWHistory = m_AEMD.pci->GetPWHistory();
    BOOL HasHistory = CreatePWHistoryList(m_AEMD.PWHistory,
                                          m_AEMD.MaxPWHistory,
                                          num_err,
                                          m_AEMD.pwhistlist,
                                          TMC_EXPORT_IMPORT) ? TRUE : FALSE;
    m_AEMD.oldNumPWHistory = m_AEMD.NumPWHistory = m_AEMD.pwhistlist.size();
    m_AEMD.oldSavePWHistory = m_AEMD.SavePWHistory = HasHistory;

    // PWPolicy fields
    m_AEMD.pci->GetPWPolicy(m_AEMD.pwp);
    m_AEMD.ipolicy = (m_AEMD.pci->GetPWPolicy().empty()) ?
                               DEFAULT_POLICY : SPECIFIC_POLICY;
    m_AEMD.oldipolicy = m_AEMD.ipolicy;

    if (m_AEMD.ipolicy == DEFAULT_POLICY) {
      m_AEMD.pwp = m_AEMD.default_pwp;
    }

    m_AEMD.oldpwp = m_AEMD.pwp;
  }

  // Only now allocate the PropertyPages - after all data there
  // to be used by their c'tors
  m_pp_basic      = new CAddEdit_Basic(this, &m_AEMD);
  m_pp_additional = new CAddEdit_Additional(this, &m_AEMD);
  m_pp_datetimes  = new CAddEdit_DateTimes(this, &m_AEMD);
  m_pp_pwpolicy   = new CAddEdit_PasswordPolicy(this, &m_AEMD);

  AddPage(m_pp_basic);
  AddPage(m_pp_additional);
  AddPage(m_pp_datetimes);
  AddPage(m_pp_pwpolicy);
}

CAddEdit_PropertySheet::~CAddEdit_PropertySheet()
{
  delete m_pp_basic;
  delete m_pp_additional;
  delete m_pp_datetimes;
  delete m_pp_pwpolicy;
}

BOOL CAddEdit_PropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // There is no OnOK for classes derived from CPropertySheet,
  // so we make our own!
  if (LOWORD(wParam) == IDOK) {
    // First send a message to all loaded pages using base class
    // function.
    // We want them all to update their variables in the Master Data area.
    // And call OnApply() rather than the default OnOK processing
    // Note: This message is only sent to PropertyPages that have been
    // loaded - i.e. the user has selected to view them, since obviously
    // the user would not have changed their values if not displayed. Duh!
    if (SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CAddEdit_PropertyPage::PP_UPDATE_VARIABLES, 0L) != 0)
      return TRUE;

    time_t t;
    switch (m_AEMD.uicaller) {
      case IDS_EDITENTRY:
        bool bIsModified, bIsPSWDModified;
        short iDCA;
        m_AEMD.pci->GetDCA(iDCA);
        // Check if modified
        bIsModified = (m_AEMD.group       != m_AEMD.pci->GetGroup()      ||
                       m_AEMD.title       != m_AEMD.pci->GetTitle()      ||
                       m_AEMD.username    != m_AEMD.pci->GetUser()       ||
                       m_AEMD.realnotes   != m_AEMD.pci->GetNotes()      ||
                       m_AEMD.URL         != m_AEMD.pci->GetURL()        ||
                       m_AEMD.autotype    != m_AEMD.pci->GetAutoType()   ||
                       m_AEMD.runcommand  != m_AEMD.pci->GetRunCommand() ||
                       m_AEMD.DCA         != iDCA                        ||
                       m_AEMD.PWHistory   != m_AEMD.pci->GetPWHistory()  ||
                       m_AEMD.locXTime    != m_AEMD.oldlocXTime          ||
                       m_AEMD.XTimeInt    != m_AEMD.oldXTimeInt          ||
                       m_AEMD.ipolicy     != m_AEMD.oldipolicy           ||
                      (m_AEMD.ipolicy     == SPECIFIC_POLICY &&
                       m_AEMD.pwp         != m_AEMD.oldpwp));

        bIsPSWDModified = m_AEMD.realpassword != m_AEMD.oldRealPassword;

        if (bIsModified) {
          // Just modify all - even though only 1 may have actually been modified
          m_AEMD.pci->SetGroup(m_AEMD.group);
          m_AEMD.pci->SetTitle(m_AEMD.title);
          m_AEMD.pci->SetUser(m_AEMD.username.IsEmpty() ?
                                   m_AEMD.defusername : m_AEMD.username);
          m_AEMD.pci->SetNotes(m_AEMD.realnotes);
          m_AEMD.pci->SetURL(m_AEMD.URL);
          m_AEMD.pci->SetAutoType(m_AEMD.autotype);
          m_AEMD.pci->SetPWHistory(m_AEMD.PWHistory);

          if (m_AEMD.ipolicy == DEFAULT_POLICY)
            m_AEMD.pci->SetPWPolicy(L"");
          else
            m_AEMD.pci->SetPWPolicy(m_AEMD.pwp);

          m_AEMD.pci->SetRunCommand(m_AEMD.runcommand);
          m_AEMD.pci->SetDCA(m_AEMD.DCA);
        }

        if (bIsPSWDModified) {
          m_AEMD.pci->UpdatePassword(m_AEMD.realpassword);
        }

        if (bIsModified && !bIsPSWDModified) {
          time(&t);
          m_AEMD.pci->SetRMTime(t);
        }

        if (m_AEMD.oldlocXTime != m_AEMD.locXTime)
          m_AEMD.pci->SetXTime(m_AEMD.tttXTime);

        if (m_AEMD.oldXTimeInt != m_AEMD.XTimeInt)
          m_AEMD.pci->SetXTimeInt(m_AEMD.XTimeInt);
        break;

      case IDS_ADDENTRY:
        m_AEMD.pci->SetGroup(m_AEMD.group);
        m_AEMD.pci->SetTitle(m_AEMD.title);
        m_AEMD.pci->SetUser(m_AEMD.username.IsEmpty() ?
                                 m_AEMD.defusername : m_AEMD.username);
        m_AEMD.pci->SetPassword(m_AEMD.realpassword);
        m_AEMD.pci->SetNotes(m_AEMD.realnotes);
        m_AEMD.pci->SetURL(m_AEMD.URL);
        m_AEMD.pci->SetAutoType(m_AEMD.autotype);
        m_AEMD.pci->SetRunCommand(m_AEMD.runcommand);
        m_AEMD.pci->SetDCA(m_AEMD.DCA);

        time(&t);
        m_AEMD.pci->SetCTime(t);

        if (m_AEMD.XTimeInt > 0 && m_AEMD.XTimeInt <= 3650)
          m_AEMD.pci->SetXTimeInt(m_AEMD.XTimeInt);

        if (m_AEMD.SavePWHistory == TRUE)
          m_AEMD.pci->SetPWHistory(MakePWHistoryHeader(TRUE, m_AEMD.MaxPWHistory, 0));

        if (m_AEMD.ibasedata > 0) {
          // Password in alias format AND base entry exists
          // No need to check if base is an alias as already done in
          // call to PWScore::GetBaseEntry
          uuid_array_t alias_uuid;
          m_AEMD.pci->GetUUID(alias_uuid);
          m_AEMD.pcore->AddDependentEntry(m_AEMD.base_uuid, alias_uuid, CItemData::ET_ALIAS);
          m_AEMD.pci->SetPassword(L"[Alias]");
          m_AEMD.pci->SetAlias();
          ItemListIter iter = m_AEMD.pcore->Find(m_AEMD.base_uuid);
          if (iter != m_AEMD.pDbx->End()) {
            const CItemData &cibase = iter->second;
            DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
            int nImage = m_AEMD.pDbx->GetEntryImage(cibase);
            m_AEMD.pDbx->SetEntryImage(di->list_index, nImage, true);
            m_AEMD.pDbx->SetEntryImage(di->tree_item, nImage, true);
          }
        } else {
          m_AEMD.pci->SetPassword(m_AEMD.realpassword);
          m_AEMD.pci->SetNormal();
        }

        if (m_AEMD.pci->IsAlias()) {
          m_AEMD.pci->SetXTime((time_t)0);
          m_AEMD.pci->SetPWPolicy(L"");
        } else {
          m_AEMD.pci->SetXTime(m_AEMD.tttXTime);
          if (m_AEMD.ipolicy == DEFAULT_POLICY)
            m_AEMD.pci->SetPWPolicy(L"");
          else
            m_AEMD.pci->SetPWPolicy(m_AEMD.pwp);
        }
        break;
      case IDS_VIEWENTRY:
        // No Update
        break;
      default:
        ASSERT(0);
        break;
    }
    // Now end it all so that OnApply isn't called again
    CPWPropertySheet::EndDialog(IDOK);
    return TRUE;
  }
  return CPWPropertySheet::OnCommand(wParam, lParam);
}
