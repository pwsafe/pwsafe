/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// DisplayFSBkupFiles.cpp : implementation file
//

#include "stdafx.h"
#include "ThisMfcApp.h" // for online help

#include "DisplayFSBkupFiles.h"
#include "GeneralMsgBox.h"

#include "resource3.h"  // String resources

using namespace std;

// CDisplayFSBkupFiles dialog
CDisplayFSBkupFiles::CDisplayFSBkupFiles(CWnd* pParent,
                                     std::wstring &wsDBDrive,
                                     std::wstring &wsDBPath,
                                     st_DBProperties &st_dbpcore,
                                     std::vector<st_recfile> &vValidEBackupfiles)
  : CPWDialog(CDisplayFSBkupFiles::IDD, pParent), m_wsDBPath(wsDBPath),
  m_st_dbpcore(st_dbpcore), m_vValidEBackupfiles(vValidEBackupfiles),
  m_iSelectedItem(-1)
{
  m_DriveType = GetDriveType(wsDBDrive.c_str());
}

void CDisplayFSBkupFiles::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_RECFILELIST, m_RFListCtrl);
}

BEGIN_MESSAGE_MAP(CDisplayFSBkupFiles, CDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_CONTINUE, OnContinue)
  ON_BN_CLICKED(IDC_DELETE, OnDelete)
  ON_BN_CLICKED(IDC_SELECT, OnSelect)
  ON_NOTIFY(NM_CLICK, IDC_RECFILELIST, OnItemSelected)
END_MESSAGE_MAP()

// CDisplayFSBkupFiles message handlers

BOOL CDisplayFSBkupFiles::OnInitDialog()
{
  CDialog::OnInitDialog();

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create CDisplayFSBkupFiles Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips(TRUE);
    // Delay initial show & reshow
    int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * 4);
    m_pToolTipCtrl->SetMaxTipWidth(250);

    CString cs_ToolTip;
    cs_ToolTip.LoadString(IDS_EBCONTINUE);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_CONTINUE), cs_ToolTip);
    cs_ToolTip.LoadString(IDC_EBDELETE);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_DELETE), cs_ToolTip);
    cs_ToolTip.LoadString(IDC_EBSELECT);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_SELECT), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_EBEXIT);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDCANCEL), cs_ToolTip);

    m_pToolTipCtrl->Activate(TRUE);
  }

  DWORD dwStyle = m_RFListCtrl.GetExtendedStyle();
  dwStyle |= (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  m_RFListCtrl.SetExtendedStyle(dwStyle);

  CString cs_text;
  cs_text.LoadString(IDS_DBNAME);
  m_RFListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_PASSPHRASE);
  m_RFListCtrl.InsertColumn(1, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_DBFORMAT);
  m_RFListCtrl.InsertColumn(2, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_NUMGROUPS);
  m_RFListCtrl.InsertColumn(3, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_NUMEMPTYGROUPS);
  m_RFListCtrl.InsertColumn(4, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_NUMENTRIES);
  m_RFListCtrl.InsertColumn(5, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_NUMATTACHMENTS);
  m_RFListCtrl.InsertColumn(6, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_WHENSAVED);
  m_RFListCtrl.InsertColumn(7, cs_text);
  cs_text.LoadString(IDS_WHOSAVED);
  m_RFListCtrl.InsertColumn(8, cs_text);
  cs_text.LoadString(IDS_WHATSAVED);
  m_RFListCtrl.InsertColumn(9, cs_text);
  cs_text.LoadString(IDS_FILEUUID);
  m_RFListCtrl.InsertColumn(10, cs_text);
  cs_text.LoadString(IDS_UNKNFLDS);
  m_RFListCtrl.InsertColumn(11, cs_text, LVCFMT_CENTER);

  int nPos = 0;

  cs_text.LoadString(IDS_NA);
  // Add in the current database as the first entry
  nPos = m_RFListCtrl.InsertItem(nPos, m_st_dbpcore.database.c_str());
  m_RFListCtrl.SetItemText(nPos, 1, cs_text);
  m_RFListCtrl.SetItemText(nPos, 2, m_st_dbpcore.databaseformat.c_str());
  m_RFListCtrl.SetItemText(nPos, 3, m_st_dbpcore.numgroups.c_str());
  m_RFListCtrl.SetItemText(nPos, 4, m_st_dbpcore.numemptygroups.c_str());
  m_RFListCtrl.SetItemText(nPos, 5, m_st_dbpcore.numentries.c_str());
  m_RFListCtrl.SetItemText(nPos, 6, m_st_dbpcore.numattachments.c_str());
  m_RFListCtrl.SetItemText(nPos, 7, m_st_dbpcore.whenlastsaved.c_str());
  m_RFListCtrl.SetItemText(nPos, 8, m_st_dbpcore.wholastsaved.c_str());
  m_RFListCtrl.SetItemText(nPos, 9, m_st_dbpcore.whatlastsaved.c_str());
  m_RFListCtrl.SetItemText(nPos, 10, m_st_dbpcore.file_uuid.c_str());
  m_RFListCtrl.SetItemText(nPos, 11, m_st_dbpcore.unknownfields.c_str());

  // Load constants and trim off leading ampersand
  std::wstring wsYES, wsNO;
  LoadAString(wsYES, IDS_YES);
  Trim(wsYES, L"&");
  LoadAString(wsNO, IDS_NO);
  Trim(wsNO, L"&");

  // Add in all recovery files found
  std::vector<st_recfile>::const_iterator filepos;
  for (filepos = m_vValidEBackupfiles.begin();
       filepos != m_vValidEBackupfiles.end();
       filepos++) {
    const st_recfile st_rf = *filepos;
    nPos = m_RFListCtrl.InsertItem(++nPos, st_rf.filename.c_str());
    // Save return codes for later
    m_vrc.push_back(st_rf.rc);
    if (st_rf.rc == 0) {
      m_RFListCtrl.SetItemText(nPos, 1, wsYES.c_str());
      m_RFListCtrl.SetItemText(nPos, 2, st_rf.dbp.databaseformat.c_str());
      m_RFListCtrl.SetItemText(nPos, 3, st_rf.dbp.numgroups.c_str());
      m_RFListCtrl.SetItemText(nPos, 4, m_st_dbpcore.numgroups.c_str());
      m_RFListCtrl.SetItemText(nPos, 5, st_rf.dbp.numentries.c_str());
      m_RFListCtrl.SetItemText(nPos, 6, st_rf.dbp.numattachments.c_str());
      m_RFListCtrl.SetItemText(nPos, 7, st_rf.dbp.whenlastsaved.c_str());
      m_RFListCtrl.SetItemText(nPos, 8, st_rf.dbp.wholastsaved.c_str());
      m_RFListCtrl.SetItemText(nPos, 9, st_rf.dbp.whatlastsaved.c_str());
      m_RFListCtrl.SetItemText(nPos, 10, st_rf.dbp.file_uuid.c_str());
      m_RFListCtrl.SetItemText(nPos, 11, st_rf.dbp.unknownfields.c_str());
    } else {
      m_RFListCtrl.SetItemText(nPos, 1, wsNO.c_str());
      m_RFListCtrl.SetItemText(nPos, 2, L"-");
      m_RFListCtrl.SetItemText(nPos, 3, L"-");
      m_RFListCtrl.SetItemText(nPos, 4, L"-");
      m_RFListCtrl.SetItemText(nPos, 5, L"-");
      m_RFListCtrl.SetItemText(nPos, 6, L"-");
      m_RFListCtrl.SetItemText(nPos, 7, L"-");
      m_RFListCtrl.SetItemText(nPos, 8, L"-");
      m_RFListCtrl.SetItemText(nPos, 9, L"-");
      m_RFListCtrl.SetItemText(nPos, 10, L"-");
    }
    // original nPos
    m_RFListCtrl.SetItemData(nPos, static_cast<DWORD>(nPos - 1));
  }

  // Resize columnss
  const int numcolumns = m_RFListCtrl.GetHeaderCtrl()->GetItemCount();
  for (int i = 0; i < numcolumns; i++) {
    m_RFListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  // Select the first item == current database
  m_RFListCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
  GetDlgItem(IDC_SELECT)->EnableWindow(FALSE);
  GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CDisplayFSBkupFiles::PreTranslateMessage(MSG *pMsg)
{
  // Do tooltips
  if (pMsg->message == WM_MOUSEMOVE) {
    if (m_pToolTipCtrl != NULL) {
      // Change to allow tooltip on disabled controls
      MSG msg = *pMsg;
      msg.hwnd = (HWND)m_pToolTipCtrl->SendMessage(TTM_WINDOWFROMPOINT, 0,
                                                   (LPARAM)&msg.pt);
      CPoint pt = pMsg->pt;
      ::ScreenToClient(msg.hwnd, &pt);

      msg.lParam = MAKELONG(pt.x, pt.y);

      // Let the ToolTip process this message.
      m_pToolTipCtrl->Activate(TRUE);
      m_pToolTipCtrl->RelayEvent(&msg);
    }
  }

  return CDialog::PreTranslateMessage(pMsg);
}

void CDisplayFSBkupFiles::OnHelp()
{
  ShowHelp(L"::/html/failsafebackups.html");
}

void CDisplayFSBkupFiles::OnContinue()
{
  CDialog::EndDialog(IDCONTINUE);  // rc = 11 > 0
}

void CDisplayFSBkupFiles::OnSelect()
{
  // Set rc = - (original recovery file index in vector)
  int ioriginal = (int)m_RFListCtrl.GetItemData(m_iSelectedItem);
  CDialog::EndDialog(-ioriginal);
}

void CDisplayFSBkupFiles::OnDelete()
{
  // Delete selected item - but allow undo via Recycle Bin
  // NOTE: There are no Recycle Bins on network drives, or
  // (most) removable drives (floppy disks, USB pen drives, CD/DVD - no,
  // USB hard disks - yes).
  // Must not delete first entry (current database)
  if (m_iSelectedItem < 1)
    return;

  if (m_DriveType != DRIVE_FIXED) {
    CGeneralMsgBox gmb;
    const CString cs_text(MAKEINTRESOURCE(IDS_NORECYCLEBINMSG));
    const CString cs_title(MAKEINTRESOURCE(IDS_NORECYCLEBINTITLE));
    if (gmb.MessageBox(cs_text, cs_title,
                       (MB_YESNO | MB_ICONQUESTION)) == IDNO) {
      return;
    }
  }

  std::wstring ws_selected = m_wsDBPath +
                std::wstring((LPCWSTR)m_RFListCtrl.GetItemText(m_iSelectedItem, 0));

  const wchar_t *lpsz_delete = ws_selected.c_str();
  wchar_t szToBeDeleted[_MAX_PATH + 1];

  wcscpy_s(szToBeDeleted, _MAX_PATH, lpsz_delete);

  // Must end with double NULL
  szToBeDeleted[ws_selected.length() + 1] = L'\0';

  SHFILEOPSTRUCT sfop;
  memset(&sfop, 0, sizeof(sfop));

  sfop.hwnd = GetActiveWindow()->GetSafeHwnd();
  sfop.wFunc = FO_DELETE;
  sfop.pFrom = szToBeDeleted;
  sfop.pTo = NULL;
  sfop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
  sfop.hNameMappings = NULL;

  int rc = SHFileOperation(&sfop);

  if (rc == 0) {
    m_RFListCtrl.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED);
    m_RFListCtrl.DeleteItem(m_iSelectedItem);
    m_iSelectedItem = -1;

    // Nothing selected now - disable buttons
    GetDlgItem(IDC_SELECT)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
  }
}

void CDisplayFSBkupFiles::OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0L;

  m_iSelectedItem = -1;
  switch (pNotifyStruct->code) {
    case NM_CLICK:
    {
      LPNMITEMACTIVATE pLVItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNotifyStruct);
      int iItem = pLVItemActivate->iItem;
      if (iItem < 0) {
        m_RFListCtrl.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED | LVIS_DROPHILITED);
        // Disable buttons 
        GetDlgItem(IDC_SELECT)->EnableWindow(FALSE);
        GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
      }
      m_iSelectedItem = iItem;
      break;
    }
    case LVN_KEYDOWN:
    {
      LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNotifyStruct);
      m_iSelectedItem = m_RFListCtrl.GetNextItem(-1, LVNI_SELECTED);
      int nCount = m_RFListCtrl.GetItemCount();
      if (pLVKeyDown->wVKey == VK_DOWN)
        m_iSelectedItem = (m_iSelectedItem + 1) % nCount;
      if (pLVKeyDown->wVKey == VK_UP)
        m_iSelectedItem = (m_iSelectedItem - 1 + nCount) % nCount;
      break;
    }
    default:
      // No idea how we got here!
      return;
  }

  if (m_iSelectedItem == -1)
    return;

  BOOL bDelete, bSelect;
  // Can't select or delete the current database
  bDelete = bSelect = (m_iSelectedItem == 0 ? FALSE : TRUE);

  if (m_iSelectedItem != 0) {
    // Can't select an entry whose passphrase is different
    int ioriginal = (int)m_RFListCtrl.GetItemData(m_iSelectedItem);
    bSelect = m_vrc[ioriginal] == 0 ? TRUE : FALSE;
  }

  // Enable/Disable buttons appropriately
  GetDlgItem(IDC_SELECT)->EnableWindow(bSelect);
  GetDlgItem(IDC_DELETE)->EnableWindow(bDelete);
}
