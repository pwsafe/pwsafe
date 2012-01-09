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

  inline int GetMajor() {return m_nMajor;}
  inline int GetMinor() {return m_nMinor;}
  inline int GetBuild() {return m_nBuild;}
  inline int GetRevision() {return m_nRevision;}

  inline CString GetSpecialBuild() {return m_SpecialBuild;}
  inline CString GetAppVersion() {return m_AppVersion;}
  inline bool IsModified() {return m_bModified;}

private:
  static PWSversion *self; // singleton

  CString m_AppVersion, m_SpecialBuild;
  int m_nMajor;
  int m_nMinor;
  int m_nBuild;
  int m_nRevision;
  bool m_bModified;
};
