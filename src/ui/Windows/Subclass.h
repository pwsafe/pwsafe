/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
////////////////////////////////////////////////////////////////
// PixieLib(TM) Copyright 1997-1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#pragma once

//////////////////
// Generic class to hook messages on behalf of a CWnd.
// Once hooked, all messages go to CSubclassWnd::WindowProc before going
// to the window. Specific subclasses can trap messages and do something.
//
// To use:
//
// * Derive a class from CSubclassWnd.
//
// * Override CSubclassWnd::WindowProc to handle messages. Make sure you call
//   CSubclassWnd::WindowProc if you don't handle the message, or your
//   window will never get messages. If you write separate message handlers,
//   you can call Default() to pass the message to the window.
//
// * Instantiate your derived class somewhere and call HookWindow(pWnd)
//   to hook your window, AFTER it has been created.
//   To unhook, call Unhook or HookWindow(NULL).
//
// This is a very important class, crucial to many of the widgets Window
// widgets implemented in PixieLib. To see how it works, look at the HOOK
// sample program.
//
class CSubclassWnd : public CObject
{
public:
  CSubclassWnd();
  ~CSubclassWnd();

  // Subclass a window. Hook(NULL) to unhook (automatic on WM_NCDESTROY)
  BOOL HookWindow(HWND  hwnd);
  BOOL HookWindow(CWnd* pWnd) { return HookWindow(pWnd->GetSafeHwnd()); }
  void Unhook() { HookWindow((HWND)NULL); }
  BOOL IsHooked() { return m_hWnd!=NULL; }

  friend LRESULT CALLBACK HookWndProc(HWND, UINT, WPARAM, LPARAM);
  friend class CSubclassWndMap;

  virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
  LRESULT Default();  // call this at the end of handler fns

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  HWND m_hWnd;             // the window hooked
  LONG_PTR m_pOldWndProc;  // ... and original window proc
  CSubclassWnd* m_pNext;   // next in chain of hooks for this window

  DECLARE_DYNAMIC(CSubclassWnd);
};
