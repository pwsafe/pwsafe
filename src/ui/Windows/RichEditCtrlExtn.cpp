/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// RichEditCtrlExtns.cpp
//

// Used in AboutDlg and GeneralMsgBox ONLY where html links are required
// NOT used in AddEdit_Basic for Notes - it uses CRichEditExtn defined in "ControlExtns.h"

#include "stdafx.h"

#include "RichEditCtrlExtn.h"

#include <algorithm>
#include <vector>
#include <string>
#include <bitset>
#include <stack>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
  Note that there is a callback function available to allow the parent to process
  clicks on any link in the text.

  bool pfcnNotifyLinkClicked (LPWSTR lpszURL, LPWSTR lpszFriendlyName, LPARAM self);

  If the parent has not registered for the callback, the link is processed by this
  control and it should open the appropriate URL in the user's default browser.
*/

/*
  Supports the following HTML formatting in a RichEditCtrl:
  Bold:          <b>.....</b>
  Italic:        <i>.....</i>
  Underline:     <u>.....</u>
  Font:          <font face="FN" size="S" color="C">.....</font>
  Web reference: <a href="...url...">Friendly Name</a>

  Notes:
  1. If you wish to use the "less than" or "greater than" symbols in the
     message, use "&lt;" and "&gt;" instead.  These will be converted to the
     '<' and '>' symbols in the final text string.

  2. Bold, Italic, Underline and Font can be nested but MUST NOT overlap - i.e.
     "<b>bold</b> and <i>italic</i>" is OK
     "<b>bold and <i>bold & italic</i> and more bold</b>" is OK
     "<b>bold and <i>bold italic</b> & italic</i>" is NOT OK, since the bold end
     tag is in the middle of the italic tags.

  3. Fonts can also be nested. i.e.
     <font face="FN1">test1<font face="FN2">test2<font color="Red">red2</font></font></font>

  4. Any unsupported HTML tags will be retained e.g. "<q>test</q>" will be
     still be "<q>test</q>" in the final text string.
*/

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlExtn

CRichEditCtrlExtn::CRichEditCtrlExtn()
  : m_pfcnNotifyLinkClicked(NULL)
{
}

CRichEditCtrlExtn::~CRichEditCtrlExtn()
{
  m_vFormat.clear();
  m_vALink.clear();
}

BEGIN_MESSAGE_MAP(CRichEditCtrlExtn, CRichEditCtrl)
  //{{AFX_MSG_MAP(CRichEditCtrlExtn)
  ON_NOTIFY_REFLECT(EN_LINK, OnLink) 
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Find "Friendly Name" from link vector
struct StartEndMatch {
  bool operator()(CRichEditCtrlExtn::ALink &al) {
    return (al.iStart == m_iStart && al.iEnd == m_iEnd);
  }

  StartEndMatch(const int i_start, const int i_end) :
  m_iStart(i_start),  m_iEnd(i_end) {}

  const int m_iStart;
  const int m_iEnd;
};

void CRichEditCtrlExtn::OnLink(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  ENLINK* pENLink = (ENLINK*)pNotifyStruct;
  CString cs_FName, cs_URL;
  CHARRANGE saveRange;
  *pLResult = 0;

  if (pNotifyStruct->code == EN_LINK) {
    if (pENLink->msg == WM_LBUTTONDOWN || pENLink->msg == WM_LBUTTONDBLCLK) {
      // Find "Friendly Name"
      if (m_vALink.empty())
        return;

      StartEndMatch StartEndMatch(pENLink->chrg.cpMin, pENLink->chrg.cpMax);
      std::vector<ALink>::const_iterator found;
      found = std::find_if(m_vALink.begin(), m_vALink.end(), StartEndMatch);

      if (found == m_vALink.end())
        return;

      // Save current selected text
      saveRange.cpMin = saveRange.cpMax = 0;
      GetSel(saveRange);

      // Select link
      SetSel(pENLink->chrg);

      // Retrieve Friendly Name
      cs_FName = GetSelText();

      // Restore selection
      SetSel(saveRange);

      // Retrieve the URL
      cs_URL = CString(found->tcszURL);
      CWaitCursor waitCursor;

      bool bCallbackProcessed(false);
      if (m_pfcnNotifyLinkClicked != NULL) {
        // Call the supplied Callback routine; if it returns "true", it has processed the link
        bCallbackProcessed = m_pfcnNotifyLinkClicked(cs_URL, cs_FName, m_NotifyInstance);
      }

      if (bCallbackProcessed) {
        *pLResult = 1;
      } else {
        // We do it!
        if (!cs_URL.IsEmpty()) {
          ::ShellExecute(NULL, NULL, cs_URL, NULL, NULL, SW_SHOWNORMAL);
          *pLResult = 1;
        }
      }
      SetSel(-1, -1);
    } else 
      if (pENLink->msg == WM_LBUTTONUP) {
        *pLResult = 1;
      }
  }
}

bool CRichEditCtrlExtn::RegisterOnLink(bool (*pfcn) (const CString &, const CString &, LPARAM), LPARAM instance)
{
  if (m_pfcnNotifyLinkClicked != NULL)
    return false;

  m_pfcnNotifyLinkClicked = pfcn;
  m_NotifyInstance = instance;
  return true;
}

void CRichEditCtrlExtn::UnRegisterOnLink()
{
  m_pfcnNotifyLinkClicked = NULL;
  m_NotifyInstance = NULL;
}

// Return whether first starting point is greater than the second when sorting links
bool CRichEditCtrlExtn::iStartCompare(st_format elem1, st_format elem2)
{
  if (elem1.iStart != elem2.iStart)
    return (elem1.iStart < elem2.iStart);

  return (elem1.iEnd < elem2.iEnd);
}

void CRichEditCtrlExtn::SetWindowText(LPCWSTR lpszString)
{
  int iError;
  CRichEditCtrl::SetWindowText(L"");
  ShowWindow(SW_HIDE);

  CString cs_formatstring(lpszString);
  CString cs_plaintext = GetTextFormatting(cs_formatstring, iError);

  // Disable Font Binding in Rich Edit (http://msdn.microsoft.com/en-us/library/windows/desktop/hh298436%28v=vs.85%29.aspx)
  // When it enabled, control tries to change font for chars, that it think aren't present in default font
  // For example, for non-English messages, it can change font from MS Sans to Calibri when it found e.g.
  // "french quotes" ('«', '»'). The worst in this case that it also may change font size and then not return
  // all back, so the last part of the message will be printed using different font type/size
  // Example of code taken from http://blogs.msdn.com/b/michkap/archive/2006/02/13/531110.aspx
  CRichEditCtrl::SendMessage(EM_SETLANGOPTIONS, 0, (LPARAM)(CRichEditCtrl::SendMessage(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOFONT));

  if (iError != 0) {
    // Had an error - show unchanged text
    CRichEditCtrl::SetWindowText(cs_formatstring);
  } else {
    CRichEditCtrl::SetWindowText(cs_plaintext);

    CHARFORMAT2 cf2;
    // Now apply formating
    if (!m_vFormat.empty()) {
      std::sort(m_vFormat.begin(), m_vFormat.end(), iStartCompare);
      std::vector<st_format>::const_iterator format_iter;
      SecureZeroMemory(&cf2, sizeof(cf2));
      cf2.cbSize = sizeof(cf2);

      for (format_iter = m_vFormat.begin();
           format_iter != m_vFormat.end(); format_iter++) {
        SetSel(format_iter->iStart, format_iter->iEnd);
        GetSelectionCharFormat(cf2);
        if (format_iter->entrytype == Bold) {
          cf2.dwMask |= CFM_BOLD;
          cf2.dwEffects |= CFE_BOLD;
        } else if (format_iter->entrytype == Italic) {
          cf2.dwMask |= CFM_ITALIC;
          cf2.dwEffects |= CFE_ITALIC;
        } else if (format_iter->entrytype == Underline) {
          cf2.dwMask |= CFM_UNDERLINE;
          cf2.dwEffects |= CFE_UNDERLINE;
        } else if (format_iter->entrytype == Colour) {
          cf2.dwMask = CFM_COLOR;
          cf2.crTextColor = format_iter->cr;
          cf2.dwEffects &= ~CFE_AUTOCOLOR;
        } else if (format_iter->entrytype == Size) {
          cf2.dwMask = CFM_SIZE;
          cf2.yHeight = (format_iter->iSize) * 20;
        } else if (format_iter->entrytype == Name) {
          cf2.dwMask = CFM_FACE;
          memcpy_s(cf2.szFaceName, sizeof(cf2.szFaceName),
                   format_iter->tcszFACENAME, sizeof(format_iter->tcszFACENAME));
        }
        SetSelectionCharFormat(cf2);
      }
    }

    if (!m_vALink.empty()) {
      SetEventMask(GetEventMask() | ENM_LINK); 
      std::vector<ALink>::const_iterator ALink_iter;
      SecureZeroMemory(&cf2, sizeof(cf2));
      cf2.cbSize = sizeof(cf2);
      cf2.dwMask = CFM_LINK;
      cf2.dwEffects = CFE_LINK;

      for (ALink_iter = m_vALink.begin(); ALink_iter != m_vALink.end(); ALink_iter++) {
        SetSel(ALink_iter->iStart, ALink_iter->iEnd);
        SetSelectionCharFormat(cf2);
      }
    } else {
      SetEventMask(GetEventMask() & ~ENM_LINK);
    }
  }
  ShowWindow(SW_SHOW);
}

CString CRichEditCtrlExtn::GetTextFormatting(const CString &csHTML, int &iError)
{
  CString csText, csToken, csHTMLTag;
  int iCurrentFontSize, iCurrentFontPointSize;
  int iDefaultFontSize, iDefaultFontPointSize;
  int iTextPosition(0);
  int curPos, oldPos;

  int ierrorcode(0);
  bool bHTMLTag;

  m_vFormat.clear();
  m_vALink.clear();

  st_format this_format;

  ALink this_ALink;

  std::bitset<3> bsFontChange;  // facename, size, color

  std::vector<CString> vLastFacenames;
  std::vector<COLORREF> vLastColours;
  std::vector<int> vLastSizes;
  std::vector<int> vFontChangeStart;
  std::vector<std::bitset<3>> vFontChange;
  std::stack<st_format> format_stack;
  std::stack<int> type_stack;

  // Validate the HTML
  curPos = 0;
  oldPos = 0;
  int iBold(0), iItalic(0), iUnderline(0), iFont(0), iAnchor(0);
  bool bNestedBold(false), bNestedItalic(false), bNestedUnderline(false),
       bNestedAnchor(false), bOverlapped(false);

  csToken = csHTML.Tokenize(L"<>", curPos);
  while (csToken != L"" && curPos != -1) {
    oldPos = curPos - csToken.GetLength() - 1;
    CString a = csHTML.Mid(oldPos - 1, 1);
    CString b = csHTML.Mid(curPos - 1, 1);
    if (csHTML.Mid(oldPos - 1, 1) == L"<" &&
      csHTML.Mid(curPos - 1, 1) == L">") {
        bHTMLTag = true;
    } else
      bHTMLTag = false;

    if (bHTMLTag) {
      // Must be a HTML Tag
      csHTMLTag = csToken;
      csHTMLTag.MakeLower();
      if (csHTMLTag == L"b") {
        type_stack.push(Bold);
        iBold++;
        if (iBold != 1) bNestedBold = true;
        goto vnext;
      } else if (csHTMLTag == L"/b") {
        int &iLastType = type_stack.top();
        if (iLastType != Bold)
          bOverlapped = true;
        iBold--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag == L"i") {
        type_stack.push(Italic);
        iItalic++;
        if (iItalic != 1) bNestedItalic = true;
        goto vnext;
      } else if (csHTMLTag == L"/i") {
        int &iLastType = type_stack.top();
        if (iLastType != Italic)
          bOverlapped = true;
        iItalic--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag == L"u") {
        type_stack.push(Underline);
        iUnderline++;
        if (iUnderline != 1) bNestedUnderline = true;
        goto vnext;
      } else if (csHTMLTag == L"/u") {
        int &iLastType = type_stack.top();
        if (iLastType != Underline)
          bOverlapped = true;
        iUnderline--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag.Left(4) == L"font") {
        type_stack.push(Font);
        iFont++;
        goto vnext;
      } else if (csHTMLTag == L"/font") {
        int &iLastType = type_stack.top();
        if (iLastType != Font)
          bOverlapped = true;
        iFont--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag.Left(6) == L"a href") {
        type_stack.push(Link);
        iAnchor++;
        if (iAnchor != 1) bNestedAnchor = true;
        goto vnext;
      } else if (csHTMLTag == L"/a") {
        int &iLastType = type_stack.top();
        if (iLastType != Link)
          bOverlapped = true;
        iAnchor--;
        type_stack.pop();
        goto vnext;
      } else { // unsupported tag or typo - return raw string
        return csHTML;
      }
    }

vnext:
    oldPos = curPos;
    csToken = csHTML.Tokenize(L"<>", curPos);
  };

  if (bNestedBold)
    ierrorcode += 1;
  if (iBold != 0)
    ierrorcode += 2;
  if (bNestedItalic)
    ierrorcode += 4;
  if (iItalic != 0)
    ierrorcode += 8;
  if (bNestedUnderline)
    ierrorcode += 16;
  if (iUnderline != 0)
    ierrorcode += 32;
  if (bNestedAnchor)
    ierrorcode += 64;
  if (iAnchor != 0)
    ierrorcode += 128;
  if (iFont != 0)
    ierrorcode += 256;
  if (bOverlapped)
    ierrorcode +=512;

  iError = ierrorcode;

  if (ierrorcode != 0) {
    return L"";
  }

  // Now really process the HTML
  NONCLIENTMETRICS ncm ={0};
  ncm.cbSize = sizeof(NONCLIENTMETRICS);

  SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                       sizeof(NONCLIENTMETRICS), &ncm, NULL);

  CWnd *pWndDesk = GetDesktopWindow();
  CDC *pDCDesk = pWndDesk->GetWindowDC();
  int logPixY = pDCDesk->GetDeviceCaps(LOGPIXELSY);
  pWndDesk->ReleaseDC(pDCDesk);

  iDefaultFontPointSize = MulDiv(ncm.lfMessageFont.lfHeight, 72, logPixY);
  iCurrentFontPointSize = iDefaultFontPointSize;
  iDefaultFontSize = ConvertPointsToSize(iCurrentFontPointSize);
  iCurrentFontSize = iDefaultFontSize;

  curPos = 0;
  oldPos = 0;

  csToken = csHTML.Tokenize(L"<>", curPos);
  while (csToken != L"" && curPos != -1) {
    oldPos = curPos - csToken.GetLength() - 1;
    CString a = csHTML.Mid(oldPos - 1, 1);
    CString b = csHTML.Mid(curPos - 1, 1);
    if (csHTML.Mid(oldPos - 1, 1) == L"<" &&
        csHTML.Mid(curPos - 1, 1) == L">") {
      bHTMLTag = true;
    } else
      bHTMLTag = false;

    if (!bHTMLTag && iAnchor != 0)
      goto next;

    if (bHTMLTag) {
      // Must be a HTML Tag
      csHTMLTag = csToken;
      csHTMLTag.MakeLower();
      if (csHTMLTag == L"b") {
        this_format.entrytype = Bold;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else
      if (csHTMLTag == L"/b") {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else
      if (csHTMLTag == L"i") {
        this_format.entrytype = Italic;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else
      if (csHTMLTag == L"/i") {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else
      if (csHTMLTag == L"u") {
        this_format.entrytype = Underline;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else
      if (csHTMLTag == L"/u") {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else
      if (csHTMLTag == L"/font") {
        if (vFontChange.empty()) { // malformed <font> token
          return csHTML;
        }
        std::bitset<3> &bsLastFont = vFontChange.back();
        int &iFontChangeStart = vFontChangeStart.back();
        st_format& cur_format = format_stack.top();
        if (bsLastFont.test(FACENAMECHANGED)) {
          CString &csLastFaceName = vLastFacenames.back();
          cur_format.entrytype = Name;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          SecureZeroMemory(cur_format.tcszFACENAME, sizeof(cur_format.tcszFACENAME));
          wcscpy_s(cur_format.tcszFACENAME, LF_FACESIZE, (LPCWSTR)csLastFaceName);
          m_vFormat.push_back(cur_format);
          vLastFacenames.pop_back();
        }
        if (bsLastFont.test(SIZECHANGED)) {
          int &iLastSize = vLastSizes.back();
          cur_format.entrytype = Size;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          cur_format.iSize = iLastSize;
          m_vFormat.push_back(cur_format);
          vLastSizes.pop_back();
          if (!vLastSizes.empty()) {
            int &i = vLastSizes.back();
            iCurrentFontPointSize = i;
            iCurrentFontSize = ConvertPointsToSize(iCurrentFontPointSize);
          } else {
            iCurrentFontPointSize = iDefaultFontPointSize;
            iCurrentFontSize = iDefaultFontSize;
          }
        }
        if (bsLastFont.test(COLOURCHANGED)) {
          COLORREF &c = vLastColours.back();
          cur_format.entrytype = Colour;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          cur_format.cr = c;
          m_vFormat.push_back(cur_format);
          vLastColours.pop_back();
        }
        vFontChange.pop_back();
        vFontChangeStart.pop_back();
        format_stack.pop();
        goto next;
      } else if (csHTMLTag == L"/a") {
        goto next;
      }

      // Check for fonts
      // <font face="xxxxx" size="n" color="xxxxx">....</font>
      if (csHTMLTag.Left(5) == L"font ") {
        CString csFontToken, csFontVerb, csFontVerbValue;
        int curFontPos(0);

        bsFontChange.reset();
        csFontToken = csHTMLTag.Tokenize(L"\"", curFontPos);
        csFontVerb = csFontToken.Right(6);
        csFontVerb.TrimLeft();
        // Skip over first token of 'font verb='
        csFontVerbValue = csHTMLTag.Tokenize(L"\"", curFontPos);
        while (csFontVerbValue != L"" && curFontPos != -1) {
          if (csFontVerb == L"face=") {
            bsFontChange.set(FACENAMECHANGED);
            vLastFacenames.push_back(csFontVerbValue);
          } else
            if (csFontVerb == L"size=") {
              bsFontChange.set(SIZECHANGED);
              iCurrentFontPointSize = ConvertSizeToPoints(csFontVerbValue, iCurrentFontSize);
              vLastSizes.push_back(iCurrentFontPointSize);
            } else
              if (csFontVerb == L"color=") {
                bsFontChange.set(COLOURCHANGED);
                COLORREF crNewFontColour = ConvertColourToColorRef(csFontVerbValue);
                vLastColours.push_back(crNewFontColour);
              }

              csFontVerb = csHTMLTag.Tokenize(L"\"", curFontPos);
              csFontVerb.TrimLeft();
              if (csFontVerb.IsEmpty() && curFontPos == -1)
                break;
              csFontVerbValue = csHTMLTag.Tokenize(L"\"", curFontPos);
        };
        vFontChange.push_back(bsFontChange);
        vFontChangeStart.push_back(iTextPosition);
        format_stack.push(this_format);
        goto next;
      }

      // check for hyperlink
      // <a href="http://www.microsoft.com">Friendly name</a>

      if (csHTMLTag.Left(7) == L"a href=") {
        long dwTEnd;
        CString csURL;
        dwTEnd = csHTMLTag.Find(L"\"", 8);
        if (dwTEnd >= 0) {
          csURL = csHTMLTag.Mid(8, dwTEnd - 8);
          if (!csURL.IsEmpty()) {
            csURL.MakeLower();
            wcscpy_s(this_ALink.tcszURL, _MAX_PATH, csURL);
          }
        }
        // Now get Friendly Name (note doing this within the while loop!)
        oldPos = curPos;
        csToken = csHTML.Tokenize(L"<>", curPos);
        csText += csToken;
        this_ALink.iStart = iTextPosition;
        iTextPosition += csToken.GetLength();
        this_ALink.iEnd = iTextPosition;
        m_vALink.push_back(this_ALink);
        goto next;
      }
    }

    csToken.Replace(L"&lt;", L"<");
    csToken.Replace(L"&gt;", L">");
    // Real text or unknown HTML tag
    if (bHTMLTag) {
      // We didn't recognise it! Put back the < & >
      csText += L"<" + csToken + L">";
      iTextPosition += csToken.GetLength() + 2;
    } else {
      csText += csToken;
      iTextPosition += csToken.GetLength();
    }

next:
    oldPos = curPos;
    csToken = csHTML.Tokenize(L"<>", curPos);
  };

  return csText;
}

COLORREF CRichEditCtrlExtn::ConvertColourToColorRef(CString &csValue)
{
  // Value is either a colour name or "#RRGGBB"
  // Note COLORREF = 0x00bbggrr but HTML = 0x00rrggbb
  // Values for named colours here are in COLORREF format
  long retval(0L);
  if (csValue.Left(1) == L"#") {
    // Convert HTML to COLORREF
    ASSERT(csValue.GetLength() == 7);
    int icolour;
    swscanf_s(csValue.Mid(1), L"%06x", &icolour);
    int ired = (icolour & 0xff0000) >> 16;
    int igreen = (icolour & 0xff00);
    int iblue = (icolour & 0xff) << 16;
    return (COLORREF)(iblue + igreen + ired);
  }

  if (csValue.CompareNoCase(L"Black") == 0)
    retval = 0L;
  else if (csValue.CompareNoCase(L"Green") == 0)
    retval = 0x008000L;
  else if (csValue.CompareNoCase(L"Silver") == 0)
    retval = 0xc0c0c0L;
  else if (csValue.CompareNoCase(L"Lime") == 0)
    retval = 0x00ff00L;
  else if (csValue.CompareNoCase(L"Gray") == 0)
    retval = 0x808080L;
  else if (csValue.CompareNoCase(L"Olive") == 0)
    retval = 0x008080L;
  else if (csValue.CompareNoCase(L"White") == 0)
    retval = 0xffffffL;
  else if (csValue.CompareNoCase(L"Yellow") == 0)
    retval = 0x00ffffL;
  else if (csValue.CompareNoCase(L"Maroon") == 0)
    retval = 0x000080L;
  else if (csValue.CompareNoCase(L"Navy") == 0)
    retval = 0x800000L;
  else if (csValue.CompareNoCase(L"Red") == 0)
    retval = 0x0000ffL;
  else if (csValue.CompareNoCase(L"Blue") == 0)
    retval = 0xff0000L;
  else if (csValue.CompareNoCase(L"Purple") == 0)
    retval = 0x800080L;
  else if (csValue.CompareNoCase(L"Teal") == 0)
    retval = 0x808000L;
  else if (csValue.CompareNoCase(L"Fuchsia") == 0)
    retval = 0xff00ffL;
  else if (csValue.CompareNoCase(L"Aqua") == 0)
    retval = 0xffff00L;

  return (COLORREF)retval;
}

int CRichEditCtrlExtn::ConvertSizeToPoints(CString &csValue, int &iCurrentSize)
{
  int retval(0), iSize, absSize;

  iSize = _wtoi(csValue);
  absSize = iSize < 0 ? (-iSize) : iSize;
  ASSERT(absSize > 0 && absSize < 8);

  if (csValue.Left(1) == L"+" || csValue.Left(1) == L"-") {
    // It is a "relative" change
    iSize = iCurrentSize + iSize;
    if (iSize < 1)
      iSize = 1;
    else
      if (iSize > 7)
        iSize = 7;
  }
  switch (iSize) {
    case 1:
      retval = 15;  // "7.5"
      break;
    case 2:
      retval = 20;  // "10"
      break;
    case 3:
      retval = 24;  // "12"
      break;
    case 4:
      retval = 27;  // "13.5"
      break;
    case 5:
      retval = 36;  // "18"
      break;
    case 6:
      retval = 48;  // "24"
      break;
    case 7:
      retval = 72;  // "36"
      break;
    default:
      ASSERT(0);
  }

  // Return value in "size"
  iCurrentSize = iSize;
  // Return value in points
  return retval;
}

int CRichEditCtrlExtn::ConvertPointsToSize(const int iCurrentPoints)
{
  if (iCurrentPoints <= 15)
    return 1;
  else if (iCurrentPoints <= 20)
    return 2;
  else if (iCurrentPoints <= 24)
    return 3;
  else if (iCurrentPoints <= 27)
    return 4;
  else if (iCurrentPoints <= 36)
    return 5;
  else if (iCurrentPoints <= 48)
    return 6;
  else
    return 7;
}
