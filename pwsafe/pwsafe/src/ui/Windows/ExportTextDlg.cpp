/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExportText.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportTextDlg.h"
#include "PwFont.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"

#include "core/pwsprefs.h"

#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "os/dir.h"

#include <iomanip>  // For setbase and setw

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t PSSWDCHAR = L'*';

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg dialog


CExportTextDlg::CExportTextDlg(CWnd* pParent /*=NULL*/, bool bAll,
                               st_SaveAdvValues *pst_SADV)
  : CPWDialog(CExportTextDlg::IDD, pParent), m_bAll(bAll), m_pst_SADV(pst_SADV),
  m_pVKeyBoardDlg(NULL), m_pctlPasskey(NULL)
{
  ASSERT(pst_SADV != NULL);

  m_passkey = L"";
  m_defexpdelim = L"\xbb";
  m_bAdvanced = false;

  m_pctlPasskey = new CSecEditExtn;
}

CExportTextDlg::~CExportTextDlg()
{
  delete m_pctlPasskey;

  if (m_pVKeyBoardDlg != NULL) {
    // Save Last Used Keyboard
    UINT uiKLID = m_pVKeyBoardDlg->GetKLID();
    std::wostringstream os;
    os.fill(L'0');
    os << std::nouppercase << std::hex << std::setw(8) << uiKLID;
    StringX cs_KLID = os.str().c_str();
    PWSprefs::GetInstance()->SetPref(PWSprefs::LastUsedKeyboard, cs_KLID);

    m_pVKeyBoardDlg->DestroyWindow();
    delete m_pVKeyBoardDlg;
  }
}

BOOL CExportTextDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();
  ApplyPasswordFont(GetDlgItem(IDC_EXPORT_TEXT_PASSWORD));
  ((CEdit*)GetDlgItem(IDC_EXPORT_TEXT_PASSWORD))->SetPasswordChar(PSSWDCHAR);

  LOGFONT lf1, lf2;
  CFont font1, font2;

  GetDlgItem(IDC_EXPWARNING1)->GetFont()->GetLogFont(&lf1);
  lf1.lfWeight = FW_BOLD;
  font1.CreateFontIndirect(&lf1);
  GetDlgItem(IDC_EXPWARNING1)->SetFont(&font1);

  GetDlgItem(IDC_EXPWARNING2)->GetFont()->GetLogFont(&lf2);
  lf2.lfWeight = FW_BOLD;
  font2.CreateFontIndirect(&lf2);
  GetDlgItem(IDC_EXPWARNING2)->SetFont(&font2);

  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  return TRUE;
}

void CExportTextDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CExportTextDlg)
  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);

  DDX_Control(pDX, IDC_EXPORT_TEXT_PASSWORD, *m_pctlPasskey);
  DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
  DDV_MaxChars(pDX, m_defexpdelim, 1);
  DDV_CheckExpDelimiter(pDX, m_defexpdelim);
  DDX_Check(pDX, IDC_EXPORT_ADVANCED, m_bAdvanced);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportTextDlg, CPWDialog)
  //{{AFX_MSG_MAP(CExportTextDlg)
  ON_BN_CLICKED(IDC_EXPORT_ADVANCED, OnAdvanced)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_MESSAGE(WM_INSERTBUFFER, OnInsertBuffer)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI CExportTextDlg::DDV_CheckExpDelimiter(CDataExchange* pDX, 
                                                  const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_NEEDDELIMITER);
      pDX->Fail();
      return;
    }   
    if (delimiter[0] == '"') {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_INVALIDDELIMITER);
      pDX->Fail();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg message handlers

void CExportTextDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/export.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExportTextDlg::OnOK() 
{
  if (UpdateData(TRUE) == FALSE)
    return;

  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);
  CPWDialog::OnOK();
}

void CExportTextDlg::OnAdvanced()
{
  ASSERT(m_pst_SADV != NULL);
  m_bAdvanced = ((CButton*)GetDlgItem(IDC_EXPORT_ADVANCED))->GetCheck();
  if (m_bAdvanced == FALSE)
    return;

  CAdvancedDlg Adv(this, m_bAll ? CAdvancedDlg::ADV_EXPORT_TEXT : CAdvancedDlg::ADV_EXPORT_ENTRYTEXT,
                   m_pst_SADV);

  INT_PTR rc = Adv.DoModal();

  if (rc == IDOK) {
    m_bAdvanced = TRUE;
    m_pst_SADV->bsFields = Adv.m_bsFields;
    m_pst_SADV->subgroup_set = Adv.m_subgroup_set;
    if (m_pst_SADV->subgroup_set == BST_CHECKED) {
      m_pst_SADV->subgroup_name = Adv.m_subgroup_name;
      m_pst_SADV->subgroup_object = Adv.m_subgroup_object;
      m_pst_SADV->subgroup_function = Adv.m_subgroup_function;
    }
  } else {
    m_bAdvanced = FALSE;
  }
  ((CButton*)GetDlgItem(IDC_EXPORT_ADVANCED))->SetCheck(m_bAdvanced);
}

void CExportTextDlg::OnVirtualKeyboard()
{
  // Shouldn't be here if couldn't load DLL. Static control disabled/hidden
  if (!CVKeyBoardDlg::IsOSKAvailable())
    return;

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->IsWindowVisible()) {
    // Already there - move to top
    m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    return;
  }

  // If not already created - do it, otherwise just reset it
  if (m_pVKeyBoardDlg == NULL) {
    StringX cs_LUKBD = PWSprefs::GetInstance()->GetPref(PWSprefs::LastUsedKeyboard);
    m_pVKeyBoardDlg = new CVKeyBoardDlg(this, cs_LUKBD.c_str());
    m_pVKeyBoardDlg->Create(CVKeyBoardDlg::IDD);
  } else {
    m_pVKeyBoardDlg->ResetKeyboard();
  }

  // Now show it and make it top
  m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return;
}

LRESULT CExportTextDlg::OnInsertBuffer(WPARAM, LPARAM)
{
  // Update the variables
  UpdateData(TRUE);

  // Get the buffer
  CSecString vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  // Find the selected characters - if any
  int nStartChar, nEndChar;
  m_pctlPasskey->GetSel(nStartChar, nEndChar);

  // If any characters seelcted - delete them
  if (nStartChar != nEndChar)
    m_passkey.Delete(nStartChar, nEndChar - nStartChar);

  // Insert the buffer
  m_passkey.Insert(nStartChar, vkbuffer);

  // Update the dialog
  UpdateData(FALSE);

  // Put cursor at end of inserted text
  m_pctlPasskey->SetSel(nStartChar + vkbuffer.GetLength(), 
                        nStartChar + vkbuffer.GetLength());

  return 0L;
}
