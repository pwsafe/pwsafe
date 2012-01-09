/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWSversion.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PWSversion.h"
#include "version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

PWSversion *PWSversion::self = NULL;

PWSversion *PWSversion::GetInstance()
{
  if (self == NULL) {
    self = new PWSversion();
  }
  return self;
}

void PWSversion::DeleteInstance()
{
  delete self;
  self = NULL;
}

PWSversion::PWSversion()
  : m_nMajor(0), m_nMinor(0), m_nBuild(0), m_nRevision(0), m_bModified(false)
{
  CString csFileVersion = WIDEN(STRFILEVER);
  m_SpecialBuild = SPECIAL_BUILD;

  CString resToken;
  int curPos = 0, i = 0;
  
  // Tokenize the file version to get the values in order
  // Revision is either a number or a number with '+',
  // so we need to get it from the file version string
  // which is of the form "MM, NN, BB, rev"
  resToken = csFileVersion.Tokenize(L",", curPos);
  while (resToken != L"" && curPos != -1) {
    resToken.Trim();
    if (resToken.IsEmpty())
      resToken = L"0";
    
    const int len = resToken.GetLength();
    switch (i) {
      case 0:
        m_nMajor = _wtoi(resToken);
        break;
      case 1:
        m_nMinor = _wtoi(resToken);
        break;
      case 2:
        m_nBuild = _wtoi(resToken);
        break;
      case 3:
        if (len > 0 && resToken.Right(1) == L"+") {
          m_bModified = true;
          resToken = resToken.Left(len - 1);
        }
        m_nRevision = _wtoi(resToken);
        break;
      default:
        ASSERT(0);
    }
    i++;
    resToken = csFileVersion.Tokenize(L",", curPos);
  };
}
