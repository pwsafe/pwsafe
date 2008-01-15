//*******************************************************************
//              
//  FILE:       Shortcut.h
//              
//  AUTHOR:     Thomas Latuske <CobUser@GMX.de>
//              
//  COMPONENT:  CShortcut
//              
//  DATE:       04.05.2004
//              
//  COMMENTS:   Update 11.05.2004:
//					- added ResolveLink
//					- added GetSpecialFolder (split off)
//              Update 09.11.2006: (ronys)
//                  - Get to compile cleanly under MSVS2005
//              
//*******************************************************************
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHORTCUT_H__2599590A_2D2B_49B1_9E63_BF123E5B67B8__INCLUDED_)
#define AFX_SHORTCUT_H__2599590A_2D2B_49B1_9E63_BF123E5B67B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
******************************************************************
*!!!! Don't forget to start OLE in your app's InitInstance: !!!!!*
******************************************************************
AfxEnableControlContainer();

// initialize OLE:
if (!AfxOleInit())
{
AfxMessageBox("Unable to initialize OLE.\nTerminating application!");
return FALSE;
}

*/

#include <ShlObj.h>

class CShortcut  
{
public:
  CShortcut();
  ~CShortcut();

  /*  This routine resolves the lnk destination: */	
  HRESULT ResolveLink(const CString &LnkName, UINT SpecialFolder,
    HWND hwnd, CString &LnkPath, CString &LnkDescription);
  /* Looks if the link with name LnkName already exists in the special folder */
  BOOL isLinkExist(const CString &LnkName, UINT SpecialFolder) const;
  /* set argument(s) that will be used when a file is send (SendTo): */
  void SetCmdArguments(const CString &sArg) {m_sCmdArg = sArg;}
  /*! Use this routine to create a ShortCut (ShellLink) for this Application */
  BOOL CreateShortCut(const CString &LnkTarget, const CString &LnkName,
    UINT SpecialFolder, const CString &LnkDescription = _T(""),
    const CString &IconLocation = _T(""), UINT IconIndex = 1);
  /*! Use this routine to delete any Shortcut */
  BOOL DeleteShortCut(const CString &LnkName, UINT SpecialFolder);

private:
  /*  This routine is a helper that finds the path to the special folder: */
  BOOL GetSpecialFolder(UINT SpecialFolder, CString &SpecialFolderString) const;
  /*  This routine is a helper that builds a long path from 8+3 one */
  int ShortToLongPathName(const CString &sShortPath, CString &sLongPath);
  CString m_sCmdArg;
};

#endif // !defined(AFX_SHORTCUT_H__2599590A_2D2B_49B1_9E63_BF123E5B67B8__INCLUDED_)
