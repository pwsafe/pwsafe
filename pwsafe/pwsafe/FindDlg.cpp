/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif
#include "FindDlg.h"
#include "DboxMain.h"
#include "ThisMfcApp.h" // for disable/enable accel.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

CFindDlg *CFindDlg::self = NULL; // for Singleton pattern

void CFindDlg::Doit(CWnd *pParent, BOOL *isCS, CMyString *lastFind, bool *bFindWraps)
{
  if (self == NULL) {
    self = new CFindDlg(pParent, isCS, lastFind);
    if (self != NULL)
      if (self->Create(CFindDlg::IDD)) {
        RECT myRect, parentRect; 
        // move find dialog so that it doesn't overlap its parent
        pParent->GetWindowRect(&parentRect);
        self->GetWindowRect(&myRect);
        // Move the dialog to the right if parent is on left side
        // of screen,and vice versa,
        // UNLESS parent is close to screen's width!
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenCenter = screenWidth/2;
        const int parentWidth = (parentRect.right - parentRect.left);
        const int parentCenter = (parentRect.right + parentRect.left)/2;

        if (parentWidth < (screenWidth * 9) / 10) {
          if (parentCenter < screenCenter) {
            // move right
            myRect.right = parentRect.right + myRect.right - myRect.left;
            myRect.left = parentRect.right;
          } else { // move left
            myRect.left = parentRect.left - (myRect.right - myRect.left);
            myRect.right = parentRect.left;
          }
          self->MoveWindow(&myRect);
        } // parent not too wide
      }
  } else {
    self->BringWindowToTop();
  }
  self->ShowWindow(SW_SHOW);
  self->m_FindWraps = *bFindWraps ? TRUE : FALSE;
  
  ((DboxMain*)pParent)->SetFindActive();  //  Prevent switch tree/list display modes
  app.DisableAccelerator(); // don't accel Del when this dlg is shown
}

CFindDlg::CFindDlg(CWnd* pParent, BOOL *isCS, CMyString *lastFind)
  : super(CFindDlg::IDD, pParent), m_indices(NULL),
    m_lastshown(-1), m_numFound(0),
    m_last_search_text(_T("")), m_last_cs_search(FALSE),
    m_lastCSPtr(isCS), m_lastTextPtr(lastFind)
{
  ASSERT(isCS !=NULL);
  ASSERT(lastFind != NULL);
  //{{AFX_DATA_INIT(CFindDlg)
  m_cs_search = *isCS;
  m_search_text = *lastFind;
  m_status = _T("");
  //}}AFX_DATA_INIT

}

void CFindDlg::EndIt()
{
  delete self;
}

CFindDlg::~CFindDlg()
{
  delete[] m_indices;
  self = NULL;
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
  super::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFindDlg)
  DDX_Check(pDX, IDC_FIND_CS, m_cs_search);
  DDX_Check(pDX, IDC_FIND_WRAP, m_FindWraps);
  DDX_Text(pDX, IDC_FIND_TEXT, m_search_text);
#if !defined(POCKET_PC)
  DDX_Text(pDX, IDC_STATUS, m_status);
#endif
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDlg, super)
	//{{AFX_MSG_MAP(CFindDlg)
	ON_BN_CLICKED(IDOK, OnFind)
	ON_BN_CLICKED(IDC_FIND_WRAP, OnWrap)
#if defined(POCKET_PC)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
#else
	ON_BN_CLICKED(IDCANCEL, OnClose)
#endif
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
    m_status.LoadString(IDS_PASSWORDLISTEMPTY);
    UpdateData(FALSE);
    return;
  }

  UpdateData(TRUE);

  if (m_search_text.IsEmpty()) {
    m_status.LoadString(IDS_ENTERSEARCHSTRING);
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

  if (m_bLastView != pParent->GetCurrentView()) {
	  m_bLastView = pParent->GetCurrentView();
	  m_lastshown = -1;  // Indices will be in different order even if search the same
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
      m_status.LoadString(IDS_NOMATCHFOUND);
      break;
    case 1:
      m_status.LoadString(IDS_FOUNDAMATCH);
      break;
    default:
      m_status.Format(IDS_FOUNDMATCHES, m_numFound);
      break;
    }
    UpdateData(FALSE);
  } // m_lastshown == -1

  // OK, so now we have a (possibly empty) list of items to select.

  if (m_numFound > 0) {
    if (m_numFound == 1) {
    	pParent->SelectEntry(m_indices[0], TRUE);
    } else {
    	m_lastshown++;
    	if(m_lastshown >= m_numFound) {
    		int rc = IDYES;
    		if (m_FindWraps == FALSE) {  // Ask
				CString cs_text, cs_title;
    			cs_text.LoadString(IDS_CONTINUESEARCH);
    			cs_title.LoadString(IDS_SEARCHNOTFOUND);
    			rc = MessageBox(cs_text, cs_title, 
					MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2);
    		}
    		switch (rc) {
    			case IDYES:
    				m_lastshown = 0;
    				pParent->SelectEntry(m_indices[m_lastshown], TRUE);
    				break;
				case IDNO:
#if defined(POCKET_PC)
					OnCancel();
#else
					OnClose();
#endif
					break;
    			case IDCANCEL:
    				break;
    			default:
    				ASSERT(FALSE);
    		}
    	} else {
    		pParent->SelectEntry(m_indices[m_lastshown], TRUE);
    	}
    	CString cs_text;
		cs_text.LoadString(IDS_FINDNEXT);
    	SetDlgItemText(IDOK, cs_text);
    }
  }
  // don't call super::OnOK - user will Cancel() to close dbox
}

void CFindDlg::OnWrap()
{
  DboxMain* pParent = (DboxMain*)GetParent();
  pParent->SetFindWrap(((CButton*)GetDlgItem(IDC_FIND_WRAP))->GetCheck() == 1);
}

#if defined(POCKET_PC)
void CFindDlg::OnCancel()
{
  UpdateData(TRUE);
  *m_lastTextPtr = m_search_text;
  *m_lastCSPtr = m_cs_search;

  app.EnableAccelerator(); // restore accel table
  super::DestroyWindow();
}
#else
void CFindDlg::OnClose() 
{
  UpdateData(TRUE);
  *m_lastTextPtr = m_search_text;
  *m_lastCSPtr = m_cs_search; 

  DboxMain* pParent = (DboxMain*)GetParent();
  pParent->SetFindInActive();  //  Allow switch tree/list display modes again
  m_bLastView = pParent->GetCurrentView();

  app.EnableAccelerator(); // restore accel table
  super::OnCancel();
}
#endif
