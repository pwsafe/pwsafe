// UCPicker.cpp : Defines the initialization routines for the DLL.
//

#include "StdAfx.h"
#include "UCPicker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUCPickerApp

BEGIN_MESSAGE_MAP(CUCPickerApp, CWinApp)
END_MESSAGE_MAP()

// CUCPickerApp construction

CUCPickerApp::CUCPickerApp()
{
}

// The one and only CUCPickerApp object

CUCPickerApp theApp;

// CUCPickerApp initialization

BOOL CUCPickerApp::InitInstance()
{
  AfxInitRichEdit2();

  // Ensure that the common control DLL is loaded.
  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icex);

  CWinApp::InitInstance();

  return TRUE;
}

UCPICKER_API BOOL GetUnicodeBuffer(CSecString& csBuffer, CSecString& csRTFBuffer, int& numchars)
{
  CUCPickerDlg dlg;

  INT_PTR rc = dlg.DoModal();

  if (rc == IDOK) {
    csBuffer = dlg.GetUnicodeBuffer();
    csRTFBuffer = dlg.GetUnicodeRTFBuffer();
    numchars = dlg.GetNumberCharacters();
    return TRUE;
  }

  return FALSE;
}
