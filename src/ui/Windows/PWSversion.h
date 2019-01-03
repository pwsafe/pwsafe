/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

  int GetMajor() const {return m_nMajor;}
  int GetMinor() const {return m_nMinor;}
  int GetBuild() const {return m_nBuild;}
  const CString &GetRevision() const {return m_Revision;}

  const CString &GetSpecialBuild() const {return m_SpecialBuild;}
  const CString &GetAppVersion() const {return m_AppVersion;}
  const CString &GetBuiltOn() const {return m_builtOn;}
  bool IsModified() {return m_bModified;}

private:
  static PWSversion *self; // singleton

  CString m_AppVersion, m_SpecialBuild;
  int m_nMajor, m_nMinor, m_nBuild;
  CString m_Revision;
  CString m_builtOn;
  bool m_bModified;
};
