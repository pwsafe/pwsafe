/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExportXML.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportXMLDlg.h"
#include "AdvancedDlg.h"
#include "PwFont.h"
#include "ThisMfcApp.h"
#include "corelib/pwsprefs.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"
#include "os/dir.h"

#include <iomanip>  // For setbase and setw

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

/////////////////////////////////////////////////////////////////////////////
// CExportXMLDlg dialog


CExportXMLDlg::CExportXMLDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CExportXMLDlg::IDD, pParent),
  m_subgroup_set(BST_UNCHECKED),
  m_subgroup_name(_T("")), m_subgroup_object(0), m_subgroup_function(0),
  m_pVKeyBoardDlg(NULL), m_pctlPasskey(NULL)
{
  m_passkey = _T("");
  m_defexpdelim = _T("\xbb");

  m_pctlPasskey = new CSecEditExtn;
}

BOOL CExportXMLDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
  m_subgroup_name.Empty();

  ApplyPasswordFont(GetDlgItem(IDC_EXPORT_XML_PASSWORD));
  ((CEdit*)GetDlgItem(IDC_EXPORT_XML_PASSWORD))->SetPasswordChar(PSSWDCHAR);
  
  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  return TRUE;
}

CExportXMLDlg::~CExportXMLDlg()
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

void CExportXMLDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CExportXMLDlg)
  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);

  DDX_Control(pDX, IDC_EXPORT_XML_PASSWORD, *m_pctlPasskey);
  DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
  DDV_MaxChars(pDX, m_defexpdelim, 1);
  DDV_CheckExpDelimiter(pDX, m_defexpdelim);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportXMLDlg, CPWDialog)
  //{{AFX_MSG_MAP(CExportXMLDlg)
  ON_BN_CLICKED(IDC_XML_ADVANCED, OnAdvanced)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_MESSAGE(WM_INSERTBUFFER, OnInsertBuffer)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI CExportXMLDlg::DDV_CheckExpDelimiter(CDataExchange* pDX, 
                                                 const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      AfxMessageBox(IDS_NEEDDELIMITER);
      pDX->Fail();
      return;
    }   
    if (delimiter[0] == '"') {
      AfxMessageBox(IDS_INVALIDDELIMITER);
      pDX->Fail();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CExportXMLDlg message handlers

void CExportXMLDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/export.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExportXMLDlg::OnOK() 
{
  if (UpdateData(TRUE) == FALSE)
    return;

  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);

  CPWDialog::OnOK();
}

void CExportXMLDlg::OnAdvanced()
{
  CAdvancedDlg Adv(this, ADV_EXPORT_XML, m_bsExport, m_subgroup_name, 
                   m_subgroup_set, m_subgroup_object, m_subgroup_function);

  INT_PTR rc = Adv.DoModal();

  if (rc == IDOK) {
    m_bsExport = Adv.m_bsFields;
    m_subgroup_set = Adv.m_subgroup_set;
    if (m_subgroup_set == BST_CHECKED) {
      m_subgroup_name = Adv.m_subgroup_name;
      m_subgroup_object = Adv.m_subgroup_object;
      m_subgroup_function = Adv.m_subgroup_function;
    }
  }
}

void CExportXMLDlg::OnVirtualKeyboard()
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

LRESULT CExportXMLDlg::OnInsertBuffer(WPARAM, LPARAM)
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
