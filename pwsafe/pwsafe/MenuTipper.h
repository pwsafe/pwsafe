////////////////////////////////////////////////////////////////
// MSDN Magazine -- November 2003
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio .NET on Windows XP. Tab size=3.
//
#pragma once
#include "PupText.h"
#include "subclass.h"

//////////////////
// Implement menu tips for any MFC main window. To use:
//
// - instantiate CMenuTipManager in your CMainFrm
// - call Install
// - implement prompt strings the normal way: as resource strings w/ID=command ID.
//
class CMenuTipManager : public CSubclassWnd {
protected:
	CPopupText m_wndTip;		// home-grown "tooltip"
	BOOL m_bMouseSelect;		// whether menu invoked by mouse
	BOOL m_bSticky;				// after first tip appears, show rest immediately

public:
	int m_iDelay;				// tooltip delay: you can change

	CMenuTipManager() : m_iDelay(2000), m_bSticky(FALSE) { }
	~CMenuTipManager() { }

	// call this to install tips
	void Install(CWnd* pWnd) { HookWindow(pWnd); }

	// Useful helpers to get window/rect of current active menu
	static CWnd* GetRunningMenuWnd();
	static void  GetRunningMenuRect(CRect& rcMenu);
	CRect GetMenuTipRect(HMENU hmenu, UINT nID);

	// Useful helper to get the prompt string for a command ID.
	// Like CFrameWnd::GetMessageString, but you don't need a frame wnd.
	static CString GetResCommandPrompt(UINT nID);

	// hook fn to trap main window's messages
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);

	// Call these handlers from your main window
	void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu);
	void OnEnterIdle(UINT nWhy, HWND hwndWho);
};

