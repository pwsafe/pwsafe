/// \file FileDialogExt.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "FileDialogExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFileDialogExt, CFileDialog)

CFileDialogExt::CFileDialogExt(BOOL bOpenFileDialog,
                               LPCTSTR lpszDefExt,
                               LPCTSTR lpszFileName,
                               DWORD dwFlags,
                               LPCTSTR lpszFilter,
                               CWnd* pParentWnd)
   : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
                 dwFlags, lpszFilter, pParentWnd)
{
   m_ext = lpszDefExt;
}


BEGIN_MESSAGE_MAP(CFileDialogExt, CFileDialog)
END_MESSAGE_MAP()


BOOL
CFileDialogExt::OnFileNameOK()
{
   if (m_ext != NULL)
   {
      if (GetFileExt() != m_ext
          && GetFileExt() != "")
      {
         CString temp = m_ext;
         temp = "This file type must have a ."
            + temp
            + " extension.";
         MessageBox(temp, "File name error", MB_OK|MB_ICONWARNING);
         return 1;
      }
   }
   return CFileDialog::OnFileNameOK();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
