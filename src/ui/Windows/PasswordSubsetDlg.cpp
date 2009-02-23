/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasswordSubsetDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PasswordSubsetDlg.h"
#include "DboxMain.h"
#include "PwFont.h"
#include "corelib/StringX.h"
#include "corelib/PWSprefs.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
  //{{AFX_MSG_MAP(CNumEdit)
  ON_WM_CHAR()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  // Ensure character is a digit or a valid delimiter
  // Otherwise just ignore it!
  if (isdigit(nChar) || nChar == _T(' ') || nChar == _T(';') || nChar == _T(',') ||
      nChar == VK_BACK) {
    CEdit::OnChar(nChar, nRepCnt, nFlags);
  }
  if (nChar == _T(' ') || nChar == _T(';') || nChar == _T(',') || nChar == VK_RETURN)
    GetParent()->SendMessage(WM_DISPLAYPASSWORDSUBSET);
}

//-----------------------------------------------------------------------------
CPasswordSubsetDlg::CPasswordSubsetDlg(CWnd* pParent, CItemData* pci)
  : CPWDialog(CPasswordSubsetDlg::IDD, pParent), m_pci(pci), m_bshown(false),
  m_warningmsg(_T(""))
{
  m_pDbx = static_cast<DboxMain *>(pParent);
}

void CPasswordSubsetDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CPasswordSubsetDlg)
  DDX_Text(pDX, IDC_SUBSET, m_subset);
  DDX_Text(pDX, IDC_STATICSUBSETWARNING, m_warningmsg);
  DDX_Control(pDX, IDC_SUBSETRESULTS, m_results);
  DDX_Control(pDX, IDC_SUBSET, m_ne_subset);
  DDX_Control(pDX, IDC_STATICSUBSETWARNING, m_stcwarningmsg);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordSubsetDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPasswordSubsetDlg)
  ON_MESSAGE(WM_DISPLAYPASSWORDSUBSET, OnDisplayStatus)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPasswordSubsetDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  ApplyPasswordFont(GetDlgItem(IDC_SUBSETRESULTS));

  CRect rect;
  PWSprefs::GetInstance()->GetPrefPSSRect(rect.top, rect.bottom, 
                                          rect.left, rect.right);

  if (rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) {
    GetWindowRect(&rect);
  }
  m_pDbx->PlaceWindow(this, &rect, SW_SHOW);

  return TRUE;
}

void CPasswordSubsetDlg::OnCancel()
{
  CRect rect;
  GetWindowRect(&rect);
  PWSprefs::GetInstance()->SetPrefPSSRect(rect.top, rect.bottom,
                                          rect.left, rect.right);
  if (m_bshown)
    CPWDialog::EndDialog(4);
  else
    CPWDialog::OnCancel();
}

LRESULT CPasswordSubsetDlg::OnDisplayStatus(WPARAM /* wParam */, LPARAM /* lParam */)
{
  UpdateData(TRUE);
  m_stcwarningmsg.SetWindowText(_T(""));
  m_stcwarningmsg.ResetColour();
  m_subset.Trim();

  int icurpos(0), lastpos;
  std::vector<int> vpos;
  CString resToken(m_subset);
  StringX sPassword = m_pci->GetPassword();
  const int ipwlengh = sPassword.length();

  while (resToken != _T("") && icurpos != -1) {
    lastpos = icurpos;
    resToken = m_subset.Tokenize(_T(";, "), icurpos);
    if (resToken == _T(""))
      continue;

    int ipos = _ttoi(resToken);
    if (ipos > ipwlengh || ipos == 0) {
      if (ipos != 0)
        m_warningmsg.Format(IDS_SUBSETINDEXTOOBIG,ipwlengh);
      else
        m_warningmsg.LoadString(IDS_SUBSETINDEXZERO);
      m_stcwarningmsg.SetWindowText(m_warningmsg);
      m_stcwarningmsg.SetColour(RGB(255, 0, 0));
      m_stcwarningmsg.Invalidate();
      vpos.clear();
      m_ne_subset.SetSel(lastpos, icurpos);
      m_ne_subset.SetFocus();
      return 0L;
    }
    vpos.push_back(ipos - 1);
  };

  std::vector<int>::const_iterator citer;
  StringX sSubset;
  for (citer = vpos.begin(); citer != vpos.end(); citer++) {
    sSubset += sPassword[*citer];
    sSubset += _T(" ");
  }
  m_results.SetWindowText(sSubset.c_str());
  m_bshown = true;
  return 1L;
}
//-----------------------------------------------------------------------------
