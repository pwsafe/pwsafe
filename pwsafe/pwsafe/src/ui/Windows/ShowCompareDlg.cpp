/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ShowCompareDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ShowCompareDlg.h"
#include "PWHistDlg.h"

#include "core/ItemData.h"
#include "core/Util.h"
#include "core/core.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CShowCompareDlg::CShowCompareDlg(CItemData *pci, CItemData *pci_other, CWnd* pParent)
  : CPWDialog(CShowCompareDlg::IDD, pParent),
  m_pci(pci), m_pci_other(pci_other),   m_ShowIdenticalFields(BST_UNCHECKED)
{
  ASSERT(m_pci != NULL && m_pci_other != NULL);

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
}

void CShowCompareDlg::DoDataExchange(CDataExchange* pDX)
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

  // Add grid lines
  m_ListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT |
                              m_ListCtrl.GetExtendedStyle());

  // Insert List columns
  CString cs_text;
  cs_text.LoadString(IDS_SELECTFIELD);
  m_ListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_CURRENT_ENTRY);
  m_ListCtrl.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_COMPARISON_ENTRY);
  m_ListCtrl.InsertColumn(2, cs_text);

  PopulateResults(false);

  return TRUE;
}

void CShowCompareDlg::PopulateResults(const bool bShowAll)
{
  // Populate List view
  // Our preferred field order
  const int iFields[] = {
    CItemData::NAME, CItemData::PASSWORD,
    CItemData::URL, CItemData::AUTOTYPE,
    CItemData::RUNCMD, CItemData::EMAIL,
    CItemData::DCA, CItemData::SHIFTDCA,
    CItemData::PROTECTED, CItemData::SYMBOLS,
    CItemData::POLICY, CItemData::POLICYNAME,
    CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME, CItemData::XTIME, CItemData::RMTIME,
    CItemData::XTIME_INT, CItemData::PWHIST, CItemData::NOTES
  };

  // Clear out contents
  m_ListCtrl.SetRedraw(FALSE);
  m_ListCtrl.DeleteAllItems();

  const StringX sxOpenBracket(L"["), sxColon(L":"), sxCloseBracket(L"]"), sxGTU(L"[Group:Title:Username]");
  StringX sxNo, sxYes, sxGTU1, sxGTU2;
  LoadAString(sxNo, IDS_NO);
  sxNo = sxNo.substr(1);  // Remove leading ampersand
  LoadAString(sxYes, IDS_YES);
  sxYes = sxYes.substr(1);  // Remove leading ampersand

  sxGTU1 = sxOpenBracket +
             m_pci->GetGroup() + sxColon + 
             m_pci->GetTitle() + sxColon +
             m_pci->GetUser() + sxCloseBracket;
  sxGTU2 = sxOpenBracket +
             m_pci_other->GetGroup() + sxColon +
             m_pci_other->GetTitle() + sxColon +
             m_pci_other->GetUser() + sxCloseBracket;

  int iPos = 0;

  for (int j = 0; j < sizeof(iFields) / sizeof(iFields[0]); j++) {
    const int i = iFields[j];
    DWORD dw(0);
    if (i == CItemData::NAME) {
      iPos = m_ListCtrl.InsertItem(iPos, sxGTU.c_str());
      m_ListCtrl.SetItemText(iPos, 1, sxGTU1.c_str());
      m_ListCtrl.SetItemText(iPos, 2, sxGTU2.c_str());
      dw = CSCWListCtrl::REDTEXT;
      m_ListCtrl.SetItemData(iPos, dw);
      iPos++;
      continue;
    }

    stringT sFieldName = m_pci->FieldName((CItemData::FieldType)i);
    StringX sxValue1 = m_pci->GetFieldValue((CItemData::FieldType)i);
    StringX sxValue2 = m_pci_other->GetFieldValue((CItemData::FieldType)i);
    time_t t1(0), t2(0);
    short int si1, si2;

    bool bPassword = (i == CItemData::PASSWORD);

    // Always add group/title/user fields - otherwise only if different values
    // Unless user wants all fields
    if (bShowAll || sxValue1 != sxValue2) {
      iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
      m_ListCtrl.SetItemData(iPos, LVCFMT_LEFT);
      if (!m_pci->CItemData::IsTextField((unsigned char)i)) {
        switch (i) {
          case CItemData::CTIME:      /* 07 */
            m_pci->GetCTime(t1);
            m_pci_other->GetCTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::PMTIME:     /* 08 */
            m_pci->GetPMTime(t1);
            m_pci_other->GetPMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::ATIME:      /* 09 */
            m_pci->GetATime(t1);
            m_pci_other->GetATime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME:      /* 0a */
            m_pci->GetXTime(t1);
            m_pci_other->GetXTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::RMTIME:     /* 0c */
            m_pci->GetRMTime(t1);
            m_pci_other->GetRMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME_INT:  /* 11 */
          case CItemData::PROTECTED:  /* 15 */
            break;
          case CItemData::DCA:        /* 13 */
          case CItemData::SHIFTDCA:   /* 17 */
            m_pci->GetDCA(si1, i == CItemData::SHIFTDCA);
            m_pci_other->GetDCA(si2, i == CItemData::SHIFTDCA);
            sxValue1 = GetDCAString(si1, i == CItemData::SHIFTDCA);
            sxValue2 = GetDCAString(si2, i == CItemData::SHIFTDCA);
            break;
          default:
            ASSERT(0);
        }
        if (i == CItemData::CTIME  || i == CItemData::ATIME || i == CItemData::XTIME ||
            i == CItemData::PMTIME || i == CItemData::RMTIME) {
          if (t1 != 0)
            sxValue1 = PWSUtil::ConvertToDateTimeString(t1, TMC_EXPORT_IMPORT);
          if (t2 != 0)
            sxValue2 = PWSUtil::ConvertToDateTimeString(t2, TMC_EXPORT_IMPORT);
        }
        if (i == CItemData::PROTECTED) {
          sxValue1 = sxValue1.empty() ? sxNo : sxYes;
          sxValue2 = sxValue2.empty() ? sxNo : sxYes;
        }
      }
      if (i == CItemData::PWHIST) {
        size_t num_err1, num_err2, MaxPWHistory1, MaxPWHistory2;
        PWHistList pwhistlist1, pwhistlist2;
        bool status1 = CreatePWHistoryList(sxValue1,
                                      MaxPWHistory1,
                                      num_err1,
                                      pwhistlist1,
                                      TMC_EXPORT_IMPORT);
        bool status2 = CreatePWHistoryList(sxValue2,
                                      MaxPWHistory2,
                                      num_err2,
                                      pwhistlist2,
                                      TMC_EXPORT_IMPORT);

        iPos++;

        sxValue1 =  status1 ? sxYes : sxNo;
        sxValue2 =  status2 ? sxYes : sxNo;
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
        size_t maxentries = max(pwhistlist1.size(), pwhistlist2.size());
        StringX sxBlank = L" ";
        for (size_t n = 0; n < maxentries; n++) {
          m_ListCtrl.SetItemData(iPos, LVCFMT_RIGHT);
          if (n < pwhistlist1.size()) {
            sxValue1 = pwhistlist1[n].changedate + sxBlank + pwhistlist1[n].password;
          }  else
            sxValue1.clear();
          if (n < pwhistlist2.size()) {
            sxValue2 = pwhistlist2[n].changedate + sxBlank + pwhistlist2[n].password;
          } else
            sxValue2.clear();

          if (bShowAll || sxValue1 != sxValue2) {
            stringT str;
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
        if (bPassword)
          dw |= CSCWListCtrl::PASSWORDFONT;
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

CString CShowCompareDlg::GetDCAString(const int iValue, const bool isShift)
{
  UINT ui(0);
  if (iValue == -1)
    ui = m_DCA[PWSprefs::GetInstance()->GetPref(isShift ?
                                          PWSprefs::ShiftDoubleClickAction :
                                          PWSprefs::DoubleClickAction)];
  else
    ui = m_DCA[iValue];

  CString cs;
  cs.LoadString(ui);
  return cs;
}
