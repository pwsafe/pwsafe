/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSVERSION_H
#define __PWSVERSION_H

/// \file PWSversion.h
//-----------------------------------------------------------------------------

#include "../os/typedefs.h" // for stringT

class PWSversion
{
public:
  static PWSversion *GetInstance(); // singleton
  static void DeleteInstance();

  int GetMajor() const {return m_nMajor;}
  int GetMinor() const {return m_nMinor;}
  int GetBuild() const {return m_nBuild;}
  const stringT &GetRevision() const {return m_Revision;}

  const stringT &GetSpecialBuild() const {return m_SpecialBuild;}
  const stringT &GetBuiltOn() const {return m_builtOn;}
  bool IsModified() const {return m_bModified;}

  // Formats "major.minor" or, if build != 0, "major.minor.build" - no leading zeroes.
  static stringT FormatVersionString(int major, int minor, int build);
  stringT FormatVersionString() const {return FormatVersionString(m_nMajor, m_nMinor, m_nBuild);}

private:
  PWSversion();
  static PWSversion *self; // singleton

  int m_nMajor, m_nMinor, m_nBuild;
  stringT m_SpecialBuild;
  stringT m_Revision;
  stringT m_builtOn;
  bool m_bModified;
};

#endif /* __PWSVERSION_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
