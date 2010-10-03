/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AttProgress.cpp : implementation file
//

#include "stdafx.h"
#include "AttProgressDlg.h"
#include "DboxMain.h"
#include "GeneralMsgBox.h"

#include "corelib/attachments.h"
#include "corelib/ItemData.h"

// CAttProgressDlg dialog

IMPLEMENT_DYNAMIC(CAttProgressDlg, CPWDialog)

CAttProgressDlg::CAttProgressDlg(CWnd* pParent, bool *pbStopVerify, bool *pbAttachmentCancel)
  : CPWDialog(CAttProgressDlg::IDD, pParent), m_pbStopVerify(pbStopVerify),
  m_pbAttachmentCancel(pbAttachmentCancel)
{
  m_pDbx = static_cast<DboxMain *>(pParent);
}

void CAttProgressDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_STATIC_FUNCTION, m_function);
  DDX_Text(pDX, IDC_STATIC_OWNER, m_owner);
  DDX_Text(pDX, IDC_STATIC_FILENAME, m_filename);
  DDX_Text(pDX, IDC_STATIC_DESCRIPTION, m_description);
  DDX_Control(pDX, IDC_ATTPROGRESS, m_progress);
}

BEGIN_MESSAGE_MAP(CAttProgressDlg, CPWDialog)
  ON_MESSAGE(ATTPRG_UPDATE_GUI, OnUpdateProgress)
  ON_MESSAGE(ATTPRG_THREAD_ENDED, OnThreadFinished)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_STOPVERIFY, OnStopVerify)
END_MESSAGE_MAP()

// CAttProgressDlg message handlers

BOOL CAttProgressDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_progress.SetRange(0, 100);
  m_progress.SetPos(0);

  if (m_pbStopVerify == NULL) {
    // Don't allow user to cancel the dialog
    GetDlgItem(IDC_STOPVERIFY)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STOPVERIFY)->EnableWindow(FALSE);
  }

  if (m_pbAttachmentCancel == NULL) {
    // Don't allow user to cancel the dialog
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
  }

  SetForegroundWindow();
  return TRUE;
}

LRESULT CAttProgressDlg::OnUpdateProgress(WPARAM wparam, LPARAM )
{
  ATTProgress *patpg = (ATTProgress *)wparam;
  StringX sxOwner;

  if (patpg->value == 0 || patpg->value < 0 || patpg->function == ATT_PROGRESS_END) {
    switch (patpg->function) {
      case ATT_PROGRESS_START:
        if (!patpg->function_text.empty()) {
          m_function = patpg->function_text.c_str();
        }
        m_owner = m_filename = m_description = L"";
        ShowWindow(SW_SHOWNORMAL);
        break;
      case ATT_PROGRESS_END:
        m_owner = m_filename = m_description = L"";
        ShowWindow(SW_HIDE);
        break;
      // Following just update progress bar at the moment
      case ATT_PROGRESS_PROCESSFILE:
      case ATT_PROGRESS_SEARCHFILE:
      case ATT_PROGRESS_EXTRACTFILE:
      {
        if (!patpg->function_text.empty()) {
          m_function = patpg->function_text.c_str();
        }
        ItemListIter iter = m_pDbx->Find(patpg->atr.entry_uuid);
        if (iter != m_pDbx->End()) {
          CItemData *pci = &iter->second;
          ASSERT(pci != NULL);
          sxOwner = L"\xab" + pci->GetGroup() + L"\xbb " +
                    L"\xab" + pci->GetTitle() + L"\xbb " +
                    L"\xab" + pci->GetUser()  + L"\xbb";
          m_owner = sxOwner.c_str();
        } else
          m_owner = L"";
        m_filename = (patpg->atr.path + patpg->atr.filename).c_str();
        m_description = patpg->atr.description.c_str();
        break;
      }
      default:
        ASSERT(0);
    }
  }

  if (patpg->value >= 0)
    m_progress.SetPos(patpg->value);

  UpdateData(FALSE);

  return 0L;
}

void CAttProgressDlg::OnStopVerify()
{
  // Indicate up the chain that the user wants to abort
  // Only if verifying the database during initial opening
  // Won't actually happen until the next message is sent to the dialog
  if (m_pbStopVerify != NULL) {
    (*m_pbStopVerify) = true;
    GetDlgItem(IDC_STOPVERIFY)->EnableWindow(FALSE);
  }
}

void CAttProgressDlg::OnCancel()
{
  // If user cancels - do not allow adding new attachments or deletion of entries
  if (m_pbAttachmentCancel == NULL)
    return;

  CGeneralMsgBox gmb;
  CString cs_msg(MAKEINTRESOURCE(IDS_CANCELATTACHMENTWARNING));
  CString cs_title(MAKEINTRESOURCE(IDS_CANCELATTACHMENTREAD));
  if (gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONSTOP) == IDYES) {
    // Do NOT cancel the dialog - let the thread end
    (*m_pbAttachmentCancel) = true;
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    GetDlgItem(IDC_STOPVERIFY)->EnableWindow(FALSE);
  }
}

LRESULT CAttProgressDlg::OnThreadFinished(WPARAM , LPARAM )
{
  // Set flag that we should close (soon as the current message handler returns)
  CPWDialog::EndDialog(IDOK);
  return 0;
}
