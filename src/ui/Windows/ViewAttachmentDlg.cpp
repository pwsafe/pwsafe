/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewAttahmentDlg.cpp : implementation file
//

#include "stdafx.h"

#include "ViewAttachmentDlg.h"
#include "GeneralMsgBox.h"

#include "os/debug.h"

/////////////////////////////////////////////////////////////////////////////
// CFloatEdit

BEGIN_MESSAGE_MAP(CFloatEdit, CEdit)
  //{{AFX_MSG_MAP(CFloatEdit)
  ON_WM_CHAR()
  ON_CONTROL_REFLECT_EX(EN_CHANGE, OnChange)
  ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, OnKillFocus)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFloatEdit::CFloatEdit() : CEdit(), m_nLastSel(0)
{
}

void CFloatEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  GetWindowText(m_csPreviousValue);
  m_nLastSel = GetSel();

  switch (nChar) {
  case VK_RETURN:
    return;
  case '.':
  {
    CString csText;
    GetWindowText(csText);
    if (csText.Find(L'.') != -1) {
      MessageBeep(0);
      return;  // Only one decimal point
    }
    break;
  }
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '\b':
  case 127:   // Delete
    break;
  default:
    MessageBeep(0);
    return;
  }

  CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CFloatEdit::OnChange()
{
  CString m_csValueText;
  CString csValue;
  GetWindowText(csValue);

  LPTSTR invalidStr;
  wcstof(csValue, &invalidStr);  // Don't need value - just validation
  if (*invalidStr) {
    SetWindowText(m_csPreviousValue);
    SetSel(m_nLastSel);
  } else {
    m_csPreviousValue = csValue;
  }

  return FALSE;
}

BOOL CFloatEdit::OnKillFocus()
{
  CString m_csValueText;
  CString csValue;
  GetWindowText(csValue);

  LPTSTR invalidStr;
  float fValue = wcstof(csValue, &invalidStr);

  if (fValue < m_fMin) {
    fValue = m_fMin;
  } else if (fValue > m_fMax) {
    fValue = m_fMax;
  }

  csValue.Format(L"%0.1f", fValue);
  SetWindowText(csValue);
  m_csPreviousValue = csValue;

  return TRUE;
}

void CFloatEdit::SetMinMax(float fMin, float fMax)
{
  if (fMin >= fMax) {
    ASSERT(0);
    return;
  }

  m_fMin = fMin;
  m_fMax = fMax;
}

/////////////////////////////////////////////////////////////////////////////
// CZoomSliderCtrl

CZoomSliderCtrl::CZoomSliderCtrl()
  : m_pParent(NULL)
{
}

CZoomSliderCtrl::~CZoomSliderCtrl()
{
}

BEGIN_MESSAGE_MAP(CZoomSliderCtrl, CSliderCtrl)
  ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CZoomSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  if (m_pParent == NULL) {
    m_pParent = dynamic_cast<CViewAttachmentDlg *>(GetParent());
  }

  CSliderCtrl::OnLButtonDown(nFlags, point);
  CRect rectClient, rectChannel;

  GetClientRect(rectClient);
  GetChannelRect(rectChannel);
  int nPos = (GetRangeMax() - GetRangeMin()) * (point.x - rectClient.left - rectChannel.left) / 
                (rectChannel.right - rectChannel.left) + GetRangeMin();

  SetPos(nPos);

  m_pParent->SetZoomEditValue(nPos);
}

/////////////////////////////////////////////////////////////////////////////
// CViewAttachmentDlg dialog

IMPLEMENT_DYNAMIC(CViewAttachmentDlg, CPWResizeDialog)

CViewAttachmentDlg::CViewAttachmentDlg(CWnd *pParent, CItemAtt *patt)
	: CPWResizeDialog(IDD_VIEWATTACHMENT, pParent), m_pToolTipCtrl(NULL),
  m_pattachment(patt)
{
}

void CViewAttachmentDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ATT_IMAGE, m_stImgAttachment);
  DDX_Control(pDX, IDC_ZOOM, m_sldrZoom);
  DDX_Control(pDX, IDC_ZOOM_VALUE, m_feZoomEdit);
  DDX_Slider(pDX, IDC_ZOOM, m_iZoomValue);
}

BEGIN_MESSAGE_MAP(CViewAttachmentDlg, CPWResizeDialog)
  ON_WM_SIZE()
  ON_WM_HSCROLL()
  ON_BN_CLICKED(IDOK, OnOK)
  ON_EN_CHANGE(IDC_ZOOM_VALUE, OnZoomChange)
  ON_NOTIFY(TTN_NEEDTEXT, 0, OnZoomTooltipText)
END_MESSAGE_MAP()

// CViewAttachmentDlg message handlers

BOOL CViewAttachmentDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);

  AddMainCtrlID(IDC_ATT_IMAGE);
  AddBtnsCtrlIDs(vibottombtns, 0);

  UINT statustext[1] = { IDS_BLANK };
  SetStatusBar(&statustext[0], 1, false);

  m_stImgAttachment.EnableScrollBars(true);
  m_stImgAttachment.SetZoomFactor(MinZoom);

  CPWResizeDialog::OnInitDialog();

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP)) {
    pws_os::Trace(L"Unable To create ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  }

  m_iZoomValue = 10;
  m_sldrZoom.SetRange(MinZoom, MaxZoom, TRUE);
  m_sldrZoom.SetPos(m_iZoomValue);
  m_sldrZoom.SetTicFreq(10);
  if (m_pToolTipCtrl) {
    m_sldrZoom.SetToolTips(m_pToolTipCtrl);
    m_sldrZoom.SetTipSide(TBTS_BOTTOM);
    m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, 0);
    m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, 0);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 5000);

    m_pToolTipCtrl->AddTool(&m_sldrZoom, LPSTR_TEXTCALLBACK);
    m_pToolTipCtrl->Activate(true);
  }

  m_feZoomEdit.SetMinMax(MinZoom / 10.0, MaxZoom / 10.0);

  CString csZoomValue;
  csZoomValue.Format(L"%0.1f", 1.0);
  m_feZoomEdit.SetWindowText(csZoomValue);

  // Get current dialog dimensions
  CRect dlgRect;
  GetClientRect(&dlgRect);

  // Set it to allow size to double
  SetMaxHeightWidth(2 * dlgRect.Height(), 2 * dlgRect.Width());

  // Should be an image file - but may not be supported by CImage - try..
  // Allocate attachment buffer
  int rc(0);
  HRESULT hr(S_OK);

  UINT imagesize = (UINT)m_pattachment->GetContentSize();
  HGLOBAL gMemory = GlobalAlloc(GMEM_MOVEABLE, imagesize);
  ASSERT(gMemory);

  if (gMemory == NULL) {
    // Ooops???
    rc = 2;  // Document this!
    goto load_error;
  }

  BYTE *pBuffer = (BYTE *)GlobalLock(gMemory);
  ASSERT(pBuffer);

  if (pBuffer == NULL) {
    rc = 3;  // Document this!
    GlobalFree(gMemory);
    goto load_error;
  }

  // Load image into buffer
  m_pattachment->GetContent(pBuffer, imagesize);

  // Put it into a IStream
  IStream *pStream = NULL;
  hr = CreateStreamOnHGlobal(gMemory, FALSE, &pStream);
  if (SUCCEEDED(hr)) {
    // Load it
    hr = m_stImgAttachment.Load(pStream);
  }

  // Clean up - no real need to trash the buffer
  if (pStream != NULL)
    pStream->Release();

  GlobalUnlock(gMemory);
  GlobalFree(gMemory);

  // Check image load (or previous error of putting content into an IStream)
  if (FAILED(hr)) {
    goto load_error;
  }

  GotoDlgCtrl(GetDlgItem(IDC_ATT_IMAGE));

  return FALSE;

load_error:
  // Ooops???
  m_stImgAttachment.IssueError(rc, hr);

  // Stop dialog opening at all
  EndDialog(IDCANCEL);
  return TRUE;
}

BOOL CViewAttachmentDlg::PreTranslateMessage(MSG *pMsg)
{
  if (m_pToolTipCtrl != NULL) {
    m_pToolTipCtrl->RelayEvent(pMsg);
  }

  return CPWResizeDialog::PreTranslateMessage(pMsg);
}

void CViewAttachmentDlg::OnZoomTooltipText(NMHDR *pNMHDR, LRESULT *pLResult)
{
  NMTTDISPINFO *pTTT = (NMTTDISPINFO *)pNMHDR;

  if (pTTT->uFlags & TTF_IDISHWND) {
    // idFrom is actually the HWND of the tool
    UINT_PTR nID = ::GetDlgCtrlID((HWND)pNMHDR->idFrom);
    if (nID == IDC_ZOOM)  {
      CString csText;
      int nPos = m_sldrZoom.GetPos();
      csText.Format(L"%0.1f", float(nPos / 10.0));
      swprintf_s(pTTT->szText, sizeof(pTTT->szText) / sizeof(wchar_t), csText);
      pTTT->hinst = AfxGetResourceHandle();
    }
  }

  *pLResult = 0;
}

void CViewAttachmentDlg::OnOK()
{
  CWnd *pwndCtrl = GetFocus();
  UINT ctrl_ID = pwndCtrl->GetDlgCtrlID();

  switch (ctrl_ID) {
  case IDC_ZOOM_VALUE:
    // Don't exit dialog if user presses enter in this edit box
    break;
  case IDOK:
    CPWResizeDialog::OnOK();
    break;
  default:
    break;
  }
}

void CViewAttachmentDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (m_stImgAttachment.IsImageLoaded()) {
    m_stImgAttachment.RedrawWindow();
  }
}

void CViewAttachmentDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  if (pScrollBar->GetDlgCtrlID() == IDC_ZOOM) {
    switch (nSBCode) {
      break;
    case SB_LINELEFT:
      nPos -= 1;
      break;
    case SB_LINERIGHT:
      nPos += 1;
      break;
    case SB_PAGELEFT:
      nPos -= 10;
      break;
    case SB_PAGERIGHT:
      nPos += 10;
      break;
    case SB_LEFT:
      nPos = MinZoom;
      break;
    case SB_RIGHT:
      nPos = MaxZoom;
      break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    default:
      return;
    }

    if (nPos < MinZoom) {
      nPos = MinZoom;
    } else if (nPos > MaxZoom) {
      nPos = MaxZoom;
    }

    CString csText;
    csText.Format(L"%0.1f", float(nPos / 10.0));
    m_feZoomEdit.SetWindowText(csText);
    m_sldrZoom.SetScrollPos(nPos, TRUE);
    m_iZoomValue = nPos;

    UpdateData(FALSE);
  }
}

void CViewAttachmentDlg::OnZoomChange()
{
  CString csText;
  m_feZoomEdit.GetWindowText(csText);
  int iZoomValue = (int)(_wtof(csText) * 10);
  if (iZoomValue >= 1 && iZoomValue <= 100) {
    m_iZoomValue = iZoomValue;
    m_sldrZoom.SetScrollPos(m_iZoomValue, TRUE);

    m_stImgAttachment.SetZoomFactor(m_iZoomValue);
  }

  UpdateData(FALSE);
}

void CViewAttachmentDlg::SetZoomEditValue(int nPos)
{
  CString csText;
  csText.Format(L"%0.1f", float(nPos / 10.0));
  m_feZoomEdit.SetWindowText(csText);

  m_iZoomValue = nPos;
  m_stImgAttachment.SetZoomFactor(m_iZoomValue);

  UpdateData(FALSE);
}
