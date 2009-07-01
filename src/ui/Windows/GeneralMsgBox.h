/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#pragma once

#include "RichEditCtrlExtn.h"
#include "PWDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox

class CGeneralMsgBox : private CDialog
{
  // Basic
public:

  // Constructor
  CGeneralMsgBox(CWnd* pParentWnd = NULL);

  // Destructor
  virtual ~CGeneralMsgBox();

  // Execute
  INT_PTR DoModal();

  // Buttons operations
  void AddButton(UINT uIDC, LPCWSTR pszText,
    BOOL bIsDefault = FALSE,
    BOOL bIsEscape = FALSE);
  void AddButton(UINT uIDC, UINT uIdText = (UINT)-1,
    BOOL bIsDefault = FALSE,
    BOOL bIsEscape = FALSE);

  // Title operations
  void SetTitle(LPCWSTR pszTitle);
  void SetTitle(UINT uIdTitle);

  // Message operations
  BOOL SetMsg(UINT uMsgId);
  BOOL SetMsg(LPCWSTR pszMsg);

  // Icon operations
  void SetIcon(HICON hIcon);
  void SetIcon(UINT uIcon);
  void SetStandardIcon(LPCWSTR pszIconName);
  void SetStandardIcon(UINT uIcon);

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
  UINT m_uDefCmdId;            // default command ID: <Return>
  UINT m_uEscCmdId;            // escape command ID: <ESC> or box close
  CStatic m_stIconCtrl;        // the icon control
  CRichEditCtrlExtn m_edCtrl;  // the RTF control

  // Button's attributes
  struct BTNDATA {
    UINT uIDC;                    // button ID
    CString strBtn;               // button Text
  };

  CArray<BTNDATA,const BTNDATA&> m_aBtns;   // buttons' attributes

  // Message attributes
  HICON m_hIcon;                   // icon handle
  CString m_strMsg;                // the message
  CString m_strTitle;              // the title

  // Overrides
  virtual BOOL OnInitDialog();
  virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  // Utility - creating the nested controls
  void CreateRtfCtrl();
  void CreateBtns();
  void CreateIcon();

  void UpdateLayout();

  int FromDlgX(int x);
  int FromDlgY(int y);
};

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - inlined member functions

inline void CGeneralMsgBox::SetTitle(LPCWSTR pszTitle)
{ m_strTitle = pszTitle; }

inline void CGeneralMsgBox::SetTitle(UINT uIdTitle)
{ VERIFY(m_strTitle.LoadString(uIdTitle)); }

inline void CGeneralMsgBox::SetMetric(int iMetric, int nValue)
{  ASSERT(0 <= iMetric && iMetric < NUM_OF_METRICS);
m_aMetrics[iMetric] = nValue; }

inline int CGeneralMsgBox::GetMetric(int iMetric)
{  ASSERT(0 <= iMetric && iMetric < NUM_OF_METRICS);
return m_aMetrics[iMetric]; }
