/// \file SysColStatic.cpp
//-----------------------------------------------------------------------------
/*
  This entire file was copied from
  http://www.codeguru.com/staticctrl/syscol_static.shtml
  and was written by Pål K. Tønder 
*/

#include "stdafx.h"
#include "SysColStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSysColStatic::CSysColStatic()
{
   m_nImageID = -1;
}

CSysColStatic::~CSysColStatic()
{
}

void CSysColStatic::ReloadBitmap(int nImageID)
{
   if(nImageID != -1)
      m_nImageID = nImageID;

   if(m_nImageID == -1)
      return;

   HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
                                       MAKEINTRESOURCE(m_nImageID),
                                       IMAGE_BITMAP,
                                       0, 0,
                                       LR_LOADMAP3DCOLORS);

   if (hBmp == NULL)
      return;

   hBmp = SetBitmap(hBmp);
   if (hBmp != NULL)
      ::DeleteObject(hBmp);
}

BEGIN_MESSAGE_MAP(CSysColStatic, CStatic)
   //{{AFX_MSG_MAP(CSysColStatic)
   ON_WM_SYSCOLORCHANGE()
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysColStatic message handlers

void CSysColStatic::OnSysColorChange() 
{
   CStatic::OnSysColorChange();
   ReloadBitmap(); 
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
