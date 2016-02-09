// UnicodeDlg.cpp : implementation file
//

#include "StdAfx.h"

#include "UCPickerDlg.h"
#include "HelpAboutDlg.h"
#include "SetFontsDlg.h"

#include "Unicode_Blocks.h"
#include "Unicode_Characters.h"
#include "TTStructures.h"
#include "Fonts.h"

#include "utility.h"

#include "resource.h"

#include "../SecString.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

// VS2015 does support char16_t and char32_t but VS2015 RC has problems!
inline void Utf32ToUtf16(uint32_t codepoint, wchar_t wc[3], ucode_info& uinfo)
{
  wc[0] = wc[1] = wc[2] = 0;
  uinfo.ucode = codepoint;

  if (codepoint < 0x10000) {
    uinfo.len = 1;
    wc[0] = static_cast<wchar_t>(codepoint);
    uinfo.iRTF1 = (short)((wc[0] > 0x7FFF) ? ((short int)wc[0] - 65536) : wc[0]);
  } else {
    if (codepoint <= 0x10FFFF) {
      uinfo.len = 2;
      codepoint -= 0x10000;
      wc[0] = (wchar_t)((codepoint >> 10) + 0xD800);
      wc[1] = (wchar_t)((codepoint & 0x3FF) + 0xDC00);
      uinfo.iRTF1 = (short)((wc[0] > 0x7FFF) ? ((short int)wc[0] - 65536) : wc[0]);
      uinfo.iRTF2 = (short)((wc[1] > 0x7FFF) ? ((short int)wc[1] - 65536) : wc[1]);
    } else {
      uinfo.len = 1;
      wc[0] = 0xFFFD;
      uinfo.iRTF1 = (short)((wc[0] > 0x7FFF) ? ((short int)wc[0] - 65536) : wc[0]);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CUCPickerDlg dialog

CUCPickerDlg::CUCPickerDlg(CWnd *pParent /*=NULL*/)
  : CDialog(CUCPickerDlg::IDD, pParent), m_pToolTipCtrl(NULL),
  m_numcharacters(0), m_offset(0), m_bDoneByValue(false), m_currentUBlock(-1)
{
  // Set background colour for for dialog as white
  m_pBkBrush.CreateSolidBrush(RGB(255, 255, 255));

  m_wcReserved[0] = 0xFFFD;
  m_wcReserved[1] = 0;

  m_wcUnsupported[0] = 0x007F;
  m_wcUnsupported[1] = 0;

  // Now load up user fonts
  LoadUserFonts();
}

CUCPickerDlg::~CUCPickerDlg()
{
  m_fntUnicode.DeleteObject();
  m_fntWarning.DeleteObject();
  m_fntRCHeadings.DeleteObject();
  m_fntDefaultUnicode.DeleteObject();
  m_fntByValue.DeleteObject();
  m_fntButtons.DeleteObject();
  m_fntDialogButtons.DeleteObject();

  if (m_pToolTipCtrl)
    delete m_pToolTipCtrl;

  m_pBkBrush.DeleteObject();

  SaveUserFonts();
}

void CUCPickerDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CUCPickerDlg)
  DDX_Control(pDX, IDC_UNICODE_BLOCK_BYNAME, m_cboxUnicodeBlockByName);
  DDX_Control(pDX, IDC_UNICODE_BLOCK_BYVALUE, m_cboxUnicodeBlockByValue);
  DDX_Control(pDX, IDC_BUFFER, m_richedit);

  DDX_Control(pDX, IDC_BUTTON_BLOCKNEXTBYNAME, m_btnN_Next);
  DDX_Control(pDX, IDC_BUTTON_BLOCKPREVBYNAME, m_btnN_Prev);
  DDX_Control(pDX, IDC_BUTTON_BLOCKNEXTBYVALUE, m_btnV_Next);
  DDX_Control(pDX, IDC_BUTTON_BLOCKPREVBYVALUE, m_btnV_Prev);
  DDX_Control(pDX, IDC_BUTTON_NEXTCHARS, m_btnC_Next);
  DDX_Control(pDX, IDC_BUTTON_PREVCHARS, m_btnC_Prev);
  DDX_Control(pDX, IDC_HELPABOUT, m_btnHelp);
  DDX_Control(pDX, IDC_SETFONTS, m_btnSetFont);
  DDX_Control(pDX, IDC_CLEARBUFFER, m_btnClearbuffer);
  DDX_Control(pDX, IDC_BACKSPACE, m_btnBackspace);
  DDX_Control(pDX, IDOK, m_btnExit);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUCPickerDlg, CDialog)
  //{{AFX_MSG_MAP(CUCPickerDlg)
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDOWN()
  ON_WM_QUERYDRAGICON()

  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDC_BUTTON_BLOCKNEXTBYNAME, OnBlockNextByName)
  ON_BN_CLICKED(IDC_BUTTON_BLOCKPREVBYNAME, OnBlockPrevByName)
  ON_BN_CLICKED(IDC_BUTTON_BLOCKNEXTBYVALUE, OnBlockNextByValue)
  ON_BN_CLICKED(IDC_BUTTON_BLOCKPREVBYVALUE, OnBlockPrevByValue)
  ON_BN_CLICKED(IDC_BUTTON_NEXTCHARS, OnNextChars)
  ON_BN_CLICKED(IDC_BUTTON_PREVCHARS, OnPrevChars)
  ON_BN_CLICKED(IDC_SETFONTS, OnSetFonts)
  ON_BN_CLICKED(IDC_HELPABOUT, OnHelpAbout)
  ON_BN_CLICKED(IDC_CLEARBUFFER, OnClearbuffer)
  ON_BN_CLICKED(IDC_BACKSPACE, OnBackspace)

  ON_CBN_SELCHANGE(IDC_UNICODE_BLOCK_BYNAME, OnUnicodeBlockByNameChange)
  ON_CBN_SELCHANGE(IDC_UNICODE_BLOCK_BYVALUE, OnUnicodeBlockByValueChange)

  ON_CONTROL_RANGE(BN_CLICKED, IDC_UCBTN00, IDC_UCBTN7F, OnUnicodeButtonClick)
  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTTNNeedText)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUCPickerDlg message handlers

BOOL CUCPickerDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  GetInstalledFonts();
  SetUnicodeBlockFonts();
  CreateAllFonts();

  // Set control fonts
  for (int i = 0; i < 8; i++) {
    GetDlgItem(IDC_STATIC_R0 + i)->SetFont(&m_fntRCHeadings);
  }

  for (int i = 0; i < 16; i++) {
    GetDlgItem(IDC_STATIC_C0 + i)->SetFont(&m_fntRCHeadings);
  }

  GetDlgItem(IDC_UNICODE_BLOCK_BYVALUE)->SetFont(&m_fntByValue);

  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
  CFont CaptionFont;
  CaptionFont.CreateFontIndirect(&ncm.lfCaptionFont);

  GetDlgItem(IDC_STATIC_CAPTION)->SetFont(&CaptionFont);

  // Set groupbox line thicker
  m_groupbox.SubclassDlgItem(IDC_STATIC_GROUPBOX, this);

  for (int i = 0; i < 128; i++) {
    m_UnicodeButtons[i].SubclassDlgItem(IDC_UCBTN00 + i, this);
    m_UnicodeButtons[i].ModifyStyle(0, BS_CENTER | BS_VCENTER);
    m_UnicodeButtons[i].SetFlatState(true);
    m_UnicodeButtons[i].SetDeadKeyState(false);
  }

  // Other buttons
  CVKBButton *pBtns[] = {&m_btnN_Next, &m_btnN_Prev,
                         &m_btnV_Next, &m_btnV_Prev,
                         &m_btnC_Next, &m_btnC_Prev,
                         &m_btnExit, &m_btnSetFont, &m_btnHelp,
                         &m_btnClearbuffer, &m_btnBackspace};

  size_t nbtns = sizeof(pBtns) / sizeof(*m_btnN_Next);

  for (size_t n = 0; n < nbtns; n++) {
    pBtns[n]->SetFont(&m_fntDialogButtons);
    pBtns[n]->SetDeadKeyState(false);
    pBtns[n]->SetColourChanges(false);
    pBtns[n]->SetButtonColour(RGB(255, 255, 255));
  }

  for (int i = 0; i < 128; i++) {
    m_UnicodeButtons[i].SetFont(&m_fntDefaultUnicode);
  }

  int nIndex;
  // Populate m_cboxUnicodeBlockByName
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    if (vUCBlocks[iBlock].imax_used == -1)
      continue;

    nIndex = m_cboxUnicodeBlockByName.AddString(vUCBlocks[iBlock].name);
    m_cboxUnicodeBlockByName.SetItemData(nIndex, iBlock);
  }

  // Populate m_cboxUnicodeBlockByValue
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    if (vUCBlocks[iBlock].imax_used == -1)
      continue;

    CString cs_name;
    cs_name.Format(L"U+%06X - U+%06X  [%s]", vUCBlocks[iBlock].imin, vUCBlocks[iBlock].imax,
                   vUCBlocks[iBlock].name);
    nIndex = m_cboxUnicodeBlockByValue.AddString(cs_name);
    m_cboxUnicodeBlockByValue.SetItemData(nIndex, iBlock);
  }

  // Map Unicode Block by name to its position in the Unicode space (ie its range)
  for (int i = 0; i < m_cboxUnicodeBlockByName.GetCount(); i++) {
    int iBlock = m_cboxUnicodeBlockByName.GetItemData(i);
    iMapUBlockName2Range[iBlock] = i;
  } //

  //m_richedit.SetNoClipboardFunctions();
  m_richedit.SetReadOnly(TRUE);
  m_richedit.SetBackgroundColor(FALSE, RGB(255, 255, 255));

  PARAFORMAT2 pf2;
  std::memset(&pf2, 0, sizeof(PARAFORMAT2));
  pf2.cbSize = sizeof(PARAFORMAT2);
  pf2.dwMask = PFM_SPACEBEFORE;
  pf2.dySpaceBefore = 24;

  m_richedit.SetParaFormat(pf2);

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
  // For somer eason the following return '10' rather than the normal '5000'
  // int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * 5000 /*iTime*/);

  // Set the tooltip
  CString csTemp;
  csTemp.LoadString(IDS_NEXT_BY_NAME);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_BLOCKNEXTBYNAME), csTemp);
  csTemp.LoadString(IDS_PREV_BY_NAME);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_BLOCKPREVBYNAME), csTemp);
  csTemp.LoadString(IDS_NEXT_BY_VALUE);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_BLOCKNEXTBYVALUE), csTemp);
  csTemp.LoadString(IDS_PREV_BY_VALUE);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_BLOCKPREVBYVALUE), csTemp);
  csTemp.LoadString(IDS_PAGE_FORWARD);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_NEXTCHARS), csTemp);
  csTemp.LoadString(IDS_PAGE_BACKWARD);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BUTTON_PREVCHARS), csTemp);

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUCPickerDlg::OnOK()
{
  MakeUnicodeRTFBuffer();
  EndDialog(IDOK);
}

void CUCPickerDlg::MakeUnicodeRTFBuffer()
{
  if (m_numcharacters == 0) {
    // No characters selected - just exit
    m_csRTFBuffer.Empty();
    return;
  }

  // We could use the CRichEditCtrl::StreamOut function but we know what we are doing!

  CSecString csHeader = L"{\\rtf1\\fbidis\\ansi\\ansicpg1252\\deff0\\nouicompat\\deflang2057";

  CSecString cs_temp;

  // Start fonttbl and add the default font
  cs_temp.Format(L"{\\fonttbl{\\f0\\fnil %s;}", DEFAULT_FONT);
  m_csRTFBuffer = csHeader + cs_temp;

  // Unique Fonts in use
  std::vector<int> vifonts;
  std::vector<int>::iterator iter;

  // Build fonttbl
  int ifontnumber = 1;
  for (int i = 0; i < m_numcharacters; i++) {
    // {\f0\fnil Arial Unicode MS;}{\f1\fnil Symbola;}{\f2\fnil Aegean;}}
    int ifont = m_viCharacterFonts[i];
    if (ifont >= 0) {
      iter = std::find(m_viCharacterFonts.begin(), m_viCharacterFonts.end(), ifont);
      if (std::distance(m_viCharacterFonts.begin(), iter) == i) {
        vifonts.push_back(ifontnumber);
        cs_temp.Format(L"{\\f%d\\fnil %s;}", ifontnumber, vsInstalledFonts.at(ifont).c_str());
        m_csRTFBuffer += cs_temp;
        ifontnumber++;
      } else {
        vifonts.push_back(vifonts[std::distance(m_viCharacterFonts.begin(), iter)]);
      }
    }
  }

  // Complete fonttbl
  m_csRTFBuffer += L"}";

  // Add unicode and start of characters
  m_csRTFBuffer += L"\\uc1\\pard";

  // Add characters with the correct font and point size 14 (fs28)
  for (int i = 0; i < m_numcharacters; i++) {
    // Add this character's font reference
    int ifont = m_viCharacterFonts[i];
    if (ifont < 0) {
      cs_temp.Format(L"\\f0\\fs28");
    } else {
      cs_temp.Format(L"\\f%d\\fs28", vifonts[i]);
    }
    m_csRTFBuffer += cs_temp;

    // Add this character in Unicode format.
    // Note: if the code > 0x7FFF (32767), then subtract 65536 for a negative number i.e.
    // \unnnnn? or \u-nnnnn? , i.e. unicode value between -32767 & 32767
    ucode_info uinfo = m_vuinfo[i];
    switch (uinfo.len) {
      case 1:
        cs_temp.Format(L"\\u%d?", uinfo.iRTF1);
        break;
      case 2:
        cs_temp.Format(L"\\u%d?\\u%d?", uinfo.iRTF1, uinfo.iRTF2);
        break;
      default:
        ASSERT(0);
    }
    m_csRTFBuffer += cs_temp;
  }

  // Go back to default font + point size 14 and end the RTF text
  m_csRTFBuffer += L"\\f0\\fs28}";
}

void CUCPickerDlg::SetUnicodeFont(CString cs_FontName)
{
  m_fntUnicode.DeleteObject();

  // create UNICODE Font and apply it to the ListCtrl.
  CDC *pDC = GetDC();

  // create UNICODE font
  LOGFONT  lf;
  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = MulDiv(14, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  wcscpy_s(lf.lfFaceName, cs_FontName);
  m_fntUnicode.CreateFontIndirect(&lf);

  for (int i = 0; i < 128; i++) {
    m_UnicodeButtons[i].SetFont(&m_fntUnicode);
  }

  // release the device context.
  ReleaseDC(pDC);
}

void CUCPickerDlg::OnUnicodeBlockByNameChange()
{
  BeginWaitCursor();

  SetRedraw(FALSE);

  GetDlgItem(IDOK)->EnableWindow(FALSE);

  int nIndex = m_cboxUnicodeBlockByName.GetCurSel();
  int iBlock = m_cboxUnicodeBlockByName.GetItemData(nIndex);

  m_currentUBlock = iBlock;

  SetUnicodeCharacters();

  SetRedraw(TRUE);

  Invalidate();

  GetDlgItem(IDOK)->EnableWindow(TRUE);

  EndWaitCursor();

  if (!m_bDoneByValue) {
    m_cboxUnicodeBlockByValue.SetCurSel(iBlock);
  }

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYNAME)->EnableWindow(nIndex == 0 ? FALSE : TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYNAME)->EnableWindow(nIndex == NUMUNICODERANGES - 1 ? FALSE : TRUE);

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYVALUE)->EnableWindow(iBlock == 0 ? FALSE : TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYVALUE)->EnableWindow(iBlock == NUMUNICODERANGES - 1 ? 
                FALSE : TRUE);

  GetDlgItem(IDC_BUTTON_PREVCHARS)->EnableWindow(m_offset == 0 ? FALSE : TRUE);
  GetDlgItem(IDC_BUTTON_NEXTCHARS)->EnableWindow(((vUCBlocks[m_currentUBlock].imin + m_offset + 0x80) > vUCBlocks[m_currentUBlock].imax) ? FALSE : TRUE);
}

void CUCPickerDlg::OnUnicodeBlockByValueChange()
{
  int nIndex = m_cboxUnicodeBlockByValue.GetCurSel();
  m_cboxUnicodeBlockByName.SetCurSel(iMapUBlockName2Range[nIndex]);
  m_bDoneByValue = true;
  OnUnicodeBlockByNameChange();
  m_bDoneByValue = false;
}

void CUCPickerDlg::SetUnicodeCharacters()
{
  const int jmin = (vUCBlocks[m_currentUBlock].imin + m_offset) / 16;
  //const int jmax = vUCBlocks[m_currentUBlock].imax_used;
  const int jmax = vUCBlocks[m_currentUBlock].imax;
  const std::vector<int> *pviReserved = vUCBlocks[m_currentUBlock].pviReserved;
  //const std::vector<short int> *pvDefaultFontIndex = vUCBlocks[m_currentUBlock].pvDefaultFontIndex;

  // Default font
  std::wstring wsFontName = DEFAULT_FONT;

  // Check if user has selected a font for this block
  if (!m_wsUserFonts[m_currentUBlock].empty()) {
    m_currentCharacterFont = vUCBlocks[m_currentUBlock].iUserFont;
    wsFontName = m_wsUserFonts[m_currentUBlock];
  } else {
    m_currentCharacterFont = vUCBlocks[m_currentUBlock].iPreferredFont;
    if (m_currentCharacterFont < 0) {
      m_currentCharacterFont = vUCBlocks[m_currentUBlock].iBestAvailableFont;
    }
    if (m_currentCharacterFont >= 0) {
      wsFontName = vsInstalledFonts.at(m_currentCharacterFont);
    }
  }

  m_fntButtons.DeleteObject();

  LOGFONT lf;
  lf.lfCharSet = DEFAULT_CHARSET;

  // create UNICODE font
  CDC *pDC = m_UnicodeButtons[0].GetDC();
  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = MulDiv(14, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  wcscpy_s(lf.lfFaceName, wsFontName.c_str());
  m_fntButtons.CreateFontIndirect(&lf);

  for (int i = 0; i < 128; i++) {
    m_UnicodeButtons[i].SetFont(&m_fntButtons, FALSE);
  }

  ReleaseDC(pDC);

  for (int i = 0; i < 8; i++) {
    // Character boundaries are always on a 0x0 boundary
    bool bTooLarge = ((i + jmin) * 16) > jmax;
    if (bTooLarge) {
      GetDlgItem(IDC_STATIC_R0 + i)->EnableWindow(FALSE);
      GetDlgItem(IDC_STATIC_R0 + i)->ShowWindow(SW_HIDE);
    } else {
    CString cs_temp;
    cs_temp.Format(L"U+%04x-", jmin + i);
    GetDlgItem(IDC_STATIC_R0 + i)->SetWindowText(cs_temp);
      GetDlgItem(IDC_STATIC_R0 + i)->EnableWindow(TRUE);
      GetDlgItem(IDC_STATIC_R0 + i)->ShowWindow(SW_SHOW);
    }
    //TRACE(L"Row %d (ID = %d) text: %s\r\n", i - jmin, i - jmin + IDC_STATIC_R0, cs_temp);

    wchar_t wc[3];
    for (int j = 0; j < 16; j++) {
      bool bReservedCharacter = pviReserved != NULL &&
        std::find(pviReserved->begin(), pviReserved->end(), i * 16 + j) != pviReserved->end();
      m_UnicodeButtons[i * 16 + j].EnableWindow(TRUE);
      m_UnicodeButtons[i * 16 + j].ShowWindow(SW_SHOW);

      if (bReservedCharacter) {
        m_UnicodeButtons[i * 16 + j].SetWindowText(m_wcReserved);
        m_UnicodeButtons[i * 16 + j].SetDeadKeyState(true);
        continue;
      }
      if (bTooLarge) {
        m_UnicodeButtons[i * 16 + j].SetWindowText(L"");
        m_UnicodeButtons[i * 16 + j].SetDeadKeyState(true);
        m_UnicodeButtons[i * 16 + j].EnableWindow(FALSE);
        m_UnicodeButtons[i * 16 + j].ShowWindow(SW_HIDE);
        continue;
      }

      ucode_info uinfo;
      Utf32ToUtf16((i + jmin) * 16 + j, wc, uinfo);

      m_UnicodeButtons[i * 16 + j].SetWindowText(wc);
      m_UnicodeButtons[i * 16 + j].SetDeadKeyState(wc[0] == 0xFFFD ? true : false);
      //TRACE(L"Button %d text %s\r\n", (i - jmin) * 16 + j, wc);
      m_UnicodeButtons[i * 16 + j].SetButtonUCode(uinfo);
    }
  }
}

BOOL CUCPickerDlg::PreTranslateMessage(MSG *pMsg)
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

HCURSOR CUCPickerDlg::OnQueryDragIcon()
{
  return (HCURSOR)m_hIcon;
}

HBRUSH CUCPickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Note despite CTLCOLOR_BTN being defined - it does not work!
  // Use subclassed button class

  switch (nCtlColor) {
    case CTLCOLOR_STATIC:
      if (pWnd->GetDlgCtrlID() == IDC_STATIC_WARNING) {
        pDC->SetTextColor(RGB(255, 0, 0));
        pDC->SelectObject(&m_fntWarning);
      }
    case CTLCOLOR_DLG:
    case CTLCOLOR_EDIT:
    case CTLCOLOR_LISTBOX:
      pDC->SetBkColor(RGB(255, 255, 255));
      return (HBRUSH)(m_pBkBrush.GetSafeHandle());
      break;
  }
  return hbr;
}

void CUCPickerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Allow draging without the caption bar!
  CDialog::OnLButtonDown(nFlags, point);

  PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}

void CUCPickerDlg::OnBlockNextByName()
{
  m_offset = 0;

  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();

  if (icurrent_selection == NUMUNICODERANGES - 1)
    return;

  int iNewIndex = icurrent_selection + 1;
  m_cboxUnicodeBlockByName.SetCurSel(iNewIndex);

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYNAME)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYNAME)->EnableWindow(iNewIndex == NUMUNICODERANGES - 1 ?
               FALSE : TRUE);

  OnUnicodeBlockByNameChange();
}

void CUCPickerDlg::OnBlockPrevByName()
{
  m_offset = 0;

  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();

  if (icurrent_selection == 0)
    return;

  int iNewIndex = icurrent_selection - 1;
  m_cboxUnicodeBlockByName.SetCurSel(iNewIndex);

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYNAME)->EnableWindow(iNewIndex == 0 ? FALSE : TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYNAME)->EnableWindow(TRUE);

  OnUnicodeBlockByNameChange();
}

void CUCPickerDlg::OnBlockNextByValue()
{
  m_offset = 0;

  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();
  int iBlock = m_cboxUnicodeBlockByName.GetItemData(icurrent_selection) + 1;

  if (iBlock == NUMUNICODERANGES)
    return;

  m_cboxUnicodeBlockByName.SetCurSel(iMapUBlockName2Range[iBlock]);

  OnUnicodeBlockByNameChange();

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYVALUE)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYVALUE)->EnableWindow(iBlock == NUMUNICODERANGES - 1 ?
                FALSE : TRUE);
}

void CUCPickerDlg::OnBlockPrevByValue()
{
  m_offset = 0;

  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();
  int iBlock = m_cboxUnicodeBlockByName.GetItemData(icurrent_selection) - 1;

  if (iBlock < 0)
    return;

  m_cboxUnicodeBlockByName.SetCurSel(iMapUBlockName2Range[iBlock]);

  OnUnicodeBlockByNameChange();

  GetDlgItem(IDC_BUTTON_BLOCKPREVBYVALUE)->EnableWindow(iBlock == 0 ? FALSE : TRUE);
  GetDlgItem(IDC_BUTTON_BLOCKNEXTBYVALUE)->EnableWindow(TRUE);
}

void CUCPickerDlg::OnNextChars()
{
  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();
  int iBlock = m_cboxUnicodeBlockByName.GetItemData(icurrent_selection);

  if ((vUCBlocks[iBlock].imin + m_offset + 0x80) < vUCBlocks[iBlock].imax_used)
    m_offset += 0x80;

  OnUnicodeBlockByNameChange();

  GetDlgItem(IDC_BUTTON_PREVCHARS)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_NEXTCHARS)->EnableWindow((vUCBlocks[iBlock].imin + m_offset + 0x80) >=
                vUCBlocks[iBlock].imax_used ? FALSE : TRUE);
}

void CUCPickerDlg::OnPrevChars()
{
  int icurrent_selection = m_cboxUnicodeBlockByName.GetCurSel();
  int iBlock = m_cboxUnicodeBlockByName.GetItemData(icurrent_selection);

  if ((vUCBlocks[iBlock].imin + m_offset) > vUCBlocks[iBlock].imin)
    m_offset -= 0x80;

  OnUnicodeBlockByNameChange();

  GetDlgItem(IDC_BUTTON_NEXTCHARS)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_PREVCHARS)->EnableWindow((vUCBlocks[iBlock].imin + m_offset) <=
                vUCBlocks[iBlock].imin ? FALSE : TRUE);
}

void CUCPickerDlg::OnUnicodeButtonClick(UINT nID)
{
  if (m_UnicodeButtons[nID - IDC_UCBTN00].GetDeadKeyState())
    return;

  CSecString cs_char;
  ucode_info uinfo;
  m_UnicodeButtons[nID - IDC_UCBTN00].GetWindowText(cs_char);
  m_UnicodeButtons[nID - IDC_UCBTN00].GetButtonUCode(uinfo);
  if (uinfo.ucode == -1) {
    // No block selected
    return;
  }

  switch (uinfo.ucode) {
    case 0x00FFFE:
    case 0x00FFFF:
    case 0x0FFFFE:
    case 0x0FFFFF:
    case 0x10FFFE:
    case 0x10FFFF:
      // Not a character
      return;
    default:
      break;
  }

  m_vuinfo.push_back(uinfo);

  // get the initial text length
  int nLength = m_richedit.GetTextLength();

  // put the selection at the end of text
  m_richedit.SetSel(nLength, nLength);

  CDC *pDC = GetDC();

  CHARFORMAT cf;
  memset(&cf, 0, sizeof(CHARFORMAT));
  cf.cbSize = sizeof(CHARFORMAT);
  cf.dwMask = CFM_FACE | CFM_SIZE;
  cf.dwEffects = 0;
  cf.yHeight = 280; // 14 point
  if (m_currentCharacterFont >= 0) {
    wcscpy_s(cf.szFaceName, vsInstalledFonts.at(m_currentCharacterFont).c_str());
  } else {
    wcscpy_s(cf.szFaceName, DEFAULT_FONT);
  }
  m_richedit.SetSelectionCharFormat(cf);

  // replace the selection
  m_richedit.ReplaceSel(cs_char);

  m_numcharacters++;
  m_viCharacterFonts.push_back(m_currentCharacterFont);

  m_csBuffer = m_csBuffer + cs_char;

  ReleaseDC(pDC);
}

static int CMAP_BUFFER_length = 0;
static char *pCMAP_BUFFER = NULL;

void CUCPickerDlg::GetInstalledFonts()
{
  LOGFONT  lf;
  std::memset(&lf, 0, sizeof(lf));
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName[0] = 0;
  lf.lfPitchAndFamily = 0;

  // Used in EnumFontFamExProc
  m_hdc = ::GetDC(NULL);

  // Enumerate fonts twice - first to get them all, next to process them
  // Note: EnumFontFamExProc will be called multiple times as DEFAULT_CHARSET
  // has been specified.  Use m_vbFontProcessed to ensure we only process
  // it once.
  CMAP_BUFFER_length = 65536;
  pCMAP_BUFFER = (char *)malloc(CMAP_BUFFER_length);

  m_bEnumFontsFirst = true;
  ::EnumFontFamiliesEx(m_hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)this, 0);

  std::sort(vsInstalledFonts.begin(), vsInstalledFonts.end());

  for (size_t i = 0; i < vsInstalledFonts.size(); i++) {
    m_vbFontProcessed.push_back(false);
  }

  m_bEnumFontsFirst = false;
  ::EnumFontFamiliesEx(m_hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)this, 0);

  m_vbFontProcessed.clear();

  free(pCMAP_BUFFER);
  CMAP_BUFFER_length = 0;
  pCMAP_BUFFER = NULL;

  ::ReleaseDC(NULL, m_hdc);

  m_hdc = NULL;
}

BOOL CALLBACK CUCPickerDlg::EnumFontFamExProc(LOGFONT *lpelfe, NEWTEXTMETRIC *lpntme,
  DWORD FontType, LPVOID lParam)
{
  UNREFERENCED_PARAMETER(lpntme);

  CUCPickerDlg *pSelf = (CUCPickerDlg *)lParam;

  if (!(FontType & TRUETYPE_FONTTYPE))
    return TRUE;

  if ((lpelfe->lfWeight != FW_NORMAL && lpelfe->lfWeight != FW_MEDIUM) ||
    lpelfe->lfItalic || lpelfe->lfUnderline || lpelfe->lfStrikeOut)
    return TRUE;

  wchar_t *wcFontName = lpelfe->lfFaceName[0] == L'@' ? lpelfe->lfFaceName + 1 : lpelfe->lfFaceName;;

  if (pSelf->m_bEnumFontsFirst) {
    // Populate vector
    if (std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), wcFontName) == 
        vsInstalledFonts.end()) {
      vsInstalledFonts.push_back(wcFontName);

      for (int i = 0; i < NUMFONTINDICES; i++) {
        if (wcscmp(DefaultFontIndex[i].name, wcFontName) == 0) {
          DefaultFontIndex[i].installed = FONT_INSTALLED;
          break;
        }
      }
    }
    return TRUE;
  }

  HFONT hfont;

  hfont = CreateFontIndirectEx((ENUMLOGFONTEXDV *)lpelfe);

  if (hfont == NULL)
    return FALSE;

  if (SelectObject(pSelf->m_hdc, hfont) == NULL) {
    DeleteObject(hfont);
    return FALSE;
  }

  DWORD dw = GetFontData(pSelf->m_hdc, CMAP, 0, NULL, 0);

  if (dw != GDI_ERROR && dw > 0) {
    if ((int)dw > CMAP_BUFFER_length) {
      free(pCMAP_BUFFER);
      CMAP_BUFFER_length = ((dw >> 12) + 1) << 12;
      pCMAP_BUFFER = (char *)malloc(CMAP_BUFFER_length);
    }
    dw = GetFontData(pSelf->m_hdc, CMAP, 0, pCMAP_BUFFER, dw);
    ASSERT(dw != GDI_ERROR && dw > 0);
    pSelf->ProcessFontCMAPTable(wcFontName, pCMAP_BUFFER);
  }

  DeleteObject(hfont);

  return TRUE;
}

void CUCPickerDlg::ProcessFontCMAPTable(const wchar_t *wcFontname, char *p)
{
  std::vector<std::wstring>::iterator iter;

  iter = std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), wcFontname);
  if (iter == vsInstalledFonts.end())
    ASSERT(0);

  const int iFontIndex = std::distance(vsInstalledFonts.begin(), iter);

  // Only process the font once
  if (m_vbFontProcessed[iFontIndex])
    return;

  char *ptr = p;
  st_cmapindex *pcmapindex = (st_cmapindex *)ptr;

  UINT16 version, numsubtables, platformID, platformSpecificID, format;
  UINT32 offset;
  bool bFormat12Found(false);

  version = SWAP_UINT16(pcmapindex->version);
  numsubtables = SWAP_UINT16(pcmapindex->numsubtables);

  //Trace(L"Font: %s \r\n", wcFontname);
  //Trace(L"\tcmapindex: version = %d; numsubtables = %d\r\n", version, numsubtables);

  ptr += sizeof(st_cmapindex);
  st_subtable_header *pcmapindexhdr = (st_subtable_header *)(ptr);

  for (int i = 0; i < numsubtables; i++) {
    platformID = SWAP_UINT16(pcmapindexhdr->platformID);
    if (platformID == 3) {
      platformSpecificID = SWAP_UINT16(pcmapindexhdr->platformSpecificID);
      offset = SWAP_UINT32(pcmapindexhdr->offset);

      //Trace(L"\t\tcmapsubheader#1: platformID = %d; platformSpecificID = %d; offset = 0x%08x\r\n",
      //  platformID, platformSpecificID, offset);

      st_format_header *pfmthdr = (st_format_header *)(p + offset);

      format = SWAP_UINT16(pfmthdr->format);

      if (format == 12) {
        bFormat12Found = true;
        break;
      }
    }
    pcmapindexhdr++;
  }

  // Reset header pointer
  pcmapindexhdr = (st_subtable_header *)(ptr);

  UINT16 ifBlocks[NUMUNICODERANGES];
  std::memset(ifBlocks, 0, NUMUNICODERANGES * sizeof(UINT16));

  for (int i = 0; i < numsubtables; i++) {
    platformID = SWAP_UINT16(pcmapindexhdr->platformID);
    platformSpecificID = SWAP_UINT16(pcmapindexhdr->platformSpecificID);
    offset = SWAP_UINT32(pcmapindexhdr->offset);
    //Trace(L"\t\tcmapsubheader#2: platformID = %d; platformSpecificID = %d; offset = 0x%08x\r\n",
    //  platformID, platformSpecificID, offset);

    if (platformID == 3) {
      st_format_header *pfmthdr = (st_format_header *)(p + offset);

      format = SWAP_UINT16(pfmthdr->format);

      switch (format) {
        case 0:
          Trace(L"\t\t\tcmap format  0 header\r\n");
          break;
        case 2:
          Trace(L"\t\t\tcmap format  2 header\r\n");
          break;
        case 4:
        {
          if (!bFormat12Found) {
            UINT16 length, language, segCountX2;
            st_format4 *pfmthdr4 = (st_format4 *)pfmthdr;
            length = SWAP_UINT16(pfmthdr4->length);
            language = SWAP_UINT16(pfmthdr4->language);

            segCountX2 = SWAP_UINT16(pfmthdr4->segCountX2);

            //Trace(L"\t\t\tcmap format  4 header: format = %d; length = %d; language = %d" \
            //  L" segCountX2 = %d;\r\n", format, length, language, segCountX2);


            UINT16 *piendCodes = &(pfmthdr4->endCode);
            UINT16 *pistartCodes = piendCodes + (segCountX2 / 2) + 1;

            for (int icodes = 0; icodes < segCountX2 / 2; icodes++) {
              UINT16 istart = SWAP_UINT16(*pistartCodes);
              UINT16 iend = SWAP_UINT16(*piendCodes);
              //Trace(L"\t\t\t\t#characters: %4d; startCode = %04x; endCode = %04x;\r\n",
              //  iend - istart + 1, istart, iend);

              short int kstart(-NUMUNICODERANGES), kend(-NUMUNICODERANGES);

              for (short int iBlock = NUMUNICODERANGES - 1; iBlock >= 0; iBlock--) {
                if (iend <= vUCBlocks[iBlock].imax) {
                  kend = iBlock;
                }
                if (istart >= (UINT16)vUCBlocks[iBlock].imin) {
                  kstart = iBlock;
                }
                if (kstart + kend >= 0)
                  break;
              }

              for (int k = kstart; k <= kend; k++) {
                if (iend < vUCBlocks[k].imax) {
                  ifBlocks[k] += iend - istart + 1;
                } else {
                  ifBlocks[k] += (UINT16)vUCBlocks[k].imax - istart + 1;
                  istart = (UINT16)vUCBlocks[k + 1].imin;
                }
              }
              piendCodes++;
              pistartCodes++;
            }
          }
          break;
        }
        case 6:
          Trace(L"\t\t\tcmap format  6 header\r\n");
          break;
        case 8:
          Trace(L"\t\t\tcmap format  8 header\r\n");
          break;
        case 10:
          Trace(L"\t\t\tcmap format 10 header\r\n");
          break;
        case 12:
        {
          //Trace(L"\t\t\tcmap format 12 header\r\n");
          UINT32 length, language, nGroups;

          st_format12 *pfmthdr12 = (st_format12 *)pfmthdr;
          length = SWAP_UINT32(pfmthdr12->length);
          language = SWAP_UINT32(pfmthdr12->language);
          nGroups = SWAP_UINT32(pfmthdr12->nGroups);
          //Trace(L"\t\t\tcmap format 12 header: format = %d; length = %d; language = %d; nGroups = %d\r\n",
          //  format, length, language, nGroups);

          st_group *pgroup = (st_group *)(pfmthdr12 + 1);
          for (UINT j = 0; j < nGroups; j++) {
            int istart = SWAP_UINT32(pgroup->startCharCode);
            int iend = SWAP_UINT32(pgroup->endCharCode);
            //Trace(L"\t\t\t\tcmap format 12 group %4d: #characters: %4d; starwchar_tCode = 0x%06x; endCharCode = 0x%06x\r\n",
            //  j, iend - istart + 1, istart, iend);
            pgroup++;

            short int kstart(-NUMUNICODERANGES), kend(-NUMUNICODERANGES);

            for (short int iBlock = NUMUNICODERANGES - 1; iBlock >= 0; iBlock--) {
              if (iend <= vUCBlocks[iBlock].imax) {
                kend = iBlock;
              }
              if (istart >= vUCBlocks[iBlock].imin) {
                kstart = iBlock;
              }
              if (kstart + kend >= 0)
                break;
            }

            for (int k = kstart; k <= kend; k++) {
              if (iend < vUCBlocks[k].imax) {
                ifBlocks[k] += (UINT16)(iend - istart) + 1;
              } else {
                ifBlocks[k] += (UINT16)(vUCBlocks[k].imax - istart) + 1;
                istart = (UINT16)vUCBlocks[k + 1].imin;
              }
            }
          }
          break;
        }
        case 13:
          Trace(L"\t\t\tcmap format 13 header\r\n");
          break;
        case 14:
          Trace(L"\t\t\tcmap format 14 header\r\n");
          break;
        default:
          Trace(L"\t\t\t*** UNKNOWN cmap format header ***\r\n");
          break;
      }
    }
    pcmapindexhdr++;
  }

  std::map<int, int> mapUBlock2NumChars;
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    if (ifBlocks[iBlock] > 0) {
      mapUBlock2NumChars[iBlock] = ifBlocks[iBlock];
    }
  }
  m_mapFont2UBlock2NumChars[iFontIndex] = mapUBlock2NumChars;

  m_vbFontProcessed[iFontIndex] = true;
}

void CUCPickerDlg::SetUnicodeBlockFonts()
{
  // Now create map UCBlocks to Fonts to NumChars!
  std::map<int, std::map<int, int>>::iterator font_iter;
  std::map<int, int>::iterator ublock_iter;
  for (font_iter = m_mapFont2UBlock2NumChars.begin(); 
           font_iter != m_mapFont2UBlock2NumChars.end(); font_iter++) {
    //int iFontIndex = font_iter->first;
    for (ublock_iter = font_iter->second.begin(); ublock_iter != font_iter->second.end(); ublock_iter++) {
      //int nBlock = ublock_iter->first;
      m_mapFont2NumChars[ublock_iter->first].insert(std::pair<int, int>(font_iter->first, ublock_iter->second));
    }
  }

  // Set the preferred & best available fonts for each uBlock
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    vUCBlocks[iBlock].iPreferredFont = -1;
    vUCBlocks[iBlock].iBestAvailableFont = -1;

    const std::vector<int> *pvifont = vUCBlocks[iBlock].pvDefaultFontIndex;

    if (pvifont != NULL && pvifont->size() > 0) {
      std::vector<int>::const_iterator citer;
      for (citer = pvifont->cbegin(); citer != pvifont->cend(); citer++) {
        int ifont = *citer;
        if (DefaultFontIndex[ifont].installed == FONT_INSTALLED) {
          std::vector<std::wstring>::iterator wsiter;
          wsiter = std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), DefaultFontIndex[ifont].name);
          if (wsiter != vsInstalledFonts.end()) {
            std::wstring wsfont = *wsiter;
            vUCBlocks[iBlock].iPreferredFont = std::distance(vsInstalledFonts.begin(), wsiter);
            break;
          }
        }
      }
    }

    if (vUCBlocks[iBlock].iPreferredFont < 0) {
      // Preferred font not present - find best available.
      int jmax = -1;
      int k = -1;
      std::map<int, int>::iterator iter;
      for (iter = m_mapFont2NumChars[iBlock].begin();
                iter != m_mapFont2NumChars[iBlock].end(); iter++) {
        int xx = iter->second;
        if (xx > jmax) {
          jmax = xx;
          k = iter->first;
          if (jmax == vUCBlocks[iBlock].numchars)
            break;
        }
      }

      if (jmax > 0)
        vUCBlocks[iBlock].iBestAvailableFont = k;
    }
  }

  // Set user selected font for each uBlock
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    if (!m_wsUserFonts[iBlock].empty()) {
      std::vector<std::wstring>::iterator wsiter;
      wsiter = std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), m_wsUserFonts[iBlock]);
      if (wsiter != vsInstalledFonts.end()) {
        std::wstring wsfont = *wsiter;
        int id = std::distance(vsInstalledFonts.begin(), wsiter);
        if (id == vUCBlocks[iBlock].iPreferredFont) {
          // User selection is the same as the preferred font
          m_wsUserFonts[iBlock].clear();
          vUCBlocks[iBlock].iUserFont = -1;
        } else {
          vUCBlocks[iBlock].iUserFont = id;
        }
      }
    }
  }
}

void CUCPickerDlg::CreateAllFonts()
{
  // create normal fonts
  CDC *pDC = GetDC();

  LOGFONT  lf;
  std::memset(&lf, 0, sizeof(lf));
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName[0] = 0;
  lf.lfPitchAndFamily = 0;
  lf.lfHeight = MulDiv(12, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_BOLD;
  lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  wcscpy_s(lf.lfFaceName, DEFAULT_FONT);
  m_fntWarning.CreateFontIndirect(&lf);

  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = MulDiv(12, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  wcscpy_s(lf.lfFaceName, L"Courier New");
  m_fntRCHeadings.CreateFontIndirect(&lf);

  CFont *pFont = GetDlgItem(IDC_UNICODE_BLOCK_BYVALUE)->GetFont();
  pFont->GetLogFont(&lf);
  wcscpy_s(lf.lfFaceName, L"Courier New");
  m_fntByValue.CreateFontIndirect(&lf);

  pFont = m_btnN_Next.GetFont();
  pFont->GetLogFont(&lf);
  lf.lfWeight = FW_SEMIBOLD;
  m_fntDialogButtons.CreateFontIndirect(&lf);

  // create UNICODE font
  std::memset(&lf, 0, sizeof(lf));
  lf.lfHeight = MulDiv(14, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  wcscpy_s(lf.lfFaceName, DEFAULT_FONT);
  m_fntDefaultUnicode.CreateFontIndirect(&lf);

  // release the device context.
  ReleaseDC(pDC);
}

void CUCPickerDlg::OnHelpAbout()
{
  CHelpAboutDlg dlg(this, &m_fntDefaultUnicode);

  dlg.DoModal();
}

void CUCPickerDlg::OnSetFonts()
{
  // Save current values
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    m_wsSaveUserFonts[iBlock] = m_wsUserFonts[iBlock];
    m_iSaveCurrentUBlockFont[iBlock] = vUCBlocks[iBlock].iUserFont;
  }

  CSetFontsDlg dlg(this);

  INT_PTR rc = dlg.DoModal();

  // Make sure back in order for display
  std::sort(vUCBlocks.begin(), vUCBlocks.end(), CompareBlockNumberA);

  if (rc == IDCANCEL) {
    // Restore values
    for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
      m_wsUserFonts[iBlock] = m_wsSaveUserFonts[iBlock];
      vUCBlocks[iBlock].iUserFont = m_iSaveCurrentUBlockFont[iBlock];
    }
  } else {
    // Check if user has selected a font for this block
    if (m_currentUBlock > -0) {
      if (!m_wsUserFonts[m_currentUBlock].empty()) {
        m_currentCharacterFont = vUCBlocks[m_currentUBlock].iUserFont;
      } else {
        m_currentCharacterFont = vUCBlocks[m_currentUBlock].iPreferredFont;
        if (m_currentCharacterFont < 0) {
          m_currentCharacterFont = vUCBlocks[m_currentUBlock].iBestAvailableFont;
        }
        if (m_currentCharacterFont >= 0) {
        }
      }
    }
  }
}

std::string GetCharacterName(size_t nChar)
{
  std::string str = UCNames[nChar];

  for (;;) {
    size_t iHash = str.find("#");
    if (iHash == std::string::npos)
      break;
    int nWord = atoi(str.substr(iHash + 1, 3).c_str());
    str.replace(iHash, 4, UCName_Words[nWord]);
  }
  return str;
}

BOOL CUCPickerDlg::OnTTNNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
  UNREFERENCED_PARAMETER(id);

  *pResult = 0;

  NMTTDISPINFO *pTTT = reinterpret_cast<NMTTDISPINFO *>(pNMHDR);
  int nID = pNMHDR->idFrom;
  BOOL bRet = FALSE;

  if (pTTT->uFlags & TTF_IDISHWND) {
    // idFrom is actually the HWND of the tool
    nID = ::GetDlgCtrlID((HWND)nID);
    if (nID >= IDC_UCBTN00 && nID <= IDC_UCBTN7F) {
      ucode_info uinfo;
      ((CVKBButton *)GetDlgItem(nID))->GetButtonUCode(uinfo);
      if (uinfo.ucode >= 0) {
        std::vector<int>::const_iterator low;
        low = std::lower_bound(vUCNameIndex.begin(), vUCNameIndex.end(), uinfo.ucode);
        if (low != vUCNameIndex.end()) {
          if (vUCNameIndex[low - vUCNameIndex.begin()] == uinfo.ucode) {
            size_t convertedChars = 0;
            wchar_t wcValue[256];
            mbstowcs_s(&convertedChars, wcValue, sizeof(wcValue) / sizeof(wchar_t),
              GetCharacterName(low - vUCNameIndex.begin()).c_str(), _TRUNCATE);
            swprintf_s(pTTT->szText, sizeof(pTTT->szText) / sizeof(wchar_t), wcValue);
          } else {
            CString cs_temp;
            if ((uinfo.ucode >= 0x00E000 && uinfo.ucode <= 0x00F8FF) ||
                (uinfo.ucode >= 0x0F0000 && uinfo.ucode <= 0x0FFFFF) ||
                (uinfo.ucode >= 0x100000 && uinfo.ucode <= 0x10FFFF)) {
              cs_temp.Format(IDS_PRIVATE, uinfo.ucode);
            } else {
            switch (uinfo.ucode) {
              case 0x00FFFE:
              case 0x00FFFF:
              case 0x0FFFFE:
              case 0x0FFFFF:
              case 0x10FFFE:
              case 0x10FFFF:
                cs_temp.LoadStringW(IDS_NOT_A_CHAR);
                break;
              default:
                cs_temp.Format(L"\\U%08X?", uinfo.ucode);
                break;
              }
            }
            swprintf_s(pTTT->szText, sizeof(pTTT->szText) / sizeof(wchar_t), (LPCWSTR)cs_temp);
          }
          pTTT->hinst = AfxGetResourceHandle();
          bRet = TRUE;
        } else {
          CString cs_temp;
          if ((uinfo.ucode >= 0x00E000 && uinfo.ucode <= 0x00F8FF) ||
              (uinfo.ucode >= 0x0F0000 && uinfo.ucode <= 0x0FFFFF) ||
              (uinfo.ucode >= 0x100000 && uinfo.ucode <= 0x10FFFF)) {
            cs_temp.Format(IDS_PRIVATE, uinfo.ucode);
          } else {
            cs_temp.Format(L"\\U%08X?", uinfo.ucode);
          }
          swprintf_s(pTTT->szText, sizeof(pTTT->szText) / sizeof(wchar_t), (LPCWSTR)cs_temp);
          bRet = TRUE;
        }
      }
    }
  }

  return bRet;
}

void CUCPickerDlg::GetConfigFile()
{
  // Get Config file directory
#ifndef _DEBUG
  const std::wstring wsConfigDir(L"\\UCPicker\\");
#else
  const std::wstring wsConfigDir(L"\\UCPickerD\\");
#endif
  std::wstring sPath, sDrive, sDir, sName, sExt;

  // returns the directory part of ::GetModuleFileName()
  wchar_t wcPath[MAX_PATH + 1];

  if (GetModuleFileName(NULL, wcPath, MAX_PATH + 1) != 0) {
    // guaranteed file name of at least one character after path '\'
    *(wcsrchr(wcPath, L'\\') + 1) = L'\0';
  } else {
    wcPath[0] = wchar_t('\\');
    wcPath[1] = wchar_t('\0');
  }
  sPath = wcPath;

  splitpath(sPath, sDrive, sDir, sName, sExt);
  sDrive += L"\\"; // Trailing slash required.

  const UINT uiDT = ::GetDriveType(sDrive.c_str());
  if (uiDT == DRIVE_FIXED || uiDT == DRIVE_REMOTE) {
    std::wstring sLocalAppDataPath;

    // Get the special folder path - do not create it if it does not exist
    BOOL brc = SHGetSpecialFolderPath(NULL, wcPath, CSIDL_LOCAL_APPDATA, FALSE);

    if (brc == TRUE) {
      // Call to 'SHGetSpecialFolderPath' worked
      sLocalAppDataPath = wcPath;
    } else {
      // Unsupported release or 'SHGetSpecialFolderPath' failed
      sLocalAppDataPath = L"";
    }

    if (brc)
      m_wsConfigFile = sLocalAppDataPath + wsConfigDir;

    if (PathFileExists(m_wsConfigFile.c_str()) == FALSE) {
      if (_wmkdir(m_wsConfigFile.c_str()) != 0)
        m_wsConfigFile = L""; // couldn't create dir!?
    }
  } else if (uiDT == DRIVE_REMOVABLE) {
    std::wstring::size_type index = sDir.rfind(L"Program\\");
    if (index != std::wstring::npos)
      m_wsConfigFile = sPath.substr(0, sPath.length() - 8);
  }

  m_wsConfigFile = m_wsConfigFile + L"config.ini";
}

bool CUCPickerDlg::LoadUserFonts()
{
  GetConfigFile();

  wchar_t wcValue[256];
  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    CString cs_Key;
    cs_Key.Format(L"Block_%06X_%06X", vUCBlocks[iBlock].imin, vUCBlocks[iBlock].imax);
    DWORD dw = GetPrivateProfileString(L"User_Fonts", cs_Key, NULL, wcValue,
                 sizeof(wcValue) / sizeof(wchar_t),
                 m_wsConfigFile.c_str());
    m_wsUserFonts[iBlock] = (dw > 0) ? wcValue : L"";
  }

  return true;
}

bool CUCPickerDlg::SaveUserFonts()
{
  std::sort(vUCBlocks.begin(), vUCBlocks.end(), CompareBlockNumberA);

  for (int iBlock = 0; iBlock < NUMUNICODERANGES; iBlock++) {
    CString cs_Key;
    cs_Key.Format(L"Block_%06X_%06X", vUCBlocks[iBlock].imin, vUCBlocks[iBlock].imax);
    size_t n = m_wsUserFonts[iBlock].length();
    WritePrivateProfileString(L"User_Fonts", cs_Key, n == 0 ? NULL : m_wsUserFonts[iBlock].c_str(),
             m_wsConfigFile.c_str());
  }

  return true;
}

int CUCPickerDlg::SetUserFont(const int iNumber, const int index, std::wstring wsFontName)
{
  if (iNumber < 0) {
    // Remove
    m_wsUserFonts[-iNumber] = L"";
    vUCBlocks[index].iUserFont = -1;
    return -1;
  } else {
    // Add
    std::vector<std::wstring>::iterator iter;

    // Find it in the installed fonts
    iter = std::find(vsInstalledFonts.begin(), vsInstalledFonts.end(), wsFontName);
    if (iter == vsInstalledFonts.end()) {
      // Not there - ignore!
      m_wsUserFonts[iNumber] = L"";
      vUCBlocks[iNumber].iUserFont = -1;
      return -1;
    }
    m_wsUserFonts[iNumber] = wsFontName;
    int iFontNumber = std::distance(vsInstalledFonts.begin(), iter);
    vUCBlocks[index].iUserFont = iFontNumber;
    return iFontNumber;
  }
}

void CUCPickerDlg::OnClearbuffer()
{
  m_richedit.SetReadOnly(FALSE);
  m_richedit.SetSel(0, -1);
  m_richedit.Clear();
  m_richedit.SetReadOnly(TRUE);

  m_csBuffer.Empty();

  m_vuinfo.clear();
  m_viCharacterFonts.clear();
  m_numcharacters = 0;
}

void CUCPickerDlg::OnBackspace()
{
  // get the initial text length - RichEdit20W
  int nLength = m_richedit.GetTextLength();
  if (nLength > 0) {
    // put the selection at the end of text
    int iNumChars = m_vuinfo.back().len;
    m_richedit.SetReadOnly(FALSE);
    m_richedit.SetSel(nLength - iNumChars, -1);
    m_richedit.Clear();
    m_richedit.SetReadOnly(TRUE);

    if ((int)m_csBuffer.GetLength() == iNumChars) {
      m_csBuffer.Empty();
    } else {
      m_csBuffer = m_csBuffer.Left(nLength - iNumChars);
    }

    m_vuinfo.pop_back();
    m_viCharacterFonts.pop_back();
    m_numcharacters--;
  }
}
