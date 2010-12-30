/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"
#include "AddEdit_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/ItemData.h"
#include "core/PWSprefs.h"

IMPLEMENT_DYNAMIC(CAddEdit_PropertySheet, CPWPropertySheet)

CAddEdit_PropertySheet::CAddEdit_PropertySheet(UINT nID, CWnd* pParent,
                                               PWScore *pcore, 
                                               CItemData *pci_original, CItemData *pci,
                                               const StringX currentDB)
  : CPWPropertySheet(nID, pParent), m_bIsModified(false), m_bChanged(false),
  m_bNotesChanged(false)
{
  m_AEMD.uicaller = nID;

  ASSERT(pParent != NULL);
  ASSERT(pcore != NULL);
  ASSERT(pci != NULL);

  m_AEMD.pDbx = static_cast<DboxMain *>(pParent);
  m_AEMD.pcore = pcore;
  m_AEMD.pci_original = pci_original;
  m_AEMD.pci = pci;

  m_AEMD.currentDB = currentDB;

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
    m_AEMD.realnotes = m_AEMD.originalrealnotesTRC = L"";
    m_AEMD.URL = L"";
    m_AEMD.email = L"";

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

    // Attributes
    m_AEMD.ucprotected = m_AEMD.olducprotected = 0;
  } else {
    SetupInitialValues();
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

BOOL CAddEdit_PropertySheet::OnInitDialog()
{
  CPropertySheet::OnInitDialog();

  // Change the Window title for Edit/View
  switch (m_AEMD.uicaller) {
    case IDS_ADDENTRY:
      break;
    case IDS_VIEWENTRY:
    case IDS_EDITENTRY:
      {
      CString cs_title;
      StringX sx_group(L""), sx_title, sx_user(L"");
      if (!m_AEMD.pci->IsGroupEmpty())
        sx_group = m_AEMD.pci->GetGroup();
      sx_title = m_AEMD.pci->GetTitle();
      if (!m_AEMD.pci->IsUserEmpty())
        sx_user = m_AEMD.pci->GetUser();

      // Set up and pass Propertysheet caption showing entry being edited/viewed
      cs_title.Format(m_AEMD.uicaller, sx_group.c_str(), sx_title.c_str(), sx_user.c_str());
      SetWindowText(cs_title);
      break;
      }
    default:
      ASSERT(0);
  }

  // Change the Control ID for the Apply button so that we get it
  switch (m_AEMD.uicaller) {
    case IDS_ADDENTRY:
      break;
    case IDS_VIEWENTRY:
      GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
      GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
      break;
    case IDS_EDITENTRY:
      GetDlgItem(ID_APPLY_NOW)->SetDlgCtrlID(IDC_AEAPPLY);
      GetDlgItem(IDOK)->EnableWindow(m_bChanged ? TRUE : FALSE);
      GetDlgItem(IDC_AEAPPLY)->EnableWindow(m_bChanged ? TRUE : FALSE);
      break;
  }
  return TRUE;
}

void CAddEdit_PropertySheet::SetChanged(const bool bChanged)
{
  if (m_bChanged != bChanged) {
    GetDlgItem(IDOK)->EnableWindow(bChanged ? TRUE : FALSE);
    if (m_AEMD.uicaller == IDS_EDITENTRY)
      GetDlgItem(IDC_AEAPPLY)->EnableWindow(bChanged ? TRUE : FALSE);
  }
  m_bChanged = bChanged;
}

BOOL CAddEdit_PropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // There is no OnOK for classes derived from CPropertySheet,
  // so we make our own!
  const int iCID = LOWORD(wParam);
  if (iCID == IDOK || iCID == IDC_AEAPPLY) {
    // First send a message to all loaded pages using base class function.
    // We want them all to update their variables in the Master Data area.
    // And call OnApply() rather than the default OnOK processing
    // Note: This message is only sent to PropertyPages that have been
    // loaded - i.e. the user has selected to view them, since obviously
    // the user would not have changed their values if not displayed. Duh!
    if (SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CPWPropertyPage::PP_UPDATE_VARIABLES, 0L) != 0)
      return TRUE;

    time_t t;
    bool bIsPSWDModified;
    short iDCA;

    switch (m_AEMD.uicaller) {
      case IDS_EDITENTRY:
        // Make as View entry if protected
        if (m_AEMD.ucprotected == m_AEMD.olducprotected &&
            m_AEMD.olducprotected != 0)
          break;

        m_AEMD.pci->GetDCA(iDCA);
        // Check if modified
        m_bIsModified = (m_AEMD.group       != m_AEMD.pci->GetGroup()      ||
                         m_AEMD.title       != m_AEMD.pci->GetTitle()      ||
                         m_AEMD.username    != m_AEMD.pci->GetUser()       ||
                         m_AEMD.realnotes   != m_AEMD.originalrealnotesTRC ||
                         m_AEMD.URL         != m_AEMD.pci->GetURL()        ||
                         m_AEMD.autotype    != m_AEMD.pci->GetAutoType()   ||
                         m_AEMD.runcommand  != m_AEMD.pci->GetRunCommand() ||
                         m_AEMD.DCA         != iDCA                        ||
                         m_AEMD.email       != m_AEMD.pci->GetEmail()      ||
                         m_AEMD.PWHistory   != m_AEMD.pci->GetPWHistory()  ||
                         m_AEMD.locXTime    != m_AEMD.oldlocXTime          ||
                         m_AEMD.XTimeInt    != m_AEMD.oldXTimeInt          ||
                         m_AEMD.ipolicy     != m_AEMD.oldipolicy           ||
                        (m_AEMD.ipolicy     == SPECIFIC_POLICY &&
                         m_AEMD.pwp         != m_AEMD.oldpwp)              ||
                         m_AEMD.ucprotected != m_AEMD.olducprotected);

        bIsPSWDModified = (m_AEMD.realpassword != m_AEMD.oldRealPassword);

        if (m_bIsModified) {
          // Just modify all - even though only 1 may have actually been modified
          m_AEMD.pci->SetGroup(m_AEMD.group);
          m_AEMD.pci->SetTitle(m_AEMD.title);
          m_AEMD.pci->SetUser(m_AEMD.username.IsEmpty() ?
                                   m_AEMD.defusername : m_AEMD.username);
          if (m_bNotesChanged)
            m_AEMD.pci->SetNotes(m_AEMD.realnotes);

          m_AEMD.pci->SetURL(m_AEMD.URL);
          m_AEMD.pci->SetAutoType(m_AEMD.autotype);
          m_AEMD.pci->SetPWHistory(m_AEMD.PWHistory);

          if (m_AEMD.ipolicy == DEFAULT_POLICY)
            m_AEMD.pci->SetPWPolicy(L"");
          else
            m_AEMD.pci->SetPWPolicy(m_AEMD.pwp);

          m_AEMD.oldipolicy = m_AEMD.ipolicy;
          m_AEMD.oldpwp = m_AEMD.pwp;

          m_AEMD.pci->SetRunCommand(m_AEMD.runcommand);
          m_AEMD.pci->SetDCA(m_AEMD.DCA);
          m_AEMD.pci->SetEmail(m_AEMD.email);
          m_AEMD.pci->SetProtected(m_AEMD.ucprotected != 0);
        }

        if (m_AEMD.XTimeInt > 0 && m_AEMD.XTimeInt <= 3650)
          m_AEMD.pci->SetXTimeInt(m_AEMD.XTimeInt);

        if (bIsPSWDModified || m_AEMD.locXTime != m_AEMD.oldlocXTime) {
          CItemData *pciA(m_AEMD.pci);
          if (m_AEMD.pci->IsAlias()) {
            pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
          }

          if (bIsPSWDModified) {
            m_AEMD.pci->UpdatePassword(m_AEMD.realpassword);
            m_AEMD.locPMTime = pciA->GetPMTimeL();
          }

          if (m_AEMD.locXTime != m_AEMD.oldlocXTime) {
            pciA->SetXTime(m_AEMD.tttXTime);
            m_AEMD.locXTime = pciA->GetXTimeL();
            m_AEMD.oldlocXTime = m_AEMD.locXTime;
          }
        }

        if (m_bIsModified && !bIsPSWDModified) {
          time(&t);
          m_AEMD.pci->SetRMTime(t);
        }

        if (m_bIsModified)
          SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CPWPropertyPage::PP_UPDATE_TIMES, 0L);

        m_bIsModified = m_bIsModified || bIsPSWDModified;
        break;

      case IDS_ADDENTRY:
        m_bIsModified = true;
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
        m_AEMD.pci->SetEmail(m_AEMD.email);
        m_AEMD.pci->SetProtected(m_AEMD.ucprotected != 0);

        time(&t);
        m_AEMD.pci->SetCTime(t);

        if (m_AEMD.XTimeInt > 0 && m_AEMD.XTimeInt <= 3650)
          m_AEMD.pci->SetXTimeInt(m_AEMD.XTimeInt);

        if (m_AEMD.SavePWHistory == TRUE)
          m_AEMD.pci->SetPWHistory(MakePWHistoryHeader(TRUE, m_AEMD.MaxPWHistory, 0));

        if (m_AEMD.ibasedata > 0) {
          // Password in alias format AND base entry exists
          // No need to check if base is an alias as already done in
          // call to PWScore::ParseBaseEntryPWD
          m_AEMD.pci->SetPassword(L"[Alias]");
          m_AEMD.pci->SetAlias();
          ItemListIter iter = m_AEMD.pcore->Find(m_AEMD.base_uuid);
          if (iter != m_AEMD.pDbx->End())
            m_AEMD.pDbx->UpdateEntryImages(iter->second);
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

        if (m_bIsModified)
          SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CPWPropertyPage::PP_UPDATE_TIMES, 0L);
        break;
      case IDS_VIEWENTRY:
        // No Update
        break;
      default:
        ASSERT(0);
        break;
    }

    // Now end it all so that OnApply isn't called again
    if (iCID == IDOK) {
      // Just end it
      CPWPropertySheet::EndDialog(IDOK);
    } else {
      // Send message to DboxMain to update entry
      m_AEMD.pDbx->SendMessage(PWS_MSG_EDIT_APPLY, (WPARAM)this, NULL);

      // Now make the original equal to new intermediate state
      *(m_AEMD.pci_original) = *(m_AEMD.pci);

      // Now Reset starting values
      SetupInitialValues();
      SetChanged(false);
    }
    m_AEMD.entrysize = m_AEMD.pci->GetSize();
    m_pp_datetimes->UpdateStats();
    return TRUE;
  }

  return CPWPropertySheet::OnCommand(wParam, lParam);
}

BOOL CAddEdit_PropertySheet::PreTranslateMessage(MSG* pMsg) 
{
  // In View mode, there is no 'Cancel' button and 'OK' is renamed 'Close'
  // Make Escape key still work as designed
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
    if (m_AEMD.uicaller == IDS_VIEWENTRY || m_AEMD.ucprotected != 0) {
      CPWPropertySheet::EndDialog(IDCANCEL);
      return TRUE;
    } else {
      // Ask user whether to allow Escape to cancel
      CPWPropertyPage *pp = (CPWPropertyPage *)GetActivePage();
      BOOL brc = pp->OnQueryCancel();
      if (brc == TRUE)
        CPWPropertySheet::EndDialog(IDCANCEL);
      
      return TRUE;
    }
  }

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    CAddEdit_PropertyPage *pp = (CAddEdit_PropertyPage *)GetActivePage();
    pp->PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CPWPropertySheet::PreTranslateMessage(pMsg);
}

void CAddEdit_PropertySheet::SetupInitialValues()
{
  // Basic Data
  m_AEMD.entrysize = m_AEMD.pci->GetSize();
  m_AEMD.pci->GetUUID(m_AEMD.entry_uuid);
  m_AEMD.group = m_AEMD.pci->GetGroup();
  m_AEMD.title = m_AEMD.pci->GetTitle();
  m_AEMD.username = m_AEMD.pci->GetUser();
  m_AEMD.realpassword = m_AEMD.oldRealPassword = m_AEMD.pci->GetPassword();
  m_AEMD.realnotes = m_AEMD.originalrealnotesTRC = m_AEMD.pci->GetNotes();
  m_AEMD.URL = m_AEMD.pci->GetURL();
  m_AEMD.email = m_AEMD.pci->GetEmail();
  m_AEMD.pci->GetProtected(m_AEMD.ucprotected);
  m_AEMD.olducprotected = m_AEMD.ucprotected;

  if (m_AEMD.realnotes.GetLength() > MAXTEXTCHARS) {
    // Limit the Notes field to what can be displayed
    m_AEMD.realnotes =  m_AEMD.realnotes.Left(MAXTEXTCHARS);
    m_AEMD.originalrealnotesTRC = m_AEMD.realnotes;
    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
    cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

  // Entry type initialisation
  m_AEMD.original_entrytype = m_AEMD.pci->GetEntryType();

  const CItemData *pciA(m_AEMD.pci);
  if (m_AEMD.pci->IsAlias()) {
    pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
  }

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

  // Note different pci depending on if Alias
  m_AEMD.locXTime = pciA->GetXTimeL();
  if (m_AEMD.locXTime.IsEmpty()) {
    m_AEMD.locXTime.LoadString(IDS_NEVER);
    m_AEMD.tttXTime = 0;
  } else {
    pciA->GetXTime(m_AEMD.tttXTime);
  }
  m_AEMD.oldlocXTime = m_AEMD.locXTime;

  // Note different pci depending on if Alias
  pciA->GetXTimeInt(m_AEMD.XTimeInt);
  m_AEMD.oldXTimeInt = m_AEMD.XTimeInt;

  // PWHistory fields
  // Note different pci depending on if Alias
  size_t num_err;
  m_AEMD.PWHistory = pciA->GetPWHistory();

  BOOL HasHistory = CreatePWHistoryList(m_AEMD.PWHistory,
                                        m_AEMD.MaxPWHistory,
                                        num_err,
                                        m_AEMD.pwhistlist,
                                        TMC_EXPORT_IMPORT) ? TRUE : FALSE;
  m_AEMD.oldNumPWHistory = m_AEMD.NumPWHistory = m_AEMD.pwhistlist.size();
  m_AEMD.oldSavePWHistory = m_AEMD.SavePWHistory = HasHistory;

  // PWPolicy fields
  // Note different pci depending on if Alias
  pciA->GetPWPolicy(m_AEMD.pwp);

  m_AEMD.ipolicy = (pciA->GetPWPolicy().empty()) ?
                             DEFAULT_POLICY : SPECIFIC_POLICY;
  m_AEMD.oldipolicy = m_AEMD.ipolicy;

  if (m_AEMD.ipolicy == DEFAULT_POLICY) {
    m_AEMD.pwp = m_AEMD.default_pwp;
  }

  m_AEMD.oldpwp = m_AEMD.pwp;

  // Set up dependents
  uuid_array_t original_uuid = {'\0'}, original_base_uuid = {'\0'};
  CItemData::EntryType entrytype = m_AEMD.pci_original->GetEntryType();

  m_AEMD.pci_original->GetUUID(original_uuid);  // Edit doesn't change this!
  if (m_AEMD.pci_original->IsBase()) {
    UUIDVector dependentslist;
    StringX csDependents(L"");

    m_AEMD.pcore->GetAllDependentEntries(original_uuid, dependentslist,
                                  m_AEMD.pci_original->IsAliasBase() ?
                                  CItemData::ET_ALIAS : CItemData::ET_SHORTCUT);
    size_t num_dependents = dependentslist.size();
    if (num_dependents > 0) {
      m_AEMD.pcore->SortDependents(dependentslist, csDependents);
    }

    m_AEMD.num_dependents = (int)num_dependents;
    m_AEMD.original_entrytype = entrytype;
    m_AEMD.dependents = CSecString(csDependents);
    dependentslist.clear();
  } else
  if (m_AEMD.pci_original->IsAlias()) {
    // Get corresponding base entry
    const CItemData *pbci = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci_original);
    ASSERT(pbci != NULL);
    if (pbci != NULL) {
      pbci->GetUUID(original_base_uuid);
      memcpy(m_AEMD.base_uuid, original_base_uuid, sizeof(uuid_array_t));
      CSecString cs_base = L"[" +
                           pbci->GetGroup() + L":" +
                           pbci->GetTitle() + L":" +
                           pbci->GetUser()  + L"]";
      m_AEMD.base = cs_base;
      m_AEMD.original_entrytype = CItemData::ET_ALIAS;
    }
  } // IsAlias
}
