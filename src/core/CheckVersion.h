/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __CHECK_VERSION_H
#define __CHECK_VERSION_H

/// \file CheckVersion.h
/**
 * A simple class to help determine if the current version is the latest,
 * based on the xml file retrieved from the project's web site.
 * Currently we don't retrieve the xml, but accept it as a parameter.
 *
 * The retrieval should really be moved to os library, and called from here.
 */
//-----------------------------------------------------------------------------

#include "os/typedefs.h" // for stringT

class CheckVersion {
 public:
 CheckVersion(int nMajor, int nMinor, int nBuild)
   : m_nMajor(nMajor), m_nMinor(nMinor), m_nBuild(nBuild) {}
  
  enum class CheckStatus {UP2DATE, NEWER_AVAILABLE, CANT_CONNECT, CANT_READ};
  CheckStatus CheckLatestVersion(const stringT &xml, stringT &latest) const;
 private:
  int m_nMajor, m_nMinor, m_nBuild;
};

#endif /* __CHECK_VERSION_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
