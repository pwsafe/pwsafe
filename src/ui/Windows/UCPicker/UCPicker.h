// UCPicker.h : main header file for the UCPicker DLL
//

#pragma once

#ifndef __AFXWIN_H__
  #error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#ifdef UCPICKER_EXPORTS
#define UCPICKER_API __declspec(dllexport)
#else
#define UCPICKER_API __declspec(dllimport)
#endif /* UCPICKER_EXPORTS */

#ifdef __cplusplus
extern "C" {
#endif /* Start bracket of __cplusplus */

UCPICKER_API BOOL GetUnicodeBuffer(CString& csBuffer, CString& csREBuffer, int& numchars);

#ifdef __cplusplus
}
#endif /* End bracket of __cplusplus */

// CUCPickerApp
// See UCPicker.cpp for the implementation of this class
//

class CUCPickerApp : public CWinApp
{
public:
  CUCPickerApp();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CUCPickerApp)
public:
  virtual BOOL InitInstance();
  //}}AFX_VIRTUAL

// Implementation

  //{{AFX_MSG(CUCPickerApp)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these UCBlocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

