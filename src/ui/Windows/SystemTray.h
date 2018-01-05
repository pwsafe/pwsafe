/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/////////////////////////////////////////////////////////////////////////////
// SystemTray.h : header file
//
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Copyright (c) 1998.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to your
// computer, causes your pet cat to fall ill, increases baldness or
// makes you car start emitting strange noises when you start it up.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 

#include "core/RUEList.h"
#include "CoolMenu.h"
#include <afxdisp.h>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CSystemTray window

class DboxMain;

class CSystemTray : public CWnd
{
  // Construction/destruction
public:
  //    CSystemTray();
  CSystemTray(CWnd *pWnd, UINT uCallbackMessage, LPCWSTR szTip, HICON icon,
              CRUEList &RUEList, UINT uID, UINT menuID);
  virtual ~CSystemTray();

  DECLARE_DYNAMIC(CSystemTray)

  // Operations
public:
  void SetTarget(CWnd *pTarget) {m_pTarget = pTarget;}
  BOOL Enabled() const {return m_bEnabled;}
  BOOL Visible() const {return !m_bHidden;}

  // Create the tray icon
  BOOL Create(CWnd *pParent, UINT uCallbackMessage, LPCWSTR szTip, HICON icon,
              UINT uID, UINT menuID);

  // Change or retrieve the Tooltip text
  BOOL SetTooltipText(LPCWSTR pszTooltipText);
  BOOL SetTooltipText(UINT nID);
  CString GetTooltipText() const;

  // Change or retrieve the icon displayed
  BOOL SetIcon(HICON hIcon);
  BOOL SetIcon(LPCWSTR lpszIconName);
  BOOL SetIcon(UINT nIDResource);
  BOOL SetStandardIcon(LPCWSTR lpIconName);
  BOOL SetStandardIcon(UINT nIDResource);
  HICON GetIcon() const;
  void HideIcon();
  void ShowIcon();
  void RemoveIcon();
  void MoveToRight();

  // For icon animation
  BOOL SetIconList(UINT uFirstIconID, UINT uLastIconID); 
  BOOL SetIconList(HICON *pHIconList, UINT nNumIcons); 
  BOOL Animate(UINT nDelayMilliSeconds, int nNumSeconds = -1);
  BOOL StepAnimation();
  BOOL StopAnimation();
  
  // Change or retrieve the window to send notification messages to
  BOOL SetNotificationWnd(CWnd *NotifyWnd);
  CWnd *GetNotificationWnd() const;

  // Default handler for tray notification message
  virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

protected:
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  BOOL m_bEnabled;   // does O/S support tray icon?
  BOOL m_bHidden;    // Has the icon been hidden?
  NOTIFYICONDATA m_tnd;

  CArray<HICON, HICON> m_IconList;
  static UINT m_nIDEvent;
  UINT_PTR m_uIDTimer;
  int m_nCurrentIcon;
  UINT m_menuID;
  COleDateTime m_StartTime;
  int m_nAnimationPeriod;
  HICON m_hSavedIcon;
  UINT m_DefaultMenuItemID;
  BOOL m_DefaultMenuItemByPos;
  CWnd *m_pTarget; // ronys
  static const UINT m_nTaskbarCreatedMsg; //thedavecollins
  const CRUEList &m_RUEList; // reference set to dboxmain's
  std::vector<RUEntryData> m_menulist;

  //{{AFX_MSG(CSystemTray)
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG
  LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pParent;
};
