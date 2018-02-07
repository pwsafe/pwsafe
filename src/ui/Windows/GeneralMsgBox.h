/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* GeneralMsgBox.h
*
* Defines a extended Message Box class with fancy features as:
* - HTML Format support via CRichEditCtrlExtn.
* - Customizable buttons
* - Customizable icon
*
* This is a cut down version of TcxMsgBox by Thales P. Carvalho but then
* significantly enhanced to support text with HTML formatting and links
* instead of a RTF string by using a CRichEditCtrlExtn control.
* See www.codeproject.com for the original code
*/

/*
* Timed messagebox taken (and significantly modified to fit this class) from article
* "MessageBox With Timeout" by Alexandr Shcherbakov in 1999 on CodeGuru
* See: http://www.codeguru.com/cpp/misc/misc/messageboxhandling/article.php/c249/
*
* Note: There is an undocumented API in User32.dll that does this too. In fact,
* the standard MS MessageBox calls this version with a timeout of 0xFFFFFFFF
* (around 47 days 17 hours).  However, as it is undocumented and requires Windows XP
* or later - have used the information in the above article to implement here.
*
* Note: Microsoft also give a method of doing this on their site - but it doesn't
* work in MFC applications (despite what they say!) due to message processing in MFC
* See: http://support.microsoft.com/?scid=181934
*/

#pragma once

#include "RichEditCtrlExtn.h"
#include "PWDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox

class CGeneralMsgBox : private CDialog
{
public:
  // Constructor
  CGeneralMsgBox(CWnd* pParentWnd = NULL);

  // Destructor
  virtual ~CGeneralMsgBox();

  // For Timed MessageBox & Implement MFC equivalents
  INT_PTR MessageBoxTimeOut(LPCWSTR lpText, LPCWSTR lpCaption = NULL, 
                     UINT uiFlags = MB_OK, DWORD dwMilliseconds = 0);
  INT_PTR MessageBox(LPCWSTR lpText, LPCWSTR lpCaption, UINT uiFlags = MB_OK);
  INT_PTR AfxMessageBox(LPCWSTR lpszText, LPCWSTR lpCaption = NULL, UINT uiFlags = MB_OK);
  INT_PTR AfxMessageBox(UINT uiIDPrompt, UINT uiFlags = MB_OK);

  // Execute
  INT_PTR DoModal();

  // Buttons operations
  void AddButton(UINT uiIDC, LPCWSTR pszText,
                 BOOL bIsDefault = FALSE, BOOL bIsEscape = FALSE);
  void AddButton(UINT uiIDC, UINT uiIDText = (UINT)-1,
                 BOOL bIsDefault = FALSE, BOOL bIsEscape = FALSE);

  // Title operations
  void SetTitle(LPCWSTR pszTitle);
  void SetTitle(UINT uiIDTitle);

  // Message operations
  BOOL SetMsg(UINT uiMsgId);
  BOOL SetMsg(LPCWSTR pszMsg);

  // Icon operations
  void SetIcon(HICON hIcon);
  void SetIcon(UINT uiIcon);
  void SetStandardIcon(LPCWSTR pszIconName);
  void SetStandardIcon(UINT uiIcon);

  // Metric enumerators (see SetMetric and GetMetric)
  enum {CX_LEFT_BORDER, CX_RIGHT_BORDER,
        CY_TOP_BORDER, CY_BOTTOM_BORDER,
        CX_ICON_MSG_SPACE, CY_BTNS_MSG_SPACE,
        CX_BTN_BORDER, CY_BTN_BORDER,
        CX_BTNS_SPACE, CX_MIN_BTN,
        NUM_OF_METRICS
  };

  // Set a metric (in dialog units)
  void SetMetric(int iMetric, int xy);

  // Get a metric (in dialog units)
  int GetMetric(int iMetric);

private:
  // Graphical attributes
  int m_aMetrics[NUM_OF_METRICS];  // basic metrics (dialog units)
  CSize m_dimMsg;                  // message dimension (pixels)
  CSize m_dimBtn;                  // button dimension (pixels)
  CSize m_dimIcon;                 // icon dimension (pixels)

  // Dialog unit base: dimensions used in d.u. <-> pixel conversion
  enum {CX_DLGUNIT_BASE = 1000, CY_DLGUNIT_BASE = 1000};

  // Pixel dimensions of the dialog unit base
  CSize m_dimDlgUnit;

  // Controls' attributes
  UINT m_uiDefCmdId;           // default command ID: <Return>
  UINT m_uiEscCmdId;           // escape command ID: <ESC> or box close
  CStatic m_stIconCtrl;        // the icon control
  CRichEditCtrlExtn m_edCtrl;  // the RTF control

  // Button's attributes
  struct BTNDATA {
    UINT uiIDC;                   // button ID
    CString strBtn;               // button Text
  };

  CArray<BTNDATA,const BTNDATA&> m_aBtns;   // buttons' attributes

  // Message attributes
  HICON m_hIcon;                   // icon handle
  CString m_strMsg;                // the message
  CString m_strTitle;              // the title

  // Overrides
  virtual BOOL OnInitDialog();
  virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pLResult);
  virtual BOOL OnCmdMsg(UINT uiID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  // Utility - creating the nested controls
  void CreateRtfCtrl();
  void CreateBtns();
  void CreateIcon();

  void UpdateLayout();

  int FromDlgX(int x);
  int FromDlgY(int y);

  // For timed out message
  static DWORD WINAPI ThreadFunction(LPVOID lpParameter);

  DWORD m_dwTimeOut;
  bool m_bTimedOut;
  INT_PTR m_nResult;
};

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - inlined member functions

inline void CGeneralMsgBox::SetTitle(LPCWSTR pszTitle)
{ m_strTitle = pszTitle; }

inline void CGeneralMsgBox::SetTitle(UINT uiIdTitle)
{ VERIFY(m_strTitle.LoadString(uiIdTitle)); }

inline void CGeneralMsgBox::SetMetric(int iMetric, int nValue)
{ ASSERT(0 <= iMetric && iMetric < NUM_OF_METRICS);
  m_aMetrics[iMetric] = nValue; }

inline int CGeneralMsgBox::GetMetric(int iMetric)
{ ASSERT(0 <= iMetric && iMetric < NUM_OF_METRICS);
  return m_aMetrics[iMetric]; }
