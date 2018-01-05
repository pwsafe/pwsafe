/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ColumnChooserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ColumnChooserDlg.h"

// CColumnChooserDlg dialog

IMPLEMENT_DYNAMIC(CColumnChooserDlg, CPWDialog)

CColumnChooserDlg::CColumnChooserDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CColumnChooserDlg::IDD, pParent)
{
}

CColumnChooserDlg::~CColumnChooserDlg()
{
}

void CColumnChooserDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CPLIST, m_ccListCtrl);
}

BEGIN_MESSAGE_MAP(CColumnChooserDlg, CPWDialog)
  //{{AFX_MSG_MAP(CColumnChooserDlg)
  ON_WM_DESTROY()
  ON_WM_SHOWWINDOW()
  ON_WM_SYSCOMMAND()
  ON_WM_STYLECHANGED()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CColumnChooserDlg message handlers

BOOL CColumnChooserDlg::Create(UINT nID, CWnd *parent)
{
  m_pLVHdrCtrl = NULL;
  return CPWDialog::Create(nID, parent);
}

BOOL CColumnChooserDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Initialise DropTarget
  m_ccListCtrl.Initialize(&m_ccListCtrl);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CColumnChooserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == SC_CLOSE)
    ShowWindow(SW_HIDE);
  else
    CPWDialog::OnSysCommand(nID, lParam);
}

void CColumnChooserDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if (m_pLVHdrCtrl != NULL && nStatus == 0)
    m_pLVHdrCtrl->SetLVState(bShow);

  CPWDialog::OnShowWindow(bShow, nStatus);
}

void CColumnChooserDlg::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
  if ((nStyleType & GWL_STYLE) == GWL_STYLE) {
    DWORD oldStyle = lpStyleStruct->styleOld & WS_VISIBLE;
    DWORD newStyle = lpStyleStruct->styleNew & WS_VISIBLE;

    if (oldStyle != newStyle && m_pLVHdrCtrl != NULL)
      m_pLVHdrCtrl->SetLVState(newStyle != 0 ? SW_SHOW : SW_HIDE);
  }
  CPWDialog::OnStyleChanged(nStyleType, lpStyleStruct);
}

void CColumnChooserDlg::PostNcDestroy()
{
  delete this;
}

void CColumnChooserDlg::OnDestroy()
{
  // Delete all items
  m_ccListCtrl.DeleteAllItems();

  // Stop Drag & Drop OLE
  m_ccListCtrl.Terminate();
}
