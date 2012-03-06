/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __XMLPREFS_H
#define __XMLPREFS_H

#include "os/typedefs.h"
#include "PWSprefs.h"

#include "pugixml/pugixml.hpp"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CXMLprefs class
//
// This class wraps access to an XML file containing user preferences.
// Usage scenarios:
// 1. Load() followed by zero or more Get()s
// 2. Lock(), Load(), zero or more Set()s, zero or more
//    DeleteSetting()s, Store(), Unlock()
/////////////////////////////////////////////////////////////////////////////

class CXMLprefs
{
  // Construction & Destruction
public:
  CXMLprefs(const stringT &configFile)
  : m_pXMLDoc(NULL), m_csConfigFile(configFile), m_bIsLocked(false) {}

  ~CXMLprefs() { UnloadXML(); }

  // Implementation
  bool Load();
  bool Store();
  bool Lock(stringT &locker); // if fails, locker points to culprit
  void Unlock();

  int Get(const stringT &csBaseKeyName, const stringT &csValueName,
          int iDefaultValue);
  stringT Get(const stringT &csBaseKeyName, const stringT &csValueName,
              const stringT &csDefaultValue);

  int Set(const stringT &csBaseKeyName, const stringT &csValueName,
          int iValue);
  int Set(const stringT &csBaseKeyName, const stringT &csValueName,
          const stringT &csValue);

  std::vector<st_prefShortcut> GetShortcuts(const stringT &csBaseKeyName);
  int SetShortcuts(const stringT &csBaseKeyName, 
                   std::vector<st_prefShortcut> v_shortcuts);

  bool DeleteSetting(const stringT &csBaseKeyName, const stringT &csValueName);
  stringT getReason() const {return m_Reason;} // why something went wrong

  // For migration of a host/user from one current configuration file to another
  bool MigrateSettings(const stringT &sNewFilename, 
                       const stringT &sHost, const stringT &sUser);
  // Remove a host/user from current configuration file
  bool RemoveHostnameUsername(const stringT &sHost, const stringT &sUser,
                              bool &bNoMoreNodes);
  
  enum {XML_SUCCESS = 0,
        XML_LOAD_FAILED,
        XML_NODE_NOT_FOUND,
        XML_PUT_TEXT_FAILED,
        XML_SAVE_FAILED};

private:
  int SetPreference(const stringT &sPath, const stringT &sValue);

  pugi::xml_document *m_pXMLDoc;
  stringT m_csConfigFile;
  bool m_bIsLocked;

  // CreateXML - bLoad will skip creation of root element
  bool CreateXML(bool bLoad);
  void UnloadXML();
  stringT m_Reason; // why something bad happenned
};
#endif /* __XMLPREFS_H */
