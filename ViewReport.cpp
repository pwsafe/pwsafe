/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewReport.cpp : implementation file
//

#include "stdafx.h"
#include "ViewReport.h"
#include "corelib/report.h"
#include "corelib/util.h"
#include "DboxMain.h"

// CViewReport dialog

IMPLEMENT_DYNAMIC(CViewReport, CPWResizeDialog)

CViewReport::CViewReport(CWnd* pParent /*=NULL*/,
                         CReport *pRpt /*=NULL*/)
	: CPWResizeDialog(CViewReport::IDD, pParent),
  m_pRpt(pRpt)
{
  m_pDbx = static_cast<DboxMain *>(pParent);

  m_pData = m_pRpt->GetData(); 
  m_dwDatasize = m_pRpt->GetDataLength();

  m_backgroundcolour = RGB(255, 255, 255);
  m_backgroundbrush.CreateSolidBrush(m_backgroundcolour);
  m_textcolor = ::GetSysColor(COLOR_WINDOWTEXT);
}

CViewReport::~CViewReport()
{
  m_backgroundbrush.DeleteObject();
}

void CViewReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDITREPORT, m_editreport);
}

BEGIN_MESSAGE_MAP(CViewReport, CPWResizeDialog)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(IDOK, Save)
  ON_BN_CLICKED(IDCANCEL, Finish)
  ON_BN_CLICKED(IDC_REPORT2CLIPBOARD, SendToClipBoard)
END_MESSAGE_MAP()

// CViewReport message handlers

BOOL CViewReport::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);
  vibottombtns.push_back(IDC_REPORT2CLIPBOARD);
  vibottombtns.push_back(IDCANCEL);

  AddMainCtrlID(IDC_EDITREPORT);
  AddBtnsCtrlIDs(vibottombtns, 2);

  UINT statustext[1] = {IDS_BLANK};
  SetStatusBar(&statustext[0], 1, false);

  CPWResizeDialog::OnInitDialog();

  if (m_pDbx == NULL)
    GetDlgItem(IDC_REPORT2CLIPBOARD)->EnableWindow(FALSE);

  // Free original handle
  ::LocalFree(m_editreport.GetHandle());

  HLOCAL h = ::LocalAlloc(LHND, m_dwDatasize + sizeof(TCHAR));
  LPTSTR lpszText = (LPTSTR)::LocalLock(h);
  memcpy((void *)lpszText, m_pData, m_dwDatasize);
  ::LocalUnlock(h);

  m_editreport.SetHandle(h);

  return FALSE;
}

void CViewReport::Save()
{
  m_pRpt->SaveToDisk();
  GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CViewReport::SendToClipBoard()
{
  CMyString cs_clipdata((LPTSTR)m_pData);

  m_pDbx->SetClipboardData(cs_clipdata);
}

void CViewReport::Finish()
{
  HLOCAL h = m_editreport.GetHandle();
  LPCTSTR lpszText = (LPCTSTR)::LocalLock(h);

  if (m_dwDatasize > 0) {
    trashMemory((void *)lpszText, m_dwDatasize);
    m_dwDatasize = 0;
  }

  ::LocalUnlock(h);
  CPWResizeDialog::OnCancel();
}

HBRUSH CViewReport::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  if (pWnd->GetDlgCtrlID() == IDC_EDITREPORT) {
    pDC->SetBkColor(m_backgroundcolour);
    pDC->SetTextColor(m_textcolor);
    return (HBRUSH) m_backgroundbrush;
  }
  
  return CPWResizeDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
