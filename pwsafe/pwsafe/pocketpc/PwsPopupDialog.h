// PwsPopupDialog
//-----------------------------------------------------------------------------
//
// This class exists to support the Pocket PC platform and the OO paradigm of
// moving common code into the superclass.
//
// We have a few popup dialogs in Password Safe that don't occupy the full
// screen and we have to steps to stop the MFC framework from forcing them to
// the full size of the screen.  We also override OnSettingChange and
// OnActivate so that we can handle the SIP (Soft Input Panel or virtual
// keyboard) being activated and deactivated.
//
// There's no reason why other WIN32 platforms shouldn't use this as the base
// class for any popup dialogs as the Pocket PC specifics are exluded for other
// platforms.

#if !defined(PwsPopupDialog_h)
#define PwsPopupDialog_h

#include "../stdafx.h"
#include "../corelib/PwsPlatform.h"

class CPwsPopupDialog : public CDialog
{
public:
	typedef CDialog		super;

	CPwsPopupDialog( );
	CPwsPopupDialog( LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL );
	CPwsPopupDialog( UINT nIDTemplate, CWnd* pParentWnd = NULL );

protected:
#if defined(POCKET_PC)
	virtual BOOL OnInitDialog();
#endif

	// Generated message map functions
	//{{AFX_MSG(CConfirmDeleteDlg)
#if defined(POCKET_PC)
	afx_msg VOID	OnSettingChange( UINT nFlags, LPCTSTR lpszSection );
	afx_msg VOID	OnActivate( UINT nState , CWnd *pWndOther , BOOL bMinimized );
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif