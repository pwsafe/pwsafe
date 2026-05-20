/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Basic.cpp : implementation file
//

#include "StdAfx.h"
#include "PasswordSafe.h"

#include "DboxMain.h"

#include "AddEdit_Basic.h"
#include "AddEdit_Basic_Tabs.h"
#include "AddEdit_PropertySheet.h"

#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"
#include "core/PWCharPool.h"
#include "os/file.h"

#include "os/debug.h"

#include "winutils.h"

using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t PSSWDCHAR = L'*';
CString CAddEdit_Basic::CS_SHOW;
CString CAddEdit_Basic::CS_HIDE;

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic property page

IMPLEMENT_DYNAMIC(CAddEdit_Basic, CAddEdit_PropertyPage)

CAddEdit_Basic::CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent,
    CAddEdit_Basic::IDD, CAddEdit_Basic::IDD_SHORT,
    pAEMD),
  m_tabs(pParent, pAEMD),
  m_bInitdone(false)
{
  if (CS_SHOW.IsEmpty()) { // one-time initializations
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT);
  }

  m_password = m_password2 = M_realpassword();

  if ((!M_pcore()->IsReadOnly() && M_protected() == 0) && 
      M_notes().GetLength() > MAXTEXTCHARS) {
    // Only truncate if in Edit mode
    M_notes() = M_notes().Left(MAXTEXTCHARS);

    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
    cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

}

void CAddEdit_Basic::DoDataExchange(CDataExchange *pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Basic)
  m_ex_password.DoDDX(pDX, m_password);
  m_ex_password2.DoDDX(pDX, m_password2);

  DDX_CBString(pDX, IDC_GROUP, static_cast<CString&>(M_group()));
  DDX_Text(pDX, IDC_TITLE, static_cast<CString&>(M_title()));
  DDX_Text(pDX, IDC_USERNAME, static_cast<CString&>(M_username()));
  DDX_Text(pDX, IDC_URL, static_cast<CString&>(M_URL()));
  DDX_Text(pDX, IDC_EMAIL, static_cast<CString&>(M_email()));

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_TWOFACTORCODE, m_btnTwoFactorCode);
  DDX_Control(pDX, IDC_STATIC_TWOFACTORCODE, m_stcTwoFactorCode);
  DDX_Control(pDX, IDC_PASSWORD_STRENGTH, m_prgStrengthMeter);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_EMAIL, m_ex_email);
  DDX_Control(pDX, IDC_MYBASE, m_ex_base);
  DDX_Control(pDX, IDC_LISTDEPENDENTS, m_cmbDependents);

  DDX_Control(pDX, IDC_STATIC_GROUP, m_stc_group);
  DDX_Control(pDX, IDC_STATIC_TITLE, m_stc_title);
  DDX_Control(pDX, IDC_STATIC_USERNAME, m_stc_username);
  DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stc_password);
  DDX_Control(pDX, IDC_STATIC_URL, m_stc_URL);
  DDX_Control(pDX, IDC_STATIC_EMAIL, m_stc_email);
  DDX_Control(pDX, IDC_STATIC_ISANALIAS, m_stc_isdependent);
  DDX_Control(pDX, IDC_STATIC_DEPENDENT, m_stc_dependent);

  DDX_Control(pDX, IDC_SMARTLABELHELP, m_Help1);
  DDX_Control(pDX, IDC_PASSWORDHELP, m_Help2);
  DDX_Control(pDX, IDC_PASSWORDHELP2, m_Help3);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Basic)
  ON_WM_TIMER()
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)
  ON_BN_CLICKED(IDC_TWOFACTORCODE, OnCopyTwoFactorCode)
  ON_STN_CLICKED(IDC_STATIC_TWOFACTORCODE, OnTwoFactorCodeStaticClicked)
  ON_BN_CLICKED(IDC_LAUNCH, OnLaunch)
  ON_BN_CLICKED(IDC_SENDEMAIL, OnSendEmail)

  ON_CBN_SELCHANGE(IDC_GROUP, OnGroupComboChanged)
  ON_CBN_EDITCHANGE(IDC_GROUP, OnGroupComboChanged)
  ON_EN_CHANGE(IDC_TITLE, OnChanged)
  ON_EN_CHANGE(IDC_USERNAME, OnChanged)
  ON_EN_CHANGE(IDC_PASSWORD2, OnChanged)
  ON_EN_CHANGE(IDC_URL, OnENChangeURL)
  ON_EN_CHANGE(IDC_EMAIL, OnENChangeEmail)
  ON_EN_CHANGE(IDC_PASSWORD, OnENChangePassword)

  ON_EN_SETFOCUS(IDC_PASSWORD, OnENSetFocusPassword)
  ON_EN_SETFOCUS(IDC_PASSWORD2, OnENSetFocusPassword2)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_GROUP, IDC_STATIC_EMAIL, OnSTCExClicked)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_NOTIFY(PSN_SETACTIVE, 0, OnPageSetActive)
  ON_NOTIFY(PSN_KILLACTIVE, 0, OnPageKillActive)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic message handlers

BOOL CAddEdit_Basic::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  Fonts *pFonts = Fonts::GetInstance();
  pFonts->ApplyPasswordFont(&m_ex_password);
  pFonts->ApplyPasswordFont(&m_ex_password2);

  // Get Add/Edit font
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the group, title, username, URL & email fields
  m_ex_group.SetFont(pFont);
  m_ex_title.SetFont(pFont);
  m_ex_username.SetFont(pFont);
  m_ex_URL.SetFont(pFont);
  m_ex_email.SetFont(pFont);
  m_ex_base.SetFont(pFont);

  // Set text in graphic-only buttons for accessibility
  GetDlgItem(IDC_COPYPASSWORD)->SetWindowText(L"Copy Password");
  GetDlgItem(IDC_TWOFACTORCODE)->SetWindowText(L"Copy Authentication Code");

  m_bTwoFactorCodeShowStatic = false;
  m_stcTwoFactorCode.SetWindowText(m_pszNotShowingCode);
  pFonts->CreateFontMatchingWindowHeight(m_stcTwoFactorCode, m_fontTwoFactorCodeStatic, 8);
  m_stcTwoFactorCode.SetFont(&m_fontTwoFactorCodeStatic);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_REDEXCLAMATION);

    // Disable help for alias password for the moment
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_SMARTLABELHELP, IDS_SMARTLABELHELP);
    AddTool(IDC_PASSWORDHELP, IDS_PASSWORDHELP);
    AddTool(IDC_PASSWORDHELP2, IDS_PASSWORDHELP2);

    // Old style tooltips
    AddTool(IDC_STATIC_EMAIL, IDS_CLICKTOCOPYPLUS1);
    AddTool(IDC_LAUNCH, IDS_CLICKTOGOPLUS);
    AddTool(IDC_SENDEMAIL, IDS_CLICKTOSEND);
    AddTool(IDC_COPYPASSWORD, IDS_CLICKTOCOPYGENPSWD);

    if (M_uicaller() == IDS_EDITENTRY && M_protected() != 0) {
      AddTool(IDC_STATIC_TUTORIAL, IDS_UNPROTECT);
    }

    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
  }

  m_stc_group.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_title.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_username.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_password.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_URL.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_email.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);

  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);
  GetDlgItem(IDC_SENDEMAIL)->EnableWindow(M_email().IsEmpty() ? FALSE : TRUE);

  if (M_uicaller() == IDS_VIEWENTRY ||
      (M_uicaller() == IDS_EDITENTRY && M_protected() != 0)) {
    // Change 'OK' to 'Close' and disable 'Cancel'
    // "CancelToClose" disables the System Command SC_CLOSE
    // from clicking X on PropertySheet so have implemented
    // CAddEdit_PropertySheet::OnSysCommand to deal with it.
    CancelToClose();

    // Disable Group Combo
    m_ex_group.EnableWindow(FALSE);

    // Disable normal Edit controls
    m_ex_title.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_username.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_password.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_password2.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_URL.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_email.SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable Button
    GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
  }

  CRect rcTabs;
  GetDlgItem(IDC_BASIC_TABHOST)->GetWindowRect(&rcTabs);
  ScreenToClient(&rcTabs);
  if (!m_tabs.Create(this, rcTabs))
    return FALSE;
  GetDlgItem(IDC_BASIC_TABHOST)->ShowWindow(SW_HIDE);

  // Populate the combo box
  m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)

  std::vector<std::wstring> vGroups;
  GetMainDlg()->GetAllGroups(vGroups);

  for (auto iter = vGroups.begin();
       iter != vGroups.end(); ++iter) {
    m_ex_group.AddString(iter->c_str());
  }

  // Make sure Group combobox is wide enough
  SetGroupComboBoxWidth();

  size_t num_unkwn(0);
  if (M_uicaller() != IDS_ADDENTRY)
    num_unkwn = M_pci()->NumberUnknownFields();

  if (num_unkwn > 0) {
    CString cs_text;
    cs_text.Format(IDS_RECORDUNKNOWNFIELDS, num_unkwn);
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->SetWindowText(cs_text);
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATICGROUPRUNKNFLDS)->ShowWindow(SW_SHOW);
  } else {
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATICGROUPRUNKNFLDS)->ShowWindow(SW_HIDE);
  }

  // Note shortcuts have their own dialog for edit.
  CString cs_text;
  if (M_original_entrytype() == CItemData::ET_ALIASBASE ||
      M_original_entrytype() == CItemData::ET_SHORTCUTBASE) {
    // Show ComboBox to allow users to view dependents
    SetUpDependentsCombo();
    m_cmbDependents.ShowWindow(SW_SHOW);
    m_cmbDependents.SetCurSel(0);

    cs_text.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
                       IDS_ISANALIASBASE : IDS_ISASHORTCUTBASE);
    m_stc_isdependent.SetWindowText(cs_text);

    m_stc_isdependent.ShowWindow(SW_SHOW);
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_ALIAS) {
    // Update password to alias form
    // Show text stating that it is an alias
    m_password = m_password2 = M_base();
    m_cmbDependents.ShowWindow(SW_HIDE);

    m_stc_isdependent.ShowWindow(SW_SHOW);
    m_ex_base.SetWindowText(M_base());
    m_ex_base.ShowWindow(SW_SHOW);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap Help buttons
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);

    m_Help3.EnableWindow(TRUE);
    m_Help3.ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_NORMAL) {
    // Normal - do none of the above
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_HIDE);
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_HIDE);
  }

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  // Load copy password bitmap
  UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;

  BOOL brc = WinUtil::LoadScaledBitmap(m_CopyPswdBitmap, nImageID, true, m_hWnd);

  ASSERT(brc);
  
  if (brc) {
    auto pBtn = static_cast<CButton*>(GetDlgItem(IDC_COPYPASSWORD));
    ASSERT(pBtn != nullptr);
    if (pBtn != nullptr)
      pBtn->SetBitmap(m_CopyPswdBitmap);
  }

  SetupAuthenticationCodeUiElements();

  UpdateData(FALSE);
  m_bInitdone = true;
  GetDlgItem(IDC_TITLE)->SetFocus();

  m_prgStrengthMeter.SetStrength(CPasswordCharPool::CalculatePasswordStrength(m_password));

  return FALSE;  // return TRUE unless you set the focus to a control
}

void CAddEdit_Basic::OnHelp()
{
  ShowHelp(L"::/html/entering_pwd.html");
}

HBRUSH CAddEdit_Basic::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CAddEdit_PropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC) {
    COLORREF *pcfOld;
    UINT nID = pWnd->GetDlgCtrlID();
    switch (nID) {
      case IDC_STATIC_GROUP:
        pcfOld = &m_group_cfOldColour;
        break;
      case IDC_STATIC_TITLE:
        pcfOld = &m_title_cfOldColour;
        break;
      case IDC_STATIC_USERNAME:
        pcfOld = &m_user_cfOldColour;
        break;
      case IDC_STATIC_PASSWORD:
        pcfOld = &m_pswd_cfOldColour;
        break;
      case IDC_STATIC_URL:
        pcfOld = &m_URL_cfOldColour;
        break;
      case IDC_STATIC_EMAIL:
        pcfOld = &m_email_cfOldColour;
        break;
      default:
        // Not one of ours - get out quick
        return hbr;
    }

    int iFlashing = static_cast<CStaticExtn*>(pWnd)->IsFlashing();
    BOOL bHighlight = static_cast<CStaticExtn*>(pWnd)->IsHighlighted();
    BOOL bMouseInWindow = static_cast<CStaticExtn*>(pWnd)->IsMouseInWindow();

    if (iFlashing != 0) {
      pDC->SetBkMode(iFlashing == 1 || (iFlashing && bHighlight && bMouseInWindow) ?
                     OPAQUE : TRANSPARENT);
      COLORREF cfFlashColour = static_cast<CStaticExtn*>(pWnd)->GetFlashColour();
      *pcfOld = pDC->SetBkColor(iFlashing == 1 ? cfFlashColour : *pcfOld);
    } else if (bHighlight) {
      pDC->SetBkMode(bMouseInWindow ? OPAQUE : TRANSPARENT);
      COLORREF cfHighlightColour = static_cast<CStaticExtn*>(pWnd)->GetHighlightColour();
      *pcfOld = pDC->SetBkColor(bMouseInWindow ? cfHighlightColour : *pcfOld);
    } else if (static_cast<CStaticExtn*>(pWnd)->GetColourState()) {
      COLORREF cfUser = static_cast<CStaticExtn*>(pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }

  // Let's get out of here
  return hbr;
}

BOOL CAddEdit_Basic::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

void CAddEdit_Basic::OnPageSetActive(NMHDR*, LRESULT* pLResult)
{
  SetupAuthenticationCodeUiElements();
  *pLResult = 0;
}

void CAddEdit_Basic::OnPageKillActive(NMHDR *, LRESULT *pLResult)
{
  StopAuthenticationCodeUi();
  *pLResult = m_tabs.IsExternalEditorActive() ? 1 : 0;
}

LRESULT CAddEdit_Basic::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      switch (M_uicaller()) {
      case IDS_EDITENTRY: {
        // BR1519 - we need to ignore the difference between \r\n and \n, regardless of how they got there.
        CSecString notes1(M_notes()), notes2(M_originalnotesTRC());
        notes1.Replace(L"\r\n", L"\n"); notes2.Replace(L"\r\n", L"\n");

        if (M_group() != M_pci()->GetGroup() ||
          M_title() != M_pci()->GetTitle() ||
          M_username() != M_pci()->GetUser() ||
          notes1 != notes2 ||
          M_URL() != M_pci()->GetURL() ||
          M_email() != M_pci()->GetEmail() ||
          (M_ipolicy() != NAMED_POLICY &&
            M_symbols() != M_pci()->GetSymbols()) ||
          M_realpassword() != M_oldRealPassword() ||
          M_twofactorkey() != M_pci()->GetTwoFactorKey() ||
          StringX(M_customfields()) != M_pci()->GetCustomFieldsRaw())
          return 1L;
      }
          break;
        case IDS_ADDENTRY: {
          bool nameClean;
          auto pref = PWSprefs::GetInstance();
          if (pref->GetPref(PWSprefs::UseDefaultUser))
            nameClean = M_username() == pref->GetPref(PWSprefs::DefaultUsername);
          else
            nameClean = M_username().IsEmpty();

          if (!M_group().IsEmpty()      ||
            !M_title().IsEmpty()        ||
            !nameClean                  ||
            !M_realpassword().IsEmpty() ||
            !M_twofactorkey().IsEmpty() ||
            !M_notes().IsEmpty()        ||
            !M_URL().IsEmpty()          ||
            !M_email().IsEmpty()        ||
            !M_symbols().IsEmpty()     ||
            !M_customfields().empty())
            return 1L;
        }
          break;
        default:
          ASSERT(0);
          break;
      } // switch (M_uicaller())
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
      break;
    case PP_UPDATE_PWPOLICY:
    case PP_UPDATE_TIMES:
      break;
    default:
      ASSERT(0);
      break;
  }
  return 0L;
}

BOOL CAddEdit_Basic::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_CONTROL &&
      GetDlgItem(IDC_LAUNCH)->IsWindowEnabled()) {
    CString cs_text(MAKEINTRESOURCE(m_bLaunchPlus ? IDS_LAUNCH : IDS_LAUNCHPLUS));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
    m_bLaunchPlus = !m_bLaunchPlus;
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Basic::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  CWnd* pFocus(nullptr);
  CGeneralMsgBox gmb;
  ItemListIter listindex;
  bool bPswdIsInAliasFormat;
  BaseEntryParms pl;

  CSecString csBase(L"");

  UpdateData(TRUE);

  M_group().EmptyIfOnlyWhiteSpace();
  M_title().EmptyIfOnlyWhiteSpace();
  M_username().EmptyIfOnlyWhiteSpace();
  M_URL().EmptyIfOnlyWhiteSpace();
  M_email().EmptyIfOnlyWhiteSpace();
  M_symbols().EmptyIfOnlyWhiteSpace();

  M_notes().EmptyIfOnlyWhiteSpace();

  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPWHidden)
      m_password2.Empty();
  }

  if (!m_isPWHidden || m_password != HIDDEN_PASSWORD)
    M_realpassword() = m_password;

  UpdateData(FALSE);

  // Check that data is valid
  if (M_title().IsEmpty()) {
    gmb.AfxMessageBox(IDS_MUSTHAVETITLE);
    pFocus = &m_ex_title;
    goto error;
  }

  if (M_realpassword().IsEmpty()) {
    gmb.AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    pFocus = &m_ex_password;
    goto error;
  }

  if (!M_group().IsEmpty() && M_group()[0] == '.') {
    gmb.AfxMessageBox(IDS_DOTINVALID);
    pFocus = &m_ex_group;
    goto error;
  }

  if (m_isPWHidden && m_password.Compare(m_password2) != 0) {
    gmb.AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    UpdateData(FALSE);
    pFocus = &m_ex_password;
    goto error;
  }

  // If there is a matching entry in our list, tell the user to try again.
  listindex = GetMainDlg()->Find(M_group(), M_title(), M_username());
  if (listindex != GetMainDlg()->End()) {
    if (M_uicaller() == IDS_ADDENTRY) { // Add entry
      gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
      pFocus = &m_ex_title;
      goto error;
    }
    else { // Edit entry
      const CItemData& listItem = GetMainDlg()->GetEntryAt(listindex);
      if (listItem.GetUUID() != M_pci()->GetUUID()) {
        gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
        pFocus = &m_ex_title;
        goto error;
      }
    }
  }

  pl.InputType = M_pci()->GetEntryType(); // where we're coming from
  bPswdIsInAliasFormat = M_pcore()->ParseAliasPassword(M_realpassword(), pl);

  if (bPswdIsInAliasFormat)
  {
    const StringX selfGTU = L"[" + M_group() + L":" + M_title() + L":" + M_username() + L"]";
    StringX errmess;
    bool yesNoError;

    bool isAliasValid = M_pcore()->CheckAliasValidity(pl, selfGTU, errmess, yesNoError);


    if (!isAliasValid) {
      UINT uiFlags = yesNoError ? (MB_YESNO | MB_DEFBUTTON2) : MB_OK;
      if (gmb.AfxMessageBox(errmess.c_str(), nullptr, uiFlags) == IDNO || !yesNoError) {
        UpdateData(FALSE);
        pFocus = &m_ex_password;
        goto error;
      }
    }

    M_base_uuid() = pl.base_uuid;
    M_ibasedata() = pl.ibasedata;

    // If we're creating a new alias, life's simple
    if (M_uicaller() == IDS_ADDENTRY) {
      M_pci()->SetAlias();
      ShowHideBaseInfo(CItemData::ET_ALIAS, selfGTU.c_str());
    }
    else {
      // Following is for editing an existing entry

      if (M_original_entrytype() == CItemData::ET_ALIASBASE ||
        M_original_entrytype() == CItemData::ET_SHORTCUTBASE) {
        // User is trying to change a base to an alias!
        CString cs_errmsg, cs_title, cs_base, cs_alias;
        cs_base.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ? IDS_EXP_ABASE : IDS_EXP_SBASE);
        cs_alias.LoadString(IDS_EXP_ALIAS);
        cs_title.Format(IDS_CHANGINGBASEENTRY, static_cast<LPCWSTR>(cs_base),
          static_cast<LPCWSTR>(cs_alias));
        cs_errmsg.Format(M_original_entrytype() == CItemData::ET_ALIASBASE ?
          IDS_CHANGINGBASEENTRY1 : IDS_CHANGINGBASEENTRY2,
          static_cast<LPCWSTR>(cs_alias));
        int rc = static_cast<int>(gmb.MessageBox(cs_errmsg, cs_title, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2));

        if (rc == IDNO) {
          UpdateData(FALSE);
          pFocus = &m_ex_password;
          goto error;
        }

        pws_os::CUUID entry_uuid = M_pci()->GetUUID();
        M_pci()->SetAlias();
        M_pci()->SetUUID(entry_uuid, CItemData::ALIASUUID);
        ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
      }
    }
    //End check

    if (bPswdIsInAliasFormat && M_ibasedata() > 0) {
      if (M_original_base_uuid() != pws_os::CUUID::NullUUID() &&
        M_original_base_uuid() != M_base_uuid()) {
        // User has changed the alias to point to a different base entry
        CItemData* pbci(nullptr);
        ItemListIter iter = M_pcore()->Find(M_base_uuid());
        if (iter != M_pcore()->GetEntryEndIter())
          pbci = &iter->second;

        ASSERT(pbci != NULL);

        if (pbci != nullptr) {
          csBase = L"[" +
            pbci->GetGroup() + L":" +
            pbci->GetTitle() + L":" +
            pbci->GetUser() + L"]";
        }
        else
          csBase.Empty();

        M_pci()->SetAlias(); 
        M_pci()->SetBaseUUID(M_base_uuid());
        ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
      }
    }

    if (M_original_base_uuid() == pws_os::CUUID::NullUUID() &&
      M_original_base_uuid() != M_base_uuid()) {
      // User has changed the normal entry into an alias
      CItemData* pbci(nullptr);
      auto iter = M_pcore()->Find(M_base_uuid());
      if (iter != M_pcore()->GetEntryEndIter())
        pbci = &iter->second;

      ASSERT(pbci != NULL);
      if (pbci != nullptr) {
        csBase = L"[" +
          pbci->GetGroup() + L":" +
          pbci->GetTitle() + L":" +
          pbci->GetUser() + L"]";
      }
      else
        csBase.Empty();

      pws_os::CUUID entry_uuid = M_pci()->GetUUID();
      M_pci()->SetAlias();
      M_pci()->SetBaseUUID(M_base_uuid());
      M_pci()->SetUUID(entry_uuid, CItemData::ALIASUUID);
      ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
    }
  } else // password's not in alias format. Check if we're changing an alias back to a normal entry
  {
    if (M_original_entrytype() == CItemData::ET_ALIAS) {
      // User has made this a normal entry
      M_pci()->SetNormal();
      ShowHideBaseInfo(CItemData::ET_NORMAL, csBase);
    }
  }
  return CAddEdit_PropertyPage::OnApply();

error:
  // Are we the current page, if not activate this page
  if (m_ae_psh->GetActivePage() != static_cast<CAddEdit_PropertyPage*>(this))
    m_ae_psh->SetActivePage(this);

  if (pFocus != nullptr)
    pFocus->SetFocus();

  if (pFocus == &m_ex_title)
    m_ex_title.SetSel(MAKEWORD(-1, 0));

  return FALSE;
}

void CAddEdit_Basic::ShowHideBaseInfo(const CItemData::EntryType &entrytype, const CSecString &csBase)
{
  switch (entrytype) {
  case CItemData::ET_ALIAS:
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_SHOW);

    m_ex_base.SetWindowText(csBase);
    m_ex_base.ShowWindow(SW_SHOW);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap help buttons/information
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(TRUE);
    m_Help3.ShowWindow(SW_SHOW);
    break;
  case CItemData::ET_NORMAL:
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_HIDE);

    m_ex_base.SetWindowText(L"");
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap help buttons/information
    m_Help2.EnableWindow(TRUE);
    m_Help2.ShowWindow(SW_SHOW);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    break;
  case CItemData::ET_ALIASBASE:
  case CItemData::ET_SHORTCUTBASE:
    break;
  // ET_SHORTCUT are edited via a different process
  // ET_LAST is just a placer
  case CItemData::ET_SHORTCUT:
  case CItemData::ET_LAST:
    break;
  case CItemData::ET_INVALID:
    ASSERT(0);
    break;
  }
}

void CAddEdit_Basic::OnENSetFocusPassword()
{
  m_ex_password.SetSel(0, -1);
}

void CAddEdit_Basic::OnENSetFocusPassword2()
{
  m_ex_password2.SetSel(0, -1);
}

void CAddEdit_Basic::OnENChangePassword()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  M_realpassword() = m_password;
  auto strength =  CPasswordCharPool::CalculatePasswordStrength(m_password);
  m_prgStrengthMeter.SetStrength(strength);
}

void CAddEdit_Basic::OnShowPassword()
{
  // Prevent special OnCommand processing
  UpdateData(TRUE);

  if (m_isPWHidden) {
    ShowPassword();
  } else {
    M_realpassword() = m_password; // save visible password

    HidePassword();
  }
  UpdateData(FALSE);
}

void CAddEdit_Basic::ShowPassword()
{
  m_isPWHidden = false;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_HIDE);
  GetDlgItem(IDC_STATIC_PASSWORD2)->ShowWindow(SW_HIDE);


  m_password = M_realpassword();
 
  m_ex_password.SetSecure(false);

  // Remove password character so that the password is displayed
  m_ex_password.SetPasswordChar(0);
  m_ex_password.Invalidate();

  // Don't need verification as the user can see the password entered
  m_password2 = L"1234567890123456789012345678901234567890";
  m_ex_password2.SetSecure(false);
  m_ex_password2.EnableWindow(FALSE);
  m_ex_password2.SetPasswordChar(0);
  m_ex_password2.Invalidate();
}

void CAddEdit_Basic::HidePassword()
{
  m_isPWHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);
  GetDlgItem(IDC_STATIC_PASSWORD2)->ShowWindow(SW_NORMAL);

  m_ex_password.SetSecure(true);
  m_ex_password2.SetSecure(true);

  // Set password character so that the password is not displayed
  m_ex_password.SetPasswordChar(PSSWDCHAR);
  m_ex_password.Invalidate();

  // Need verification as the user can not see the password entered
  m_password2 = m_password;

  m_ex_password2.SetSecureText(m_password2);
  m_ex_password2.EnableWindow(TRUE);
  m_ex_password2.SetPasswordChar(PSSWDCHAR);
  m_ex_password2.Invalidate();
}

void CAddEdit_Basic::OnGeneratePassword()
{
  UpdateData(TRUE);

  if (QuerySiblings(PP_UPDATE_PWPOLICY, 0L) != 0L) {
    return;
  }

  // Here we change the Alias's password
  StringX passwd;
  if (M_ipolicy() == NAMED_POLICY) {
    PWPolicy st_pp;
    GetMainDlg()->GetPolicyFromName(M_policyname(), st_pp);
    GetMainDlg()->MakeRandomPassword(passwd, st_pp);
  } else {
    PWPolicy policy(M_pwp());
    if (M_symbols().IsEmpty()) {
      // No specific entry symbols - use default
      policy.symbols = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultSymbols);
    } else {
      // This entry has its own list of symbols
      policy.symbols = static_cast<LPCWSTR>(M_symbols());
    }
    GetMainDlg()->MakeRandomPassword(passwd, policy);
  }

  M_realpassword() = m_password = passwd.c_str();
  if (m_isPWHidden) {
    m_password2 = m_password;
  }
  m_ae_psh->SetChanged(true);
  UpdateData(FALSE);
  m_prgStrengthMeter.SetStrength(CPasswordCharPool::CalculatePasswordStrength(m_password));

  QuerySiblings(PP_UPDATE_PWPOLICY, 0L);
}

void CAddEdit_Basic::OnGroupComboChanged()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnENChangeURL()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);
}

void CAddEdit_Basic::OnENChangeEmail()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  GetDlgItem(IDC_SENDEMAIL)->EnableWindow(M_email().IsEmpty() ? FALSE : TRUE);
}

void CAddEdit_Basic::OnSTCExClicked(UINT nID)
{
  UpdateData(TRUE);

  CSecString cs_data;
  ClipboardDataSource cds;

  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_GROUP:
      m_stc_group.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_group();
      cds = CItemData::GROUP;
      break;
    case IDC_STATIC_TITLE:
      m_stc_title.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_title();
      cds = CItemData::TITLE;
      break;
    case IDC_STATIC_USERNAME:
      m_stc_username.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_username();
      cds = CItemData::USER;
      break;
    case IDC_STATIC_PASSWORD:
      m_stc_password.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_realpassword();
      if (M_pci() && M_pci()->IsAlias()) {
        const CItemData *pcbi = M_pcore()->GetBaseEntry(M_pci());
        if (pcbi != nullptr) // can be null if user changed password, breaking relation
          cs_data = M_pci()->GetEffectiveFieldValue(CItem::PASSWORD, pcbi);
      }
      cds = CItemData::PASSWORD;
      break;
    case IDC_STATIC_URL:
      cs_data = M_URL();
      m_stc_URL.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cds = CItemData::URL;
      break;
    case IDC_STATIC_EMAIL:
      cs_data = M_email();
      // If Ctrl pressed - also copy to URL field with the 'mailto:' prefix
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 && !M_email().IsEmpty()) {
        M_URL() = L"mailto:" + cs_data;
        UpdateData(FALSE);
      }
      m_stc_email.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cds = CItemData::EMAIL;
      break;
    default:
      ASSERT(0);
  }
  GetMainDlg()->SetClipboardData(cs_data);
  GetMainDlg()->UpdateLastClipboardAction(cds);
}

void CAddEdit_Basic::OnLaunch()
{
  UpdateData(TRUE);
  std::vector<size_t> vactionverboffsets;

  CSecString sPassword(M_realpassword()), sLastPassword(M_lastpassword());
  const CSecString stotpauthcode = m_AEMD.pci->GetTotpAuthCode();

  if (m_AEMD.pci->IsAlias()) {
    CItemData *pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
    ASSERT(pciA != NULL);
    sPassword = pciA->GetPassword();
    sLastPassword = pciA->GetPreviousPassword();
  }

  StringX sx_autotype = PWSAuxParse::GetAutoTypeString(M_autotype(),
                                                       M_group(),
                                                       M_title(),
                                                       M_username(),
                                                       sPassword,
                                                       sLastPassword,
                                                       M_notes(),
                                                       M_URL(),
                                                       M_email(),
                                                       stotpauthcode,
                                                       &M_customfields(),
                                                       vactionverboffsets);

  const bool bDoAutoType = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  GetMainDlg()->LaunchBrowser(M_URL(), sx_autotype, vactionverboffsets, bDoAutoType);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::URL);

  if (bDoAutoType) {
    // Reset button
    BYTE KeyState[256];
    ASSERT(GetKeyboardState(&KeyState[0]));
    KeyState[VK_CONTROL] = 0x80;
    ASSERT(SetKeyboardState(&KeyState[0]));
    CString cs_text(MAKEINTRESOURCE(IDS_LAUNCH));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
  }
  m_bLaunchPlus = false;
}

void CAddEdit_Basic::OnSendEmail()
{
  UpdateData(TRUE);

  GetMainDlg()->SendEmail(M_email());
  GetMainDlg()->UpdateLastClipboardAction(CItemData::EMAIL);
}

#include "core.h" // XXX temporary until refactor finished
bool CAddEdit_Basic::CheckNewPassword(const StringX &group, const StringX &title,
                                      const StringX &user, const StringX &password,
                                      const bool bIsEdit, const CItemData::EntryType InputType, 
                                      pws_os::CUUID &base_uuid, int &ibasedata, bool &b_msg_issued)
{
  // b_msg_issued - whether this routine issued a message
  b_msg_issued = false;
  CGeneralMsgBox gmb;

  // Called from Add and Edit entry
  // Returns false if not a special alias or shortcut password
  BaseEntryParms pl;
  pl.InputType = InputType;

  bool bPswdIsInAliasFormat = M_pcore()->ParseAliasPassword(password, pl);

  // Copy data back before possibly returning
  ibasedata = pl.ibasedata;
  base_uuid = pl.base_uuid;
  if (!bPswdIsInAliasFormat)
    return false;

  // if we ever return 'false', this routine will have issued a message to the user
  b_msg_issued = true;

  if (bIsEdit && 
    (pl.csPwdGroup == group && pl.csPwdTitle == title && pl.csPwdUser == user)) {
    // In Edit, check user isn't changing entry to point to itself (circular/self reference)
    // Can't happen during Add as already checked entry does not exist so if accepted the
    // password would be treated as an unusual "normal" password
    gmb.AfxMessageBox(IDSC_ALIASCANTREFERTOITSELF, MB_OK);
    return false;
  }

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
  // is only valid if n = -1 or -2.

  if (pl.ibasedata < 0) {
    if (InputType == CItemData::ET_SHORTCUT) {
      // Target must exist (unlike for aliases where it could be an unusual password)
      if (pl.bMultipleEntriesFound)
        gmb.AfxMessageBox(IDS_MULTIPLETARGETSFOUND, MB_OK);
      else
        gmb.AfxMessageBox(IDS_TARGETNOTFOUND, MB_OK);
      return false;
    }

    CString cs_msg;
    const CString cs_msgA(MAKEINTRESOURCE(IDSC_ALIASNOTFOUNDA));
    const CString cs_msgZ(MAKEINTRESOURCE(IDSC_ALIASNOTFOUNDZ));
    INT_PTR rc(IDNO);
    switch (pl.ibasedata) {
      case -1: // [t] - must be title as this is the only mandatory field
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDSC_ALIASNOTFOUND0A,
                        pl.csPwdTitle.c_str());  // multiple entries exist with title=x
        else
          cs_msg.Format(IDSC_ALIASNOTFOUND0B,
                        pl.csPwdTitle.c_str());  // no entry exists with title=x
        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ,
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -2: // [g,t], [t:u]
        // In this case the 2 fields from the password are in Group & Title
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDSC_ALIASNOTFOUND1A, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        else
          cs_msg.Format(IDSC_ALIASNOTFOUND1B, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -3: // [g:t:u], [g:t:], [:t:u], [:t:] (title cannot be empty)
      {
        const bool bGE = pl.csPwdGroup.empty();
        const bool bTE = pl.csPwdTitle.empty();
        const bool bUE = pl.csPwdUser.empty();
        if (bTE) {
          // Title is mandatory for all entries!
          gmb.AfxMessageBox(IDSC_BASEHASNOTITLE, MB_OK);
          rc = IDNO;
          break;
        } else if (!bGE && !bUE)  // [x:y:z]
          cs_msg.Format(IDSC_ALIASNOTFOUND2A, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (!bGE && bUE)     // [x:y:]
          cs_msg.Format(IDSC_ALIASNOTFOUND2B, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str());
        else if (bGE && !bUE)     // [:y:z]
          cs_msg.Format(IDSC_ALIASNOTFOUND2C, 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (bGE && bUE)      // [:y:]
          cs_msg.Format(IDSC_ALIASNOTFOUND0B, 
                        pl.csPwdTitle.c_str());

        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      }
      default:
        // Never happens
        ASSERT(0);
    }
    if (rc == IDNO)
      return false;
  }

  if (pl.ibasedata > 0) { // base entry exists
    CGeneralMsgBox gmbx;
    if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
      // An alias can only point to a normal entry or an alias base entry
      CString cs_msg;
      cs_msg.Format(IDSC_BASEISALIAS, 
                    pl.csPwdGroup.c_str(),
                    pl.csPwdTitle.c_str(),
                    pl.csPwdUser.c_str());
      gmbx.AfxMessageBox(cs_msg, NULL, MB_OK);
      return false;
    } else { // <= 0, no base entry. What's TargetType in this case??
      if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
        // An alias can only point to a normal entry or an alias base entry
        CString cs_msg;
        cs_msg.Format(IDSC_ABASEINVALID, 
                      pl.csPwdGroup.c_str(),
                      pl.csPwdTitle.c_str(), 
                      pl.csPwdUser.c_str());
        gmbx.AfxMessageBox(cs_msg, NULL, MB_OK);
        return false;
      } else {
        return true;
      }
    }
  }
  return true; // All OK
}

void CAddEdit_Basic::SetGroupComboBoxWidth()
{
  // Find the longest string in the combo box.
  CString str;
  CSize sz;
  int dx = 0;
  TEXTMETRIC tm;
  CDC *pDC = m_ex_group.GetDC();
  CFont *pFont = m_ex_group.GetFont();

  // Select the listbox font, save the old font
  CFont *pOldFont = pDC->SelectObject(pFont);

  // Get the text metrics for avg char width
  pDC->GetTextMetrics(&tm);

  for (int i = 0; i < m_ex_group.GetCount(); i++) {
    m_ex_group.GetLBText(i, str);
    sz = pDC->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    if (sz.cx > dx)
      dx = sz.cx;
  }

  // Select the old font back into the DC
  pDC->SelectObject(pOldFont);
  m_ex_group.ReleaseDC(pDC);

  // Adjust the width for the vertical scroll bar and the left and right border.
  dx += WinUtil::GetSystemMetrics(SM_CXVSCROLL, m_hWnd) + 2 * WinUtil::GetSystemMetrics(SM_CXEDGE, m_hWnd);

  // Set the width of the list box so that every item is completely visible.
  m_ex_group.SetDroppedWidth(dx);
}

void CAddEdit_Basic::OnCopyPassword()
{
  UpdateData(TRUE);

  StringX effectivePassword = m_password;

  if (M_pci() && M_pci()->IsAlias()) {
    const CItemData *pcbi = M_pcore()->GetBaseEntry(M_pci());
    if (pcbi != nullptr) // can be null if user changed password, breaking relation
      effectivePassword = M_pci()->GetEffectiveFieldValue(CItem::PASSWORD, pcbi);
  }
  GetMainDlg()->SetClipboardData(effectivePassword);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);
}

void CAddEdit_Basic::OnCopyTwoFactorCode()
{
  CSecString sTwoFactorKey(GetTwoFactorKey());
  if (sTwoFactorKey.IsEmpty()) {
    CGeneralMsgBox gmb;
    CString cs_title(MAKEINTRESOURCE(IDS_TWOFACTORCODE_ERROR_TITLE));
    CString cs_message(MAKEINTRESOURCE(IDS_TWOFACTORCODEBUTTON_NOTCONFIGURED));
    gmb.MessageBox(cs_message, cs_title, MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  m_bTwoFactorCodeClipboard = true;
  m_bTwoFactorCodeClipboardFirstTime = true;
  UpdateData(TRUE);
  UpdateAuthCode();
}

void CAddEdit_Basic::OnTwoFactorCodeStaticClicked()
{
  CSecString sTwoFactorKey(GetTwoFactorKey());
  if (sTwoFactorKey.IsEmpty()) {
    CGeneralMsgBox gmb;
    CString cs_title(MAKEINTRESOURCE(IDS_TWOFACTORCODE_ERROR_TITLE));
    CString cs_message(MAKEINTRESOURCE(IDS_TWOFACTORCODEBUTTON_NOTCONFIGURED));
    gmb.MessageBox(cs_message, cs_title, MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  m_bTwoFactorCodeShowStatic = !m_bTwoFactorCodeShowStatic;
  if (m_bTwoFactorCodeShowStatic) {
    m_sxLastAuthCode.clear();
    UpdateAuthCode();
  } else {
    m_stcTwoFactorCode.SetWindowText(m_pszNotShowingCode);
    if (!m_bTwoFactorCodeClipboard)
      m_sxLastAuthCode.clear();
  }
}

CSecString CAddEdit_Basic::GetTwoFactorKey()
{
  CSecString twoFactorKey;
  if (!M_pci() || !M_pci()->IsAlias())
    twoFactorKey = M_twofactorkey();
  else {
    const CItemData* pcbi = M_pcore()->GetBaseEntry(M_pci());
    if (pcbi != nullptr)
      twoFactorKey = M_pci()->GetEffectiveFieldValue(CItem::TWOFACTORKEY, pcbi);
  }
  return twoFactorKey;
}

void CAddEdit_Basic::UpdateAuthCode()
{
  CItemData* pci_cred = M_pci_credential();
  if (!pci_cred)
    return;

  // During Add/Edit, the UI may have updated 2FA info.
  // Use latest 2FA info to produce the auth code.
  CItemData ciTemp(*pci_cred);
  ciTemp.SetTwoFactorKey(GetTwoFactorKey());

  StringX sxAuthCode;
  double ratio;
  auto r = GetMainDlg()->GetTwoFactoryAuthenticationCode(ciTemp, sxAuthCode, &ratio);
  if (r != PWSTotp::Success) {
    StopAuthenticationCodeUi();
    return;
  }

  m_btnTwoFactorCode.SetPercent(100.0 * ratio);

  if (!m_bTwoFactorCodeClipboard && !m_bTwoFactorCodeShowStatic) {
    m_sxLastAuthCode.clear();
    return;
  }

  if (!m_bTwoFactorCodeClipboardFirstTime && sxAuthCode == m_sxLastAuthCode)
    return;

  if (m_bTwoFactorCodeShowStatic)
    m_stcTwoFactorCode.SetWindowText(sxAuthCode.c_str());

  if (m_bTwoFactorCodeClipboard) {

    ClipboardStatus clipboardStatus = GetMainDlg()->GetLastSensitiveClipboardItemStatus();

    // If not first time and last copy not present on clipboard...
    if (!m_bTwoFactorCodeClipboardFirstTime && clipboardStatus != SuccessSensitivePresent) {

      if (clipboardStatus != ClipboardNotAvailable) {
        m_bTwoFactorCodeClipboard = false;
        m_sxLastAuthCode.clear();
      }

      return;
    }

    m_bTwoFactorCodeClipboard = GetMainDlg()->SetClipboardData(sxAuthCode);
    ASSERT(m_bTwoFactorCodeClipboard);
    if (!m_bTwoFactorCodeClipboard) {
      m_sxLastAuthCode.clear();
      return;
    }

    m_bTwoFactorCodeClipboardFirstTime = false;

    GetMainDlg()->UpdateLastClipboardAction(ClipboardDataSource::AuthCode);
  }

  if (m_bTwoFactorCodeShowStatic || m_bTwoFactorCodeClipboard)
    m_sxLastAuthCode = sxAuthCode;
}

void CAddEdit_Basic::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent != TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN) {
    CAddEdit_PropertyPage::OnTimer(nIDEvent);
    return;
  }

  CSecString twoFactorKey = GetTwoFactorKey();
  if (twoFactorKey.IsEmpty()) {
    KillTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN);
    return;
  }

  UpdateAuthCode();
}

PWSTotp::TOTP_Result CAddEdit_Basic::ValidateTotpConfiguration(double *pRatio)
{
  CItemData* pci_cred = M_pci_credential();
  if (!pci_cred)
    return PWSTotp::TotpKeyNotFound;

  // During Add/Edit, the UI may have updated 2FA info.
  // Use latest 2FA info to produce the auth code.
  CItemData ciTemp(*pci_cred);
  ciTemp.SetTwoFactorKey(GetTwoFactorKey());

  PWSTotp::TOTP_Result r = PWSTotp::ValidateTotpConfiguration(ciTemp, nullptr, pRatio);
  if (r != PWSTotp::Success) {
    StopAuthenticationCodeUi();
    CGeneralMsgBox gmb;
    CString cs_title(MAKEINTRESOURCE(IDS_TWOFACTORCODE_ERROR_TITLE));
    CString cs_message(MAKEINTRESOURCE(IDS_TWOFACTORCODE_ERROR_MESSAGE));
    cs_message += L" ";
    cs_message += PWSTotp::GetTotpErrorString(r).c_str();
    cs_message += L".";
    gmb.MessageBox(cs_message, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }
  return r;
}

void CAddEdit_Basic::SetupAuthenticationCodeUiElements()
{
  if (!GetTwoFactorKey().IsEmpty() && ValidateTotpConfiguration() == PWSTotp::Success) {
    m_stcTwoFactorCode.ShowWindow(SW_SHOW);
    m_btnTwoFactorCode.SetPieColor(RGB(0, 192, 255));
    m_btnTwoFactorCode.SetPercent(0);
    AddTool(IDC_TWOFACTORCODE, IDS_TWOFACTORCODEBUTTON_CONFIGURED);
    AddTool(IDC_STATIC_TWOFACTORCODE, IDS_TWOFACTORCODESTATIC_CONFIGURED);
    SetTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN, USER_TIMER_MINIMUM, NULL);
  } else {
    m_btnTwoFactorCode.SetPieColor(::GetSysColor(COLOR_GRAYTEXT));
    m_btnTwoFactorCode.SetPercent(25);
    AddTool(IDC_TWOFACTORCODE, IDS_TWOFACTORCODEBUTTON_NOTCONFIGURED);
    AddTool(IDC_STATIC_TWOFACTORCODE, IDS_TWOFACTORCODEBUTTON_NOTCONFIGURED);
    m_bTwoFactorCodeShowStatic = false;
    m_stcTwoFactorCode.SetWindowText(m_pszNotShowingCode);
    m_stcTwoFactorCode.ShowWindow(SW_HIDE);
    StopAuthenticationCodeUi();
  }
}

void CAddEdit_Basic::StopAuthenticationCodeUi()
{
  KillTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN);
  m_bTwoFactorCodeClipboard = false;
  m_bTwoFactorCodeClipboardFirstTime = false;
  m_bTwoFactorCodeShowStatic = false;
  m_sxLastAuthCode.clear();
}

void CAddEdit_Basic::SetUpDependentsCombo()
{
  // Get Add/Edit font
  CFont *pFont = Fonts::GetInstance()->GetAddEditFont();
  m_cmbDependents.SetFont(pFont);

  CString cs_type;
  cs_type.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
      IDS_LIST_OF_ALIASES : IDS_LIST_OF_SHORTCUTS);

  m_stc_dependent.SetWindowText(cs_type);

  for (size_t i = 0; i < M_vsxdependents().size(); i++) {
    m_cmbDependents.AddString(M_vsxdependents()[i].c_str());
  }

  SetComboBoxWidth();
}

void CAddEdit_Basic::SetComboBoxWidth()
{
  // Find the longest string in the combo box.
  CString str;
  CSize sz;
  int dx = 0;
  TEXTMETRIC tm;
  CDC *pDC = m_cmbDependents.GetDC();
  CFont *pFont = m_cmbDependents.GetFont();

  // Select the listbox font, save the old font
  CFont *pOldFont = pDC->SelectObject(pFont);

  // Get the text metrics for avg char width
  pDC->GetTextMetrics(&tm);

  for (int i = 0; i < m_cmbDependents.GetCount(); i++) {
    m_cmbDependents.GetLBText(i, str);
    sz = pDC->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    if (sz.cx > dx)
      dx = sz.cx;
  }

  // Select the old font back into the DC
  pDC->SelectObject(pOldFont);
  m_cmbDependents.ReleaseDC(pDC);

  // Adjust the width for the vertical scroll bar and the left and right border.
  dx += WinUtil::GetSystemMetrics(SM_CXVSCROLL, m_hWnd) + 2 * WinUtil::GetSystemMetrics(SM_CXEDGE, m_hWnd);

  // If the width of the list box is too small, adjust it so that every
  // item is completely visible.
  m_cmbDependents.SetDroppedWidth(dx);
}
