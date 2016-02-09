
#include "StdAfx.h"

#include "HelpAboutDlg.h"

CHelpAboutDlg::CHelpAboutDlg(CWnd *pParent /*=NULL*/, CFont *pFont)
  : CDialog(CHelpAboutDlg::IDD, pParent), m_pFont(pFont)
{
  // Set background colour for for dialog as white
  m_pBkBrush.CreateSolidBrush(RGB(255, 255, 255));

  m_wcReserved[0] = 0xFFFD;
  m_wcReserved[1] = 0;

  m_wcUnsupported[0] = 0x007F;
  m_wcUnsupported[1] = 0;
}

CHelpAboutDlg::~CHelpAboutDlg()
{
  m_pBkBrush.DeleteObject();
  m_fntDialogButtons.DeleteObject();
}

void CHelpAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CHelpAboutDlg)
  DDX_Control(pDX, IDC_RICHEDIT_HELPABOUT, m_RECEx);

  DDX_Control(pDX, IDC_UNSUPPORTED, m_btnUnsupported);
  DDX_Control(pDX, IDC_RESERVED, m_btnReserved);
  DDX_Control(pDX, IDOK, m_btnOK);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHelpAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CHelpAboutDlg)
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDOWN()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CHelpAboutDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
  CFont CaptionFont;
  CaptionFont.CreateFontIndirect(&ncm.lfCaptionFont);

  GetDlgItem(IDC_STATIC_CAPTION)->SetFont(&CaptionFont);

  CFont *font = m_btnOK.GetFont();

  LOGFONT logFont;
  font->GetLogFont(&logFont);
  logFont.lfWeight = FW_SEMIBOLD;

  m_fntDialogButtons.CreateFontIndirect(&logFont);

  m_btnOK.SetFont(&m_fntDialogButtons);
  m_btnOK.SetDeadKeyState(false);
  m_btnOK.SetColourChanges(false);
  m_btnOK.SetButtonColour(RGB(255, 255, 255));

  m_btnReserved.SetWindowTextW(m_wcReserved);
  m_btnReserved.SetDeadKeyState(true);
  m_btnReserved.ModifyStyle(0, BS_CENTER | BS_VCENTER);
  m_btnReserved.SetFont(m_pFont);

  m_btnUnsupported.SetWindowTextW(m_wcUnsupported);
  m_btnUnsupported.SetDeadKeyState(false);
  m_btnUnsupported.ModifyStyle(0, BS_CENTER | BS_VCENTER);
  m_btnUnsupported.SetFont(m_pFont);
  m_btnUnsupported.SetColourChanges(false);

  m_RECEx.SetWindowText(CString(MAKEINTRESOURCE(IDS_HELPABOUT)));

  return TRUE;
}

void CHelpAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Allow draging without the caption bar!
  CDialog::OnLButtonDown(nFlags, point);

  PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}

HBRUSH CHelpAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Note despite CTLCOLOR_BTN being defined - it does not work!
  // Use subclassed button class

  switch (nCtlColor) {
    case CTLCOLOR_STATIC:
    case CTLCOLOR_DLG:
    case CTLCOLOR_EDIT:
    case CTLCOLOR_LISTBOX:
      pDC->SetBkColor(RGB(255, 255, 255));
      return (HBRUSH)(m_pBkBrush.GetSafeHandle());
      break;
  }
  return hbr;
}

