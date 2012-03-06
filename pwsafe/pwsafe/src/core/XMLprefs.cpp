/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// XMLprefs.cpp : implementation file
//

#include "XMLprefs.h"
#include "PWSprefs.h"
#include "core.h"
#include "StringXStream.h"
#include "Util.h"

#include "pugixml/pugixml.hpp"

#include "os/typedefs.h"
#include "os/sleep.h"
#include "os/debug.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CXMLprefs

bool CXMLprefs::Lock(stringT &locker)
{
  locker = _T("");
  int tries = 10;
  do {
    m_bIsLocked = PWSprefs::LockCFGFile(m_csConfigFile, locker);
    if (!m_bIsLocked) {
      pws_os::sleep_ms(200);
    }
  } while (!m_bIsLocked && --tries > 0);
  return m_bIsLocked;
}

void CXMLprefs::Unlock()
{
  PWSprefs::UnlockCFGFile(m_csConfigFile);
  m_bIsLocked = false;
}

bool CXMLprefs::CreateXML(bool bLoad)
{
  // Call with bLoad set when about to Load, else
  // this also adds a toplevel root element
  ASSERT(m_pXMLDoc == NULL);
  m_pXMLDoc = new pugi::xml_document;
  
  if (!bLoad && m_pXMLDoc != NULL) {
    // Insert
    // <?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
    // <Pwsafe_Settings>
    // </ Pwsafe_Settings>
    pugi::xml_node decl = m_pXMLDoc->prepend_child(pugi::node_declaration);
    if (decl == NULL)
      return false;

    decl.append_attribute(_T("version")).set_value(_T("1.0"));
    decl.append_attribute(_T("encoding")).set_value(_T("utf-8"));
    decl.append_attribute(_T("standalone")).set_value(_T("yes"));
    
    pugi::xml_node node = m_pXMLDoc->append_child(_T("Pwsafe_Settings"));
    return node != NULL;
  } else
    return m_pXMLDoc != NULL;
}

bool CXMLprefs::Load()
{
  // Already loaded?
  if (m_pXMLDoc != NULL)
    return true;

  bool alreadyLocked = m_bIsLocked;
  if (!alreadyLocked) {
    stringT locker;
    if (!Lock(locker)) {
      LoadAString(m_Reason, IDSC_XMLLOCK_CFG_FAILED);
      m_Reason += _T("\n  "); m_Reason += locker;
      return false;
    }
  }

  if (!CreateXML(true)) {
    LoadAString(m_Reason, IDSC_XMLCREATE_CFG_FAILED);
    return false;
  }

  pugi::xml_parse_result result = m_pXMLDoc->load_file(m_csConfigFile.c_str());

  if (!result) {
    // An XML load error occurred so display the reason
    // Note: "result.description()" returns char* even in Unicode builds.
    stringT sErrorDesc;
#ifdef UNICODE
    sErrorDesc = pugi::as_wide(result.description());
#else
    sErrorDesc = result.description();
#endif
    Format(m_Reason, IDSC_XMLFILEERROR,
           sErrorDesc.c_str(), m_csConfigFile.c_str(), result.offset);
    delete m_pXMLDoc;
    m_pXMLDoc = NULL;
    return false;
  } // load failed

  // If we locked it, we should unlock it...
  if (!alreadyLocked)
    Unlock();
  
  // If OK - delete any error message
  m_Reason.clear();
  return true;
}

bool CXMLprefs::Store()
{
  bool retval = false;
  bool alreadyLocked = m_bIsLocked;
  pugi::xml_node decl;

  if (!alreadyLocked) {
    stringT locker;
    if (!Lock(locker)) {
      LoadAString(m_Reason, IDSC_XMLLOCK_CFG_FAILED);
      m_Reason += _T("\n  "); m_Reason += locker;
      return false;
    }
  }

  // Although technically possible, it doesn't make sense
  // to create a toplevel document here, since we'd then
  // be saving an empty document.
  ASSERT(m_pXMLDoc != NULL);
  if (m_pXMLDoc == NULL) {
    LoadAString(m_Reason, IDSC_XMLCREATE_CFG_FAILED);
    retval = false;
    goto exit;
  }

  decl = m_pXMLDoc->prepend_child(pugi::node_declaration);
  if (decl == NULL)
    goto exit;

  decl.append_attribute(_T("version")).set_value(_T("1.0"));
  decl.append_attribute(_T("encoding")).set_value(_T("utf-8"));
  decl.append_attribute(_T("standalone")).set_value(_T("yes"));

  retval = m_pXMLDoc->save_file(m_csConfigFile.c_str(), _T("  "),
                         pugi::format_default | pugi::format_write_bom,
                         pugi::encoding_utf8);

exit:
  // If we locked it, we should unlock it...
  if (!alreadyLocked)
    Unlock();
  
  // If OK - delete any error message
  if (retval)
    m_Reason.clear();

  return retval;
}

int CXMLprefs::SetPreference(const stringT &sPath, const stringT &sValue)
{
  // Find the node specified by the path, creating it if it does not already exist
  // and add the requested value.

  // Notes:
  // This routine only adds plain character data to the node, see comments at the end.
  // If the node already has plain character data, it is replaced.
  // If the node exists multiple times with the same path, only the first is altered -
  //   see description of function "first_element_by_path" in the pugixml manual.

  int iRetVal = XML_SUCCESS;
  
  // First see if the node already exists
  pugi::xml_node node = m_pXMLDoc->first_element_by_path(sPath.c_str(), _T('\\'));
  
  if (node == NULL) {
    // Not there - let's build it
    // Split up path and then add all nodes in the path (if they don't exist)
    // Start at the top
    node = m_pXMLDoc->root();
  
    stringT::size_type pos, lastPos(0);

    // Find first "non-delimiter".
    pos = sPath.find_first_of(_T('\\'), lastPos);

    // Get all nodes in the path, if they exist fine, if not, create them
    while (pos != stringT::npos || lastPos != stringT::npos) {
      // Retrieve next node name from path
      stringT snode = sPath.substr(lastPos, pos - lastPos);
      
      // Try to get it
      pugi::xml_node child = node.child(snode.c_str());

      // If not there, add it otherwise use it for the next iteration
      node = (child == NULL) ? node.append_child(snode.c_str()) : child;
    
      // Skip delimiter and find next "non-delimiter"
      lastPos = sPath.find_first_not_of(_T('\\'), pos);
      pos = sPath.find_first_of(_T('\\'), lastPos);
    }
  }

  //  ***** VERY IMPORTANT *****
  // Note, as documented in the manual under "Document object model/Tree Structure",
  // nodes can be of various types.  Element nodes found above, do not have a value.
  // To add a value to an element node, one has to add a child node of type
  // 'node_pcdata' (plain character data node) or 'node_cdata' (character data node),
  // the latter using <![CDATA[[...]]> to encapsulate the data.

  // If the node has data in its first pcdata child use it, otherwise add a pcdata child
  pugi::xml_node prefnode = (node.first_child().type() == pugi::node_pcdata) ?
     node.first_child() : node.append_child(pugi::node_pcdata);

  if (!prefnode.set_value(sValue.c_str()))
     iRetVal = XML_PUT_TEXT_FAILED;

  return iRetVal;
}

// Get a int value
int CXMLprefs::Get(const stringT &csBaseKeyName, const stringT &csValueName, 
                   int iDefaultValue)
{
  /*
    Since XML is text based and we have no schema, just convert to a string and
    call the Get(String) method.
  */
  int iRetVal = iDefaultValue;
  ostringstreamT os;
  os << iDefaultValue;
  istringstreamT is(Get(csBaseKeyName, csValueName, os.str()));
  is >> iRetVal;

  return iRetVal;
}

// Get a string value
stringT CXMLprefs::Get(const stringT &csBaseKeyName, const stringT &csValueName, 
                       const stringT &csDefaultValue)
{
  ASSERT(m_pXMLDoc != NULL); // shouldn't be called if not loaded
  if (m_pXMLDoc == NULL) // just in case
    return csDefaultValue;

  stringT csValue = csDefaultValue;

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  pugi::xml_node preference = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  if (preference != NULL) {
    const TCHAR *val = preference.child_value(); 
    csValue = (val != NULL) ? val : _T("");
  }

  return csValue;
}

// Set a int value
int CXMLprefs::Set(const stringT &csBaseKeyName, const stringT &csValueName,
                   int iValue)
{
  /*
    Since XML is text based and we have no schema, just convert to a string and
    call the SetSettingString method.
  */
  int iRetVal = 0;
  stringT csValue = _T("");

  Format(csValue, _T("%d"), iValue);

  iRetVal = Set(csBaseKeyName, csValueName, csValue);

  return iRetVal;
}

// Set a string value
int CXMLprefs::Set(const stringT &csBaseKeyName, const stringT &csValueName, 
                   const stringT &csValue)
{
  // m_pXMLDoc may be NULL if Load() not called before Set,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return false;

  int iRetVal = XML_SUCCESS;

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  iRetVal = SetPreference(csKeyName, csValue);

  return iRetVal;
}

// Delete a key or chain of keys
bool CXMLprefs::DeleteSetting(const stringT &csBaseKeyName, const stringT &csValueName)
{
  // m_pXMLDoc may be NULL if Load() not called before DeleteSetting,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return false;

  bool bRetVal = false;

  stringT csKeyName(csBaseKeyName);

  if (!csValueName.empty()) {
    csKeyName += _T("\\");
    csKeyName += csValueName;
  }

  pugi::xml_node base_node = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));
  if (base_node == NULL)
    return false;

  pugi::xml_node node_parent = base_node.parent();
  if (node_parent != NULL) {
    size_t last_slash = csKeyName.find_last_of(_T("\\"));
    stringT sKey = csKeyName.substr(last_slash + 1);
    bRetVal = node_parent.remove_child(sKey.c_str());
  }

  return bRetVal;
}

void CXMLprefs::UnloadXML()
{
  delete m_pXMLDoc;
  m_pXMLDoc = NULL;
}

std::vector<st_prefShortcut> CXMLprefs::GetShortcuts(const stringT &csBaseKeyName)
{
  std::vector<st_prefShortcut> v_Shortcuts;
  ASSERT(m_pXMLDoc != NULL); // shouldn't be called if not loaded

  if (m_pXMLDoc == NULL)     // just in case
    return v_Shortcuts;

  v_Shortcuts.clear();
  
  stringT csKeyName(csBaseKeyName);

  // Get shortcuts
  pugi::xml_node all_shortcuts = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  if (all_shortcuts == NULL)
    return v_Shortcuts;

  pugi::xml_node shortcut;
  // Now go through all shortcuts
  for (shortcut = all_shortcuts.first_child(); shortcut; 
       shortcut = shortcut.next_sibling()) {
    st_prefShortcut cur;
    int itemp;

    cur.cModifier = 0;
    cur.id = shortcut.attribute(_T("id")).as_uint();
    itemp = shortcut.attribute(_T("Key")).as_int();
    cur.siVirtKey = static_cast<short int>(itemp);
    itemp = shortcut.attribute(_T("Ctrl")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_CONTROL;
    itemp = shortcut.attribute(_T("Alt")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_ALT;
    itemp = shortcut.attribute(_T("Shift")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_SHIFT;

    // wxWidgets only - set values so not lost in XML file 
    // but not use them in Windows MFC - they are never tested in MFC code
    // when creating the necessary hotkeys/shortcuts
    itemp = shortcut.attribute(_T("Win")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_WIN;
    itemp = shortcut.attribute(_T("Meta")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_META;
    itemp = shortcut.attribute(_T("Cmd")).as_int();
    cur.cModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_CMD;

    v_Shortcuts.push_back(cur);
  }
  return v_Shortcuts;
}

int CXMLprefs::SetShortcuts(const stringT &csBaseKeyName, 
                            std::vector<st_prefShortcut> v_shortcuts)
{
  // m_pXMLDoc may be NULL if Load() not called before Set,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return XML_LOAD_FAILED;

  int iRetVal = XML_SUCCESS;

  // csBaseKeyName is "Pwsafe_Settings\host\user\Shortcuts"
  // Even if this does not exist, "Pwsafe_Settings\host\user" must so we can just go
  // up one level to create it (see SetPreference for when we don't know how many levels
  // we have to go up before we find an existing node
  stringT csKeyName(csBaseKeyName);

  // Get all shortcuts
  pugi::xml_node all_shortcuts = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  // Not there - go up one node and try to add it.
  if (all_shortcuts == NULL) {
    // Add node - go up a level in path
    size_t last_slash = csKeyName.find_last_of(_T("\\"));
    stringT sPath = csKeyName.substr(0, last_slash);
    stringT sKey = csKeyName.substr(last_slash + 1);
    all_shortcuts = m_pXMLDoc->first_element_by_path(sPath.c_str(), _T('\\'));
    
    ASSERT(all_shortcuts != NULL);
    if (all_shortcuts != NULL) {
      all_shortcuts = all_shortcuts.append_child(sKey.c_str());
    }
  }

  // If still NULL - give up!!!
  if (all_shortcuts == NULL)
    return XML_PUT_TEXT_FAILED;

  for (size_t i = 0; i < v_shortcuts.size(); i++) { 
    pugi::xml_node shortcut = all_shortcuts.append_child(_T("Shortcut"));
    
    // If we can't add this - give up!
    if (shortcut == NULL)
      return XML_PUT_TEXT_FAILED;
      
    shortcut.set_value(_T(""));
    
    pugi::xml_attribute attrib;

    attrib = shortcut.append_attribute(_T("id"));
    attrib.set_value(v_shortcuts[i].id);
    attrib = shortcut.append_attribute(_T("Ctrl"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_CONTROL) ==
                       PWS_HOTKEYF_CONTROL ? 1 : 0);
    attrib = shortcut.append_attribute(_T("Alt"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_ALT) ==
                       PWS_HOTKEYF_ALT ? 1 : 0);
    attrib = shortcut.append_attribute(_T("Shift"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_SHIFT) ==
                       PWS_HOTKEYF_SHIFT ? 1 : 0);
    // wxWidgets only - set values but do not use in Windows MFC
    attrib = shortcut.append_attribute(_T("Meta"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_META) ==
                       PWS_HOTKEYF_META ? 1 : 0);
    attrib = shortcut.append_attribute(_T("Win"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_WIN) ==
                       PWS_HOTKEYF_WIN ? 1 : 0);
    attrib = shortcut.append_attribute(_T("Cmd"));
    attrib.set_value((v_shortcuts[i].cModifier & PWS_HOTKEYF_CMD) ==
                       PWS_HOTKEYF_CMD ? 1 : 0);
    attrib = shortcut.append_attribute(_T("Key"));
    attrib.set_value(v_shortcuts[i].siVirtKey);
  }
  return iRetVal;
}

bool CXMLprefs::MigrateSettings(const stringT &sNewFilename, 
                                const stringT &sHost, const stringT &sUser)
{
  // Validate parameters
  if (sNewFilename.empty() || sHost.empty() || sUser.empty())
    return false;

  if (m_pXMLDoc == NULL)
    return false;

  pugi::xml_node root_node = m_pXMLDoc->first_element_by_path(_T("Pwsafe_Settings"), _T('\\'));
  if (root_node == NULL)
    return false;

  // Delete all hosts - except the one supplied
  for (pugi::xml_node host = root_node.first_child(); host;
       host = root_node.next_sibling()) {

    if (host.name() != sHost.c_str()) {
      root_node.remove_child(host);
    } else {
      // Same host - now delete all other users on that host
      for (pugi::xml_node user = host.first_child(); user;
           user = host.next_sibling()) {
         if (user.name() != sUser.c_str()) {
           host.remove_child(user);
         }
      }
    }
  }

  // Save just host/user in new file
  pugi::xml_node decl = m_pXMLDoc->prepend_child(pugi::node_declaration);
  if (decl == NULL)
    return false;

  decl.append_attribute(_T("version")).set_value(_T("1.0"));
  decl.append_attribute(_T("encoding")).set_value(_T("utf-8"));
  decl.append_attribute(_T("standalone")).set_value(_T("yes"));

  bool result = m_pXMLDoc->save_file(sNewFilename.c_str(), _T("  "),
                         pugi::format_default | pugi::format_write_bom,
                         pugi::encoding_utf8);

  if (!result) {
    return false;
  }
  return true;
}

bool CXMLprefs::RemoveHostnameUsername(const stringT &sHost, const stringT &sUser,
                                       bool &bNoMoreNodes)
{
  // If all OK, after removal of supplied hostname/username, then 
  // bNoMoreNodes indicates if there still remain more nodes (hostname/username)
  // in the configuration file
  bNoMoreNodes = false;

  // Validate parameters
  if (sHost.empty() || sUser.empty())
    return false;

  if (m_pXMLDoc == NULL)
    return false;

  stringT sPath = stringT(_T("Pwsafe_Settings//")) + sHost;
  pugi::xml_node node = m_pXMLDoc->first_element_by_path(sPath.c_str(), _T('\\'));
  
  if (node == NULL)
    return false;
  
  if (!node.remove_child(sUser.c_str()))
    return false;

  // Check if more children
  bNoMoreNodes = node.first_child() == NULL;
  return true;
}
