/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

// PWFontDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "resource3.h"
#include "ThisMfcApp.h"
#include "PWFontDialog.h"
#include "SampleTextDlg.h"
#include "dlgs.h"

#ifndef _WIN32_WCE // CFontDialog is not supported for Windows CE.

// CPWFontDialog

IMPLEMENT_DYNAMIC(CPWFontDialog, CFontDialog)

static UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
static CPWFontDialog *pwfd_self(NULL);

CPWFontDialog::CPWFontDialog(LPLOGFONT lplfInitial, DWORD dwFlags, CDC* pdcPrinter, CWnd* pParentWnd) : 
	CFontDialog(lplfInitial, dwFlags, pdcPrinter, pParentWnd)
{
  m_cf.Flags |= CF_ENABLETEMPLATE | CF_ENABLEHOOK | CF_SHOWHELP;
  m_cf.Flags &= (~CF_EFFECTS);
  m_cf.hInstance = AfxGetResourceHandle();
  m_cf.lpTemplateName = MAKEINTRESOURCE(IDD_PWFONTDIALOG);
  m_cf.hwndOwner = m_hWnd;
  m_cf.lpfnHook = CFHookProc;
  pwfd_self = this;
  m_sampletext.LoadString(IDS_SAMPLETEXT);
}

CPWFontDialog::~CPWFontDialog()
{
  pwfd_self = NULL;
}

BEGIN_MESSAGE_MAP(CPWFontDialog, CFontDialog)
END_MESSAGE_MAP()

// CPWFontDialog message handlers

static UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, 
                                    WPARAM wParam, LPARAM /* lParam */)
{
  // lParam = m_cf.lCustData when uiMsg == WM_INITDIALOG
  // but we don't need it as 'pwfd_self' must be set before calling
  // since we cannot be sure WM_INITDIALOG is the first call.
  // Note: psh15 == pshhelp == "Help Button"
	if (uiMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED && 
      LOWORD(wParam) == psh15) {
    CSampleTextDlg stDlg(NULL, pwfd_self->m_sampletext);
    app.DisableAccelerator();
    INT_PTR rc = stDlg.DoModal();
    app.EnableAccelerator();
    if (rc == IDOK) {
      pwfd_self->m_sampletext = stDlg.m_sampletext;
      ::SetDlgItemText(hdlg, stc5, pwfd_self->m_sampletext);
      ::InvalidateRect(hdlg, NULL, TRUE);
    }
    return TRUE;  // We processed message
	}

  // For some reason, we keep having to change the sample text!
  ::SetDlgItemText(hdlg, stc5, pwfd_self->m_sampletext);

  return FALSE; // We didn't process message
}

#endif // !_WIN32_WCE
