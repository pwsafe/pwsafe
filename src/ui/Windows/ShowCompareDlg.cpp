/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ShowCompareDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ShowCompareDlg.h"
#include "DboxMain.h"
#include "InfoDisplay.h"

#include "core/ItemData.h"
#include "core/Util.h"
#include "core/core.h"
#include "core/PWHistory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CShowCompareDlg::CShowCompareDlg(CItemData *pci, CItemData *pci_other, CWnd *pParent,
                                 const bool bDifferentDB)
  : CPWDialog(CShowCompareDlg::IDD, pParent),
  m_pci(pci), m_pci_other(pci_other), m_ShowIdenticalFields(BST_UNCHECKED),
  m_pNotesDisplay(NULL), m_bDifferentDB(bDifferentDB)
{
  ASSERT(m_pci != NULL && m_pci_other != NULL && pParent != NULL);

  // Set up DCA to string values
  m_DCA.resize(PWSprefs::maxDCA + 1);

  m_DCA[PWSprefs::DoubleClickAutoType] = IDSC_DCAAUTOTYPE;
  m_DCA[PWSprefs::DoubleClickBrowse] = IDSC_DCABROWSE;
  m_DCA[PWSprefs::DoubleClickBrowsePlus] = IDSC_DCABROWSEPLUS;
  m_DCA[PWSprefs::DoubleClickCopyNotes] = IDSC_DCACOPYNOTES;
  m_DCA[PWSprefs::DoubleClickCopyPassword] = IDSC_DCACOPYPASSWORD;
  m_DCA[PWSprefs::DoubleClickCopyPasswordMinimize] = IDSC_DCACOPYPASSWORDMIN;
  m_DCA[PWSprefs::DoubleClickCopyUsername] = IDSC_DCACOPYUSERNAME;
  m_DCA[PWSprefs::DoubleClickViewEdit] = IDSC_DCAVIEWEDIT;
  m_DCA[PWSprefs::DoubleClickRun] = IDSC_DCARUN;
  m_DCA[PWSprefs::DoubleClickSendEmail] = IDSC_DCASENDEMAIL;
}

CShowCompareDlg::~CShowCompareDlg()
{
  // Don't do the following, as CInfoDisplay::PostNcDestroy()
  // does 'delete this' (ugh).
  // delete m_pNotesDisplay;
}

void CShowCompareDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ITEMLIST, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CShowCompareDlg, CPWDialog)
  ON_BN_CLICKED(IDC_SHOW_IDENTICAL_FIELDS, OnShowIdenticalFields)
END_MESSAGE_MAP()

BOOL CShowCompareDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_ListCtrl.Initialize();

  // Add grid lines
  m_ListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT |
                              m_ListCtrl.GetExtendedStyle());
  m_ListCtrl.UpdateRowHeight(false);// Update height because LVS_EX_GRIDLINES style changed
  // Insert List columns
  CString cs_text;
  cs_text.LoadString(IDS_SELECTFIELD);
  m_ListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_CURRENT_ENTRY);
  m_ListCtrl.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_COMPARISON_ENTRY);
  m_ListCtrl.InsertColumn(2, cs_text);

  PopulateResults(false);

  m_pNotesDisplay = new CInfoDisplay;
  if (!m_pNotesDisplay->Create(0, 0, L"", this)) {
    // failed
    delete m_pNotesDisplay;
    m_pNotesDisplay = NULL;
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

CString ConvertKeyBoardShortcut(int32 &iKBShortcut)
{
  CString kbs(L"");
  if (iKBShortcut != 0) {
    CString cs_temp;
    WORD wVirtualKeyCode = iKBShortcut & 0xff;
    WORD wPWSModifiers = iKBShortcut >> 16;

    if (iKBShortcut != 0) {
      if (wPWSModifiers & PWS_HOTKEYF_ALT) {
        cs_temp.LoadString(IDS_ALTP);
        kbs += cs_temp;
      }
      if (wPWSModifiers & PWS_HOTKEYF_CONTROL) {
        cs_temp.LoadString(IDS_CTRLP);
        kbs += cs_temp;
      }
      if (wPWSModifiers & PWS_HOTKEYF_SHIFT) {
        cs_temp.LoadString(IDS_SHIFTP);
        kbs += cs_temp;
      }
      cs_temp.Format(L"%c", wVirtualKeyCode);
      kbs += cs_temp;
    }
  }
  return kbs;
}

void CShowCompareDlg::PopulateResults(bool bShowAll)
{
  // Populate List view
  // Our preferred field order
  const int iFields[] = {
    CItemData::NAME,                        // Special processing
    CItemData::PASSWORD,                    // Special processing
    CItemData::ENTRYTYPE,                   // Special processing
    CItemData::URL, CItemData::AUTOTYPE,
    CItemData::RUNCMD, CItemData::EMAIL,
    CItemData::DCA, CItemData::SHIFTDCA,
    CItemData::PROTECTED, CItemData::SYMBOLS,
    CItemData::POLICY, CItemData::POLICYNAME, CItemData::KBSHORTCUT, CItemData::ATTREF,
    CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME, CItemData::XTIME,
    CItemData::RMTIME, CItemData::XTIME_INT, CItemData::PWHIST, CItemData::NOTES
  };

  // Check we have considered all user fields
  // Too convoluted to change this process to a switch statement where the compiler can
  // produce an error if any of the CItemData::FieldType enum values is missing!

  // Exclude 5: UUID/GROUP/TITLE/USERNAME & RESERVED (01,02,03,04,0B) but
  // Include 1: ENTRYTYPE
  // The developer will still need to ensure new fields are processed below
  // Put in compilation check as this may not be regression tested every time
  static_assert((sizeof(iFields) / sizeof(iFields[0]) == (CItem::LAST_USER_FIELD - 5 + 1)),
    "Check user comparison items - there are some missing! They must be before LAST_USER_FIELD");

  StringX sxDefPolicyStr;
  LoadAString(sxDefPolicyStr, IDSC_DEFAULT_POLICY);

  // Clear out contents
  m_ListCtrl.SetRedraw(FALSE);
  m_ListCtrl.DeleteAllItems();

  const StringX sxOpenBracket(L"["), sxColon(L":"), sxCloseBracket(L"]"),
          sxGTU(L"[Group:Title:Username]");
  StringX sxNo, sxYes, sxGTU1, sxGTU2, sxGTUBase1, sxGTUBase2;
  LoadAString(sxNo, IDS_NO);
  sxNo = sxNo.substr(1);    // Remove leading ampersand
  LoadAString(sxYes, IDS_YES);
  sxYes = sxYes.substr(1);  // Remove leading ampersand
  CItemData *pci(m_pci), *pci_other(m_pci_other);
  CItemData *pci_base(NULL), *pci_other_base(NULL);

  sxGTU1 = sxOpenBracket +
             m_pci->GetGroup() + sxColon + 
             m_pci->GetTitle() + sxColon +
             m_pci->GetUser() + sxCloseBracket;
  sxGTU2 = sxOpenBracket +
             m_pci_other->GetGroup() + sxColon +
             m_pci_other->GetTitle() + sxColon +
             m_pci_other->GetUser() + sxCloseBracket;

  if (m_pci->IsAlias() || m_pci->IsShortcut()) {
    pci_base = GetMainDlg()->GetBaseEntry(m_pci);
    sxGTUBase1 = sxOpenBracket +
               pci_base->GetGroup() + sxColon + 
               pci_base->GetTitle() + sxColon +
               pci_base->GetUser() + sxCloseBracket;
    // If shortcut - use base entry for everything
    if (m_pci->IsShortcut())
      pci = pci_base;
  }
  if (m_pci_other->IsAlias() || m_pci_other->IsShortcut()) {
    pci_other_base = GetMainDlg()->GetBaseEntry(m_pci_other);
    sxGTUBase2 = sxOpenBracket +
               pci_other_base->GetGroup() + sxColon + 
               pci_other_base->GetTitle() + sxColon +
               pci_other_base->GetUser() + sxCloseBracket;
    // If shortcut - use base entry for everything
    if (m_pci_other->IsShortcut())
      pci_other = pci_other_base;
  }

  int iPos = 0;

  for (int j = 0; j < sizeof(iFields) / sizeof(iFields[0]); j++) {
    const int i = iFields[j];
    DWORD dw(0);

    if (i == CItemData::NAME) {
      // Special processing - put in [g:t:u]
      iPos = m_ListCtrl.InsertItem(iPos, sxGTU.c_str());
      m_ListCtrl.SetItemText(iPos, 1, sxGTU1.c_str());
      m_ListCtrl.SetItemText(iPos, 2, sxGTU2.c_str());
      dw = CSCWListCtrl::REDTEXT;
      m_ListCtrl.SetItemData(iPos, dw);
      iPos++;
      continue;
    }

    if (i == CItemData::ENTRYTYPE) {
      // Special processing: NOTE TESTS ARE DONE USING ORIGINAL m_pci & m_pci_other
      /*
        Entry 1 / 2 | Normal/Base | Alias | Shortcut |
        ------------|-------------|-------|----------|
        Normal/Base |      a      |   b   |    d     |
        ------------|-------------|-------|----------|
        Alias       |      b      |   c   |    e     |
        ------------|-------------|-------|----------|
        Shortcut    |      d      |   e   |    f     |
        ----------------------------------------------
        
        a. If both are normal entries or base entries - ignore this field
        b. If one is normal/base and the other is an alias - use the base's password
           and show the alias's base [g:t:u] here
        c. If both are aliases, - use their respective base's passwords
           and show their base [g:t:u] here
        d. If one is a shortcut, use information from its base entry and show its base
           [g:t:u] here.
        e. If one is an alias and the other a shortcut, use the alias's base for
           its password, use all information from the shortcut's base entry and show
           their base [g:t:u] here.
        f. If both are shortcuts, use information from their base entries and show
           their base [g:t:u] here.
      */
      const CString cs_type(MAKEINTRESOURCE(IDS_ENTRYTYPE));
      const CString cs_et1 = GetEntryTypeString(m_pci->GetEntryType());
      const CString cs_et2 = GetEntryTypeString(m_pci_other->GetEntryType());

      CString cs_label;
      StringX sxText1, sxText2;
      bool bAddBaseGTURow(false);

      // 'a' : both normal or base entries
      // However, if both normal or same type of base, don't show unless "Show All"
      if (!bShowAll && !m_pci->IsDependent() && !m_pci_other->IsDependent() &&
          cs_et1 == cs_et2)
        continue;

      // 'b' or 'c' : 1 normal/base & 1 alias or both aliases (no shortcuts)
      // However, if both aliases of same base, don't show unless "Show All"
      if (!bShowAll && m_pci->IsAlias() && m_pci_other->IsAlias() && 
          sxGTUBase1 == sxGTUBase2)
        continue;

      if ((m_pci->IsAlias() || m_pci_other->IsAlias()) &&
          (!m_pci->IsShortcut() && !m_pci_other->IsShortcut())) {
        cs_label.LoadString(IDS_EXP_ABASE);
        sxText1 = m_pci->IsAlias() ? sxGTUBase1 : L"-";
        sxText2 = m_pci_other->IsAlias() ? sxGTUBase2 : L"-";
        bAddBaseGTURow = true;
      }

      // 'd' or 'f' : 1 shortcut & 1 normal/base or both shortcuts (no aliases)
      // However, if both shortcuts of same base, don't show unless "Show All"
      if (!bShowAll && m_pci->IsShortcut() && m_pci_other->IsShortcut() && 
          sxGTUBase1 == sxGTUBase2)
        continue;

      if ((m_pci->IsShortcut() && !m_pci_other->IsAlias()) ||
          (!m_pci->IsAlias() && m_pci_other->IsShortcut())) {
        cs_label.LoadString(IDS_EXP_SBASE);
        sxText1 = m_pci->IsShortcut() ? sxGTUBase1 : L"-";
        sxText2 = m_pci_other->IsShortcut() ? sxGTUBase2 : L"-";
        bAddBaseGTURow = true;
      }

      // 'e' : 1 shortcut & 1 alias
      if ((m_pci->IsShortcut() && m_pci_other->IsAlias()) ||
          (m_pci->IsAlias() && m_pci_other->IsShortcut())) {
        const CString cs_label1(MAKEINTRESOURCE(m_pci->IsAlias() ? IDS_EXP_ABASE : IDS_EXP_SBASE));
        const CString cs_label2(MAKEINTRESOURCE(m_pci_other->IsAlias() ? IDS_EXP_ABASE : IDS_EXP_SBASE));
        cs_label = cs_label1 + L" / " + cs_label2;
        sxText1 = sxGTUBase1;
        sxText2 = sxGTUBase2;
        bAddBaseGTURow = true;
      }

      // Show entry types
      iPos = m_ListCtrl.InsertItem(iPos, cs_type);
      m_ListCtrl.SetItemText(iPos, 1, cs_et1);
      m_ListCtrl.SetItemText(iPos, 2, cs_et2);
      if (cs_et1 != cs_et2)
        dw = CSCWListCtrl::REDTEXT;
      m_ListCtrl.SetItemData(iPos, dw);
      iPos++;

      // If required, show the base entry [g:t:u]
      if (bAddBaseGTURow) {
        iPos = m_ListCtrl.InsertItem(iPos, cs_label);
        m_ListCtrl.SetItemText(iPos, 1, sxText1.c_str());
        m_ListCtrl.SetItemText(iPos, 2, sxText2.c_str());
        if (sxText1 != sxText2)
          dw = CSCWListCtrl::REDTEXT;
        m_ListCtrl.SetItemData(iPos, dw);
        iPos++;
        continue;
      }
      continue;
    }

    // Now use pci, pci_other, which will different if either entry is a shortcut
    time_t t1(0), t2(0);
    short int si1, si2;
    StringX sxValue1, sxValue2;
    std::wstring sFieldName = pci->FieldName((CItemData::FieldType)i);

    // Get field values
    // For aliases - use base entry passwords
    if (i == CItemData::PASSWORD && m_pci->IsAlias())
      sxValue1 = pci_base->GetFieldValue((CItemData::FieldType)i);
    else
      sxValue1 = pci->GetFieldValue((CItemData::FieldType)i);

    if (i == CItemData::PASSWORD && m_pci_other->IsAlias())
      sxValue2 = pci_other_base->GetFieldValue((CItemData::FieldType)i);
    else
      sxValue2 = pci_other->GetFieldValue((CItemData::FieldType)i);

    if (i == CItemData::POLICY && m_bDifferentDB) {
      // If different databases and both policies are their respective defaults
      // If these are not the same, force the difference to be shown by making one different
      if (sxValue1.empty() && sxValue2.empty() && 
          PWSprefs::GetInstance()->GetDefaultPolicy() !=
          PWSprefs::GetInstance()->GetDefaultPolicy(m_bDifferentDB)) {
        sxValue1 = L"-";
      }
    }

    // Always add group/title/user fields - otherwise only if different values
    // Unless user wants all fields
    if (bShowAll || sxValue1 != sxValue2) {
      iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
      m_ListCtrl.SetItemData(iPos, LVCFMT_LEFT);
      if (!pci->CItemData::IsTextField((unsigned char)i)) {
        switch (i) {
          case CItemData::CTIME:      /* 0x07 */
            pci->GetCTime(t1);
            pci_other->GetCTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::PMTIME:     /* 0x08 */
            pci->GetPMTime(t1);
            pci_other->GetPMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::ATIME:      /* 0x09 */
            pci->GetATime(t1);
            pci_other->GetATime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME:      /* 0x0a */
            pci->GetXTime(t1);
            pci_other->GetXTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::RMTIME:     /* 0x0c */
            pci->GetRMTime(t1);
            pci_other->GetRMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME_INT:  /* 0x11 */
          case CItemData::PROTECTED:  /* 0x15 */
            break;
          case CItemData::ATTREF:     /* 0x1a */
            sxValue1 = pci->HasAttRef() ? sxNo : sxYes;
            sxValue2 = pci_other->HasAttRef() ? sxNo : sxYes;
            break;
          case CItemData::DCA:        /* 0x13 */
          case CItemData::SHIFTDCA:   /* 0x17 */
            pci->GetDCA(si1, i == CItemData::SHIFTDCA);
            pci_other->GetDCA(si2, i == CItemData::SHIFTDCA);
            sxValue1 = GetDCAString(si1, i == CItemData::SHIFTDCA);
            sxValue2 = GetDCAString(si2, i == CItemData::SHIFTDCA);
            break;
          case CItemData::KBSHORTCUT:  /* 0x19 */
          {
            int32 iKBShortcut;
            pci->GetKBShortcut(iKBShortcut);
            sxValue1 = ConvertKeyBoardShortcut(iKBShortcut);
            pci_other->GetKBShortcut(iKBShortcut);
            sxValue2 = ConvertKeyBoardShortcut(iKBShortcut);
            break;
          }
          default:
            ASSERT(0);
        }
        if (i == CItemData::CTIME  || i == CItemData::ATIME || i == CItemData::XTIME ||
            i == CItemData::PMTIME || i == CItemData::RMTIME) {
          if (t1 != 0)
            sxValue1 = PWSUtil::ConvertToDateTimeString(t1, PWSUtil::TMC_EXPORT_IMPORT);
          if (t2 != 0)
            sxValue2 = PWSUtil::ConvertToDateTimeString(t2, PWSUtil::TMC_EXPORT_IMPORT);
        }
        if (i == CItemData::PROTECTED) {
          sxValue1 = sxValue1.empty() ? sxNo : sxYes;
          sxValue2 = sxValue2.empty() ? sxNo : sxYes;
        }
      }
      if (i == CItemData::POLICY) {
        PWPolicy pwp1, pwp2;
        StringX sxPolicy1 = pci->GetPWPolicy();
        StringX sxPolicy2 = pci_other->GetPWPolicy();

        if (pci->GetPolicyName().empty()) {
          if (sxPolicy1.empty()) {
            pwp1 = PWSprefs::GetInstance()->GetDefaultPolicy();
          }  else {
            pci->GetPWPolicy(pwp1);
          }
          StringX sxTemp1 = pwp1.GetDisplayString();
          if (sxPolicy1.empty())
            Format(sxValue1, IDS_FORMAT_CMP_POLICY, sxTemp1.c_str(), sxDefPolicyStr.c_str());
          else
            sxValue1 = sxTemp1;
        }
        if (pci_other->GetPolicyName().empty()) {
          if (sxPolicy2.empty()) {
            pwp2 = PWSprefs::GetInstance()->GetDefaultPolicy(m_bDifferentDB);
          }  else {
            pci_other->GetPWPolicy(pwp2);
          }   
          StringX sxTemp2 = pwp2.GetDisplayString();
          if (sxPolicy2.empty())
            Format(sxValue2, IDS_FORMAT_CMP_POLICY, sxTemp2.c_str(), sxDefPolicyStr.c_str());
          else
            sxValue2 = sxTemp2;
        }
      }
      if (i == CItemData::PWHIST) {
        size_t num_err1, num_err2, MaxPWHistory1, MaxPWHistory2;
        PWHistList pwhistlist1, pwhistlist2;
        bool status1 = CreatePWHistoryList(sxValue1,
                                      MaxPWHistory1,
                                      num_err1,
                                      pwhistlist1,
                                      PWSUtil::TMC_EXPORT_IMPORT);
        bool status2 = CreatePWHistoryList(sxValue2,
                                      MaxPWHistory2,
                                      num_err2,
                                      pwhistlist2,
                                      PWSUtil::TMC_EXPORT_IMPORT);

        // If any password history value is different - it must be red
        if (sxValue1 != sxValue2)
          dw = LVCFMT_LEFT | CSCWListCtrl::REDTEXT;
        else
          dw = LVCFMT_LEFT;
        m_ListCtrl.SetItemData(iPos, dw);
        
        // Now add sub-fields
        iPos++;

        sxValue1 = status1 ? sxYes : sxNo;
        sxValue2 = status2 ? sxYes : sxNo;
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHACTIVE);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        Format(sxValue1, L"%d", MaxPWHistory1);
        Format(sxValue2, L"%d", MaxPWHistory2);
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHMAX);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        Format(sxValue1, L"%d", pwhistlist1.size());
        Format(sxValue2, L"%d", pwhistlist2.size());
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHNUM);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        LoadAString(sFieldName, IDS_PWHENTRY);
        size_t maxentries = std::max(pwhistlist1.size(), pwhistlist2.size());
        StringX sxBlank = L" ";
        for (size_t n = 0; n < maxentries; n++) {
          m_ListCtrl.SetItemData(iPos, LVCFMT_RIGHT);
          if (n < pwhistlist1.size()) {
            sxValue1 = pwhistlist1[n].changedate + sxBlank + pwhistlist1[n].password;
          }  else
            sxValue1 = L"";
          if (n < pwhistlist2.size()) {
            sxValue2 = pwhistlist2[n].changedate + sxBlank + pwhistlist2[n].password;
          } else
            sxValue2 = L"";

          if (bShowAll || sxValue1 != sxValue2) {
            std::wstring str;
            Format(str, L"%s - %d", sFieldName.c_str(), n+1);
            iPos = m_ListCtrl.InsertItem(iPos, str.c_str());
            m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
            m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
            dw = LVCFMT_RIGHT;
            if (sxValue1 != sxValue2)
              dw |= CSCWListCtrl::REDTEXT;
            m_ListCtrl.SetItemData(iPos, dw);
            iPos++;
          }
        }
      } else {
        m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
        m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
        dw = LVCFMT_LEFT;
        if (sxValue1 != sxValue2)
          dw |= CSCWListCtrl::REDTEXT;
        if (i == CItemData::PASSWORD)
          dw |= CSCWListCtrl::PASSWORDFONT;
        if (i == CItemData::NOTES)
          dw |= CSCWListCtrl::NOTES;
        m_ListCtrl.SetItemData(iPos, dw);
      }
    }
    iPos++;
  }

  CRect rectLV;
  m_ListCtrl.GetClientRect(rectLV);
  int nColWidth  = (rectLV.Width()) / 3;
  int nColWidth0 = int(0.8 * nColWidth);
  int nColWidth12 = int(1.08 * nColWidth);

  m_ListCtrl.SetColumnWidth(0, nColWidth0);
  m_ListCtrl.SetColumnWidth(1, nColWidth12);
  m_ListCtrl.SetColumnWidth(2, nColWidth12);

  m_ListCtrl.SetRedraw(TRUE);
  m_ListCtrl.Invalidate();
}

void CShowCompareDlg::OnShowIdenticalFields()
{
  m_ShowIdenticalFields = ((CButton *)GetDlgItem(IDC_SHOW_IDENTICAL_FIELDS))->GetCheck();

  PopulateResults(m_ShowIdenticalFields == BST_CHECKED);
}

CString CShowCompareDlg::GetDCAString(int iValue, bool isShift) const
{
  UINT ui(0);
  if (iValue == -1)
    ui = m_DCA[PWSprefs::GetInstance()->GetPref(isShift ?
           PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction)];
  else
    ui = m_DCA[iValue];

  CString cs(MAKEINTRESOURCE(ui));
  if (iValue == -1) {
    CString cs1(cs);
    cs.Format(IDS_APP_DEFAULT, static_cast<LPCWSTR>(cs1));
  }
  return cs;
}

CString CShowCompareDlg::GetEntryTypeString(CItemData::EntryType et) const
{
  UINT ui(IDSC_UNKNOWN);
  switch (et) {
    case CItemData::ET_NORMAL:
      ui = IDS_EXP_NORMAL;
      break;
    case CItemData::ET_ALIASBASE:
      ui = IDS_EXP_ABASE;
      break;
    case CItemData::ET_ALIAS:
      ui = IDSC_ALIAS;
      break;
    case CItemData::ET_SHORTCUTBASE:
      ui = IDS_EXP_SBASE;
      break;
    case CItemData::ET_SHORTCUT:
      ui = IDSC_SHORTCUT;
      break;
    case CItemData::ET_INVALID:
    default:
      ASSERT(0);
  }
  CString cs(MAKEINTRESOURCE(ui));
  return cs;
}

bool CShowCompareDlg::SetNotesWindow(const CPoint ptClient, const bool bVisible)
{
  CPoint ptScreen(ptClient);
  StringX sx_notes(L"");

  if (m_pNotesDisplay == NULL)
    return false;

  if (!bVisible) {
    m_pNotesDisplay->SetWindowText(sx_notes.c_str());
    m_pNotesDisplay->ShowWindow(SW_HIDE);
    return false;
  }

  m_ListCtrl.ClientToScreen(&ptScreen);

  LVHITTESTINFO lvhti;
  lvhti.pt = ptClient;
  int nItem = m_ListCtrl.SubItemHitTest(&lvhti);
  
  if (nItem == -1 ||
      (m_ListCtrl.GetItemData(nItem) & CSCWListCtrl::NOTES) != CSCWListCtrl::NOTES)
    return false;

  switch (lvhti.iSubItem) {
    case 1:
    case 2:
      sx_notes = m_ListCtrl.GetItemText(nItem, lvhti.iSubItem);
      break;
    default:
      return false;
  }

  ptScreen.y += ::GetSystemMetrics(SM_CYCURSOR) / 2; // half-height of cursor

  if (!sx_notes.empty()) {
    Replace(sx_notes, StringX(L"\r\n"), StringX(L"\n"));
    Remove(sx_notes, L'\r');
  }

  // move window
  CSecString cs_oldnotes;
  m_pNotesDisplay->GetWindowText(cs_oldnotes);
  if (LPCWSTR(cs_oldnotes) != sx_notes)
    m_pNotesDisplay->SetWindowText(sx_notes.c_str());

  m_pNotesDisplay->SetWindowPos(NULL, ptScreen.x, ptScreen.y, 0, 0,
                                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  m_pNotesDisplay->ShowWindow(!sx_notes.empty() ? SW_SHOWNA : SW_HIDE);

  return !sx_notes.empty();
}
