/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file CheckVersion.cpp
//-----------------------------------------------------------------------------

/*
 * The latest version information is in
 * https://pwsafe.org/latest.xml
 *
 * And is of the form:
 * <VersionInfo>
 *  <Product name="PasswordSafe" variant="PC" major="3" minor="28" build="0" rev="4786" />
 *  <Product name="PasswordSafe" variant="PPc" major="1" minor="9" build="2" rev="100" /> <!-- obsolete -->
 *  <Product name="PasswordSafe" variant="U3" major="3" minor="28" build="0" rev="4786" /> <!-- obsolete -->
 *  <Product name="PasswordSafe" variant="Linux" major="0" minor="7" build="0" rev="4527:4532" />
 * </VersionInfo>
 *
 * Note: The "rev" is the git commit number. Displayed, not used.
 */

#include "CheckVersion.h"
#include "SysInfo.h"
#include "StringX.h" // for Format()

#include "pugixml/pugixml.hpp"

#include "os/pws_tchar.h"  // For Linux build not finding _tcslen!

static bool SafeCompare(const TCHAR *v1, const TCHAR *v2)
{
  return (v1 != nullptr && v2 != nullptr && stringT(v1) == v2);
}

CheckVersion::CheckStatus
CheckVersion::CheckLatestVersion(const stringT &xml, stringT &latest) const
{
  // Parse the file we just retrieved
  pugi::xml_document doc; 
  pugi::xml_parse_result result = doc.load(xml.c_str());
  if (!result)
    return CheckStatus::CANT_READ;

  pugi::xml_node Root = doc.first_child();

  if (!Root || !SafeCompare(Root.name(), _T("VersionInfo")))
    return CheckStatus::CANT_READ;

  for (pugi::xml_node_iterator it = Root.begin(); it != Root.end(); ++it) {
    if (SafeCompare(it->name(), _T("Product"))) {
      const TCHAR *prodName = it->attribute(_T("name")).value();
      if (_tcslen(prodName) == 0)
        continue;

      if (SafeCompare(prodName, _T("PasswordSafe"))) {
        const TCHAR *pVariant = it->attribute(_T("variant")).value();
        if (_tcslen(pVariant) == 0)
          continue;

        const stringT variant(pVariant);
        // Determine which variant is relevant for us
        if ((SysInfo::IsUnderU3()  && variant == _T("U3"))    ||
            (SysInfo::IsLinux()    && variant == _T("Linux")) ||
            (!SysInfo::IsUnderU3() && !SysInfo::IsLinux() &&
             variant == _T("PC"))) {
            const int xmajor = it->attribute(_T("major")).as_int();
            const int xminor = it->attribute(_T("minor")).as_int();
            const int xbuild = it->attribute(_T("build")).as_int();
            const TCHAR *xrevision = it->attribute(_T("rev")).as_string(_T("0"));
            // Not using svn rev info - too volatile
            if ((xmajor > m_nMajor) ||
                (xmajor == m_nMajor && xminor > m_nMinor) ||
                (xmajor == m_nMajor && xminor == m_nMinor &&
                 xbuild > m_nBuild)) {
                if (xbuild == 0) { // hide build # if zero (formal release)
                  Format(latest, L"PasswordSafe V%d.%02d (%s)",
                         xmajor, xminor, xrevision);
                } else {
                  Format(latest, L"PasswordSafe V%d.%02d.%02d (%s)",
                         xmajor, xminor, xbuild, xrevision);
                }
                return CheckStatus::NEWER_AVAILABLE;
            }
            return CheckStatus::UP2DATE;
        } // handled our variant
      } // Product name == PasswordSafe
    } // Product element
  } // IterateChildren
  return CheckStatus::CANT_READ;
}
