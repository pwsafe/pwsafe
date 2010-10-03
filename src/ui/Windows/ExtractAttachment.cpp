// ExtractAttachment.cpp : implementation file
//

#include "stdafx.h"
#include "ExtractAttachment.h"
#include "PWAttLC.h"
#include "ThisMfcApp.h" // For Help
#include "PWFileDialog.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"

#include "resource.h"
#include "resource3.h"
#include "corelib/StringX.h"
#include "corelib/util.h"
#include "corelib/sha1.h"

#include "os/file.h"
#include "os/dir.h"

#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>

// CExtractAttachment dialog

IMPLEMENT_DYNAMIC(CExtractAttachment, CPWDialog)

CExtractAttachment::CExtractAttachment(CWnd* pParent)
  : CPWDialog(CExtractAttachment::IDD, pParent),
  m_pToolTipCtrl(NULL), m_bInitdone(false)
{
  m_pDbx = static_cast<DboxMain *>(pParent);
}

CExtractAttachment::~CExtractAttachment()
{
  delete m_pToolTipCtrl;
}

void CExtractAttachment::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ATTACHMENT_LIST, m_AttLC);
}

BEGIN_MESSAGE_MAP(CExtractAttachment, CPWDialog)
  ON_BN_CLICKED(IDC_EXTRACT, OnExtract)
  ON_COMMAND(ID_HELP, OnHelp)
  ON_NOTIFY(NM_CLICK, IDC_ATTACHMENT_LIST, OnAttachmentListSelected)
END_MESSAGE_MAP()

// CExtractAttachment message handlers

BOOL CExtractAttachment::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CPWDialog::PreTranslateMessage(pMsg);
}

BOOL CExtractAttachment::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CHeaderCtrl *pHCtrl = m_AttLC.GetHeaderCtrl();
  ASSERT(pHCtrl != NULL);
  m_AttLCHeader.SubclassWindow(pHCtrl->GetSafeHwnd());
  m_AttLCHeader.SetStopChangeFlag(true);

  m_AttLC.Init(CPWAttLC::EXTRACT, (CWnd *)m_pDbx);

  // Make some columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;

  DWORD dwExStyle;
  dwExStyle = m_AttLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES;
  m_AttLC.SetExtendedStyle(dwExStyle);

  // Nothing is selected yet
  GetDlgItem(IDC_EXTRACT)->EnableWindow(FALSE);

  // Add existing attachments
  m_AttLC.AddAttachments(m_vATRecords);

  UpdateData(FALSE);

  m_bInitdone = true;
  return TRUE;
}

void CExtractAttachment::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/extract.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExtractAttachment::OnExtract()
{
  // Ignore if no entry selected
  if (m_AttLC.m_iItem == -1)
    return;

  ATRecord atr = m_vATRecords[m_AttLC.m_iItem];

  m_pDbx->DoAttachmentExtraction(atr);

  GetDlgItem(IDC_EXTRACT)->EnableWindow(FALSE);
  return;
}

void CExtractAttachment::OnAttachmentListSelected(NMHDR *pNMHDR, LRESULT *pResult)
{
  int iItem(-1);
  switch (pNMHDR->code) {
    case NM_CLICK:
    {
      LPNMITEMACTIVATE pLVItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
      iItem = pLVItemActivate->iItem;
      break;
    }
    case LVN_KEYDOWN:
    {
      LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
      iItem = m_AttLC.GetNextItem(-1, LVNI_SELECTED);
      int nCount = m_AttLC.GetItemCount();
      if (pLVKeyDown->wVKey == VK_DOWN)
        iItem = (iItem + 1) % nCount;
      if (pLVKeyDown->wVKey == VK_UP)
        iItem = (iItem - 1 + nCount) % nCount;
      break;
    }
    default:
      // No idea how we got here!
      return;
  }

  if (iItem != -1)
    GetDlgItem(IDC_EXTRACT)->EnableWindow(TRUE);

  *pResult = 0;
}
