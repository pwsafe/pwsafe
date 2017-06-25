//*******************************************************************
//              
//  FILE:       Shortcut.cpp
//              
//  AUTHOR:     Thomas Latuske <CobUser@GMX.de>
//              
//  COMPONENT:  CShortcut
//              
//  DATE:       04.05.2004
//              
//  COMMENTS:   Update 11.05.2004:
//                 - added ResolveLink
//                 - added GetSpecialFolder (split off)
//              Update 09.11.2006: (ronys)
//                 - Get to compile cleanly under MSVS2005
//                 - const correctness, char -> TCHAR, minor cleanups
//              Update 01.07.2009: (c-273)
//                 - TCHAR -> wchar_t
//*******************************************************************

// Includes
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Shortcut.h"

#include "os/debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CShortcut::CShortcut()
{
  m_sCmdArg.Empty();
}

CShortcut::~CShortcut()
{
}

  /*!
  ***************************************************************
  
  \param LnkTarget      - The File/Folder the link belongs to
  \param LnkName        - The name of the ShortCut
  \param SpecialFolder  - where to put the shortcut (See #defines below)
  \param LnkDescription - an application can use it to store
                          any text information and can retrieve
                          it with "IShellLink::GetDescription"
  \param IconLocation   - path to the file where the icon is located
                          that should be used. Can be an empty string
  \param IconIndex      - the # of the icon in the file
  
  \return BOOL          - ShortCut created or not
  
  Defines for Special Folders:
  
  SendTo Menu/Folder:              CSIDL_SENDTO
  Desktop for current User         CSIDL_DESKTOP
  Desktop:                         CSIDL_COMMON_DESKTOPDIRECTORY
  Autostart for current User:      CSIDL_STARTUP
  Autostart:                       CSIDL_COMMON_STARTUP
  Start-menu for current User:     CSIDL_STARTMENU
  Start-menu:                      CSIDL_STARTMENU
  Programms-menu for current User: CSIDL_COMMON_STARTMENU
  and some more.....
*****************************************************************/

BOOL CShortcut::CreateShortCut(const CString &LnkTarget,
                               const CString &LnkName, UINT SpecialFolder,
                               const CString &LnkDescription,
                               const CString &IconLocation, UINT IconIndex)
{
  CFile cfFull;
  CString sExePath, sExe, sSpecialFolder;

  wchar_t *chTmp = sExePath.GetBuffer(MAX_PATH);

  GetModuleFileName(NULL, chTmp, MAX_PATH);

  sExePath.ReleaseBuffer();

  // Find the Special Folder:
  if (!GetSpecialFolder(SpecialFolder, sSpecialFolder))
    return FALSE;

  sSpecialFolder += LnkName + L"." + L"lnk";

  if (LnkTarget == L"_this") {
    cfFull.SetFilePath(sExePath);
    sExe = cfFull.GetFileName();
    sExe.Delete(sExe.Find(L".") + 1, 3);
  } else {
    sExePath = LnkTarget;
  }

  // Create the ShortCut:
  CoInitialize(NULL);
  BOOL bRet = FALSE;
  IShellLink* psl;

  if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IShellLink,
                                 (LPVOID*) &psl))) {
    IPersistFile* ppf;

    psl->SetPath(sExePath);
    psl->SetDescription(LnkDescription);

    if (!m_sCmdArg.IsEmpty())
      psl->SetArguments(m_sCmdArg);

    if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf))) {
      /* Call IShellLink::SetIconLocation with the file containing
         the icon and the index of the icon */
      if (!IconLocation.IsEmpty()) {
        HRESULT hr = psl->SetIconLocation(IconLocation, IconIndex);
        if (FAILED(hr))
          pws_os::Trace(L"IconLocation not changed!\n");
      }

      if (SUCCEEDED(ppf->Save(sSpecialFolder, TRUE)))
      {
        bRet = TRUE;
      }
      ppf->Release();
    }
    psl->Release();
  } 

  pws_os::Trace(bRet ? L"Lnk Written!\n" :
               L"Lnk NOT Written! CreateShortCut(...) failed!\n");
  return bRet;
}

/*!
***************************************************************

  \param LnkName       - The name of the ShortCut
  \param SpecialFolder - Location of the shortcut (See #defines below)
  
  \return void
  
  Defines for Special Folders:
  
  SendTo Menu/Folder:              CSIDL_SENDTO
  Desktop for current User         CSIDL_DESKTOP
  Desktop:                         CSIDL_COMMON_DESKTOPDIRECTORY
  Autostart for current User:      CSIDL_STARTUP
  Autostart:                       CSIDL_COMMON_STARTUP
  Start-menu for current User:     CSIDL_STARTMENU
  Start-menu:                      CSIDL_STARTMENU
  Programms-menu for current User: CSIDL_COMMON_STARTMENU
  and some more.....
*****************************************************************/

BOOL CShortcut::DeleteShortCut(const CString &LnkName, UINT SpecialFolder)
{
  CFile cfFull;
  CString sExePath, sExe, sSpecialFolder;
  wchar_t *chTmp = sExePath.GetBuffer(MAX_PATH);

  GetModuleFileName(NULL, chTmp, MAX_PATH);
  sExePath.ReleaseBuffer();

  if (!GetSpecialFolder(SpecialFolder, sSpecialFolder))
    return FALSE;

  // Work with the special folder's path (contained in szPath)
  cfFull.SetFilePath(sExePath);
  sExe = cfFull.GetFileName();
  sExe.Delete(sExe.Find(L".") + 1, 3);
  sSpecialFolder += LnkName + L"." + L"lnk";

  // DELETE THE LINK:
  SHFILEOPSTRUCT FIO = {0};
  //  FIO.pTo=NULL; // MUST be NULL
  FIO.wFunc = FO_DELETE;
  FIO.fFlags = FOF_NOERRORUI | FOF_NOCONFIRMATION;

  if (sSpecialFolder.Find(L'\0') != sSpecialFolder.GetLength()) {
    FIO.fFlags |= FOF_MULTIDESTFILES;
  }

  if (sSpecialFolder.Right(1)) {
    sSpecialFolder += L'\0';
  }

  FIO.pFrom = &*sSpecialFolder;

  int bD = SHFileOperation(&FIO);

  if (!bD) {
    pws_os::Trace(L"Lnk Deleted!\n");
    return TRUE;
  } else {
    pws_os::Trace(L"Lnk NOT Deleted! DeleteShortCut(...) FAILED!\n");
    return FALSE;
  }
}

/*********************************************
This routine tests if a link already exists:
*********************************************/
BOOL CShortcut::isLinkExist(const CString &LnkName,
                            UINT SpecialFolder) const
{
  CFileStatus cfStatus;
  CString sSpecialFolder;
  // Find the Special Folder:
  if (!GetSpecialFolder(SpecialFolder, sSpecialFolder))
    return FALSE;

  // Work with the special folder's path (contained in szPath)
  sSpecialFolder += L"\\";
  sSpecialFolder += LnkName + L"." + L"lnk";

  BOOL brc = CFile::GetStatus(sSpecialFolder, cfStatus);
  pws_os::Trace(L"%s = %s\n", brc == TRUE ? L"Full file name" : L"File NOT available", 
                cfStatus.m_szFullName);
  return brc;
}

/*********************************************
This routine resolves the lnk destination:
  
  \param LnkName         - The name of the ShortCut\n
  \param SpecialFolder   - where to put the shortcut (See #defines above (MSDN))
  \param hwnd            - handle of the parent window for MessageBoxes
                           the shell may need to display
  \param LnkPath         - Reference to a CString that receives the linkpath if
                           routine is successful else the string will be empty
  \param LnkDescription  - Reference to a CString that receives the Description
                           of the link else the string will be empty
  
  \returns a HRESULT
*********************************************/
HRESULT CShortcut::ResolveLink(const CString &LnkName, UINT SpecialFolder,
                               HWND hwnd, CString &LnkPath,
                               CString &LnkDescription)
{
  HRESULT hres;     
  IShellLink* psl;
  wchar_t *szGotPath = LnkPath.GetBuffer(MAX_PATH); 
  wchar_t *szDescription = LnkDescription.GetBuffer(MAX_PATH);
  CString sLnkFile, sSpecialFolder;
  CString sLong;
  WIN32_FIND_DATA wfd;  

  // get the path to the special folder:
  if (!GetSpecialFolder(SpecialFolder, sSpecialFolder))
    return 1; // return ERROR
  // build a linkfile:
  sLnkFile = sSpecialFolder + LnkName + L".lnk";

  // Get a pointer to the IShellLink interface. 
  hres = CoCreateInstance(CLSID_ShellLink,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IShellLink,
                          (LPVOID*) &psl);
  if (SUCCEEDED(hres)) {
    IPersistFile* ppf;  
    // Get a pointer to the IPersistFile interface. 
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

    if (SUCCEEDED(hres)) { 
      hres = ppf->Load(sLnkFile, STGM_READ); 
      if (SUCCEEDED(hres)) {   // Resolve the link. 
        hres = psl->Resolve(hwnd, SLR_ANY_MATCH); 

        if (SUCCEEDED(hres)) {  
          // Get the path to the link target. 
          hres = psl->GetPath(szGotPath, MAX_PATH,
                              (WIN32_FIND_DATA *)&wfd, 
                              SLGP_SHORTPATH); 

          LnkPath.ReleaseBuffer();

          if (!SUCCEEDED(hres)) {
            LnkDescription.ReleaseBuffer();
            return hres; // application-defined function  
          }
          // Get the description of the target:
          hres = psl->GetDescription(szDescription, MAX_PATH); 
          LnkDescription.ReleaseBuffer();

          if (!SUCCEEDED(hres)) 
            return hres; 
        } 
      }
      // Release the pointer to the IPersistFile interface. 
      ppf->Release();         
    } 
    // Release the pointer to the IShellLink interface. 
    psl->Release();
  }
  // whether OS is <= NT4 or not... use this helper:
  if (ShortToLongPathName(LnkPath, sLong) > LnkPath.GetLength())
    LnkPath = sLong;

  return hres; 
}

/*********************************************
  This routine is a helper that finds the path to the special folder:
  
  \param SpecialFolder        - an UINT-define (See #defines above or (MSDN))
  \param SpecialFolderString  - Reference to a CString that receives the
                                path to the special folder
  \returns a BOOL - Found or not
*********************************************/
BOOL CShortcut::GetSpecialFolder(UINT SpecialFolder,
                                 CString &SpecialFolderString) const
{
  HRESULT hr;
  // Find the Special Folder:
  // Allocate a pointer to an Item ID list
  LPITEMIDLIST pidl;

  // Get a pointer to an item ID list that
  // represents the path of a special folder
  hr = SHGetSpecialFolderLocation(NULL, SpecialFolder, &pidl);
  if (SUCCEEDED(hr)) {  // Convert the item ID list's binary
    // representation into a file system path
    wchar_t szPath[_MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath)) {
      // Allocate a pointer to an IMalloc interface
      LPMALLOC pMalloc;

      // Get the address of our task allocator's IMalloc interface
      hr = SHGetMalloc(&pMalloc);

      // Free the item ID list allocated by SHGetSpecialFolderLocation
      pMalloc->Free(pidl);

      // Free our task allocator
      pMalloc->Release();

      // Work with the special folder's path (contained in szPath)
      SpecialFolderString = szPath;
      SpecialFolderString += L"\\";
      return TRUE;
    }
  }
  return FALSE;
}

/*********************************************
  This routine is a helper that builds a long path of a short (8+3) path:
  
  \param sShortPath  - short path to convert
  \param sLongPath   - string that receives the long path
  
  \returns a BOOL - Found or not
*********************************************/
// if the extension contains '~', replace it with '*'!!!!!!!!!!!!!!!
int CShortcut::ShortToLongPathName(const CString &sShortPath, CString &sLongPath)
{
  // Catch null pointers.
  if (sShortPath.IsEmpty())  return 0;

  // Check whether the input path is valid.
  if (0xffffffff == GetFileAttributes(sShortPath)) return 0;

  // Special characters.
  CString sep = L"\\";
  CString colon = L":";

  CString sDrive, sCutPath, sTmpShort;

  // Copy the short path into the work buffer and convert forward
  // slashes to backslashes.
  CString path = sShortPath;
  path.Replace(L"/", sep);

  // We need a marker for stepping through the path.
  int right = 0;

  // Parse the first bit of the path.
  if (path.GetLength() >= 2 && isalpha(path[0]) && colon == path[1]) {// Drive letter?
    if (2 == path.GetLength()) { // ’bare’ drive letter
      right = -1; // skip main block
    } else if (sep == path[2]) { // drive letter + backslash
      // FindFirstFile doesn’t like "X:\"
      if (3 == path.GetLength()) {
        right = -1; // skip main block
      }
    }
    else return 0; // parsing failure
  } else if (path.GetLength() >= 1 && sep == path[0]) {
    if (1 == path.GetLength()) { // ’bare’ backslash
      right = -1; // skip main block
    } else {
      if (sep == path[1]) { // is it UNC?
        // Find end of machine name
        right = path.Find(sep);

        if (right == -1)
          return 0;
        // Find end of share name
        right = path.Find(sep);
        if (right == 1)
          return 0;
      }
      ++right;
    }
  }

  // else FindFirstFile will handle relative paths
  // The data block for FindFirstFile.
  WIN32_FIND_DATA fd;

  // extract the Drive-name:
  sDrive = path.Left(3);
  path.Delete(0, 3);
  sTmpShort = sDrive;
  sLongPath = sDrive;

  // Main parse block - step through path.
  while (right != -1) {
    // Find next separator.
    right = path.Find(sep);
    if (right == 0) {
      path.Delete(0, 1);
      sTmpShort += sep;
      right = path.Find(sep);
    } else if (right == -1 && path.GetLength() <= 0) {
      return sLongPath.GetLength();
    }
    // Temporarily replace the separator with a null character so that
    // the path so far can be passed to FindFirstFile.
    if (right == -1)
      sCutPath = path.Left(path.GetLength());
    else
      sCutPath = path.Left(right);

    path.Delete(0, right);

    sTmpShort += sCutPath;

    // See what FindFirstFile makes of the path so far.
    HANDLE hf = FindFirstFile(sTmpShort, &fd);

    if (INVALID_HANDLE_VALUE == hf)
      return 0;
    FindClose(hf);

    // The file was found add it to the output path:
    if (right != -1)
      sLongPath += fd.cFileName + sep;
    else
      sLongPath += fd.cFileName;
  }

  // Check whether the output path is valid:
  if (0xffffffff == GetFileAttributes(sLongPath))  return 0;

  // return the number of characters in the long path:
  return sLongPath.GetLength();
}
