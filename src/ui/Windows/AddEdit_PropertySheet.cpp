/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "DboxMain.h"
#include "AddEdit_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/ItemData.h"
#include "core/PWSprefs.h"

using pws_os::CUUID;

IMPLEMENT_DYNAMIC(CAddEdit_PropertySheet, CPWPropertySheet)

CAddEdit_PropertySheet::CAddEdit_PropertySheet(UINT nID, CWnd* pParent,
                                               PWScore *pcore,
                                               CItemData *pci_original, CItemData *pci,
                                               const bool bLongPPs,
                                               const StringX currentDB)
  : CPWPropertySheet(nID, pParent, bLongPPs), m_bIsModified(false), m_bChanged(false),
  m_bNotesChanged(false), m_bSymbolsChanged(false)
{
  m_AEMD.bLongPPs = bLongPPs;
  m_AEMD.uicaller = nID;

  ASSERT(pParent != NULL);
  ASSERT(pcore != NULL);
  ASSERT(pci != NULL);

  m_AEMD.pcore = pcore;
  m_AEMD.pci_original = pci_original;
  m_AEMD.pci = pci;

  m_AEMD.currentDB = currentDB;

  m_AEMD.entry_uuid = pws_os::CUUID::NullUUID();
  m_AEMD.base_uuid = pws_os::CUUID::NullUUID();

  PWSprefs *prefs = PWSprefs::GetInstance();

  m_AEMD.default_pwp = prefs->GetDefaultPolicy();
  m_AEMD.default_symbols = prefs->GetPref(PWSprefs::DefaultSymbols);

  // Preferences min/max values
  m_AEMD.prefminPWLength = (short)prefs->GetPrefMinVal(PWSprefs::PWDefaultLength);
  m_AEMD.prefmaxPWLength = (short)prefs->GetPrefMaxVal(PWSprefs::PWDefaultLength);
  m_AEMD.prefminPWHNumber = (short)prefs->GetPrefMinVal(PWSprefs::NumPWHistoryDefault);
  m_AEMD.prefmaxPWHNumber = (short)prefs->GetPrefMaxVal(PWSprefs::NumPWHistoryDefault);

  // Set up data used by all Property Pages, as appropriate
  if (m_AEMD.uicaller == IDS_ADDENTRY) {
    // Basic initialisation
    m_AEMD.group = L"";
    m_AEMD.title = L"";
    m_AEMD.username = L"";
    m_AEMD.realpassword = L"";
    m_AEMD.lastpassword = L"";
    m_AEMD.notes = m_AEMD.originalnotesTRC = L"";
    m_AEMD.URL = L"";
    m_AEMD.email = L"";
    m_AEMD.symbols = m_AEMD.oldsymbols = L"";

    // Entry type initialisation
    m_AEMD.original_entrytype = CItemData::ET_NORMAL;

    // Additional initialisation
    m_AEMD.autotype = L"";
    m_AEMD.runcommand = L"";
    m_AEMD.oldDCA = m_AEMD.DCA = m_AEMD.oldShiftDCA = m_AEMD.ShiftDCA = -1;

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
    m_AEMD.iownsymbols = m_AEMD.ioldownsymbols = DEFAULT_SYMBOLS;
    m_AEMD.symbols = L"";
    m_AEMD.policyname = m_AEMD.oldpolicyname = L"";

    // Protected
    m_AEMD.ucprotected = 0;
    
    // Entry Keyboard shortcut
    m_AEMD.oldKBShortcut = m_AEMD.KBShortcut = 0;
  } else {
    SetupInitialValues();
  }

  // Only now allocate the PropertyPages - after all data there
  // to be used by their c'tors
  m_pp_basic      = new CAddEdit_Basic(this, &m_AEMD);
  m_pp_additional = new CAddEdit_Additional(this, &m_AEMD);
  m_pp_datetimes  = new CAddEdit_DateTimes(this, &m_AEMD);
  m_pp_pwpolicy   = new CAddEdit_PasswordPolicy(this, &m_AEMD);
  if (pcore->GetReadFileVersion() == PWSfile::V40)
    m_pp_attachment = new CAddEdit_Attachment(this, &m_AEMD);
  else
    m_pp_attachment = NULL;

  AddPage(m_pp_basic);
  AddPage(m_pp_additional);
  AddPage(m_pp_datetimes);
  AddPage(m_pp_pwpolicy);
  if (pcore->GetReadFileVersion() == PWSfile::V40)
    AddPage(m_pp_attachment);
}

CAddEdit_PropertySheet::~CAddEdit_PropertySheet()
{
  delete m_pp_basic;
  delete m_pp_additional;
  delete m_pp_datetimes;
  delete m_pp_pwpolicy;
  delete m_pp_attachment;
}

BEGIN_MESSAGE_MAP(CAddEdit_PropertySheet, CPWPropertySheet)
  //{{AFX_MSG_MAP(CAddEdit_PropertySheet)
  ON_WM_SYSCOMMAND()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAddEdit_PropertySheet::OnSysCommand(UINT nID, LPARAM lParam)
{
  const UINT nSysID = nID & 0xFFF0;

  // Cancel external editor in use for Notes
  if (nSysID == SC_CLOSE && m_pp_basic->IsNotesExternalEditorActive()) {
    CGeneralMsgBox gmb;
    CString cs_message(MAKEINTRESOURCE(IDS_CANCEL_EXT_EDITOR)),
      cs_title(MAKEINTRESOURCE(IDS_EXT_EDITOR_ACTIVE));
    INT_PTR rc = gmb.MessageBox(cs_message, cs_title, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION);
    if (rc == IDYES)
      m_pp_basic->CancelThreadWait();
    
    // Do NOT cancel the whole edit by processing the message as this will free Add/Edit Basic
    // property page whilst it is still being executed causing an application crash.
    // So just return causing the external edit to be cancelled and return passed back
    // to Add/Edit
    return;
  }

  if (nSysID == SC_CLOSE &&
      (m_AEMD.uicaller == IDS_VIEWENTRY ||
      (m_AEMD.uicaller == IDS_EDITENTRY &&  m_AEMD.ucprotected != 0))) {
    EndDialog(IDCANCEL);
    return;
  }
  CPWPropertySheet::OnSysCommand(nID, lParam);
}

BOOL CAddEdit_PropertySheet::OnInitDialog()
{
  CPWPropertySheet::OnInitDialog();

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

      // Set up and pass Property sheet caption showing entry being edited/viewed
      // If entry is protected, set to 'View' even if DB is in R/W mode
      cs_title.Format(m_AEMD.ucprotected != 0 ? IDS_PROTECTEDENTRY : m_AEMD.uicaller,
                      sx_group.c_str(), sx_title.c_str(), sx_user.c_str());
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
      GetDlgItem(ID_APPLY_NOW)->EnableWindow(m_bChanged || m_bSymbolsChanged ? TRUE : FALSE);
      break;
  }
  
  return TRUE;  // return TRUE unless you set the focus to a control
}

void CAddEdit_PropertySheet::SetSymbolsChanged(bool bSymbolsChanged)
{
  m_bSymbolsChanged = bSymbolsChanged;
  bool bChanged = m_bChanged || m_bSymbolsChanged;

  GetDlgItem(IDOK)->EnableWindow(bChanged ? TRUE : FALSE);
  if (m_AEMD.uicaller == IDS_EDITENTRY)
    GetDlgItem(ID_APPLY_NOW)->EnableWindow(bChanged ? TRUE : FALSE);
}

void CAddEdit_PropertySheet::SetChanged(const bool bChanged)
{
  // Can't be modified if protected
  if (m_AEMD.ucprotected != 0 && bChanged) {
    ASSERT(0);
    return;
  }

  if (m_bChanged != bChanged) {
    if (m_AEMD.uicaller == IDS_EDITENTRY)
      GetDlgItem(ID_APPLY_NOW)->EnableWindow(bChanged ? TRUE : FALSE);
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

      // Set up and pass Property sheet caption showing entry being edited/viewed
      // If entry is protected, set to 'View' even if DB is in R/W mode
      cs_title.Format(m_AEMD.ucprotected != 0 ? IDS_PROTECTEDENTRY : m_AEMD.uicaller,
                      sx_group.c_str(), sx_title.c_str(), sx_user.c_str());

      const CString asterisk = L"*";
      if (bChanged)
        cs_title += asterisk;
      else
        cs_title.TrimRight(asterisk[0]);

      SetWindowText(cs_title);
      break;
    }
    default:
      ASSERT(0);
    }
    m_bChanged = bChanged;
  }
}

BOOL CAddEdit_PropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // There is no OnOK for classes derived from CPropertySheet,
  // so we make our own!
  const int iCID = LOWORD(wParam);
  if (HIWORD(wParam) == BN_CLICKED && (iCID == IDOK || iCID == ID_APPLY_NOW)) {
    // Don't care what user has done if entry is protected or DB R-O.
    if (m_AEMD.ucprotected != 0 || m_AEMD.uicaller == IDS_VIEWENTRY) {
      CPWPropertySheet::EndDialog(IDOK);
      return TRUE;
    }

    BOOL brc = OnApply(iCID);
    if (brc == TRUE)
      return TRUE;
  }

  return CPWPropertySheet::OnCommand(wParam, lParam);
}

BOOL CAddEdit_PropertySheet::OnApply(const int &iCID)
{
  // First send a message to all loaded pages using base class function.
  // We want them all to update their variables in the Master Data area.
  // And call OnApply() rather than the default OnOK processing
  // Note: This message is only sent to PropertyPages that have been
  // loaded - i.e. the user has selected to view them, since obviously
  // the user would not have changed their values if not displayed. Duh!
  if (SendMessage(PSM_QUERYSIBLINGS,
              (WPARAM)CPWPropertyPage::PP_UPDATE_VARIABLES, 0L) != 0) {
    return TRUE;
  }

  time_t t;
  bool bIsPSWDModified(false);
  short iDCA, iShiftDCA;

  switch (m_AEMD.uicaller) {
    case IDS_EDITENTRY:
      // Make as View entry if protected
      if (m_AEMD.ucprotected != 0)
        break;

      m_AEMD.pci->GetDCA(iDCA);
      m_AEMD.pci->GetShiftDCA(iShiftDCA);

      // Check if modified
      m_bIsModified = (m_AEMD.group       != m_AEMD.pci->GetGroup()      ||
                       m_AEMD.title       != m_AEMD.pci->GetTitle()      ||
                       m_AEMD.username    != m_AEMD.pci->GetUser()       ||
                       m_AEMD.notes       != m_AEMD.originalnotesTRC     ||
                       m_AEMD.URL         != m_AEMD.pci->GetURL()        ||
                       m_AEMD.autotype    != m_AEMD.pci->GetAutoType()   ||
                       m_AEMD.runcommand  != m_AEMD.pci->GetRunCommand() ||
                       m_AEMD.DCA         != iDCA                        ||
                       m_AEMD.ShiftDCA    != iShiftDCA                   ||
                       m_AEMD.email       != m_AEMD.pci->GetEmail()      ||
                       m_AEMD.symbols     != m_AEMD.oldsymbols           ||
                       m_AEMD.PWHistory   != m_AEMD.pci->GetPWHistory()  ||
                       m_AEMD.locXTime    != m_AEMD.oldlocXTime          ||
                       m_AEMD.XTimeInt    != m_AEMD.oldXTimeInt          ||
                       m_AEMD.ipolicy     != m_AEMD.oldipolicy           ||
                       (m_AEMD.ipolicy    == SPECIFIC_POLICY &&
                        m_AEMD.pwp        != m_AEMD.oldpwp)              ||
                       (m_AEMD.ipolicy    == NAMED_POLICY &&
                        m_AEMD.policyname != m_AEMD.oldpolicyname)       ||
                       m_AEMD.KBShortcut  != m_AEMD.oldKBShortcut        ||
                       m_AEMD.attachment  != m_AEMD.oldattachment);

      bIsPSWDModified = (m_AEMD.realpassword != m_AEMD.oldRealPassword);

      if (m_bIsModified) {
        // Just modify all - even though only 1 may have actually been modified
        m_AEMD.pci->SetGroup(m_AEMD.group);
        m_AEMD.pci->SetTitle(m_AEMD.title);
        m_AEMD.pci->SetUser(m_AEMD.username.IsEmpty() ?
                                  m_AEMD.defusername : m_AEMD.username);
        if (m_bNotesChanged)
          m_AEMD.pci->SetNotes(m_AEMD.notes);

        m_AEMD.pci->SetURL(m_AEMD.URL);
        m_AEMD.pci->SetAutoType(m_AEMD.autotype);
        m_AEMD.pci->SetPWHistory(m_AEMD.PWHistory);
        m_AEMD.PWHistory = m_AEMD.PWHistory;
        m_AEMD.oldNumPWHistory = m_AEMD.NumPWHistory;
        m_AEMD.oldMaxPWHistory = m_AEMD.MaxPWHistory;
        m_AEMD.oldSavePWHistory = m_AEMD.SavePWHistory;

        switch (m_AEMD.ipolicy) {
          case DEFAULT_POLICY:
            m_AEMD.pci->SetPWPolicy(L"");
            m_AEMD.policyname = L"";
            m_AEMD.pci->SetPolicyName(L"");
            break;
          case NAMED_POLICY:
            m_AEMD.pci->SetPWPolicy(L"");
            m_AEMD.pci->SetPolicyName(m_AEMD.policyname);
            break;
          case SPECIFIC_POLICY:
            m_AEMD.pci->SetPWPolicy(m_AEMD.pwp);
            m_AEMD.policyname = L"";
            m_AEMD.pci->SetPolicyName(L"");
            break;
          }

        m_AEMD.oldipolicy = m_AEMD.ipolicy;
        m_AEMD.oldpwp = m_AEMD.pwp;
        m_AEMD.oldsymbols = m_AEMD.symbols;
        m_AEMD.oldpolicyname = m_AEMD.policyname;

        m_AEMD.pci->SetRunCommand(m_AEMD.runcommand);
        m_AEMD.pci->SetDCA(m_AEMD.DCA);
        m_AEMD.pci->SetShiftDCA(m_AEMD.ShiftDCA);
        m_AEMD.pci->SetEmail(m_AEMD.email);
        m_AEMD.pci->SetSymbols(m_AEMD.symbols);
        m_AEMD.pci->SetProtected(m_AEMD.ucprotected != 0);

        m_AEMD.oldKBShortcut = m_AEMD.KBShortcut;
        m_AEMD.pci->SetKBShortcut(m_AEMD.KBShortcut);

        // TODO - What if user has removed the old attachment or changed it? (Rony)
        if (m_AEMD.attachment.HasUUID()) {
          m_AEMD.pci->SetAttUUID(m_AEMD.attachment.GetUUID());
          m_AEMD.pcore->PutAtt(m_AEMD.attachment);
        } else {
          m_AEMD.pci->ClearAttUUID();
          if (m_AEMD.oldattachment.HasUUID())
            m_AEMD.pcore->RemoveAtt(m_AEMD.oldattachment.GetUUID());
        }
      } // m_bIsModified

      m_AEMD.pci->SetXTimeInt(m_AEMD.XTimeInt);

      if (bIsPSWDModified || m_AEMD.locXTime != m_AEMD.oldlocXTime) {
        CItemData *pciA(m_AEMD.pci);
        if (m_AEMD.pci->IsAlias()) {
          pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
        }

        if (bIsPSWDModified) {
          m_AEMD.pci->UpdatePassword(m_AEMD.realpassword);
          m_AEMD.locPMTime = m_AEMD.pci->GetPMTimeL();
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
      m_AEMD.pci->SetNotes(m_AEMD.notes);
      m_AEMD.pci->SetURL(m_AEMD.URL);
      m_AEMD.pci->SetAutoType(m_AEMD.autotype);
      m_AEMD.pci->SetRunCommand(m_AEMD.runcommand);
      m_AEMD.pci->SetDCA(m_AEMD.DCA);
      m_AEMD.pci->SetShiftDCA(m_AEMD.ShiftDCA);
      m_AEMD.pci->SetEmail(m_AEMD.email);
      m_AEMD.pci->SetSymbols(m_AEMD.symbols);
      m_AEMD.pci->SetProtected(m_AEMD.ucprotected != 0);
      m_AEMD.pci->SetKBShortcut(m_AEMD.KBShortcut);

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
        if (iter != GetMainDlg()->End())
          GetMainDlg()->UpdateEntryImages(iter->second);
      } else {
        m_AEMD.pci->SetPassword(m_AEMD.realpassword);
        m_AEMD.pci->SetNormal();
      }

      if (m_AEMD.pci->IsAlias()) {
        m_AEMD.pci->SetXTime((time_t)0);
        m_AEMD.pci->SetPWPolicy(L"");
      } else {
        m_AEMD.pci->SetXTime(m_AEMD.tttXTime);

        switch (m_AEMD.ipolicy) {
          case DEFAULT_POLICY:
            m_AEMD.pci->SetPWPolicy(L"");
            m_AEMD.policyname = L"";
            m_AEMD.pci->SetPolicyName(L"");
            break;
          case NAMED_POLICY:
            m_AEMD.pci->SetPWPolicy(L"");
            m_AEMD.pci->SetPolicyName(m_AEMD.policyname);
            break;
          case SPECIFIC_POLICY:
            m_AEMD.pci->SetPWPolicy(m_AEMD.pwp);
            m_AEMD.policyname = L"";
            m_AEMD.pci->SetPolicyName(L"");
            break;
        }
      }

      // TODO - Add attachment if present (Rony)

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
    GetMainDlg()->SendMessage(PWS_MSG_EDIT_APPLY, (WPARAM)this, NULL);

    // After all the commands have been executed, hopefully all fields
    // have been updated consistently (especially true for editing aliases)
    // Refresh entry from DB
    CItemData *pci(NULL);
    ItemListIter iter = m_AEMD.pcore->Find(m_AEMD.entry_uuid);

    // ASSERT should never happen as we are editing this entry!
    ASSERT(iter != m_AEMD.pcore->GetEntryEndIter());

    // Now make the original equal to new intermediate state
    *(m_AEMD.pci_original) = *(m_AEMD.pci) = *(pci = &iter->second);

    // Now Reset starting values
    SetupInitialValues();
    SetChanged(false);
  }
  m_AEMD.entrysize = m_AEMD.pci->GetSize();
  m_pp_datetimes->UpdateStats();

  // Update the password history only if the password has been changed,
  // password history is being saved and the Add_Additional property page
  // has already been shown
  if (bIsPSWDModified && m_AEMD.SavePWHistory == TRUE && m_pp_additional->HasBeenShown()) {
    m_pp_additional->UpdatePasswordHistoryLC();
  }
  return TRUE;
}

BOOL CAddEdit_PropertySheet::PreTranslateMessage(MSG *pMsg)
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
  // This is called once when the property sheet is being initialised for Add & Edit
  // However, it is called again whenever the user does OnApply during Edit

  // Basic Data
  m_AEMD.entrysize = m_AEMD.pci->GetSize();
  m_AEMD.entry_uuid = m_AEMD.pci->GetUUID();
  m_AEMD.group = m_AEMD.pci->GetGroup();
  m_AEMD.title = m_AEMD.pci->GetTitle();
  m_AEMD.username = m_AEMD.pci->GetUser();
  m_AEMD.realpassword = m_AEMD.oldRealPassword = m_AEMD.pci->GetPassword();
  m_AEMD.lastpassword = m_AEMD.pci->GetPreviousPassword();
  m_AEMD.notes = m_AEMD.originalnotesTRC = m_AEMD.pci->GetNotes();
  m_AEMD.URL = m_AEMD.pci->GetURL();
  m_AEMD.email = m_AEMD.pci->GetEmail();
  m_AEMD.symbols = m_AEMD.oldsymbols = m_AEMD.pci->GetSymbols();
  m_AEMD.ioldownsymbols = m_AEMD.symbols.IsEmpty() == TRUE ?
                            DEFAULT_SYMBOLS : OWN_SYMBOLS;
  m_AEMD.iownsymbols = m_AEMD.ioldownsymbols;
  m_AEMD.pci->GetProtected(m_AEMD.ucprotected);

  if ((!m_AEMD.pcore->IsReadOnly() && m_AEMD.ucprotected == 0) &&
      m_AEMD.notes.GetLength() > MAXTEXTCHARS) {
    // Limit the Notes field to what can be displayed in Edit mode
    m_AEMD.notes =  m_AEMD.notes.Left(MAXTEXTCHARS);
    m_AEMD.originalnotesTRC = m_AEMD.notes;
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
    if (pciA == NULL) {
      CString err(MAKEINTRESOURCE(IDS_ABASE_MISSING));
      throw err;
    }
  }

  // Additional data
  m_AEMD.autotype = m_AEMD.pci->GetAutoType();
  m_AEMD.runcommand = m_AEMD.pci->GetRunCommand();
  m_AEMD.pci->GetDCA(m_AEMD.DCA);
  m_AEMD.oldDCA = m_AEMD.DCA;
  m_AEMD.pci->GetShiftDCA(m_AEMD.ShiftDCA);
  m_AEMD.oldShiftDCA = m_AEMD.ShiftDCA;
  
  m_AEMD.pci->GetKBShortcut(m_AEMD.KBShortcut);
  m_AEMD.oldKBShortcut = m_AEMD.KBShortcut;

  // Date Time fields
  m_AEMD.locCTime = m_AEMD.pci->GetCTimeL();
  if (m_AEMD.locCTime.IsEmpty())
    m_AEMD.locCTime.LoadString(IDS_NA);

  m_AEMD.locATime = m_AEMD.pci->GetATimeL();
  if (m_AEMD.locATime.IsEmpty())
    m_AEMD.locATime.LoadString(IDS_NA);

  m_AEMD.locRMTime = m_AEMD.pci->GetRMTimeL();
  if (m_AEMD.locRMTime.IsEmpty())
    m_AEMD.locRMTime = m_AEMD.locCTime;

  m_AEMD.locPMTime = m_AEMD.pci->GetPMTimeL();
  if (m_AEMD.locPMTime.IsEmpty())
    m_AEMD.locPMTime = m_AEMD.locCTime;

  if (!m_AEMD.locPMTime.IsEmpty()) // ??? always true ???
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
  // If user changes the password of its base entry from the
  // alias, we do record them in the base entry if the user wants them.
  // For an alias, we will show its base entry's password history
  size_t num_err;
  if (m_AEMD.pci->IsAlias())
    m_AEMD.PWHistory = pciA->GetPWHistory();
  else
    m_AEMD.PWHistory = m_AEMD.pci->GetPWHistory();

  BOOL HasHistory = CreatePWHistoryList(m_AEMD.PWHistory,
                                        m_AEMD.MaxPWHistory,
                                        num_err,
                                        m_AEMD.pwhistlist,
                                        PWSUtil::TMC_EXPORT_IMPORT) ? TRUE : FALSE;
  m_AEMD.oldNumPWHistory = m_AEMD.NumPWHistory = m_AEMD.pwhistlist.size();
  m_AEMD.oldSavePWHistory = m_AEMD.SavePWHistory = HasHistory;
  if (m_AEMD.MaxPWHistory == 0)
    m_AEMD.MaxPWHistory = PWSprefs::GetInstance()->GetPref(PWSprefs::NumPWHistoryDefault);
  
  // PWPolicy fields
  // Note different pci depending on if Alias
  pciA->GetPWPolicy(m_AEMD.pwp);
  m_AEMD.policyname = m_AEMD.oldpolicyname = pciA->GetPolicyName();

  if (!m_AEMD.policyname.IsEmpty())
    m_AEMD.ipolicy = NAMED_POLICY;
  else
  if (pciA->GetPWPolicy().empty())
    m_AEMD.ipolicy = DEFAULT_POLICY;
  else
    m_AEMD.ipolicy = SPECIFIC_POLICY;

  m_AEMD.oldipolicy = m_AEMD.ipolicy;

  if (m_AEMD.ipolicy == DEFAULT_POLICY) {
    m_AEMD.pwp = m_AEMD.default_pwp;
  }

  m_AEMD.oldpwp = m_AEMD.pwp;

  // Set up dependents
  m_AEMD.base_uuid = m_AEMD.original_base_uuid = pws_os::CUUID::NullUUID();

  pws_os::CUUID original_base_uuid(pws_os::CUUID::NullUUID());

  pws_os::CUUID original_uuid = m_AEMD.pci_original->GetUUID();  // Edit doesn't change this!
  if (m_AEMD.pci->IsBase()) {
    UUIDVector dependentslist;
    std::vector<StringX> vsxDependents;

    m_AEMD.pcore->GetAllDependentEntries(original_uuid, dependentslist,
                                  m_AEMD.pci->IsAliasBase() ?
                                  CItemData::ET_ALIAS : CItemData::ET_SHORTCUT);
    
    if (!dependentslist.empty()) {
      m_AEMD.pcore->SortDependents(dependentslist, vsxDependents);
    }

    m_AEMD.vsxdependents = vsxDependents;
    dependentslist.clear();
  } else
  if (m_AEMD.pci->IsAlias()) {
    // Get corresponding base entry
    const CItemData *pbci = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
    ASSERT(pbci != NULL);
    if (pbci != NULL) {
      m_AEMD.base_uuid = m_AEMD.original_base_uuid = pbci->GetUUID();
      m_AEMD.base = L"[" +
                pbci->GetGroup() + L":" +
                pbci->GetTitle() + L":" +
                pbci->GetUser()  + L"]";
      m_AEMD.realpassword = m_AEMD.oldRealPassword = m_AEMD.base;
    }
  } // IsAlias

  // Attachment
  if (m_AEMD.pci->HasAttRef()) {
    ASSERT(m_AEMD.pcore->HasAtt(m_AEMD.pci->GetAttUUID()));
    m_AEMD.oldattachment = m_AEMD.attachment = 
      m_AEMD.pcore->GetAtt(m_AEMD.pci->GetAttUUID());
  }
}
