
#include "StdAfx.h"

#include "SetFontsDlg.h"
#include "UCPickerDlg.h"
#include "FontCoverageDlg.h"
#include "GFResizeDialogHelper.h"

#include "Fonts.h"
#include "Unicode_Blocks.h"

#include "resource.h"

#include "utility.h"

#include <algorithm>


CSetFontsDlg::CSetFontsDlg(CWnd *pParent /*=NULL*/)
  : CDialog(CSetFontsDlg::IDD, pParent), m_pParent((CUCPickerDlg *)pParent),
  m_nSortedColumn1(0), m_bSortAscending1(true), m_nSortedColumn2(0), m_bSortAscending2(true),
  m_pToolTipCtrl(NULL), m_bResizing(false), m_nDisplayType(ALL), m_nRangeItem(-1)
{
  // Set background colour for for dialog as white
  m_pBkBrush.CreateSolidBrush(RGB(255, 255, 255));
}

CSetFontsDlg::~CSetFontsDlg()
{
  m_fntListCtrl.DeleteObject();
  m_BoldItalicFont.DeleteObject();
  m_fntDialogButtons.DeleteObject();

  m_pBkBrush.DeleteObject();

  if (m_pToolTipCtrl)
    delete m_pToolTipCtrl;
}

void CSetFontsDlg::DoDataExchange(CDataExchange *pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CSetFontsDlg)
  DDX_Control(pDX, IDC_UNICODE_RANGES, m_UnicodeRangeList);
  DDX_Control(pDX, IDC_AVAILABLE_FONTS, m_AvailableFontList);

  DDX_Control(pDX, IDC_RICHEDIT_FONTS, m_RECEx);

  DDX_Control(pDX, IDC_SELECTFONT, m_btnSelect);
  DDX_Control(pDX, IDC_RESETALLFONTS, m_btnResetAll);
  DDX_Control(pDX, IDCANCEL, m_btnCancel);
  DDX_Control(pDX, IDOK, m_btnOK);

  DDX_Control(pDX, IDC_DISPLAYTYPES, m_cbxDisplayType);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetFontsDlg, CDialog)
  //{{AFX_MSG_MAP(CSetFontsDlg)
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDOWN()
  ON_WM_GETMINMAXINFO()
  ON_WM_NCHITTEST()
  ON_WM_SIZE()
  ON_WM_SIZING()

  ON_BN_CLICKED(IDC_SELECTFONT, OnSelectFont)
  ON_BN_CLICKED(IDC_RESETALLFONTS, OnResetAllFonts)
  ON_CBN_SELCHANGE(IDC_DISPLAYTYPES, OnDisplayChange)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_UNICODERANGELISTHDRCTRL, OnSortUnicodeRangeList)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_AVAILABLEFONTLISTHDRCTRL, OnSortAvailableFontsList)
  ON_NOTIFY(LVN_GETDISPINFO, IDC_UNICODE_RANGES, OnGetDispInfoRangeList)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_UNICODE_RANGES, OnUnicodeRangeChanged)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_AVAILABLE_FONTS, OnAvailableFontChanged)
  ON_NOTIFY(NM_RCLICK, IDC_UNICODE_RANGES, OnRClickURFont)
  ON_NOTIFY(NM_RCLICK, IDC_AVAILABLE_FONTS, OnRClickAFFont)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_UNICODE_RANGES, OnCustomDrawUnicodeRangeList)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_AVAILABLE_FONTS, OnCustomDrawAvailableFontsList)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSetFontsDlg::OnInitDialog()
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

  m_btnSelect.SetFont(&m_fntDialogButtons);
  m_btnSelect.SetDeadKeyState(false);
  m_btnSelect.SetColourChanges(false);
  m_btnSelect.SetButtonColour(RGB(255, 255, 255));
  m_btnSelect.EnableWindow(FALSE);

  m_btnResetAll.SetFont(&m_fntDialogButtons);
  m_btnResetAll.SetDeadKeyState(false);
  m_btnResetAll.SetColourChanges(false);
  m_btnResetAll.SetButtonColour(RGB(255, 255, 255));

  m_btnCancel.SetFont(&m_fntDialogButtons);
  m_btnCancel.SetDeadKeyState(false);
  m_btnCancel.SetColourChanges(false);
  m_btnCancel.SetButtonColour(RGB(255, 255, 255));

  m_btnOK.SetFont(&m_fntDialogButtons);
  m_btnOK.SetDeadKeyState(false);
  m_btnOK.SetColourChanges(false);
  m_btnOK.SetButtonColour(RGB(255, 255, 255));

  CRect rect;
  GetWindowRect(&rect);
  m_dfAspectRatio = (double)rect.Width() / (double)rect.Height();

  CRect gripperRect;
  GetClientRect(&gripperRect);

  gripperRect.left = gripperRect.right - GRIPPER_SQUARE_SIZE;
  gripperRect.top = gripperRect.bottom - GRIPPER_SQUARE_SIZE;
  m_gripper.Create(WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN | WS_CLIPSIBLINGS, gripperRect, this, 0);

  m_Resizer.Init(this);

  m_Resizer.AddCtrl(IDC_UNICODE_RANGES, DT_LEFT | DT_TOP, FALSE, FALSE);
  m_Resizer.AddCtrl(IDC_AVAILABLE_FONTS, DT_RIGHT | DT_TOP, TRUE, FALSE);
  m_Resizer.AddCtrl(IDC_SELECTFONT, DT_RIGHT | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDC_RESETALLFONTS, DT_RIGHT | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDC_RICHEDIT_FONTS, DT_RIGHT | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDOK, DT_CENTER | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDCANCEL, DT_CENTER | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDC_STATIC_DISPLAY, DT_LEFT | DT_BOTTOM, TRUE, TRUE);
  m_Resizer.AddCtrl(IDC_DISPLAYTYPES, DT_LEFT | DT_BOTTOM, TRUE, TRUE);

  m_URLCHeader.SubclassWindow(m_UnicodeRangeList.GetHeaderCtrl()->GetSafeHwnd());
  m_URLCHeader.SetNoChangeColumn(0);

  m_AFLCHeader.SubclassWindow(m_AvailableFontList.GetHeaderCtrl()->GetSafeHwnd());
  m_AFLCHeader.SetStopChangeFlag(true);

  CFont *pHdrFont = m_URLCHeader.GetFont();

  CDC *pDC = GetDC();
  LOGFONT lf;
  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = -MulDiv(10, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  wcscpy_s(lf.lfFaceName, L"Courier New");
  m_fntListCtrl.CreateFontIndirect(&lf);

  lf.lfWeight = FW_BOLD;
  lf.lfItalic = TRUE;
  m_BoldItalicFont.CreateFontIndirect(&lf);

  m_UnicodeRangeList.SetFont(&m_fntListCtrl);
  m_AvailableFontList.SetFont(&m_fntListCtrl);

  std::memset(&lf, 0, sizeof(lf));
  lf.lfWeight = FW_BOLD;
  pHdrFont->GetLogFont(&lf);
  CFont HdrFont;
  HdrFont.CreateFontIndirect(&lf);
  m_URLCHeader.SetFont(&HdrFont, TRUE);
  m_AFLCHeader.SetFont(&HdrFont, TRUE);
  HdrFont.DeleteObject();

  // release the device context.
  ReleaseDC(pDC);

  m_cbxDisplayType;

  int nIndex;
  // Populate m_cbxDisplayType USER_SELECTED, PREFFERRED, BEST_AVAILABLE, NONE_FOUND 
  CString csTemp;
  csTemp.LoadString(IDS_ALL);
  nIndex = m_cbxDisplayType.AddString(csTemp);
  m_cbxDisplayType.SetItemData(nIndex, ALL);

  csTemp.LoadString(IDS_USER_SELECTED);
  nIndex = m_cbxDisplayType.AddString(csTemp);
  m_cbxDisplayType.SetItemData(nIndex, USER_SELECTED);

  csTemp.LoadString(IDS_PREFERRED);
  nIndex = m_cbxDisplayType.AddString(csTemp);
  m_cbxDisplayType.SetItemData(nIndex, PREFFERRED);

  csTemp.LoadString(IDS_BEST_AVAILABLE);
  nIndex = m_cbxDisplayType.AddString(csTemp);
  m_cbxDisplayType.SetItemData(nIndex, BEST_AVAILABLE);

  csTemp.LoadString(IDS_UNSUPPORTED);
  nIndex = m_cbxDisplayType.AddString(csTemp);
  m_cbxDisplayType.SetItemData(nIndex, NONE_FOUND);

  m_cbxDisplayType.SetCurSel(0);

  m_URLCHeader.SetDlgCtrlID(IDC_UNICODERANGELISTHDRCTRL);
  m_UnicodeRangeList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

  csTemp.LoadString(IDS_RANGE);
  m_UnicodeRangeList.InsertColumn(UNICODE_RANGE, csTemp, LVCFMT_LEFT | LVCFMT_FIXED_WIDTH);
  csTemp.LoadString(IDS_BLOCK_NAME);
  m_UnicodeRangeList.InsertColumn(UNICODE_NAME, csTemp, LVCFMT_LEFT);
  csTemp.LoadString(IDS_CURRENT_FONT);
  m_UnicodeRangeList.InsertColumn(CURRECT_FONT, csTemp, LVCFMT_LEFT);

  // Sort data just incase was sorted differently last visit
  std::sort(vUCBlocks.begin(), vUCBlocks.end(), CompareBlockNumberA);

  // Load up virtual CListCtrl
  m_UnicodeRangeList.SetItemCount(NUMUNICODERANGES);

  int nColumnCount = m_UnicodeRangeList.GetHeaderCtrl()->GetItemCount();

  for (int i = 0; i < nColumnCount; i++) {
    m_UnicodeRangeList.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_UnicodeRangeList.GetColumnWidth(i);
    m_UnicodeRangeList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_UnicodeRangeList.GetColumnWidth(i);
    m_UnicodeRangeList.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }

  CHeaderCtrl *pAvailableFontListCtrl = m_AvailableFontList.GetHeaderCtrl();
  pAvailableFontListCtrl->SetDlgCtrlID(IDC_AVAILABLEFONTLISTHDRCTRL);

  m_AvailableFontList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

  csTemp.LoadString(IDS_FONT_NAME);
  m_AvailableFontList.InsertColumn(FONT_NAME, csTemp, LVCFMT_LEFT);
  csTemp.LoadString(IDS_PERCENT_SUPPORT);
  m_AvailableFontList.InsertColumn(PERCENTAGE, csTemp, LVCFMT_LEFT);
  csTemp.LoadString(IDS_CHARACTERS);
  m_AvailableFontList.InsertColumn(FONT_NUMCHARS, csTemp, LVCFMT_LEFT);

  m_AvailableFontList.DeleteAllItems();

  nColumnCount = m_AvailableFontList.GetHeaderCtrl()->GetItemCount();

  for (int i = 0; i < nColumnCount; i++) {
    m_AvailableFontList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  m_RECEx.SetWindowText(CString(MAKEINTRESOURCE(IDS_FONTS)));

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return TRUE;
  }

  // Tooltips
  EnableToolTips();

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);

  // Quadruple the time to allow reading by user
  int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

  // Set the tooltip
  csTemp.LoadString(IDS_AVAILABLE_FONTS);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_AVAILABLE_FONTS), csTemp);

  return TRUE;
}

void CSetFontsDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Allow draging without the caption bar!
  CDialog::OnLButtonDown(nFlags, point);

  PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}

LRESULT CSetFontsDlg::OnNcHitTest(CPoint point)
{
  LRESULT lRet = HTCLIENT;

  CRect rcClient, rcFrame;
  GetWindowRect(&rcFrame);
  rcClient = rcFrame;

  CSize sizeBorder(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));

  rcClient.InflateRect(-(4 * sizeBorder.cx), -(4 * sizeBorder.cy));
  rcFrame.InflateRect(2 * sizeBorder.cx, 2 * sizeBorder.cy);

  if (rcFrame.PtInRect(point)) {
    if (point.x > rcClient.right) {
      if (point.y < rcClient.top) {
        lRet = HTTOPRIGHT;
      } else if (point.y > rcClient.bottom) {
        lRet = HTBOTTOMRIGHT;
      } else {
        lRet = HTRIGHT;
      }
    } else if (point.x < rcClient.left) {
      if (point.y < rcClient.top) {
        lRet = HTTOPLEFT;
      } else if (point.y > rcClient.bottom) {
        lRet = HTBOTTOMLEFT;
      } else {
        lRet = HTLEFT;
      }
    } else if (point.y < rcClient.top) {
      lRet = HTTOP;
    } else if (point.y > rcClient.bottom) {
      lRet = HTBOTTOM;
    }
  }

  return lRet;
}

HBRUSH CSetFontsDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
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

BOOL CSetFontsDlg::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST) {
    if (m_pToolTipCtrl != NULL) {
      // Change to allow tooltip on disabled controls
      MSG msg = *pMsg;
      msg.hwnd = (HWND)m_pToolTipCtrl->SendMessage(TTM_WINDOWFROMPOINT, 0,
        (LPARAM)&msg.pt);
      CPoint pt = pMsg->pt;
      ::ScreenToClient(msg.hwnd, &pt);

      msg.lParam = MAKELONG(pt.x, pt.y);

      // Let the ToolTip process this message.
      m_pToolTipCtrl->RelayEvent(&msg);
    }
  }

  return CDialog::PreTranslateMessage(pMsg);
}

void CSetFontsDlg::OnUnicodeRangeChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLISTVIEW *pNMLV = reinterpret_cast<NMLISTVIEW *>(pNMHDR);
  *pResult = 0;

  if (!((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED)))
    return;

  m_AvailableFontList.SetRedraw(FALSE);

  m_AvailableFontList.DeleteAllItems();
  m_btnSelect.EnableWindow(FALSE);

  m_nRangeItem = pNMLV->iItem;

  unicode_block UBlock;
  UBlock = (m_nDisplayType == ALL) ? vUCBlocks[m_nRangeItem] : vUCBlocks_Subset[m_nRangeItem];
  int nBlock = UBlock.iNumber;

  std::map<int, int>::iterator iter;

  for (iter = m_pParent->m_mapFont2NumChars[nBlock].begin(); iter != m_pParent->m_mapFont2NumChars[nBlock].end(); iter++) {
    CString cs_temp;
    int iFontIndex = iter->first;
    //int iNumChars = iter->second;

    int iIndex = m_AvailableFontList.InsertItem(0, vsInstalledFonts[iFontIndex].c_str());
    m_AvailableFontList.SetItemData(iIndex, iFontIndex);

    if (UBlock.numchars != 0) {
      float percentage = float(iter->second * 100) / float(UBlock.numchars);
      cs_temp.Format(L"    %3.1f", percentage);
      m_AvailableFontList.SetItemText(iIndex, PERCENTAGE, cs_temp.Right(5));
    } else {
      m_AvailableFontList.SetItemText(iIndex, PERCENTAGE, L" ");
    }

    cs_temp.Format(L"     %u", iter->second);
    m_AvailableFontList.SetItemText(iIndex, FONT_NUMCHARS, cs_temp.Right(5));
  }

  for (int i = 0; i < m_AFLCHeader.GetItemCount(); i++) {
    m_AvailableFontList.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_AvailableFontList.GetColumnWidth(i);
    m_AvailableFontList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_AvailableFontList.GetColumnWidth(i);
    m_AvailableFontList.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }

  m_AvailableFontList.SetRedraw(TRUE);
}

void CSetFontsDlg::OnAvailableFontChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLISTVIEW *pNMLV = reinterpret_cast<NMLISTVIEW *>(pNMHDR);
  *pResult = 0;

  if (!(pNMLV->uChanged & LVIF_STATE))
    return;

  CString csTemp;

  if (pNMLV->uNewState & LVIS_SELECTED) {
    unicode_block UBlock;
    UBlock = (m_nDisplayType == ALL) ? vUCBlocks[m_nRangeItem] : vUCBlocks_Subset[m_nRangeItem];

    if (m_AvailableFontList.GetItemData(pNMLV->iItem) == (DWORD)UBlock.iUserFont) {
      m_btnSelect.EnableWindow(TRUE);
      csTemp.LoadString(IDS_DESELECT_FONT);
      m_btnSelect.SetWindowText(csTemp);
    } else {
      // Can't make preferred font a user selected font
      if (m_AvailableFontList.GetItemData(pNMLV->iItem) != (DWORD)UBlock.iPreferredFont) {
        m_btnSelect.EnableWindow(TRUE);
        csTemp.LoadString(IDS_SELECT_FONT);
        m_btnSelect.SetWindowText(csTemp);
      }
    }
  } else {
    m_btnSelect.EnableWindow(FALSE);
    csTemp.LoadString(IDS_SELECT_FONT);
    m_btnSelect.SetWindowText(csTemp);
  }
}

void CSetFontsDlg::OnSortAvailableFontsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  HD_NOTIFY *phdn = reinterpret_cast<HD_NOTIFY *>(pNMHDR);
  *pResult = 0;

  // Need to find Unicode Range item
  POSITION posRange = m_UnicodeRangeList.GetFirstSelectedItemPosition();

  if (posRange == NULL)
    return;

  m_nRangeItem = m_UnicodeRangeList.GetNextSelectedItem(posRange);

  if (phdn->iItem == m_nSortedColumn2)
    m_bSortAscending2 = !m_bSortAscending2;
  else
    m_bSortAscending2 = true;

  m_nSortedColumn2 = phdn->iItem;

  m_AvailableFontList.SortItems(SortAvailableFontsList, (LPARAM)this);

  HDITEM hdi = {0};
  hdi.mask = HDI_FORMAT;

  m_AFLCHeader.GetItem(m_nSortedColumn2, &hdi);

  for (int i = 0; i < m_AFLCHeader.GetItemCount(); i++) {
    m_AFLCHeader.GetItem(i, &hdi);
    if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
      hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
      m_AFLCHeader.SetItem(i, &hdi);
    }
  }

  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= (m_bSortAscending2 ? HDF_SORTUP : HDF_SORTDOWN);
  m_AFLCHeader.SetItem(m_nSortedColumn2, &hdi);
}

int CALLBACK CSetFontsDlg::SortAvailableFontsList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CSetFontsDlg *pSelf = (CSetFontsDlg *)lParamSort;

  std::wstring wsFont1, wsFont2;
  int iNumChars1, iNumChars2;
  int result(0), nBlock;

  switch (pSelf->m_nSortedColumn2) {
    case FONT_NAME:
      wsFont1 = vsInstalledFonts[lParam1];
      wsFont2 = vsInstalledFonts[lParam2];
      result = wsFont1.compare(wsFont2);
      break;
    case PERCENTAGE:
    case FONT_NUMCHARS:
      nBlock = (pSelf->m_nDisplayType == ALL) ? vUCBlocks[pSelf->m_nRangeItem].iNumber :
                    vUCBlocks_Subset[pSelf->m_nRangeItem].iNumber;
      iNumChars1 = pSelf->m_pParent->m_mapFont2NumChars[nBlock][(int)lParam1];
      iNumChars2 = pSelf->m_pParent->m_mapFont2NumChars[nBlock][(int)lParam2];
      if (iNumChars1 < iNumChars2)
        result = -1;
      else if (iNumChars1 > iNumChars2)
        result = 1;
      break;
    default:
      ASSERT(0);
  }

  if (!pSelf->m_bSortAscending2)
    result = -result;

  return result;
}

void CSetFontsDlg::OnSortUnicodeRangeList(NMHDR *pNMHDR, LRESULT *pResult)
{
  HD_NOTIFY *phdn = reinterpret_cast<HD_NOTIFY *>(pNMHDR);
  *pResult = 0;

  if (phdn->iItem == m_nSortedColumn1)
    m_bSortAscending1 = !m_bSortAscending1;
  else
    m_bSortAscending1 = true;

  m_nSortedColumn1 = phdn->iItem;
  m_UnicodeRangeList.SetSortColumn(m_nSortedColumn1);

  m_UnicodeRangeList.SetRedraw(FALSE);
  m_UnicodeRangeList.SetItemCount(0);
  switch (m_nSortedColumn1) {
    case UNICODE_RANGE:
      if (m_nDisplayType == ALL)
        std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockNumberA : CompareBlockNumberD);
      else
        std::sort(vUCBlocks_Subset.begin(), vUCBlocks_Subset.end(), m_bSortAscending1 ? CompareBlockNumberA : CompareBlockNumberD);
      break;
    case UNICODE_NAME:
      if (m_nDisplayType == ALL)
        std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockNameA : CompareBlockNameD);
      else
        std::sort(vUCBlocks_Subset.begin(), vUCBlocks_Subset.end(), m_bSortAscending1 ? CompareBlockNameA : CompareBlockNameD);
      break;
    case CURRECT_FONT:
      if (m_nDisplayType == ALL)
        std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);
      else
        std::sort(vUCBlocks_Subset.begin(), vUCBlocks_Subset.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);
      break;
    default:
      ASSERT(0);
  }

  m_UnicodeRangeList.SetItemCount(NUMUNICODERANGES);
  m_UnicodeRangeList.SetRedraw(TRUE);

  HDITEM hdi = {0};
  hdi.mask = HDI_FORMAT;

  m_URLCHeader.GetItem(m_nSortedColumn1, &hdi);

  for (int i = 0; i < m_URLCHeader.GetItemCount(); i++) {
    m_URLCHeader.GetItem(i, &hdi);
    if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
      hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
      m_URLCHeader.SetItem(i, &hdi);
    }
  }

  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= (m_bSortAscending1 ? HDF_SORTUP : HDF_SORTDOWN);
  m_URLCHeader.SetItem(m_nSortedColumn1, &hdi);
}

void CSetFontsDlg::OnRClickURFont(NMHDR *pNMHDR, LRESULT *pResult)
{
  NM_LISTVIEW *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);
  *pResult = 0;

  // Check user clicked on font
  LVHITTESTINFO lvhti;
  lvhti.pt = pNMListView->ptAction;
  m_UnicodeRangeList.SubItemHitTest(&lvhti);

  if (!(lvhti.flags & LVHT_ONITEMLABEL) && lvhti.iSubItem != CURRECT_FONT)
    return;

  // Get font name from CListCtrl - ignore if blank
  std::wstring wsFontName;
  wsFontName = m_UnicodeRangeList.GetItemText(pNMListView->iItem, CURRECT_FONT);
  if (wsFontName.empty())
    return;

  CMenu menu;

  menu.LoadMenu(IDM_PROPERTIES); // our context menu
  CMenu *pContextMenu = menu.GetSubMenu(0);

  POINT p;
  p.x = pNMListView->ptAction.x;
  p.y = pNMListView->ptAction.y;

  ::ClientToScreen(pNMHDR->hwndFrom, &p);
  int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
    p.x, p.y, this);

  if (nID != 0) {
    std::vector<std::wstring>::iterator wsiter;
    wsiter = std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), wsFontName);
    int iFontIndex = std::distance(vsInstalledFonts.begin(), wsiter);
    std::map<int, std::map<int, int>>::iterator font_iter;
    font_iter = m_pParent->m_mapFont2UBlock2NumChars.find(iFontIndex);
    if (font_iter == m_pParent->m_mapFont2UBlock2NumChars.end())
      return;

    CFontCoverageDlg dlg(this, wsFontName, &font_iter->second);

    dlg.DoModal();
  }
}

void CSetFontsDlg::OnRClickAFFont(NMHDR *pNMHDR, LRESULT *pResult)
{
  NM_LISTVIEW *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);
  *pResult = 0;

  CPoint pt(pNMListView->ptAction);
  CMenu menu;

  menu.LoadMenu(IDM_PROPERTIES); // our context menu
  CMenu *pContextMenu = menu.GetSubMenu(0);

  POINT p;
  p.x = pt.x;
  p.y = pt.y;

  ::ClientToScreen(pNMHDR->hwndFrom, &p);
  int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
    p.x, p.y, this);

  if (nID != 0) {
    std::map<int, std::map<int, int>>::iterator font_iter;
    int iFontIndex = m_AvailableFontList.GetItemData(pNMListView->iItem);
    font_iter = m_pParent->m_mapFont2UBlock2NumChars.find(iFontIndex);
    if (font_iter == m_pParent->m_mapFont2UBlock2NumChars.end())
      return;

    std::wstring wsFontName;
    wsFontName = m_AvailableFontList.GetItemText(pNMListView->iItem, FONT_NAME);
    CFontCoverageDlg dlg(this, wsFontName, &font_iter->second);

    dlg.DoModal();
  }
}

struct FindBlock {
  FindBlock(int iNumber) : m_iNumber(iNumber) { }

  bool operator()(unicode_block const &uc) const
  {
    return uc.iNumber == m_iNumber;
  }

  int m_iNumber;
};

void CSetFontsDlg::OnSelectFont()
{
  POSITION posRange = m_UnicodeRangeList.GetFirstSelectedItemPosition();
  POSITION posFont = m_AvailableFontList.GetFirstSelectedItemPosition();

  if (posRange == NULL || posFont == NULL)
    return;

  m_nRangeItem = m_UnicodeRangeList.GetNextSelectedItem(posRange);
  int nFontItem = m_AvailableFontList.GetNextSelectedItem(posFont);

  unicode_block UBlock;
  UBlock = (m_nDisplayType == ALL) ? vUCBlocks[m_nRangeItem] : vUCBlocks_Subset[m_nRangeItem];

  int nOldFontItem(-1);

  for (int i = 0; i < m_AvailableFontList.GetItemCount(); i++) {
    if (m_AvailableFontList.GetItemData(i) == (DWORD)UBlock.iUserFont) {
      nOldFontItem = i;
      break;
    }
  }

  if (nOldFontItem != nFontItem)
    SelectFont(nFontItem, nOldFontItem);
  else
    ResetFont(nFontItem);
}

void CSetFontsDlg::SelectFont(const int nFontItem, const int nOldFontItem)
{
  std::wstring ws_font = m_AvailableFontList.GetItemText(nFontItem, 0);

  // Set font
  if (m_nDisplayType == ALL) {
    m_pParent->SetUserFont(vUCBlocks[m_nRangeItem].iNumber, m_nRangeItem, ws_font);
  } else {
    FindBlock fb(vUCBlocks_Subset[m_nRangeItem].iNumber);
    std::vector<unicode_block>::iterator iter;
    iter = std::find_if(vUCBlocks.begin(), vUCBlocks.end(), fb);

    if (iter != vUCBlocks.end()) {
      int index = std::distance(vUCBlocks.begin(), iter);
      // Set main vector
      int iFontNumber = m_pParent->SetUserFont(vUCBlocks_Subset[m_nRangeItem].iNumber, index, ws_font);
      // Manually set subset vector
      vUCBlocks_Subset[m_nRangeItem].iUserFont = iFontNumber;
    }
  }

  // Repaint old selected font (if any)
  if (nOldFontItem >= 0) {
    m_AvailableFontList.Update(nOldFontItem);
  }

  // Now paint new one
  m_AvailableFontList.Update(nFontItem);

  // If Unicode Range ClistCtrl was sorted on the font AND all Uniblocks visible,
  // we should re-sort it as it has changed
  if (m_nDisplayType == ALL) {
    if (m_nSortedColumn1 == CURRECT_FONT) {
      // Resort and repaint
      if (m_nDisplayType == ALL)
        std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);
      else
        std::sort(vUCBlocks_Subset.begin(), vUCBlocks_Subset.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);

      m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
      m_nRangeItem = -1;
      m_UnicodeRangeList.Invalidate();

      m_AvailableFontList.SetItemState(nFontItem, 0, LVIS_SELECTED);
      m_AvailableFontList.DeleteAllItems();
      m_AvailableFontList.Invalidate();
    } else {
      // Otherwise just repaint the one item
      m_UnicodeRangeList.Update(m_nRangeItem);
    }
  } else {
    // Only delete item if it didn't have a user selected font before
    if (nOldFontItem < 0) {
      m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
      vUCBlocks_Subset.erase(vUCBlocks_Subset.begin() + m_nRangeItem);
      m_UnicodeRangeList.DeleteItem(m_nRangeItem);
      m_nRangeItem = -1;

      m_AvailableFontList.SetItemState(nFontItem, 0, LVIS_SELECTED);
      m_AvailableFontList.DeleteAllItems();
    }
    
    m_AvailableFontList.Invalidate();
  }

  m_UnicodeRangeList.SetFocus();
  m_btnSelect.SetWindowText(L"Deselect Font");
}

void CSetFontsDlg::ResetFont(const int nFontItem)
{
  int iNumber = (m_nDisplayType == ALL) ? vUCBlocks[m_nRangeItem].iNumber : vUCBlocks_Subset[m_nRangeItem].iNumber;

  // Reset font
  if (m_nDisplayType == ALL) {
    m_pParent->SetUserFont(-iNumber, m_nRangeItem, L"");
  } else {
    // Main dialog doesn't know about user font subset - find main index!
    // Find corresponding block in main vector
    FindBlock fb(vUCBlocks_Subset[m_nRangeItem].iNumber);
    std::vector<unicode_block>::iterator iter;
    iter = std::find_if(vUCBlocks.begin(), vUCBlocks.end(), fb);
    if (iter != vUCBlocks.end()) {
      int index = std::distance(vUCBlocks.begin(), iter);
      // Rest main vector
      m_pParent->SetUserFont(-iNumber, index, L"");
      // Manually reset subset vector
      vUCBlocks_Subset[m_nRangeItem].iUserFont = -1;
    }
  }

  // If Unicode Range ClistCtrl was sorted on the font AND all Uniblocks visible,
  // we should re-sort it as it has changed
  if (m_nDisplayType == ALL) {
    if (m_nSortedColumn1 == CURRECT_FONT) {
      // Resort and repaint
      if (m_nDisplayType == ALL)
        std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);
      else
        std::sort(vUCBlocks_Subset.begin(), vUCBlocks_Subset.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);

      m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
      m_nRangeItem = -1;
      m_UnicodeRangeList.Invalidate();

      m_AvailableFontList.SetItemState(nFontItem, 0, LVIS_SELECTED);
      m_AvailableFontList.DeleteAllItems();
      m_AvailableFontList.Invalidate();
    } else {
      // Otherwise just repaint the one item
      m_UnicodeRangeList.Update(m_nRangeItem);
      m_AvailableFontList.Update(nFontItem);
    }
  } else {
    // Need to delete the changed item and refresh Unicode Range subset
    m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
    vUCBlocks_Subset.erase(vUCBlocks_Subset.begin() + m_nRangeItem);
    m_UnicodeRangeList.DeleteItem(m_nRangeItem);
    m_nRangeItem = -1;

    m_AvailableFontList.SetItemState(nFontItem, 0, LVIS_SELECTED);
    m_AvailableFontList.DeleteAllItems();
    m_AvailableFontList.Invalidate();
  }

  m_UnicodeRangeList.SetFocus();
  CString csTemp(MAKEINTRESOURCE(IDS_SELECT_FONT));
  m_btnSelect.SetWindowText(csTemp);
}

void CSetFontsDlg::OnResetAllFonts()
{
  // Reset main vector
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    m_pParent->SetUserFont(-iBlock, iBlock, L"");
  }

  vUCBlocks_Subset.clear();

  if (m_nDisplayType != ALL) {
    m_nRangeItem = -1;
    m_UnicodeRangeList.DeleteAllItems();
    m_UnicodeRangeList.Invalidate();
    m_UnicodeRangeList.SetFocus();

    m_AvailableFontList.DeleteAllItems();
    m_AvailableFontList.Invalidate();

    m_cbxDisplayType.SetCurSel(0);
    return;
  }

  // If Unicode Range ClistCtrl was sorted on the font, we should re-sort it as it has changed
  if (m_nSortedColumn1 == CURRECT_FONT) {
    // Re-sort and repaint
    std::sort(vUCBlocks.begin(), vUCBlocks.end(), m_bSortAscending1 ? CompareBlockFontA : CompareBlockFontD);

    m_AvailableFontList.DeleteAllItems();

    POSITION posRange = m_UnicodeRangeList.GetFirstSelectedItemPosition();

    if (posRange != NULL) {
      m_nRangeItem = m_AvailableFontList.GetNextSelectedItem(posRange);
      m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
      m_nRangeItem = -1;
    }
  }

  CString csTemp(MAKEINTRESOURCE(IDS_SELECT_FONT));
  m_btnSelect.SetWindowText(csTemp);
  m_btnSelect.EnableWindow(FALSE);
  m_AvailableFontList.Invalidate();
  m_UnicodeRangeList.Invalidate();

  m_UnicodeRangeList.SetFocus();
}

void CSetFontsDlg::OnCustomDrawAvailableFontsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLVCUSTOMDRAW *pNMCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNMHDR);
  *pResult = CDRF_DODEFAULT;

  unicode_block UBlock;

  switch (pNMCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pResult |= CDRF_NOTIFYITEMDRAW;      // request item draw notify
      break;
    case CDDS_ITEMPREPAINT:
      m_bRowHighlighted = m_AvailableFontList.GetItemState(pNMCD->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED;
      pNMCD->clrTextBk = m_bRowHighlighted ? RGB(180, 255, 255) : CLR_DEFAULT;
      *pResult |= CDRF_NOTIFYSUBITEMDRAW;   // request sub-item draw notify
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:  // subitem pre-paint
    {
      if (m_bRowHighlighted) {
        pNMCD->nmcd.uItemState &= ~CDIS_SELECTED;
      }
      pNMCD->clrText = CLR_DEFAULT;
      pNMCD->clrTextBk = m_bRowHighlighted ? RGB(180, 255, 255) : CLR_DEFAULT;
      int iFontIndex = (int)pNMCD->nmcd.lItemlParam;
      UBlock = (m_nDisplayType == ALL) ? vUCBlocks[m_nRangeItem] : vUCBlocks_Subset[m_nRangeItem];
      if (iFontIndex == UBlock.iUserFont) {
        pNMCD->clrText = CF_USERSELECTEDFONT;  // User selected font - red
        ::SelectObject(pNMCD->nmcd.hdc, (HFONT)m_BoldItalicFont);
        *pResult |= CDRF_NEWFONT;
      } else if (iFontIndex == UBlock.iPreferredFont) {
        pNMCD->clrText = CF_PREFERREDFONT; // Preferred font - blue
        ::SelectObject(pNMCD->nmcd.hdc, (HFONT)m_BoldItalicFont);
        *pResult |= CDRF_NEWFONT;
      } else if (iFontIndex == UBlock.iBestAvailableFont) {
        pNMCD->clrText = CF_BESTAVAILABLEFONT; // Best Available font - green
        ::SelectObject(pNMCD->nmcd.hdc, (HFONT)m_BoldItalicFont);
        *pResult |= CDRF_NEWFONT;
      }
      *pResult |= CDRF_NOTIFYPOSTPAINT;     // request post-paint notify
      break;
    }
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM: // subitem post-paint
      if (m_bRowHighlighted) {
        pNMCD->nmcd.uItemState |= CDIS_SELECTED;
      }
      break;
  }
}

void CSetFontsDlg::OnCustomDrawUnicodeRangeList(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLVCUSTOMDRAW *pNMCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNMHDR);
  *pResult = CDRF_DODEFAULT;

  int item = pNMCD->nmcd.dwItemSpec;
  unicode_block UBlock;

  switch (pNMCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pResult |= CDRF_NOTIFYITEMDRAW;      // request item draw notify
      break;
    case CDDS_ITEMPREPAINT:
      m_bRowHighlighted = m_UnicodeRangeList.GetItemState(pNMCD->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED;
      pNMCD->clrTextBk = m_bRowHighlighted ? RGB(180, 255, 255) : CLR_DEFAULT;
      *pResult |= CDRF_NOTIFYSUBITEMDRAW;   // request sub-item draw notify
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:  // subitem pre-paint
      if (m_bRowHighlighted) {
        pNMCD->nmcd.uItemState &= ~CDIS_SELECTED;
      }
      pNMCD->clrText = CLR_DEFAULT;
      pNMCD->clrTextBk = m_bRowHighlighted ? RGB(180, 255, 255) : CLR_DEFAULT;
      if (pNMCD->iSubItem == CURRECT_FONT) {
        //CString cs_temp = m_UnicodeRangeList.GetItemText(pNMCD->nmcd.dwItemSpec, 0);
        //cs_temp = cs_temp.Mid(1, 6);
        //int nBlock = FindBlock(cs_temp);
        UBlock = (m_nDisplayType == ALL) ? vUCBlocks[item] : vUCBlocks_Subset[item];
        if (UBlock.iUserFont >= 0) {
          pNMCD->clrText = CF_USERSELECTEDFONT;  // User selected font - red
          ::SelectObject(pNMCD->nmcd.hdc, (HFONT)m_BoldItalicFont);
          *pResult |= CDRF_NEWFONT;
        } else if (UBlock.iPreferredFont >= 0) {
          pNMCD->clrText = CF_PREFERREDFONT; // Preferred font - blue
        } else if (UBlock.iBestAvailableFont >= 0) {
          pNMCD->clrText = CF_BESTAVAILABLEFONT; // Best Available font - orange
          ::SelectObject(pNMCD->nmcd.hdc, (HFONT)m_BoldItalicFont);
          *pResult |= CDRF_NEWFONT;
        }
      }
      *pResult |= CDRF_NOTIFYPOSTPAINT;     // request post-paint notify
      break;
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM: // subitem post-paint
      if (m_bRowHighlighted) {
        pNMCD->nmcd.uItemState |= CDIS_SELECTED;
      }
      break;
  }
}

void CSetFontsDlg::OnGetMinMaxInfo(MINMAXINFO *lpMMI)
{
  m_Resizer.GetMinMaxInfo(lpMMI);
}

void CSetFontsDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);
  m_Resizer.OnSize(cx, cy);

  CRect clientRect;
  GetClientRect(&clientRect);
  if (m_gripper.m_hWnd != NULL) {
    m_gripper.SetWindowPos(NULL,
      clientRect.right - GRIPPER_SQUARE_SIZE,
      clientRect.bottom - GRIPPER_SQUARE_SIZE,
      GRIPPER_SQUARE_SIZE,
      GRIPPER_SQUARE_SIZE,
      SWP_NOZORDER | SWP_SHOWWINDOW);
  }
}

void CSetFontsDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
  CRect rc;
  GetWindowRect(&rc);
  ScreenToClient(&rc);
  int iWidth = rc.Width();

  bool bAdjustSize = false;
  if (!m_bResizing) {
    int nWidth = pRect->right - pRect->left;
    int nHeight = pRect->bottom - pRect->top;

    int nAdjustedWidth = (int)((double)nHeight * m_dfAspectRatio);
    int nAdjustedHeight = (int)((double)nWidth / m_dfAspectRatio);

    // Adjust the rectangle's dimensions
    switch (fwSide) {
      case WMSZ_LEFT:                // Adjust bottom
      case WMSZ_RIGHT:
        pRect->bottom = pRect->top + nAdjustedHeight;
        bAdjustSize = true;
        break;

      case WMSZ_TOP:                 // Adjust right
      case WMSZ_BOTTOM:
        pRect->right = pRect->left + nAdjustedWidth;
        bAdjustSize = true;
        break;

      case WMSZ_TOPLEFT:             // Adjust left
      case WMSZ_BOTTOMLEFT:
        pRect->left = pRect->right - nAdjustedWidth;
        bAdjustSize = true;
        break;

      case WMSZ_TOPRIGHT:            // Adjust right
      case WMSZ_BOTTOMRIGHT:
        pRect->right = pRect->left + nAdjustedWidth;
        bAdjustSize = true;
        break;

      default:
        break;
    }
  }

  if (bAdjustSize) {
    // Resize the window again, to the dimensions we prefer - play with width of Block Name
    m_bResizing = true;
    int xWidth = m_UnicodeRangeList.GetColumnWidth(UNICODE_NAME) + pRect->right - pRect->left - iWidth;

    MoveWindow(pRect);

    m_UnicodeRangeList.SetColumnWidth(UNICODE_NAME, xWidth);
    m_bResizing = false;
  } else {
    CDialog::OnSizing(fwSide, pRect);
  }
}

void CSetFontsDlg::OnGetDispInfoRangeList(NMHDR *pNMHDR, LRESULT *pResult)
{
  NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);
  *pResult = 0;

  // Create a pointer to the item
  LV_ITEM *pItem = &(pDispInfo)->item;

  if (pItem->mask & LVIF_TEXT) {
    CString cs_temp;

    // Which item number?
    int itemid = pItem->iItem;

    unicode_block UBlock;
    UBlock = (m_nDisplayType == ALL) ? vUCBlocks[itemid] : vUCBlocks_Subset[itemid];

    // Do the list need text information?
    switch (pItem->iSubItem) {
      case UNICODE_RANGE:
        cs_temp.Format(L"[%06X-%06X]", UBlock.imin, UBlock.imax);
        break;
      case UNICODE_NAME:
        cs_temp = UBlock.name;
        break;
      case CURRECT_FONT:
        if (UBlock.iUserFont >= 0) {
          cs_temp = vsInstalledFonts[UBlock.iUserFont].c_str();
        } else if (UBlock.iPreferredFont >= 0) {
          cs_temp = vsInstalledFonts[UBlock.iPreferredFont].c_str();
        } else if (UBlock.iBestAvailableFont >= 0) {
          cs_temp = vsInstalledFonts[UBlock.iBestAvailableFont].c_str();
        } else
          cs_temp = L"";
        break;
      default:
        ASSERT(0);
    }

    // Copy the text to the LV_ITEM structure
    // Maximum number of characters is in pItem->cchTextMax
    lstrcpyn(pItem->pszText, cs_temp, pItem->cchTextMax);
  }
}


void CSetFontsDlg::OnDisplayChange()
{
  m_UnicodeRangeList.SetItemState(m_nRangeItem, 0, LVIS_SELECTED);
  m_nRangeItem = -1;

  m_UnicodeRangeList.SetRedraw(FALSE);
  m_AvailableFontList.SetRedraw(FALSE);
  m_UnicodeRangeList.DeleteAllItems();
  m_AvailableFontList.DeleteAllItems();

  vUCBlocks_Subset.clear();

  int nIndex = m_cbxDisplayType.GetCurSel();
  m_nDisplayType = m_cbxDisplayType.GetItemData(nIndex);

  switch (m_nDisplayType) {
    case ALL:
      m_UnicodeRangeList.SetItemCount(NUMUNICODERANGES);
      break;
    case USER_SELECTED:
      for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
        if (vUCBlocks[iBlock].iUserFont >= 0)
          vUCBlocks_Subset.push_back(vUCBlocks[iBlock]);
      }
      m_UnicodeRangeList.SetItemCount(vUCBlocks_Subset.size());
      break;
    case PREFFERRED:
      for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
        if (vUCBlocks[iBlock].iUserFont < 0 && vUCBlocks[iBlock].iPreferredFont >= 0)
          vUCBlocks_Subset.push_back(vUCBlocks[iBlock]);
      }
      m_UnicodeRangeList.SetItemCount(vUCBlocks_Subset.size());
      break;
    case BEST_AVAILABLE:
      for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
        if (vUCBlocks[iBlock].iUserFont < 0 && vUCBlocks[iBlock].iPreferredFont < 0 &&
          vUCBlocks[iBlock].iBestAvailableFont >= 0)
          vUCBlocks_Subset.push_back(vUCBlocks[iBlock]);
      }
      m_UnicodeRangeList.SetItemCount(vUCBlocks_Subset.size());
      break;
    case NONE_FOUND:
      for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
        if (vUCBlocks[iBlock].iUserFont < 0 && vUCBlocks[iBlock].iPreferredFont < 0 &&
          vUCBlocks[iBlock].iBestAvailableFont < 0)
          vUCBlocks_Subset.push_back(vUCBlocks[iBlock]);
      }
      m_UnicodeRangeList.SetItemCount(vUCBlocks_Subset.size());
      break;
  }

  m_nSortedColumn1 = 0;
  m_bSortAscending1 = true;

  m_UnicodeRangeList.SetRedraw(TRUE);
  m_AvailableFontList.SetRedraw(TRUE);

  m_AvailableFontList.Invalidate();
  m_UnicodeRangeList.Invalidate();
}
