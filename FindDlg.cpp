// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "resource.h"
#include "FindDlg.h"
#include "DboxMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

CFindDlg *CFindDlg::self = NULL; // for Singleton pattern

void CFindDlg::Doit(CWnd *pParent)
{
  if (self == NULL) {
    self = new CFindDlg(pParent);
    if (self != NULL)
      if (self->Create(CFindDlg::IDD))
	self->ShowWindow(SW_SHOW);
  } else {
    self->BringWindowToTop();
  }
}

CFindDlg::CFindDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CFindDlg::IDD, pParent), m_indices(NULL),
    m_lastshown(-1), m_numFound(0),
    m_last_search_text(_T("")), m_last_cs_search(FALSE)
{
  //{{AFX_DATA_INIT(CFindDlg)
  m_cs_search = FALSE;
  m_search_text = _T("");
  m_status = _T("");
	//}}AFX_DATA_INIT
}

CFindDlg::~CFindDlg()
{
  delete[] m_indices;
  self = NULL;
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindDlg)
	DDX_Check(pDX, IDC_FIND_CS, m_cs_search);
	DDX_Text(pDX, IDC_FIND_TEXT, m_search_text);
	DDX_Text(pDX, IDC_STATUS, m_status);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	//{{AFX_MSG_MAP(CFindDlg)
	ON_BN_CLICKED(IDOK, OnFind)
	ON_BN_CLICKED(IDCANCEL, OnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers

void CFindDlg::OnFind() 
{

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);
  
  const int numEntries = pParent->GetNumEntries();

  if (numEntries == 0) {
    // Right Thing would be to disable find menu item if list is empty
    m_status = _T("Password list is empty.");
    UpdateData(FALSE);
    return;
  }

  UpdateData(TRUE);

  if (m_search_text.IsEmpty()) {
    m_status = _T("Please enter a search string");
    UpdateData(FALSE);
    return;
  }

  // If the user changes the search text or cs, then this is a new search:
  if (m_search_text != m_last_search_text ||
      m_cs_search != m_last_cs_search) {
    m_last_search_text = m_search_text;
    m_last_cs_search = m_cs_search;
    m_lastshown = -1;
  }

  if (m_lastshown == -1) {

    if (m_indices != NULL) {
      // take care of the pathological case where someone added or deleted
      // an entry while this dialog box is open
      delete[] m_indices;
    }
    m_indices = new int[numEntries];

    m_numFound = pParent->FindAll(m_search_text, m_cs_search, m_indices);

    switch (m_numFound) {
    case 0:
      m_status = _T("No matches found.");
      break;
    case 1:
      m_status = _T("Found 1 match.");
      break;
    default:
      m_status.FormatMessage(_T("Found %1!d! matches."), m_numFound);
      break;
    }
    UpdateData(FALSE);
  } // m_lastshown == -1

  // OK, so now we have a (possibly empty) list of items to select.
  
  if (m_numFound > 0) {
    m_lastshown = (m_lastshown + 1) % m_numFound; //from -1 to 0, cycle afterwards
    pParent->SelectEntry(m_indices[m_lastshown], TRUE);
  }
  if (m_numFound > 1) {
      SetDlgItemText(IDOK, _T("Find Next"));
  }
  // don't call CDialog::OnOK - user will Cancel() to close dbox
}

void CFindDlg::OnClose() 
{
  self = NULL;
  CDialog::OnCancel();
}
