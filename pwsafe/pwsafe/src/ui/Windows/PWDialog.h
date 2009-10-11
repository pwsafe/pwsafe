/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* PasswordSafe-specific base class for child dialog boxes
*
* All dialog box classes whose instances are children of the
* main dialog box (DboxMain) should be derived from this class
* instead of directly from CDialog
*/

#pragma once

#include <afxwin.h>
#include <afxmt.h> // for CMutex
#include <list>
#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#if defined(POCKET_PC)
#include "pocketpc/PwsPopupDialog.h"
typedef CPwsPopupDialog CPWDialog;
#else

class CPWDialogTracker; // forward declaration

class CPWDialog : public CDialog
{
public:
  CPWDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL)
    : CDialog(nIDTemplate, pParentWnd) {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual INT_PTR DoModal();

  static CPWDialogTracker *GetDialogTracker();

  DECLARE_DYNAMIC(CPWDialog)
private:
  static CPWDialogTracker *sm_tracker;
};

class CPWDialogTracker
{
public:
  CPWDialogTracker();
  ~CPWDialogTracker();

  bool AnyOpenDialogs() const;
  void AddOpenDialog(CWnd *dlg);
  void RemoveOpenDialog(CWnd *dlg);
  void Apply(void (*f)(CWnd *)); // applies f to all open dialogs

private:
  mutable CMutex m_mutex; // to protect access to our list of open dialogs
  // CWnd = CDialog & CPropertySheet common ancestor!
  std::list<CWnd *> m_dialogs;
};

#endif /* POCKET_PC */
