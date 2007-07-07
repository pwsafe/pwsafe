// ColumnChooserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ColumnChooserDlg.h"

// CColumnChooserDlg dialog

IMPLEMENT_DYNAMIC(CColumnChooserDlg, CDialog)

CColumnChooserDlg::CColumnChooserDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CColumnChooserDlg::IDD, pParent)
{
}

CColumnChooserDlg::~CColumnChooserDlg()
{
}

void CColumnChooserDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CPLIST, m_ccListCtrl);
}

BEGIN_MESSAGE_MAP(CColumnChooserDlg, CDialog)
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
  m_pDbx = (void *)parent;
  return CDialog::Create(nID, parent);
}

BOOL CColumnChooserDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Pass on pointer to DboxMain
  m_ccListCtrl.SetDboxPointer(m_pDbx);

  // Initialise DropTarget
  m_ccListCtrl.Initialize(&m_ccListCtrl);

  return TRUE;
}

void CColumnChooserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == SC_CLOSE)
    ShowWindow(SW_HIDE);
  else
    CDialog::OnSysCommand(nID, lParam);
}

void CColumnChooserDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if (m_pLVHdrCtrl != NULL && nStatus == 0)
    m_pLVHdrCtrl->SetLVState(bShow);

  CDialog::OnShowWindow(bShow, nStatus);
}

void CColumnChooserDlg::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
  if ((nStyleType & GWL_STYLE) == GWL_STYLE) {
    DWORD oldStyle = lpStyleStruct->styleOld & WS_VISIBLE;
    DWORD newStyle = lpStyleStruct->styleNew & WS_VISIBLE;

    if (oldStyle != newStyle && m_pLVHdrCtrl != NULL)
      m_pLVHdrCtrl->SetLVState(newStyle != 0 ? SW_SHOW : SW_HIDE);
  }
  CDialog::OnStyleChanged(nStyleType, lpStyleStruct);
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
