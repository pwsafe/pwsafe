
#include "PwsPopupDialog.h"
#include "PocketPC.h"

CPwsPopupDialog::CPwsPopupDialog( )
: super()
{
}


CPwsPopupDialog::CPwsPopupDialog( LPCTSTR lpszTemplateName, CWnd* pParentWnd )
: super( lpszTemplateName, pParentWnd )
{
}


CPwsPopupDialog::CPwsPopupDialog( UINT nIDTemplate, CWnd* pParentWnd )
: super( nIDTemplate, pParentWnd )
{
}


BEGIN_MESSAGE_MAP(CPwsPopupDialog, super)
#if defined(POCKET_PC)
	ON_WM_SETTINGCHANGE()
	ON_WM_ACTIVATE()
#endif
END_MESSAGE_MAP()


#if defined(POCKET_PC)
/*
 * Augment OnInitDialog to stop the dialog being displayed full-screen.
 */
BOOL CPwsPopupDialog::OnInitDialog()
{
	m_bFullScreen = FALSE;
	CenterWindow();
	return super::OnInitDialog();
}


VOID CPwsPopupDialog::OnSettingChange( UINT nFlags, LPCTSTR lpszSection )
{
	CRect	cRect;
	CRect	pRect;
	CRect	rRect;
	SIPINFO	sip;

	// TODO: move the dialog box to a sensible position

	memset( (void*) &sip, 0, sizeof( sip ) );
	sip.cbSize = sizeof( sip );

	SipGetInfo( &sip );
	GetWindowRect( &cRect );
	GetParent()->GetWindowRect( &pRect );

	// If SIP is being displayed and it will overlap this dialog, move this
	// dialog up to try to move it out of the way, or at least maximize the
	// visible area.  If the SIP is being removed from the display don't
	// alter the dialogs position.

	if ( sip.fdwFlags & SIPF_ON )
	{
		// Will there be overlap?

		if ( cRect.bottom > sip.rcSipRect.top )
		{
			// Yes - SIP will overlap this dialog

			int	delta;
			int	height;

			delta			 = cRect.bottom - sip.rcSipRect.top;
			height			 = cRect.Height();
			cRect.top		-= delta;

			// If this dialog has moved outside its parent, move it back.

			if ( cRect.top < pRect.top )
			{
				cRect.top	 = pRect.top;
			}
			cRect.bottom	 = cRect.top + height;
			MoveWindow( &cRect );
		}
	}

	// And now let CWnd update the system metrics etc.
	CWnd::OnSettingChange( nFlags, lpszSection );
}


VOID CPwsPopupDialog::OnActivate( UINT nState , CWnd *pWndOther , BOOL bMinimized )
{
	CWnd::OnActivate( nState, pWndOther, bMinimized );
}
#endif

