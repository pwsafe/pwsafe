// ViewAttachments.cpp : implementation file
//

#include "stdafx.h"
#include "ViewAttachments.h"
#include "PWAttLC.h"
#include "ThisMfcApp.h" // For Help
#include "DboxMain.h"

#include "resource.h"
#include "resource3.h"
#include "corelib/StringX.h"
#include "corelib/util.h"
#include "corelib/sha1.h"

#include "os/file.h"
#include "os/dir.h"

#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>

// CViewAttachments dialog

IMPLEMENT_DYNAMIC(CViewAttachments, CPWDialog)

CViewAttachments::CViewAttachments(CWnd* pParent)
  : CPWDialog(CViewAttachments::IDD, pParent),
  m_pToolTipCtrl(NULL), m_bInitdone(false),
  m_bSortAscending(true), m_iSortedColumn(0)
{
  m_pDbx = static_cast<DboxMain *>(pParent);
}

CViewAttachments::~CViewAttachments()
{
  delete m_pToolTipCtrl;
}

void CViewAttachments::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ATTACHMENT_LIST, m_AttLC);
}

BEGIN_MESSAGE_MAP(CViewAttachments, CPWDialog)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_COMMAND(ID_HELP, OnHelp)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_VIEW_ATTLC_HEADER, OnColumnClick)
END_MESSAGE_MAP()

// CViewAttachments message handlers

BOOL CViewAttachments::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CPWDialog::PreTranslateMessage(pMsg);
}


BOOL CViewAttachments::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_AttLC.Init(CPWAttLC::VIEW, (CWnd *)m_pDbx, m_pDbx->IsDBReadOnly());

  // Make some columns centered
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;

  DWORD dwExStyle;
  dwExStyle = m_AttLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES;
  m_AttLC.SetExtendedStyle(dwExStyle);

  // Add attachment to ListCtrl
  m_AttLC.AddAttachments(m_vATRecordsEx);

  UpdateData(FALSE);

  m_bInitdone = true;
  return TRUE;
}

void CViewAttachments::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/extract.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CViewAttachments::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
  NMHEADER *pNMHeaderCtrl  = (NMHEADER *)pNMHDR;

  // Get column number to CItemData value
  int isortcolumn = pNMHeaderCtrl->iItem;

  if (m_iSortedColumn == isortcolumn) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    m_iSortedColumn = isortcolumn;
    m_bSortAscending = true;
  }

  m_AttLC.SortItems(AttCompareFunc, (LPARAM)this);

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP   0x0400
#define HDF_SORTDOWN 0x0200
#endif

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  m_AttLC.GetHeaderCtrl()->GetItem(isortcolumn, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  m_AttLC.GetHeaderCtrl()->SetItem(isortcolumn, &hdi);

  *pResult = TRUE;
}

/*
* Compare function used by m_AttLC.SortItems()
*/
int CALLBACK CViewAttachments::AttCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                              LPARAM lParamSort)
{

  // m_bSortAscending to determine the direction of the sort (duh)

  CViewAttachments *self = (CViewAttachments *)lParamSort;
  const int nSortColumn = self->m_iSortedColumn;
  const ATRecordEx *pLHS = &(self->m_vATRecordsEx[(size_t)lParam1]);
  const ATRecordEx *pRHS = &(self->m_vATRecordsEx[(size_t)lParam2]);
  int iResult(0);
  bool b1, b2;

  switch(nSortColumn) {
    case 0:
      // Group
      iResult = pLHS->sxGroup.compare(pRHS->sxGroup);
      break;
    case 1:
      // Title
      iResult = pLHS->sxTitle.compare(pRHS->sxTitle);
      break;
    case 2:
      // User
      iResult = pLHS->sxUser.compare(pRHS->sxUser);
      break;
    case 3:
      // Flag column - Secure Erasure program defined and exists in this environment
      b1 = (pLHS->atr.flags & ATT_ERASEPGMEXISTS) == ATT_ERASEPGMEXISTS;
      b2 = (pRHS->atr.flags & ATT_ERASEPGMEXISTS) == ATT_ERASEPGMEXISTS;
      if ((b1 && b2) || (!b1 && !b2))
        iResult = 0;
      else
      if (b1 && !b2)
        iResult = -1;
      else
        iResult = 1;
      break;
    case 4:
      // Flag column - Extract to removeable drive only
      b1 = (pLHS->atr.flags & ATT_EXTRACTTOREMOVEABLE) == ATT_EXTRACTTOREMOVEABLE;
      b2 = (pRHS->atr.flags & ATT_EXTRACTTOREMOVEABLE) == ATT_EXTRACTTOREMOVEABLE;
      if ((b1 && b2) || (!b1 && !b2))
        iResult = 0;
      else
      if (b1 && !b2)
        iResult = -1;
      else
        iResult = 1;
      break;
    case 5:
      // Filename
      iResult = pLHS->atr.filename.compare(pRHS->atr.filename);
      break;
    case 6:
      // Description
      iResult = pLHS->atr.description.compare(pRHS->atr.description);
      break;
    case 7:
      // Path
      iResult = pLHS->atr.path.compare(pRHS->atr.path);
      break;
    default:
      iResult = 0; // should never happen - just keep compiler happy
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending) {
    iResult *= -1;
  }
  return iResult;
}
