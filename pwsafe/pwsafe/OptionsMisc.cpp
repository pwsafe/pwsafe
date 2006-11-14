/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// OptionsMisc.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h" // for DoubleClickAction enums

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource3.h"  // String resources
#endif
#include "OptionsMisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc property page

IMPLEMENT_DYNCREATE(COptionsMisc, CPropertyPage)

COptionsMisc::COptionsMisc() : CPropertyPage(COptionsMisc::IDD), m_ToolTipCtrl(NULL)
{
	//{{AFX_DATA_INIT(COptionsMisc)
	//}}AFX_DATA_INIT
}

COptionsMisc::~COptionsMisc()
{
	delete m_ToolTipCtrl;
}

void COptionsMisc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(COptionsMisc)
	DDX_Check(pDX, IDC_CONFIRMDELETE, m_confirmdelete);
	DDX_Check(pDX, IDC_MAINTAINDATETIMESTAMPS, m_maintaindatetimestamps);
	DDX_Check(pDX, IDC_ESC_EXITS, m_escexits);
	DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
	DDX_Check(pDX, IDC_HOTKEY_ENABLE, m_hotkey_enabled);
// JHF class CHotKeyCtrl not defined under WinCE
#if !defined(POCKET_PC)
	DDX_Control(pDX, IDC_HOTKEY_CTRL, m_hotkey);
#endif
	DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
	DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
	DDX_Text(pDX, IDC_DEFUSERNAME, m_defusername);
	DDX_Text(pDX, IDC_OTHERBROWSERLOCATION, m_otherbrowserlocation);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsMisc, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsMisc)
	ON_BN_CLICKED(IDC_HOTKEY_ENABLE, OnEnableHotKey)
	ON_BN_CLICKED(IDC_USEDEFUSER, OnUsedefuser)
	ON_BN_CLICKED(IDC_BROWSEFORLOCATION, OnBrowseForLocation)
	ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnComboChanged) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsMisc::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  // For some reason, MFC calls us twice when initializing.
  // Populate the combo box only once.
  if(m_dblclk_cbox.GetCount() == 0) {
  	// add the strings in alphabetical order
  	int nIndex;
  	CString cs_text;
  	cs_text.LoadString(IDS_DCAAUTOTYPE);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickAutoType);
	m_DCA_to_Index[PWSprefs::DoubleClickAutoType] = nIndex;

	cs_text.LoadString(IDS_DCABROWSE);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickBrowse);
	m_DCA_to_Index[PWSprefs::DoubleClickBrowse] = nIndex;

	cs_text.LoadString(IDS_DCACOPYNOTES);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyNotes);
	m_DCA_to_Index[PWSprefs::DoubleClickCopyNotes] = nIndex;

	cs_text.LoadString(IDS_DCACOPYPASSWORD);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyPassword);
	m_DCA_to_Index[PWSprefs::DoubleClickCopyPassword] = nIndex;

	cs_text.LoadString(IDS_DCACOPYUSERNAME);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyUsername);
	m_DCA_to_Index[PWSprefs::DoubleClickCopyUsername] = nIndex;

	cs_text.LoadString(IDS_DCAVIEWEDIT);
	nIndex = m_dblclk_cbox.AddString(cs_text);
	m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickViewEdit);
	m_DCA_to_Index[PWSprefs::DoubleClickViewEdit] = nIndex;
  }

  if (m_doubleclickaction < PWSprefs::minDCA ||
	  m_doubleclickaction > PWSprefs::maxDCA)
  	m_doubleclickaction = PWSprefs::DoubleClickCopyPassword;
  m_dblclk_cbox.SetCurSel(m_DCA_to_Index[m_doubleclickaction]);

  // JHF ditto here
#if !defined(POCKET_PC)
  m_hotkey.SetHotKey(LOWORD(m_hotkey_value),HIWORD(m_hotkey_value));
  if (m_hotkey_enabled == FALSE)
    m_hotkey.EnableWindow(FALSE);
#endif

  GetDlgItem(IDC_OTHERBROWSERLOCATION)->SetWindowText(m_csBrowser);

  OnUsedefuser();

	// Tooltips on Property Pages
	EnableToolTips();

	m_ToolTipCtrl = new CToolTipCtrl;
	if (!m_ToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
		TRACE("Unable To create Property Page ToolTip\n");
		return TRUE;
	}

	// Activate the tooltip control.
	m_ToolTipCtrl->Activate(TRUE);
	m_ToolTipCtrl->SetMaxTipWidth(300);

	// Set the tooltip
	// Note naming convention: string IDS_xxx corresponds to control IDC_xxx
	CString cs_ToolTip;
	cs_ToolTip.LoadString(IDS_MAINTAINDATETIMESTAMPS);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_MAINTAINDATETIMESTAMPS), cs_ToolTip);
	cs_ToolTip.LoadString(IDS_OTHERBROWSERLOCATION);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_OTHERBROWSERLOCATION), cs_ToolTip);

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers

void COptionsMisc::OnEnableHotKey() 
{
	// JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  if (((CButton*)GetDlgItem(IDC_HOTKEY_ENABLE))->GetCheck() == 1) {
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(TRUE);
	GetDlgItem(IDC_HOTKEY_CTRL)->SetFocus();
  } else
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(FALSE);
#endif
}

void COptionsMisc::OnComboChanged()
{
	int nIndex = m_dblclk_cbox.GetCurSel();
	m_doubleclickaction = m_dblclk_cbox.GetItemData(nIndex);
}

void COptionsMisc::OnUsedefuser()
{
   if (((CButton*)GetDlgItem(IDC_USEDEFUSER))->GetCheck() == 1)
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(FALSE);
   }
   else
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(TRUE);
   }
}

void COptionsMisc::OnOK() 
{
  UpdateData(TRUE);
  // JHF ditto
#if !defined(POCKET_PC)
  WORD wVirtualKeyCode, wModifiers;
  m_hotkey.GetHotKey(wVirtualKeyCode, wModifiers);
  DWORD v = wVirtualKeyCode | (wModifiers << 16);
  m_hotkey_value = v;
#endif
  CPropertyPage::OnOK();  
}


void COptionsMisc::OnBrowseForLocation()
{
	CString cs_initiallocation, cs_title;
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	int rc;

	if (m_csBrowser.IsEmpty())
		cs_initiallocation = _T("C:\\");
	else {
#if _MSC_VER >= 1400
		_tsplitpath_s(m_csBrowser, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
#else
		_tsplitpath(m_csBrowser, drive, dir, NULL, NULL);
		_tmakepath(path_buffer, drive, dir, NULL, NULL);
#endif
		cs_initiallocation = CString(path_buffer);
	}

    CFileDialog fd(TRUE,
                   NULL,
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_DONTADDTORECENT | 
				   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                   _T("Programs (*.exe)|*.exe|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);

	cs_title.LoadString(IDS_SELECTBROWSER);
	fd.m_ofn.lpstrTitle = cs_title;
	fd.m_ofn.lpstrInitialDir = cs_initiallocation;

    rc = fd.DoModal();
    if (rc == IDOK) {
      m_csBrowser = fd.GetPathName();
	  GetDlgItem(IDC_OTHERBROWSERLOCATION)->SetWindowText(m_csBrowser);
	}
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL COptionsMisc::PreTranslateMessage(MSG* pMsg)
{
	if (m_ToolTipCtrl != NULL)
		m_ToolTipCtrl->RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}
