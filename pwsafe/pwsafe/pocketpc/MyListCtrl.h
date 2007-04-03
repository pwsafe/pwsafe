// MyListCtrl.h
//-----------------------------------------------------------------------------

#ifndef MyListCtrl_h
#define MyListCtrl_h

#include "../corelib/PwsPlatform.h"

#if defined(POCKET_PC)

#include "../stdafx.h"

class DboxMain;

//-----------------------------------------------------------------------------
class CMyListCtrl : public CListCtrl
{
	friend class DboxMain;

public:
	typedef CListCtrl	super;

protected:
	afx_msg BOOL OnContextMenu( NMHDR *pNotifyStruct, LRESULT *result );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	DECLARE_MESSAGE_MAP()
};

#endif // defined(POCKET_PC)

#endif