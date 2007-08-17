/**
 * PasswordSafe-specific base class for child dialog boxes
 *
 * All dialog box classes whose instances are children of the
 * main dialog box (DboxMain) should be derived from ths class
 * instead of directly from CDialog
 */

#ifndef __PWDIALOG_H
#define __PWDIALOG_H
#include <afxwin.h>

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#if defined(POCKET_PC)
#include "pocketpc/PwsPopupDialog.h"
typedef	CPwsPopupDialog CPWDialog;
#else
class CPWDialog : public CDialog
{
 protected:
  CPWDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL)
    : CDialog(nIDTemplate, pParentWnd) {}
  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
 public:
  DECLARE_DYNAMIC(CPWDialog)
};
#endif /* POCKET_PC */

#endif /* __PWDIALOG_H */
