/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file PWSversion.h
//-----------------------------------------------------------------------------

class PWSversion
{
public:
  PWSversion();

  static PWSversion *GetInstance(); // singleton
  static void DeleteInstance();

  int GetMajor() {return m_nMajor;}
  int GetMinor() {return m_nMinor;}
  int GetBuild() {return m_nBuild;}
  int GetRevision() {return m_nRevision;}

  CString GetSpecialBuild() {return m_SpecialBuild;}
  CString GetAppVersion() {return m_AppVersion;}
  bool IsModified() {return m_bModified;}

private:
  static PWSversion *self; // singleton

  CString m_AppVersion, m_SpecialBuild;
  int m_nMajor, m_nMinor, m_nBuild, m_nRevision;
  bool m_bModified;
};
