// FontCoverageDlg.cpp : implementation file
//

#include "StdAfx.h"

#include "FontCoverageDlg.h"
#include "Unicode_Blocks.h"

#include "resource.h"

#include <algorithm>

// CFontCoverageDlg dialog

IMPLEMENT_DYNAMIC(CFontCoverageDlg, CDialog)

CFontCoverageDlg::CFontCoverageDlg(CWnd* pParent /*=NULL*/, std::wstring wsFontname,
    MapFont2NumChars *pMapFont2NumChars)
  : CDialog(CFontCoverageDlg::IDD, pParent), m_wsFontname(wsFontname),
  m_pMapFont2NumChars(pMapFont2NumChars), m_nSortedColumn(0),
  m_bSortAscending(true)
{
  // Set background colour for for dialog as white
  m_pBkBrush.CreateSolidBrush(RGB(255, 255, 255));
}

CFontCoverageDlg::~CFontCoverageDlg()
{
  m_fntListCtrl.DeleteObject();
  m_fntDialogButtons.DeleteObject();

  m_pBkBrush.DeleteObject();
}

void CFontCoverageDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFontCoverageDlg)
  DDX_Control(pDX, IDC_FONTCOVERAGELIST, m_lcFontCoverage);

  DDX_Control(pDX, IDOK, m_btnOK);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFontCoverageDlg, CDialog)
  //{{AFX_MSG_MAP(CFontCoverageDlg)
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDOWN()

  ON_NOTIFY(NM_CUSTOMDRAW, IDC_FONTCOVERAGELIST, OnNMCustomdrawList)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_FONTCOVERAGELISTHDRCTRL, OnSort)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CFontCoverageDlg message handlers

BOOL CFontCoverageDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  m_lcFontCoverage.GetHeaderCtrl()->SetDlgCtrlID(IDC_FONTCOVERAGELISTHDRCTRL);
  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
  CFont CaptionFont;
  CaptionFont.CreateFontIndirect(&ncm.lfCaptionFont);

  GetDlgItem(IDC_STATIC_CAPTION)->SetFont(&CaptionFont);

  CString cs_title;
  cs_title.Format(IDS_FONT_COVERAGE, m_wsFontname.c_str());
  GetDlgItem(IDC_STATIC_CAPTION)->SetWindowText(cs_title);

  CFont *font = m_btnOK.GetFont();

  LOGFONT logFont;
  font->GetLogFont(&logFont);
  logFont.lfWeight = FW_SEMIBOLD;

  m_fntDialogButtons.CreateFontIndirect(&logFont);

  m_btnOK.SetFont(&m_fntDialogButtons);
  m_btnOK.SetDeadKeyState(false);
  m_btnOK.SetColourChanges(false);
  m_btnOK.SetButtonColour(RGB(255, 255, 255));

  m_LCHeader.SubclassWindow(m_lcFontCoverage.GetHeaderCtrl()->GetSafeHwnd());
  m_LCHeader.SetStopChangeFlag(true);

  CFont *pHdrFont = m_LCHeader.GetFont();

  CDC *pDC = GetDC();
  LOGFONT lf;
  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = MulDiv(11, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  wcscpy_s(lf.lfFaceName, L"Courier New");
  m_fntListCtrl.CreateFontIndirect(&lf);

  m_lcFontCoverage.SetFont(&m_fntListCtrl);

  std::memset(&lf, 0, sizeof(lf));
  lf.lfWeight = FW_BOLD;
  pHdrFont->GetLogFont(&lf);
  CFont HdrFont;
  HdrFont.CreateFontIndirect(&lf);
  m_LCHeader.SetFont(&HdrFont, TRUE);
  m_lcFontCoverage.GetHeaderCtrl()->SetFont(&HdrFont, TRUE);
  HdrFont.DeleteObject();

  MapFont2NumChars::iterator iter;

  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    m_numFontChars[iBlock] = 0;
  }

  for (iter = m_pMapFont2NumChars->begin(); iter != m_pMapFont2NumChars->end(); iter++) {
    m_numFontChars[iter->first] = iter->second;
  }

  CString csTemp;
  csTemp.LoadString(IDS_UNICODE_BLOCK);
  m_lcFontCoverage.InsertColumn(0, csTemp, LVCFMT_LEFT);
  csTemp.LoadString(IDS_PERCENT_COVERAGE);
  m_lcFontCoverage.InsertColumn(1, csTemp, LVCFMT_LEFT);

  // Need full unicode range
  std::vector<unicode_block> temp_vUCBlocks = vUCBlocks;
  std::sort(temp_vUCBlocks.begin(), temp_vUCBlocks.end(), CompareBlockNumberA);

  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    if (temp_vUCBlocks[iBlock].imax_used == -1)
      continue;

    CString cs_temp;
    cs_temp.Format(L"[%06X-%06X] %s", temp_vUCBlocks[iBlock].imin, temp_vUCBlocks[iBlock].imax, temp_vUCBlocks[iBlock].name);
    int iIndex = m_lcFontCoverage.InsertItem(0, cs_temp);

    // L"        (%d%%)%c      "
    //   123456789   01 2345678  -            12345678901234567890
    m_lcFontCoverage.SetItemText(iIndex, 1, L"                    ");
    m_lcFontCoverage.SetItemData(iIndex, iBlock);
  }

  int nColumnCount = m_lcFontCoverage.GetHeaderCtrl()->GetItemCount();
  for (int i = 0; i < nColumnCount; i++) {
    m_lcFontCoverage.SetColumnWidth(i, LVSCW_AUTOSIZE);
  }

  temp_vUCBlocks.clear();

  return TRUE;
}

void CFontCoverageDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Allow draging without the caption bar!
  CDialog::OnLButtonDown(nFlags, point);

  PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}

HBRUSH CFontCoverageDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Note despite CTLCOLOR_BTN being defined - it does not work!
  // Use subclassed button class

  switch (nCtlColor) {
    case CTLCOLOR_STATIC:
    case CTLCOLOR_DLG:
      pDC->SetBkColor(RGB(255, 255, 255));
      return (HBRUSH)(m_pBkBrush.GetSafeHandle());
      break;
  }
  return hbr;
}

void CFontCoverageDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLVCUSTOMDRAW *pNMCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNMHDR);
  *pResult = CDRF_DODEFAULT;

  switch (pNMCD->nmcd.dwDrawStage)
  {
    case CDDS_PREPAINT:
      *pResult |= CDRF_NOTIFYITEMDRAW;      // request item draw notify
      break;
    case CDDS_ITEMPREPAINT:
      *pResult |= CDRF_NOTIFYSUBITEMDRAW;   // request sub-item draw notify
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:  // subitem pre-paint
      pNMCD->clrText = CLR_DEFAULT;
      pNMCD->clrTextBk = CLR_DEFAULT;
      *pResult |= CDRF_NOTIFYPOSTPAINT;     // request post-paint notify
      break;
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM: // subitem post-paint
      SubitemPostPaint(pNMCD);
      break;
  }
}

void CFontCoverageDlg::SubitemPostPaint(LPNMLVCUSTOMDRAW pNMCD)
{
  const int nItem = (int)pNMCD->nmcd.dwItemSpec;

  if (pNMCD->iSubItem == 1) {
    CString cs_text;
    CRect rcSubItem;
    m_lcFontCoverage.GetSubItemRect(nItem, 1, LVIR_LABEL, rcSubItem);

    CDC *pDC = CDC::FromHandle(pNMCD->nmcd.hdc);
    int i = (int)pNMCD->nmcd.lItemlParam;

    if (vUCBlocks[i].numchars != 0) {
      float percentage = float(m_numFontChars[i] * 100) / float(vUCBlocks[i].numchars);
      int ip = static_cast<int>(percentage);

      if (percentage > 100)
        ip = 100;

      if (ip > 0) {
        int iw = (int)(rcSubItem.Width() * (100 - percentage) / 100);
        CRect rcPercetageRect = rcSubItem;
        rcPercetageRect.DeflateRect(0, 0, iw, 0);

        for (int iw = 0; iw < rcPercetageRect.Width(); iw++) {
          float x = (float)((iw * 100.0) / rcSubItem.Width());
          int r = (int)((x > 50 ? 1 - 2 * (x - 50) / 100.0 : 1.0) * 255);
          int g = (int)((x > 50 ? 1.0 : 2 * x / 100.0) * 255);

          pDC->FillSolidRect(rcPercetageRect.left + iw, rcPercetageRect.top, 1, rcPercetageRect.Height(), RGB(r, g, 0));
        }
      }
      if (ip > 0) {
        cs_text.Format(L"        (%d%%)%c      ", ip, percentage > 100 ? L'+' : L' ');
        pDC->DrawText(cs_text, &rcSubItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
      }
    }
  }
}

void CFontCoverageDlg::OnSort(NMHDR *pNMHDR, LRESULT *pResult)
{
  HD_NOTIFY *phdn = reinterpret_cast<HD_NOTIFY *>(pNMHDR);
  *pResult = 0;

  if (phdn->iItem == m_nSortedColumn)
    m_bSortAscending = !m_bSortAscending;
  else
    m_bSortAscending = true;

  m_nSortedColumn = phdn->iItem;

  m_lcFontCoverage.SortItems(SortFontCoverageList, (LPARAM)this);

  HDITEM hdi = {0};
  hdi.mask = HDI_FORMAT;

  CHeaderCtrl *pHeader = m_lcFontCoverage.GetHeaderCtrl();

  pHeader->GetItem(m_nSortedColumn, &hdi);

  for (int i = 0; i < pHeader->GetItemCount(); i++) {
    pHeader->GetItem(i, &hdi);
    if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
      hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
      pHeader->SetItem(i, &hdi);
    }
  }

  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= (m_bSortAscending ? HDF_SORTUP : HDF_SORTDOWN);
  pHeader->SetItem(m_nSortedColumn, &hdi);
}

int CFontCoverageDlg::SortFontCoverageList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CFontCoverageDlg *pSelf = (CFontCoverageDlg *)lParamSort;

  float percentage1, percentage2;
  int result(0);

  switch (pSelf->m_nSortedColumn) {
    case FC_UNICODE_RANGE:
      result = (lParam1 < lParam2) ? -1 : 1;
      break;
    case FC_PERCENTAGE:
      percentage1 = float(pSelf->m_numFontChars[lParam1] * 100) / float(vUCBlocks[lParam1].numchars);
      percentage2 = float(pSelf->m_numFontChars[lParam2] * 100) / float(vUCBlocks[lParam2].numchars);
      if (percentage1 < percentage2)
        result = -1;
      else if (percentage1 > percentage2)
        result = 1;
      break;
    default:
      ASSERT(0);
  }

  if (!pSelf->m_bSortAscending)
    result = -result;

  return result;
}
