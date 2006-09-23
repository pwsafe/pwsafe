#pragma once

// FindDlg.h : header file
//

#include "corelib/PwsPlatform.h"
#include "corelib/MyString.h"

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

class CFindDlg : public SUPERCLASS
{
  // Construction
 public:
	typedef SUPERCLASS		super;

  static void Doit(CWnd* pParent, BOOL *isCS, CMyString *lastFind,
                   bool *bFindWraps); // implement Singleton pattern
  ~CFindDlg();
  static void EndIt();
  // Dialog Data
  //{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_FIND };
  BOOL m_cs_search;
  BOOL m_FindWraps;
  CMyString	m_search_text;
  CString	m_status;
	//}}AFX_DATA


  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CFindDlg)
 protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
 protected:

  // Generated message map functions
  //{{AFX_MSG(CFindDlg)
  afx_msg void OnFind();
  afx_msg void OnWrap();
#if defined(POCKET_PC)
  afx_msg void OnCancel();
#else
  afx_msg void OnClose();
#endif
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()

 private:
  CFindDlg(CWnd* pParent, BOOL *isCS, CMyString *lastFind);
  static CFindDlg *self;
  int *m_indices; // array of found items
  int m_lastshown; // last index selected, -1 indicates no search done yet
  int m_numFound; // number of items that matched, as returned by DboxMain::FindAll
  CMyString m_last_search_text;
  BOOL m_last_cs_search;

  CMyString *m_lastTextPtr;
  BOOL *m_lastCSPtr;
  bool m_bLastView;
};

#undef SUPERCLASS

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
