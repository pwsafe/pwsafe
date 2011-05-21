/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#define PWS_MSG_WIZARD_EXECUTE_THREAD_ENDED (WM_APP + 65)

IMPLEMENT_DYNAMIC(CWZFinish, CWZPropertyPage)

CWZFinish::CWZFinish(CWnd *pParent, UINT nIDCaption, const int nType)
 : CWZPropertyPage(IDD, nIDCaption, nType), m_pothercore(NULL), m_prpt(NULL),
   m_pExecuteThread(NULL), m_bInProgress(false), m_bComplete(false), 
   m_bInitDone(false), m_bViewingReport(false), m_status(-1), m_numProcessed(-1)
{
  // Save pointer to my PropertySheet
  m_pWZPSH = (CWZPropertySheet *)pParent;
}

CWZFinish::~CWZFinish()
{
  delete m_prpt;
}

BEGIN_MESSAGE_MAP(CWZFinish, CWZPropertyPage)
  //{{AFX_MSG_MAP(CWZFinish)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_VIEWREPORT, OnViewReport)
  ON_MESSAGE(PWS_MSG_WIZARD_EXECUTE_THREAD_ENDED, OnExecuteThreadEnded)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWZFinish::OnHelp()
{
  CString cs_HelpTopic = app.GetHelpFileName() + L"::/html/wzfinish.html";
  ::HtmlHelp(this->GetSafeHwnd(), (LPCWSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
}

BOOL CWZFinish::OnInitDialog()
{
  CWZPropertyPage::OnInitDialog();

  // Disable Finish until processing complete
  m_pWZPSH->SetWizardButtons(PSWIZB_DISABLEDFINISH);

  m_pWZPSH->GetDlgItem(ID_WIZBACK)->EnableWindow(FALSE);
  m_pWZPSH->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);
  m_pWZPSH->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
  m_pWZPSH->GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

  m_bInitDone = true;

  return TRUE;
}

static UINT WZExecuteThread(LPVOID pParam)
{
  WZExecuteThreadParms *pthdpms = (WZExecuteThreadParms *)pParam;

  int status(PWScore::SUCCESS);

  switch (pthdpms->nID) {
    case ID_MENUITEM_COMPARE:
      status = pthdpms->pWZPSH->WZPSHDoCompare(pthdpms->pcore,
                   pthdpms->bAdvanced, pthdpms->prpt) ? 1 : -1;
      break;
    case ID_MENUITEM_MERGE:
      pthdpms->csResults = pthdpms->pWZPSH->WZPSHDoMerge(pthdpms->pcore, 
                   pthdpms->bAdvanced, pthdpms->prpt);
      break;
    case ID_MENUITEM_SYNCHRONIZE:
      pthdpms->pWZPSH->WZPSHDoSynchronize(pthdpms->pcore,
                   pthdpms->bAdvanced, pthdpms->numProcessed, pthdpms->prpt);
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      status = pthdpms->pWZPSH->WZPSHDoExportText(pthdpms->sx_Filename, 
                   pthdpms->nID == ID_MENUITEM_EXPORT2PLAINTEXT,
                   pthdpms->pWZPSH->GetDelimiter(), pthdpms->bAdvanced, 
                   pthdpms->numProcessed, pthdpms->prpt);
      break;
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      status = pthdpms->pWZPSH->WZPSHDoExportXML(pthdpms->sx_Filename,
                   pthdpms->nID == ID_MENUITEM_EXPORT2XML,
                   pthdpms->pWZPSH->GetDelimiter(), pthdpms->bAdvanced,
                   pthdpms->numProcessed, pthdpms->prpt);
      break;
    default:
      ASSERT(0);
      break;
  }

  // Set the thread return code for caller
  pthdpms->status = status;

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
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
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
      m_pothercore = new PWScore;
      bOtherIsDB = true;
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      m_pothercore = NULL;
      break;
    default:
      ASSERT(0);
      break;
  }

  INT_PTR rc(PWScore::SUCCESS);
  const StringX sx_Filename2 = m_pWZPSH->GetOtherDBFile();

  if (bOtherIsDB) {
    // Not really needed but...
    m_pothercore->ClearData();

    // Reading a new file changes the preferences!
    const StringX sxSavePrefString(PWSprefs::GetInstance()->Store());
    const bool bDBPrefsChanged = PWSprefs::GetInstance()->IsDBprefsChanged();

    const StringX passkey = m_pWZPSH->GetPassKey();

    rc = m_pothercore->ReadFile(sx_Filename2, passkey);

    // Reset database preferences - first to defaults then add saved changes!
    PWSprefs::GetInstance()->Load(sxSavePrefString);
    PWSprefs::GetInstance()->SetDBprefsChanged(bDBPrefsChanged);

    switch (rc) {
      case PWScore::SUCCESS:
        break;
      case PWScore::CANT_OPEN_FILE:
        cs_temp.Format(IDS_CANTOPENREADING, sx_Filename2.c_str());
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
        break;
      case PWScore::BAD_DIGEST:
        cs_temp.Format(IDS_FILECORRUPT, sx_Filename2.c_str());
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
        break;
#ifdef DEMO
      case PWScore::LIMIT_REACHED:
        cs_temp.Format(IDS_LIMIT_MSG2, MAXDEMO);
        cs_title.LoadString(IDS_LIMIT_TITLE);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
        break;
#endif
      default:
        cs_temp.Format(IDS_UNKNOWNERROR, sx_Filename2.c_str());
        cs_title.LoadString(IDS_FILEREADERROR);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
        break;
    }

    if (rc != PWScore::SUCCESS) {
      m_pothercore->ClearData();
      m_pothercore->SetCurFile(L"");
      delete m_pothercore;
      m_pothercore = NULL;
      return rc;
    }
  }

  if (rc == PWScore::SUCCESS) {
    if (bOtherIsDB)
      m_pothercore->SetCurFile(sx_Filename2);

    m_pWZPSH->WZPSHSetUpdateWizardWindow(GetDlgItem(IDC_ENTRY));

    if (m_prpt == NULL)
      m_prpt = new CReport;

    const bool bAdvanced = m_pWZPSH->GetAdvanced();

    WZExecuteThreadParms *pthdpms = new WZExecuteThreadParms;
    pthdpms->pWZFinish = this;
    pthdpms->nID = nID;
    pthdpms->pWZPSH = m_pWZPSH;
    pthdpms->pcore = m_pothercore;
    pthdpms->prpt = m_prpt;
    pthdpms->sx_Filename = sx_Filename2;
    pthdpms->bAdvanced = bAdvanced;

    m_pExecuteThread = AfxBeginThread(WZExecuteThread, pthdpms,
                                THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

    if (m_pExecuteThread == NULL) {
      pws_os::Trace(_T("Unable to create Execute thread\n"));
      return PWScore::FAILURE;
    }

    // Stop automatic deletion and then resume thread
    m_pExecuteThread->m_bAutoDelete = FALSE;
    m_pExecuteThread->ResumeThread();
  }
 
  return rc;
}

LRESULT CWZFinish::OnExecuteThreadEnded(WPARAM wParam, LPARAM )
{
  WZExecuteThreadParms *pthdpms = (WZExecuteThreadParms *)wParam;

  // Wait for it to actually end
  WaitForSingleObject(m_pExecuteThread->m_hThread, INFINITE);

  m_bComplete = true;

  // Now tidy up (m_bAutoDelete was set to FALSE)
  delete m_pExecuteThread;
  m_pExecuteThread = NULL;

  m_bComplete = true;
  m_pWZPSH->SetCompleted(true);

  // Was in DboxMain::Merge etc - but not allowed when called from a worker thread!!!!!
  m_pWZPSH->WZPSHUpdateGUIDisplay();

  // Can't do UI (show results dialog) from a worker thread!
  CString cs_results;
  switch (pthdpms->nID) {
    case ID_MENUITEM_COMPARE:
      if (pthdpms->status != 1) {
        cs_results = m_pWZPSH->WZPSHShowCompareResults(m_pWZPSH->WZPSHGetCurFile(), 
                                                       pthdpms->sx_Filename,
                                                       m_pothercore, m_prpt);
        m_prpt->EndReport();
      } else {
        cs_results.LoadString(IDS_IDENTICALDATABASES);
      }
      break;
    case ID_MENUITEM_MERGE:
      cs_results = pthdpms->csResults.c_str();
      break;
    case ID_MENUITEM_SYNCHRONIZE:
      cs_results.Format(IDS_SYNCHRONIZED, pthdpms->numProcessed);
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      cs_results.Format(IDS_EXPORTED, pthdpms->numProcessed);
      break;
    default:
      ASSERT(0);
      break;
  }

  GetDlgItem(IDC_STATIC_WZRESULTS)->SetWindowText(cs_results);
  GetDlgItem(IDC_STATIC_WZRESULTS)->ShowWindow(SW_SHOW);
  GetDlgItem(IDC_STATIC_WZRESULTS)->EnableWindow(TRUE);

  m_pWZPSH->SetNumProcessed(pthdpms->numProcessed);

  UINT nID = pthdpms->nID;
  m_status = pthdpms->status;
  delete pthdpms;

  if (m_pothercore != NULL) {
    if (m_pothercore->IsLockedFile(m_pothercore->GetCurFile().c_str()))
      m_pothercore->UnlockFile(m_pothercore->GetCurFile().c_str());

    m_pothercore->ClearData();
    m_pothercore->SetCurFile(L"");
    delete m_pothercore;
    m_pothercore = NULL;
  }

  // Enable Finish button
  m_pWZPSH->SetWizardButtons(PSWIZB_FINISH);

  // Enable View Report button
  GetDlgItem(IDC_VIEWREPORT)->EnableWindow(TRUE);

  m_pWZPSH->WZPSHSetUpdateWizardWindow(NULL);

  CString cs_text;
  if (m_status != 0) {
    UINT uiMsg(0);
    switch (nID) {
      case ID_MENUITEM_COMPARE:
        uiMsg = IDS_WZCOMPARE;
        break;
      case ID_MENUITEM_MERGE:
        uiMsg = IDS_WZMERGE;
        break;
      case ID_MENUITEM_SYNCHRONIZE:
        uiMsg = IDS_WZSYNCH;
        break;
      case ID_MENUITEM_EXPORT2PLAINTEXT:
      case ID_MENUITEM_EXPORTENT2PLAINTEXT:
        uiMsg = IDS_WZEXPORTTEXT;
        break;
      case ID_MENUITEM_EXPORT2XML:
      case ID_MENUITEM_EXPORTENT2XML:
        uiMsg = IDS_WZEXPORTXML;
        break;
      default:
        ASSERT(0);
        break;
    }
    CString cs_temp(MAKEINTRESOURCE(uiMsg));
    cs_text.Format(IDS_WZACTIONFAILED, cs_temp);
  } else
    cs_text.LoadString(IDS_COMPLETE);

  GetDlgItem(IDC_STATIC_WZPROCESSING)->SetWindowText(cs_text);
  GetDlgItem(IDC_ENTRY)->ShowWindow(SW_HIDE);

  return 0L;
}

void CWZFinish::OnViewReport()
{
  if (m_bViewingReport)
    return;

  if (m_prpt != NULL) {
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
