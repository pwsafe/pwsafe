/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Additional.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"
#include "ThisMfcApp.h"    // For Help
#include "GeneralMsgBox.h"
#include "DboxMain.h"

#include "AddEdit_Additional.h"
#include "AddEdit_PropertySheet.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"

#include "core/core.h"
#include "resource3.h"

using pws_os::CUUID;

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Additional property page

IMPLEMENT_DYNAMIC(CAddEdit_Additional, CAddEdit_PropertyPage)

CAddEdit_Additional::CAddEdit_Additional(CWnd * pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Additional::IDD, CAddEdit_Additional::IDD_SHORT,
                          pAEMD),
  m_UseDefaultDCA(TRUE), m_UseDefaultShiftDCA(TRUE), m_ClearPWHistory(false),
  m_bSortAscending(true), m_pToolTipCtrl(NULL), m_bInitdone(false), m_iSortedColumn(-1)
{
  if (M_MaxPWHistory() == 0)
    M_MaxPWHistory() = PWSprefs::GetInstance()->
                           GetPref(PWSprefs::NumPWHistoryDefault);
}

CAddEdit_Additional::~CAddEdit_Additional()
{
  delete m_pToolTipCtrl;
}

void CAddEdit_Additional::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Additional)
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)M_autotype());
  DDX_Text(pDX, IDC_RUNCMD, (CString&)M_runcommand());

  DDX_Check(pDX, IDC_DCA_DEFAULT, m_UseDefaultDCA);
  DDX_Check(pDX, IDC_SHIFT_DCA_DEFAULT, m_UseDefaultShiftDCA);

  DDX_Control(pDX, IDC_AUTOTYPE, m_ex_autotype);
  DDX_Control(pDX, IDC_RUNCMD, m_ex_runcommand);
  DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
  DDX_Control(pDX, IDC_SHIFT_DOUBLE_CLICK_ACTION, m_shiftdblclk_cbox);

  if (M_uicaller() != IDS_ADDENTRY) {
    DDX_Control(pDX, IDC_STATIC_AUTO, m_stc_autotype);
    DDX_Control(pDX, IDC_STATIC_RUNCMD, m_stc_runcommand);
  }

  // Password History
  DDX_Control(pDX, IDC_PWHISTORY_LIST, m_PWHistListCtrl);
  DDX_Check(pDX, IDC_SAVE_PWHIST, M_SavePWHistory());
  DDX_Text(pDX, IDC_MAXPWHISTORY, M_MaxPWHistory());
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Additional, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Additional)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_EN_CHANGE(IDC_AUTOTYPE, OnChanged)
  ON_EN_CHANGE(IDC_MAXPWHISTORY, OnChanged)
  ON_EN_CHANGE(IDC_RUNCMD, OnChanged)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_AUTO, IDC_STATIC_RUNCMD, OnSTCExClicked)
  ON_BN_CLICKED(IDC_DCA_DEFAULT, OnSetDCACheck)
  ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnDCAComboChanged)
  ON_BN_CLICKED(IDC_SHIFT_DCA_DEFAULT, OnSetShiftDCACheck)
  ON_CBN_SELCHANGE(IDC_SHIFT_DOUBLE_CLICK_ACTION, OnShiftDCAComboChanged)

  // Password History
  ON_BN_CLICKED(IDC_CLEAR_PWHIST, OnClearPWHist)
  ON_BN_CLICKED(IDC_SAVE_PWHIST, OnCheckedSavePasswordHistory)
  ON_BN_CLICKED(IDC_PWH_COPY_ALL, OnPWHCopyAll)

  ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
  ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
  ON_NOTIFY(NM_CLICK, IDC_PWHISTORY_LIST, OnHistListClick)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Additional message handlers

BOOL CAddEdit_Additional::PreTranslateMessage(MSG* pMsg)
{
  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Additional::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx (0, WS_EX_CONTROLPARENT);

  CString cs_dats;
  StringX sx_dats = PWSprefs::GetInstance()->
                           GetPref(PWSprefs::DefaultAutotypeString);
  if (sx_dats.empty())
    cs_dats = DEFAULT_AUTOTYPE;
  else
    cs_dats.Format(IDS_DEFAULTAUTOTYPE, sx_dats.c_str());

  GetDlgItem(IDC_DEFAULTAUTOTYPE)->SetWindowText(cs_dats);

  if (M_uicaller() != IDS_ADDENTRY) {
    m_pToolTipCtrl = new CToolTipCtrl;
    if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
      pws_os::Trace(L"Unable To create CAddEdit_Additional Dialog ToolTip\n");
      delete m_pToolTipCtrl;
      m_pToolTipCtrl = NULL;
    } else {
      EnableToolTips();
      // Delay initial show & reshow
      int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
      m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, iTime);
      m_pToolTipCtrl->SetMaxTipWidth(300);

      CString cs_ToolTip;
      cs_ToolTip.LoadString(IDS_CLICKTOCOPYEXPAND);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_AUTO), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_RUNCMD), cs_ToolTip);

      m_pToolTipCtrl->Activate(TRUE);
    }

    m_stc_autotype.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_runcommand.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  }

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Disable normal Edit controls
    GetDlgItem(IDC_AUTOTYPE)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_RUNCMD)->SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable Checkbox
    GetDlgItem(IDC_DCA_DEFAULT)->EnableWindow(FALSE);
    GetDlgItem(IDC_DOUBLE_CLICK_ACTION)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHIFT_DCA_DEFAULT)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHIFT_DOUBLE_CLICK_ACTION)->EnableWindow(FALSE);
  }

  // For some reason, MFC calls us twice when initializing.
  // Populate the combo box only once.
  SetupDCAComboBoxes(&m_dblclk_cbox);
  SetupDCAComboBoxes(&m_shiftdblclk_cbox);

  if (M_DCA() < PWSprefs::minDCA || M_DCA() > PWSprefs::maxDCA) {
    short iDCA = (short)PWSprefs::GetInstance()->
                      GetPref(PWSprefs::DoubleClickAction);
    m_dblclk_cbox.SetCurSel(m_DCA_to_Index[iDCA]);
    m_dblclk_cbox.EnableWindow(FALSE);
    m_UseDefaultDCA = TRUE;
  } else {
    m_dblclk_cbox.SetCurSel(m_DCA_to_Index[M_DCA()]);
    m_dblclk_cbox.EnableWindow(TRUE);
    m_UseDefaultDCA = FALSE;
  }

  if (M_ShiftDCA() < PWSprefs::minDCA || M_ShiftDCA() > PWSprefs::maxDCA) {
    short iDCA = (short)PWSprefs::GetInstance()->
                      GetPref(PWSprefs::ShiftDoubleClickAction);
    m_shiftdblclk_cbox.SetCurSel(m_DCA_to_Index[iDCA]);
    m_shiftdblclk_cbox.EnableWindow(FALSE);
    m_UseDefaultShiftDCA = TRUE;
  } else {
    m_shiftdblclk_cbox.SetCurSel(m_DCA_to_Index[M_ShiftDCA()]);
    m_shiftdblclk_cbox.EnableWindow(TRUE);
    m_UseDefaultShiftDCA = FALSE;
  }

  // Password History
  M_oldMaxPWHistory() = M_MaxPWHistory();

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(M_SavePWHistory());

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXPWHISTORY));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos((int)M_MaxPWHistory());

  if (M_uicaller() == IDS_ADDENTRY) {
    GetDlgItem(IDC_CLEAR_PWHIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PWHISTORY_LIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PWH_COPY_ALL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_PWH_EDIT)->ShowWindow(SW_HIDE);
    UpdateData(FALSE);
    m_bInitdone = true;
    return TRUE;
  }

  GetDlgItem(IDC_STATIC_PWH_ADD)->ShowWindow(SW_HIDE);

  BOOL bpwh_count = M_pwhistlist().empty() ? FALSE : TRUE;
  GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(bpwh_count);
  GetDlgItem(IDC_PWHISTORY_LIST)->EnableWindow(bpwh_count);
  GetDlgItem(IDC_PWH_COPY_ALL)->EnableWindow(bpwh_count);

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);
  }

  m_PWHistListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  CString cs_text;
  cs_text.LoadString(IDS_SETDATETIME);
  m_PWHistListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_PASSWORD);
  m_PWHistListCtrl.InsertColumn(1, cs_text);

  PWHistList::iterator iter;
  DWORD nIdx;
  for (iter = M_pwhistlist().begin(), nIdx = 0;
       iter != M_pwhistlist().end(); iter++, nIdx++) {
    int nPos = 0;
    const PWHistEntry pwhentry = *iter;
    if (pwhentry.changedate != L"1970-01-01 00:00:00")
      nPos = m_PWHistListCtrl.InsertItem(nPos, pwhentry.changedate.c_str());
    else {
      cs_text.LoadString(IDS_UNKNOWN);
      cs_text.Trim();
      nPos = m_PWHistListCtrl.InsertItem(nPos, cs_text);
    }
    m_PWHistListCtrl.SetItemText(nPos, 1, pwhentry.password.c_str());
    m_PWHistListCtrl.SetItemData(nPos, nIdx);
  }

  m_PWHistListCtrl.SetRedraw(FALSE);
  for (int i = 0; i < 2; i++) {
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }
  m_PWHistListCtrl.SetRedraw(TRUE);

  wchar_t buffer[10];
#if (_MSC_VER >= 1400)
  swprintf_s(buffer, 10, L"%d", M_NumPWHistory());
#else
  swprintf(buffer, L"%d", M_NumPWHistory());
#endif

  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OLDPW1)->EnableWindow(FALSE);
  }

  UpdateData(FALSE);
  m_bInitdone = true;
  return TRUE;
}

void CAddEdit_Additional::SetupDCAComboBoxes(CComboBox *pcbox)
{
  if (pcbox->GetCount() == 0) {
    // ComboBox now sorted - no need to add in English alphabetical order
    int nIndex;
    CString cs_text;

    cs_text.LoadString(IDSC_DCAAUTOTYPE);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickAutoType);

    cs_text.LoadString(IDSC_DCABROWSE);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickBrowse);

    cs_text.LoadString(IDSC_DCABROWSEPLUS);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickBrowsePlus);

    cs_text.LoadString(IDSC_DCACOPYNOTES);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyNotes);

    cs_text.LoadString(IDSC_DCACOPYPASSWORD);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyPassword);

    cs_text.LoadString(IDSC_DCACOPYPASSWORDMIN);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyPasswordMinimize);

    cs_text.LoadString(IDSC_DCACOPYUSERNAME);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyUsername);

    cs_text.LoadString(IDSC_DCAVIEWEDIT);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickViewEdit);

    cs_text.LoadString(IDSC_DCARUN);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickRun);

    cs_text.LoadString(IDSC_DCASENDEMAIL);
    nIndex = pcbox->AddString(cs_text);
    pcbox->SetItemData(nIndex, PWSprefs::DoubleClickSendEmail);

    for (int i = 0; i < pcbox->GetCount(); i++) {
      int ival = (int)pcbox->GetItemData(i);
      m_DCA_to_Index[ival] = i;
    }
  }
}
void CAddEdit_Additional::OnChanged()
{
  if (!m_bInitdone || m_AEMD.uicaller != IDS_EDITENTRY)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/entering_pwd_add.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

HBRUSH CAddEdit_Additional::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CAddEdit_PropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC && M_uicaller() != IDS_ADDENTRY) {
    COLORREF *pcfOld;
    UINT nID = pWnd->GetDlgCtrlID();
    switch (nID) {
      case IDC_STATIC_AUTO:
        pcfOld = &m_autotype_cfOldColour;
        break;
      case IDC_STATIC_RUNCMD:
        pcfOld = &m_runcmd_cfOldColour;
        break;
      default:
        // Not one of ours - get out quick
        return hbr;
        break;
    }
    int iFlashing = ((CStaticExtn *)pWnd)->IsFlashing();
    BOOL bHighlight = ((CStaticExtn *)pWnd)->IsHighlighted();
    BOOL bMouseInWindow = ((CStaticExtn *)pWnd)->IsMouseInWindow();

    if (iFlashing != 0) {
      pDC->SetBkMode(iFlashing == 1 || (iFlashing && bHighlight && bMouseInWindow) ?
                     OPAQUE : TRANSPARENT);
      COLORREF cfFlashColour = ((CStaticExtn *)pWnd)->GetFlashColour();
      *pcfOld = pDC->SetBkColor(iFlashing == 1 ? cfFlashColour : *pcfOld);
    } else if (bHighlight) {
      pDC->SetBkMode(bMouseInWindow ? OPAQUE : TRANSPARENT);
      COLORREF cfHighlightColour = ((CStaticExtn *)pWnd)->GetHighlightColour();
      *pcfOld = pDC->SetBkColor(bMouseInWindow ? cfHighlightColour : *pcfOld);
    } else if (((CStaticExtn *)pWnd)->GetColourState()) {
      COLORREF cfUser = ((CStaticExtn *)pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }

  // Let's get out of here
  return hbr;
}

BOOL CAddEdit_Additional::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

LRESULT CAddEdit_Additional::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_SavePWHistory()   != M_oldSavePWHistory()  ||
          M_NumPWHistory()    != M_pwhistlist().size() ||
          (M_SavePWHistory()  == TRUE &&
           M_MaxPWHistory()   != M_oldMaxPWHistory()))
        return 1L;
      switch (M_uicaller()) {
        case IDS_EDITENTRY:
          if (M_autotype()    != M_pci()->GetAutoType()   ||
              M_runcommand()  != M_pci()->GetRunCommand() ||
              M_DCA()         != M_oldDCA()               ||
              M_ShiftDCA()    != M_oldShiftDCA())
            return 1L;
          break;
        case IDS_ADDENTRY:
          if (!M_autotype().IsEmpty()     ||
              !M_runcommand().IsEmpty()   ||
              M_DCA()      != M_oldDCA()  ||
              M_ShiftDCA() != M_oldShiftDCA())
            return 1L;
          break;
      }
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
      break;
  }
  return 0L;
}

BOOL CAddEdit_Additional::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  CWnd *pFocus(NULL);
  CGeneralMsgBox gmb;

  UpdateData(TRUE);
  M_autotype().EmptyIfOnlyWhiteSpace();
  M_runcommand().EmptyIfOnlyWhiteSpace();

  if (M_runcommand().GetLength() > 0) {
    //Check Run Command parses - don't substitute
    std::wstring errmsg;
    size_t st_column;
    bool bAutoType(false);
    StringX sxAutotype(L"");
    bool bURLSpecial;
    PWSAuxParse::GetExpandedString(M_runcommand(), L"", NULL,
                                   bAutoType, sxAutotype, errmsg, st_column,
                                   bURLSpecial);
    if (errmsg.length() > 0) {
      CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
      CString cs_temp(MAKEINTRESOURCE(IDS_RUN_IGNOREORFIX));
      CString cs_errmsg;
      cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
      cs_errmsg += cs_temp;
      INT_PTR rc = gmb.MessageBox(cs_errmsg, cs_title,
                           MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      if (rc == IDNO) {
        UpdateData(FALSE);
        // Are we the current page, if not activate this page
        if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
          m_ae_psh->SetActivePage(this);

        pFocus = &m_ex_runcommand;
        goto error;
      }
    }
  }

  /* Handle history header.
   *
   * Header is in the form fmmnn, where:
   * f = {0,1} if password history is on/off
   * mm = 2 digits max size of history list
   * nn = 2 digits current size of history list
   *
   * Special case: history empty and password history off - do nothing
   *
   */

  if (m_ClearPWHistory == TRUE) {
    M_pwhistlist().clear();
    M_PWHistory() = M_PWHistory().Left(5);
  }

  if (M_SavePWHistory() == TRUE &&
      (M_MaxPWHistory() < 1 || M_MaxPWHistory() > 255)) {
    gmb.AfxMessageBox(IDS_DEFAULTNUMPWH);
    pFocus = GetDlgItem(IDC_MAXPWHISTORY);
    goto error;
  }

  if (!(M_PWHistory().IsEmpty() && M_SavePWHistory() == FALSE)) {
    wchar_t buffer[6];
#if (_MSC_VER >= 1400)
    swprintf_s(buffer, 6, L"%1x%02x%02x",
              (M_SavePWHistory() == FALSE) ? 0 : 1,
              M_MaxPWHistory(),
              M_pwhistlist().size());
#else
    swprintf(buffer, L"%1x%02x%02x",
             (M_SavePWHistory() == FALSE) ? 0 : 1,
             M_MaxPWHistory(),
             M_pwhistlist().size());
#endif
    if (M_PWHistory().GetLength() >= 5) {
      for (int i = 0; i < 5; i++) M_PWHistory().SetAt(i, buffer[i]);
    } else {
      M_PWHistory() = buffer;
    }
  }

  return CAddEdit_PropertyPage::OnApply();

error:
  // Are we the current page, if not activate this page
  if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
    m_ae_psh->SetActivePage(this);

  if (pFocus != NULL)
    pFocus->SetFocus();

  if (pFocus == GetDlgItem(IDC_MAXPWHISTORY))
    ((CEdit *)pFocus)->SetSel(MAKEWORD(-1, 0));

  return FALSE;
}

void CAddEdit_Additional::OnSetDCACheck()
{
  m_ae_psh->SetChanged(true);

  BOOL bEnable = ((CButton *)GetDlgItem(IDC_DCA_DEFAULT))->GetCheck() == BST_CHECKED ?
                   FALSE : TRUE;

  // Assuming FALSE = 0 & TRUE = 1
  m_UseDefaultDCA = 1 - bEnable;
  m_dblclk_cbox.EnableWindow(bEnable);
  if (bEnable == FALSE)
    M_DCA() = -1;
  else
    OnDCAComboChanged();
}

void CAddEdit_Additional::OnSetShiftDCACheck()
{
  m_ae_psh->SetChanged(true);

  BOOL bEnable = ((CButton *)GetDlgItem(IDC_SHIFT_DCA_DEFAULT))->GetCheck() == BST_CHECKED ?
                   FALSE : TRUE;

  // Assuming FALSE = 0 & TRUE = 1
  m_UseDefaultShiftDCA = 1 - bEnable;
  m_shiftdblclk_cbox.EnableWindow(bEnable);
  if (bEnable == FALSE)
    M_ShiftDCA() = -1;
  else
    OnShiftDCAComboChanged();
}

void CAddEdit_Additional::OnDCAComboChanged()
{
  m_ae_psh->SetChanged(true);

  int nIndex = m_dblclk_cbox.GetCurSel();
  M_DCA() = (short)m_dblclk_cbox.GetItemData(nIndex);
}

void CAddEdit_Additional::OnShiftDCAComboChanged()
{
  m_ae_psh->SetChanged(true);

  int nIndex = m_shiftdblclk_cbox.GetCurSel();
  M_ShiftDCA() = (short)m_shiftdblclk_cbox.GetItemData(nIndex);
}

void CAddEdit_Additional::OnSTCExClicked(UINT nID)
{
  UpdateData(TRUE);
  StringX sxData;
  int iaction(0);
  std::vector<size_t> vactionverboffsets;

  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_AUTO:
      m_stc_autotype.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      // If Ctrl pressed - just copy un-substituted Autotype string
      // else substitute
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        if (M_autotype().IsEmpty())
          sxData = PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DefaultAutotypeString);
        else
          sxData = StringX(M_autotype());
      } else {
        sxData = PWSAuxParse::GetAutoTypeString(StringX(M_autotype()),
                                                StringX(M_group()),
                                                StringX(M_title()),
                                                StringX(M_username()),
                                                StringX(M_realpassword()),
                                                StringX(M_realnotes()),
                                                vactionverboffsets);
      }
      iaction = CItemData::AUTOTYPE;
      break;
    case IDC_STATIC_RUNCMD:
      m_stc_runcommand.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      // If Ctrl pressed - just copy un-substituted Run Command
      // else substitute
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 || M_runcommand().IsEmpty()) {
        sxData = StringX(M_runcommand());
      } else {
        std::wstring errmsg;
        size_t st_column;
        bool bURLSpecial;
        sxData = PWSAuxParse::GetExpandedString(M_runcommand(),
                                                 M_currentDB(),
                                                 M_pci(),
                                                 M_pDbx()->m_bDoAutoType,
                                                 M_pDbx()->m_AutoType,
                                                 errmsg, st_column, bURLSpecial);
        if (errmsg.length() > 0) {
          CGeneralMsgBox gmb;
          CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
          CString cs_errmsg;
          cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
          gmb.MessageBox(cs_errmsg, cs_title, MB_ICONERROR);
        }
      }
      iaction = CItemData::RUNCMD;
      break;
    default:
      ASSERT(0);
  }
  M_pDbx()->SetClipboardData(sxData);
  M_pDbx()->UpdateLastClipboardAction(iaction);
}

void CAddEdit_Additional::OnCheckedSavePasswordHistory()
{
  M_SavePWHistory() = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck() == BST_CHECKED ?
                           TRUE : FALSE;
  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(M_SavePWHistory());
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnClearPWHist()
{
  m_ClearPWHistory = true;
  m_PWHistListCtrl.DeleteAllItems();
  M_pwhistlist().clear();
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnHistListClick(NMHDR *pNotifyStruct, LRESULT *)
{
  LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pNotifyStruct;
  ASSERT(lpnmitem != NULL);
  int item = lpnmitem->iItem;
  if (item == -1)
    return;

  size_t itempos = size_t(m_PWHistListCtrl.GetItemData(item));
  const PWHistEntry pwhentry = M_pwhistlist()[itempos];
  M_pDbx()->SetClipboardData(pwhentry.password);
}

void CAddEdit_Additional::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = true;

    m_iSortedColumn = phdn->iItem;
    m_PWHistListCtrl.SortItems(PWHistCompareFunc, (LPARAM)this);

    // Note: WINVER defines the minimum system level for which this is program compiled and
    // NOT the level of system it is running on!
    // In this case, these values are defined in Windows XP and later and supported
    // by V6 of comctl32.dll (supplied with Windows XP) and later.
    // They should be ignored by earlier levels of this dll or .....
    //     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_PWHistListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= (m_bSortAscending ? HDF_SORTUP : HDF_SORTDOWN);
    m_PWHistListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CAddEdit_Additional::PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                                    LPARAM closure)
{
  CAddEdit_Additional *self = (CAddEdit_Additional *)closure;
  int nSortColumn = self->m_iSortedColumn;
  size_t Lpos = (size_t)lParam1;
  size_t Rpos = (size_t)lParam2;
  const PWHistEntry pLHS = self->M_pwhistlist()[Lpos];
  const PWHistEntry pRHS = self->M_pwhistlist()[Rpos];
  CSecString password1, changedate1;
  CSecString password2, changedate2;
  time_t t1, t2;

  int iResult;
  switch(nSortColumn) {
    case 0:
      t1 = pLHS.changetttdate;
      t2 = pRHS.changetttdate;
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case 1:
      password1 = pLHS.password;
      password2 = pRHS.password;
      iResult = ((CString)password1).Compare(password2);
      break;
    default:
      iResult = 0; // should never happen - just keep compiler happy
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending)
    iResult *= -1;

  return iResult;
}

void CAddEdit_Additional::OnPWHCopyAll()
{
  CSecString HistStr;
  PWHistList::iterator iter;

  for (iter = M_pwhistlist().begin(); iter != M_pwhistlist().end(); iter++) {
    const PWHistEntry &ent = *iter;
    HistStr += ent.changedate;
    HistStr += L"\t";
    HistStr += ent.password;
    HistStr += L"\r\n";
  }

  M_pDbx()->SetClipboardData(HistStr);
}
