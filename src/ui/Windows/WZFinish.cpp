/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"

#include "GeneralMsgBox.h"

#include "WZPropertyPage.h"
#include "WZPropertySheet.h"
#include "WZFinish.h"

#include "core/PWScore.h"
#include "core/PWSprefs.h"
#include "core/Util.h"

#include "resource3.h"
#include "core/core.h"

#define PWS_MSG_WIZARD_EXECUTE_THREAD_ENDED (WM_APP + 65)

IMPLEMENT_DYNAMIC(CWZFinish, CWZPropertyPage)

CWZFinish::CWZFinish(CWnd *pParent, UINT nIDCaption, const int nType)
 : CWZPropertyPage(IDD, nIDCaption, nType), m_pothercore(nullptr), m_prpt(nullptr),
   m_pExecuteThread(nullptr), m_bInProgress(false), m_bComplete(false), 
  m_bInitDone(false), m_bViewingReport(false), m_status(-1), m_numProcessed(-1)
{
  m_pWZPSH = (CWZPropertySheet *)pParent;
}

CWZFinish::~CWZFinish()
{
  delete m_prpt;
}

BEGIN_MESSAGE_MAP(CWZFinish, CWZPropertyPage)
  //{{AFX_MSG_MAP(CWZFinish)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_ABORT, OnAbort)
  ON_BN_CLICKED(IDC_VIEWREPORT, OnViewReport)
  ON_MESSAGE(PWS_MSG_WIZARD_EXECUTE_THREAD_ENDED, OnExecuteThreadEnded)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWZFinish::OnHelp()
{
  ShowHelp(L"::/html/wzfinish.html");
}

BOOL CWZFinish::OnInitDialog()
{
  CWZPropertyPage::OnInitDialog();

  // Disable Finish until processing complete
  m_pWZPSH->SetWizardButtons(PSWIZB_DISABLEDFINISH);

  m_pWZPSH->GetDlgItem(ID_WIZBACK)->EnableWindow(FALSE);
  m_pWZPSH->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);
  
  switch (m_pWZPSH->GetID()) {
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
    case ID_MENUITEM_SYNCHRONIZE:
    {
      // Override IDCANCEL otherwise we don't get control
      m_pWZPSH->GetDlgItem(IDCANCEL)->SetDlgCtrlID(IDC_ABORT);

      // Set button text
      CString cs_Abort(MAKEINTRESOURCE(IDS_ABORT));
      m_pWZPSH->GetDlgItem(IDC_ABORT)->SetWindowText(cs_Abort);
      break;
    }
    // don't show report button when there's nothing to report:
    // The following export operations cannot fail, hence no sense
    // in confusing user with report
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
      // XML export may fail, so we'll show the report button if so
      // at the end of the export
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
      GetDlgItem(IDC_VIEWREPORT)->ShowWindow(SW_HIDE);
      // deliberate fallthrough
    default:
      m_pWZPSH->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
      m_pWZPSH->GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
  }

  m_bInitDone = true;

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CWZFinish::OnAbort()
{
  m_thdpms.bCancel = true;

  // Stop multiple presses
  m_pWZPSH->GetDlgItem(IDC_ABORT)->EnableWindow(FALSE);
}

void CWZFinish::DisableAbort()
{
  m_pWZPSH->GetDlgItem(IDC_ABORT)->EnableWindow(FALSE);
}

static UINT WZExecuteThread(LPVOID pParam)
{
  WZExecuteThreadParms *pthdpms = (WZExecuteThreadParms *)pParam;

  int status(PWScore::SUCCESS);

  switch (pthdpms->nID) {
    case ID_MENUITEM_COMPARE:
      status = pthdpms->pWZPSH->WZPSHDoCompare(pthdpms->pcore,
                   pthdpms->bAdvanced, pthdpms->prpt, &pthdpms->bCancel) ? 0 : -1;
      pthdpms->pWZFinish->DisableAbort();
      break;
    case ID_MENUITEM_MERGE:
      pthdpms->csResults = pthdpms->pWZPSH->WZPSHDoMerge(pthdpms->pcore, 
                   pthdpms->bAdvanced, pthdpms->prpt, &pthdpms->bCancel);
      pthdpms->pWZFinish->DisableAbort();
      break;
    case ID_MENUITEM_SYNCHRONIZE:
      pthdpms->pWZPSH->WZPSHDoSynchronize(pthdpms->pcore,
                   pthdpms->bAdvanced, pthdpms->numProcessed, pthdpms->prpt,
                   &pthdpms->bCancel);
      pthdpms->pWZFinish->DisableAbort();
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      status = pthdpms->pWZPSH->WZPSHDoExportText(pthdpms->sx_Filename, 
                   pthdpms->nID,
                   pthdpms->pWZPSH->GetDelimiter(), pthdpms->bAdvanced, 
                   pthdpms->numProcessed, pthdpms->prpt);
      break;
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
      status = pthdpms->pWZPSH->WZPSHDoExportXML(pthdpms->sx_Filename,
                   pthdpms->nID,
                   pthdpms->pWZPSH->GetDelimiter(), pthdpms->bAdvanced,
                   pthdpms->numProcessed, pthdpms->prpt);
      break;
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
      status = pthdpms->pWZPSH->WZPSHDoExportDB(pthdpms->sx_Filename,
                   pthdpms->nID, pthdpms->bExportDBFilters,
                   pthdpms->sx_exportpasskey,
                   pthdpms->numProcessed, pthdpms->prpt);
      break;
    default:
      ASSERT(0);
      break;
  }

  pthdpms->status = status; // Set the thread return code for caller
  pthdpms->pWZFinish->PostMessage(PWS_MSG_WIZARD_EXECUTE_THREAD_ENDED, (WPARAM)pthdpms, 0);

  return 0;
}

BOOL CWZFinish::OnSetActive()
{
  BOOL brc = CWZPropertyPage::OnSetActive();

  UINT uifilemsg(0);
  const UINT nID = m_pWZPSH->GetID();
  switch (nID) {
    case ID_MENUITEM_COMPARE:
      uifilemsg = IDS_WZCOMPAREDB;
      break;
    case ID_MENUITEM_MERGE:
      uifilemsg = IDS_WZMERGEDB;
      break;
    case ID_MENUITEM_SYNCHRONIZE:
      uifilemsg = IDS_WZSYNCHRONIZEDB;
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
      uifilemsg = IDS_WZEXPORTFILE;
      break;
    default:
      ASSERT(0);
      break;
  }

  CString cs_text(MAKEINTRESOURCE(uifilemsg));
  GetDlgItem(IDC_STATIC_WZFILE)->SetWindowText(cs_text);

  GetDlgItem(IDC_DATABASE)->SetWindowText(m_pWZPSH->GetOtherDBFile().c_str());

  if (!m_bInProgress && m_bInitDone && brc != 0) {
    ExecuteAction();
  }

  return brc;
}

int CWZFinish::ExecuteAction()
{
  m_bInProgress = true;
  CGeneralMsgBox gmb;
  CString cs_temp, cs_title;

  const UINT nID = m_pWZPSH->GetID();
  bool bOtherIsDB(false);

  switch (nID) {
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
    case ID_MENUITEM_SYNCHRONIZE:
      m_pothercore = new PWScore; // NOT PWSAuxCore, as we handle db prefs explicitly
      bOtherIsDB = true;
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
      break;
    default:
      ASSERT(0);
      break;
  }

  int rc(PWScore::SUCCESS);
  const StringX sx_Filename2 = m_pWZPSH->GetOtherDBFile();

  if (bOtherIsDB) {
    // Not really needed but...
    m_pothercore->ClearDBData();

    // Reading a new file changes the preferences as they are instance dependent
    // not core dependent
    PWSprefs *prefs =  PWSprefs::GetInstance();

    const StringX sxSavePrefString(prefs->Store());
    const bool bSaveIfDBPrefsChanged = prefs->IsDBprefsChanged();

    const StringX passkey = m_pWZPSH->GetPassKey();

    // Read the other database
    rc = m_pothercore->ReadFile(sx_Filename2, passkey);

    // Save all the 'other core' preferences in the copy - to use for
    // 'other' default Password Policy when needed in Compare, Merge & Sync
    prefs->SetupCopyPrefs();

    // Reset database preferences - first to defaults then add saved changes!
    prefs->Load(sxSavePrefString);
    prefs->SetDBprefsChanged(bSaveIfDBPrefsChanged);

    switch (rc) {
      case PWScore::SUCCESS:
        break;
      case PWScore::CANT_OPEN_FILE:
        cs_temp.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(sx_Filename2.c_str()));
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
        break;
      case PWScore::BAD_DIGEST:
        cs_temp.Format(IDS_FILECORRUPT, static_cast<LPCWSTR>(sx_Filename2.c_str()));
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
        break;
      default:
        cs_temp.Format(IDS_UNKNOWNERROR, static_cast<LPCWSTR>(sx_Filename2.c_str()));
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
        break;
    }

    if (rc != PWScore::SUCCESS) {
      m_pothercore->ClearDBData();
      m_pothercore->SetCurFile(L"");
      delete m_pothercore;
      m_pothercore = nullptr;
      return rc;
    }
  }

  if (rc == PWScore::SUCCESS) {
    if (bOtherIsDB)
      m_pothercore->SetCurFile(sx_Filename2);

    m_pWZPSH->WZPSHSetUpdateWizardWindow(GetDlgItem(IDC_ENTRY));

    if (m_prpt == nullptr)
      m_prpt = new CReport;

    const bool bAdvanced = m_pWZPSH->GetAdvanced();
    const bool bExportDBFilters = m_pWZPSH->GetExportDBFilters();

    m_thdpms.pWZFinish = this;
    m_thdpms.nID = nID;
    m_thdpms.pWZPSH = m_pWZPSH;
    m_thdpms.pcore = m_pothercore;
    m_thdpms.prpt = m_prpt;
    m_thdpms.sx_Filename = sx_Filename2;
    m_thdpms.sx_exportpasskey = m_pWZPSH->GetExportPassKey();
    m_thdpms.bAdvanced = bAdvanced;
    m_thdpms.bExportDBFilters = bExportDBFilters;

    m_pExecuteThread = AfxBeginThread(WZExecuteThread, &m_thdpms,
                                THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

    if (m_pExecuteThread == nullptr) {
      pws_os::Trace(L"Unable to create Execute thread\n");
      return PWScore::FAILURE;
    }

    // Stop automatic deletion and then resume thread
    m_pExecuteThread->m_bAutoDelete = FALSE;
    m_pExecuteThread->ResumeThread();
  }
 
  return rc;
}

LRESULT CWZFinish::OnExecuteThreadEnded(WPARAM , LPARAM )
{
  // Wait for it to actually end
  WaitForSingleObject(m_pExecuteThread->m_hThread, INFINITE);

  // Now tidy up (m_bAutoDelete was set to FALSE)
  delete m_pExecuteThread;
  m_pExecuteThread = nullptr;

  m_bComplete = true;
  m_pWZPSH->SetCompleted(true);

  // Was in DboxMain::Merge etc - but not allowed when called from a worker thread!!!!!
  m_pWZPSH->WZPSHUpdateGUIDisplay();

  CString cs_results;
  if (m_thdpms.bCancel) {
    cs_results.LoadString(IDS_OPERATION_ABORTED);
  } else {
   // Can't do UI (show results dialog) from a worker thread!
    switch (m_thdpms.nID) {
      case ID_MENUITEM_COMPARE:
        if (m_thdpms.status != 0) {
          cs_results = m_pWZPSH->WZPSHShowCompareResults(m_pWZPSH->WZPSHGetCurFile(), 
                                                         m_thdpms.sx_Filename,
                                                         m_pothercore, m_prpt);
          m_prpt->EndReport();
        } else {
          cs_results.LoadString(IDS_IDENTICALDATABASES);
        }
        break;
      case ID_MENUITEM_MERGE:
        cs_results = m_thdpms.csResults.c_str();
        break;
      case ID_MENUITEM_SYNCHRONIZE:
        cs_results.Format(IDS_SYNCHRONIZED, m_thdpms.numProcessed);
        break;
      case ID_MENUITEM_EXPORT2PLAINTEXT:
      case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      case ID_MENUITEM_EXPORTENT2DB:
      case ID_MENUITEM_EXPORTGRP2DB:
      case ID_MENUITEM_EXPORTFILTERED2DB:
        cs_results.Format(IDS_EXPORTED, m_thdpms.numProcessed);
        break;
      case ID_MENUITEM_EXPORT2XML:
      case ID_MENUITEM_EXPORTENT2XML:
      case ID_MENUITEM_EXPORTGRP2XML:
        cs_results.Format(IDS_EXPORTED, m_thdpms.numProcessed);
        if (m_thdpms.status == PWScore::OK_WITH_ERRORS) {
          // for export we hide the View Report button, but if something went wrong
          // we show it.
          // Might be worth adopting this approach in general...
          GetDlgItem(IDC_VIEWREPORT)->ShowWindow(SW_SHOW);
          CString cs_errors(MAKEINTRESOURCE(IDSC_XMLCHARACTERERRORS));
          cs_results += L"\n\n";
          cs_results += cs_errors;
        }
        break;
      default:
        ASSERT(0);
        break;
    }
    m_pWZPSH->SetNumProcessed(m_thdpms.numProcessed);
  }

  // Tidy up other core
  if (m_pothercore != nullptr) {
    m_pothercore->SafeUnlockCurFile();
    m_pothercore->ClearDBData();
    m_pothercore->SetCurFile(L"");
    delete m_pothercore;
    m_pothercore = nullptr;
  }

  GetDlgItem(IDC_STATIC_WZRESULTS)->SetWindowText(cs_results);
  GetDlgItem(IDC_STATIC_WZRESULTS)->ShowWindow(SW_SHOW);
  GetDlgItem(IDC_STATIC_WZRESULTS)->EnableWindow(TRUE);

  m_status = m_thdpms.status;

  // Enable Finish button
  m_pWZPSH->SetWizardButtons(PSWIZB_FINISH);

  // Enable View Report button
  GetDlgItem(IDC_VIEWREPORT)->EnableWindow(TRUE);

  m_pWZPSH->WZPSHSetUpdateWizardWindow(nullptr);

  // In Compare status == 0 means identical, status != 0 means different
  // Details placed in results summary.
  // In other functions, status != 0 means : failed.
  CString cs_text, cs_temp;
  if (m_thdpms.bCancel) {
    cs_text.LoadString(IDS_OPERATION_ABORTED);
  } else {
    if (m_thdpms.nID == ID_MENUITEM_COMPARE) {
      cs_text.LoadStringW(IDS_COMPARECOMPLETE);
    } else {
      if (m_status != 0) {
        UINT uiMsg(0);
        switch (m_thdpms.nID) {
          case ID_MENUITEM_MERGE:
            uiMsg = IDS_WZMERGE;
            break;
          case ID_MENUITEM_SYNCHRONIZE:
            uiMsg = IDS_WZSYNCH;
            break;
          case ID_MENUITEM_EXPORT2PLAINTEXT:
          case ID_MENUITEM_EXPORTENT2PLAINTEXT:
          case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
            uiMsg = IDS_WZEXPORTTEXT;
            break;
          case ID_MENUITEM_EXPORT2XML:
          case ID_MENUITEM_EXPORTENT2XML:
          case ID_MENUITEM_EXPORTGRP2XML:
            uiMsg = IDS_WZEXPORTXML;
            break;
          default:
            ASSERT(0);
            break;
        }
        cs_temp.LoadString(uiMsg);
        cs_text.Format(IDS_WZACTIONFAILED, static_cast<LPCWSTR>(cs_temp));
      } else
        cs_text.LoadString(IDS_COMPLETE);
    }
  }

  GetDlgItem(IDC_STATIC_WZPROCESSING)->SetWindowText(cs_text);
  GetDlgItem(IDC_ENTRY)->ShowWindow(SW_HIDE);

  return 0L;
}

void CWZFinish::OnViewReport()
{
  if (m_bViewingReport)
    return;

  if (m_prpt != nullptr) {
    // Stop us doing it again and stop user ending Wizard
    m_bViewingReport = true;
    m_pWZPSH->EnableWindow(FALSE);

    // Show report
    m_pWZPSH->WZPSHViewReport(*m_prpt);

    // OK - let them end the Wizard and look at the report again
    m_bViewingReport = false;
    m_pWZPSH->EnableWindow(TRUE);
  }
}
