// MyListCtrl.cpp
//-----------------------------------------------------------------------------
// This code is based on an article on the Pocket PC Developer Network site:
// http://www.pocketpcdn.com/
//
// {kjp} Something to address - we assume that this->GetParent() is a pointer
// {kjp} to an instance of DboxMain - we really should use dynamic_cast - but
// {kjp} it seems this is an eVC 4.0 feature (i.e Pocket PC 2003 and CE .NET
// {kjp} compact) and is not present in eVC 3.0.

#include "../PwsPlatform.h"

#if defined(POCKET_PC)

#include "MyListCtrl.h"
#include "../DboxMain.h"
#include "resource.h"

#define HANDLE_LBUTTON	0		// set to 1 to handle ourselves

BEGIN_MESSAGE_MAP(CMyListCtrl,CListCtrl)
	ON_NOTIFY_REFLECT(GN_CONTEXTMENU, OnContextMenu)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


/*
 * When the user taps the screen we'll get here.  What we'll do is check to
 * see whether the user has tapped on a list item, and if so well then look
 * to see if it's a tap-and-hold (TAH) event.  For a TAH event, Default()
 * is called which causes a GN_CONTEXTMENU event, handled by OnContextMenu.
 * For a non-TAH event we just defer to the superclass to select the item.
 * But if they click anywhere in the list control but not on an item we'll
 * just ignore the event which will stop the TAH red dots from appearing.
 */
void CMyListCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
#if HANDLE_LBUTTON != 0
	unsigned	htFlags;
	SHRGINFO	shrgi	= { 0 };

	shrgi.cbSize		= sizeof( SHRGINFO );
	shrgi.hwndClient	= m_hWnd;
	shrgi.ptDown.x		= point.x;
	shrgi.ptDown.y		= point.y;
	shrgi.dwFlags		= SHRG_NOTIFYPARENT;
//	shrgi.dwFlags		= SHRG_RETURNCMD;

	if ( this->HitTest( point, &htFlags ) != -1 )
	{
		// User has tapped on a list item

		if ( ::SHRecognizeGesture( &shrgi ) == GN_CONTEXTMENU )
		{
			// And it's a tap-and-hold event

			Default();
			return;
		}
	}
	else if ( (htFlags & (LVHT_ABOVE | LVHT_BELOW | LVHT_TOLEFT | LVHT_TORIGHT)) == 0 )
	{
		// User has tapped somewhere in the list control, but not on an item.
		// Suppress the TAH event.

		return;
	}
#endif
	CListCtrl::OnLButtonDown( nFlags, point );
}


/*
 * A TAH event occurred on a list item - display the popup menu.  All events
 * are passed to the parent for handling.
 */
BOOL CMyListCtrl::OnContextMenu( NMHDR *pNotifyStruct, LRESULT *result )
{
	if ( this->GetSelectedCount() > 0 )
	{
		PNMRGINFO	pInfo = (PNMRGINFO) pNotifyStruct;
		CMenu		popup;

		popup.LoadMenu( IDR_POPMENU );
		popup.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN, pInfo->ptAction.x, pInfo->ptAction.y, this->GetParent() );
	}
	*result = 0;
	return TRUE;
}

#endif