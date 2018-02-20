/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "os/pws_tchar.h"
#include "os/sleep.h"
#include "os/debug.h"
#include "os/KeySend.h"

#include <vector>
#include <algorithm>

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
  // this just adds the XML declaration and a toplevel root element
  ASSERT(m_pXMLDoc == nullptr);
  m_pXMLDoc = new pugi::xml_document;
  
  if (!bLoad && m_pXMLDoc != nullptr) {
    // Insert
    // <Pwsafe_Settings>
    // </ Pwsafe_Settings>
    
    pugi::xml_node node = m_pXMLDoc->append_child(_T("Pwsafe_Settings"));
    return node != nullptr;
  } else
    return m_pXMLDoc != nullptr;
}

bool CXMLprefs::XML_Load()
{
  // Already loaded?
  if (m_pXMLDoc != nullptr)
    return true;

  bool alreadyLocked = m_bIsLocked;
  if (!alreadyLocked) {
    stringT locker;
    if (!Lock(locker)) {
      LoadAString(m_Reason, IDSC_XMLLOCK_CFG_FAILED);
      m_Reason += _T("\n"); m_Reason += locker;
      // Show filename for troubleshooting
      m_Reason += _T("\n"); m_Reason += m_csConfigFile;
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
    sErrorDesc = pugi::as_wide(result.description());
    Format(m_Reason, IDSC_XMLFILEERROR,
           sErrorDesc.c_str(), m_csConfigFile.c_str(), result.offset);
    delete m_pXMLDoc;
    m_pXMLDoc = nullptr;
    return false;
  } // load failed

  // If we locked it, we should unlock it...
  if (!alreadyLocked)
    Unlock();
  
  // If OK - delete any error message
  m_Reason.clear();
  return true;
}

static void SortPreferences(pugi::xml_node parent)
{
  // Note: pugi::xml_node is really a pointer and so this routine
  // doesn't need its argument defined as a reference
  // Sort the application preferences of this host/user (case insensitive)
  std::vector<pugi::xml_node> children(parent.begin(), parent.end());

  std::sort(children.begin(), children.end(),
    [](pugi::xml_node l, pugi::xml_node r) {
          return _tcsicmp(l.name(), r.name()) < 0;
        });

  for (pugi::xml_node n : children) {
    parent.append_move(n);
  }
}

bool CXMLprefs::XML_Store(const stringT &csBaseKeyName)
{
  bool retval = false;
  bool alreadyLocked = m_bIsLocked;
  pugi::xml_node decl;
  pugi::xml_node nodePrefs;

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
  ASSERT(m_pXMLDoc != nullptr);
  if (m_pXMLDoc == nullptr) {
    LoadAString(m_Reason, IDSC_XMLCREATE_CFG_FAILED);
    retval = false;
    goto exit;
  }

  decl = m_pXMLDoc->prepend_child(pugi::node_declaration);
  if (decl == nullptr)
    goto exit;

  decl.append_attribute(_T("version")) = _T("1.0");
  decl.append_attribute(_T("encoding")) = _T("utf-8");
  decl.append_attribute(_T("standalone")) = _T("yes");

  nodePrefs = m_pXMLDoc->first_element_by_path(csBaseKeyName.c_str(), _T('\\'));
  if (nodePrefs != nullptr)
    SortPreferences(nodePrefs);

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

int CXMLprefs::SetPreference(const stringT &sPath, const stringT &sValue,
                             std::vector<st_prefAttribs> *pvprefAttribs)
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
  
  if (node == nullptr) {
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
      node = (child == nullptr) ? node.append_child(snode.c_str()) : child;
    
      // Skip delimiter and find next "non-delimiter"
      lastPos = sPath.find_first_not_of(_T('\\'), pos);
      pos = sPath.find_first_of(_T('\\'), lastPos);
    }
  }

  //  ***** VERY IMPORTANT *****
  // Note, as documented in the pugi manual under "Document object model/Tree Structure",
  // nodes can be of various types.  Element nodes found above, do not have a value.
  // To add a value to an element node, one has to add a child node of type
  // 'node_pcdata' (plain character data node) or 'node_cdata' (character data node),
  // the latter using <![CDATA[[...]]> to encapsulate the data.

  // If the node has data in its first pcdata child use it, otherwise add a pcdata child
  pugi::xml_node prefnode = (node.first_child().type() == pugi::node_pcdata) ?
     node.first_child() : node.append_child(pugi::node_pcdata);

  if (!prefnode.set_value(sValue.c_str())) {
    iRetVal = XML_PUT_TEXT_FAILED;
  } else {
    pugi::xml_node parentnode = prefnode.parent();
    // Delete all existing attributes
    while (pugi::xml_attribute attrib = parentnode.first_attribute()) {
      parentnode.remove_attribute(attrib);
    }

    pugi::xml_attribute attrib;
    if (pvprefAttribs != nullptr && !pvprefAttribs->empty()) {
      // Add attributes
      for (size_t i = 0; i < pvprefAttribs->size(); i++) {
        st_prefAttribs &st_pa = (*pvprefAttribs)[i];
        attrib = parentnode.append_attribute(st_pa.name.c_str());
        attrib = st_pa.value.c_str();
      }
    }
  }

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

  // These Integer preferences may have attributes
  std::vector<stringT> vsWithAttributes = { L"HotKey" };

  int iRetVal = iDefaultValue;
  ostringstreamT os;
  os << iDefaultValue;
  istringstreamT is(Get(csBaseKeyName, csValueName, os.str()));
  is >> iRetVal;

  auto iter = std::find(vsWithAttributes.begin(), vsWithAttributes.end(), csValueName);
  if (iter != vsWithAttributes.end()) {
    iRetVal = GetWithAttributes(csBaseKeyName, csValueName, iRetVal);
  }

  return iRetVal;
}

int CXMLprefs::GetWithAttributes(const stringT &csBaseKeyName, const stringT &csValueName,
                                 int iValue)
{
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;
  int iRevVal(iValue);

  // Get preference
  pugi::xml_node preference = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  // Not there - return default
  if (preference == nullptr) {
    return iRevVal;
  }

  if (csValueName == _T("HotKey")) {
    // This is needed as HotKey was originally used MFC values for the modifier
    // but really needs to use our PWS modifier that can be converted to the
    // correct format as required - e.g. Windows, MFC, Linux (when implemented) etc.
    // The new preference still has an integer value that is based on MFC Hotkey modifiers
    // but ignores this and uses the attributes instead.
    WORD wVirtualKeyCode = iValue & 0xff;
    WORD wHKModifiers = iValue >> 16;
    WORD wPWSModifiers(0);

    if (iValue != 0) {
      if (preference.attribute(_T("Key")) == nullptr) {
        // Attribute NOT defined and so this is the OLD MFC value
        // Convert to PWS format to use throughout comnverting as needed
        if (wHKModifiers & HOTKEYF_ALT)
          wPWSModifiers |= PWS_HOTKEYF_ALT;
        if (wHKModifiers & HOTKEYF_CONTROL)
          wPWSModifiers |= PWS_HOTKEYF_CONTROL;
        if (wHKModifiers & HOTKEYF_SHIFT)
          wPWSModifiers |= PWS_HOTKEYF_SHIFT;
        if (wHKModifiers & HOTKEYF_EXT)
          wPWSModifiers |= PWS_HOTKEYF_EXT;

        iRevVal = (wPWSModifiers << 16) + wVirtualKeyCode;
        return iRevVal;
      }

      // New version - build HotKey value from attributes
      int itemp;
      itemp = preference.attribute(_T("Key")).as_int();
      wVirtualKeyCode = static_cast<short int>(itemp);

      itemp = preference.attribute(_T("Ctrl")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_CONTROL;
      itemp = preference.attribute(_T("Alt")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_ALT;
      itemp = preference.attribute(_T("Shift")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_SHIFT;
      itemp = preference.attribute(_T("Ext")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_EXT;

      // wxWidgets only - set values so not lost in XML file 
      // but not used in Windows MFC - they are never tested in MFC code
      // when creating the necessary hotkeys/shortcuts
      itemp = preference.attribute(_T("Win")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_WIN;
      itemp = preference.attribute(_T("Meta")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_META;
      itemp = preference.attribute(_T("Cmd")).as_int();
      wPWSModifiers |= itemp == 0 ? 0 : PWS_HOTKEYF_CMD;

      iRevVal = (wPWSModifiers << 16) + wVirtualKeyCode;
    }
  }

  return iRevVal;
}

// Get a string value
stringT CXMLprefs::Get(const stringT &csBaseKeyName, const stringT &csValueName, 
                       const stringT &csDefaultValue)
{
  ASSERT(m_pXMLDoc != nullptr); // shouldn't be called if not loaded
  if (m_pXMLDoc == nullptr) // just in case
    return csDefaultValue;

  stringT csValue = csDefaultValue;

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  pugi::xml_node preference = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  if (preference != nullptr) {
    const TCHAR *val = preference.child_value(); 
    csValue = (val != nullptr) ? val : _T("");
  }

  return csValue;
}

// Set a int value
int CXMLprefs::Set(const stringT &csBaseKeyName, const stringT &csValueName,
                   int iValue)
{
  /*
    Since XML is text based and we have no schema, just convert to a string and
    call the Set String method.
  */

  // These Integer preferences need attributes
  std::vector<stringT> vsWithAttributes = { L"HotKey" };

  int iRetVal(0);

  auto iter = std::find(vsWithAttributes.begin(), vsWithAttributes.end(), csValueName);
  if (iter != vsWithAttributes.end()) {
    iRetVal = SetWithAttributes(csBaseKeyName, csValueName, iValue);
  } else {
    stringT csValue = L"";

    Format(csValue, L"%d", iValue);

    iRetVal = Set(csBaseKeyName, csValueName, csValue);
  }

  return iRetVal;
}

// Set a string value
int CXMLprefs::Set(const stringT &csBaseKeyName, const stringT &csValueName, 
                   const stringT &csValue)
{
  // m_pXMLDoc may be nullptr if Load() not called before Set,
  // or if called & failed

  if (m_pXMLDoc == nullptr && !CreateXML(false))
    return false;

  int iRetVal(XML_SUCCESS);

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  iRetVal = SetPreference(csKeyName, csValue);

  return iRetVal;
}

// Set a integer value with attributes
int CXMLprefs::SetWithAttributes(const stringT &csBaseKeyName, const stringT &csValueName,
                                 const int &iValue)
{
  // m_pXMLDoc may be nullptr if Load() not called before Set,
  // or if called & failed

  if (m_pXMLDoc == nullptr && !CreateXML(false))
    return false;

  int iRetVal(XML_SUCCESS);

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;
  stringT csValue = L"";
  Format(csValue, L"%d", iValue);

  std::vector<st_prefAttribs> vprefAttribs;

  st_prefAttribs st_pa;

  if (csValueName == _T("HotKey")) {
    // New HotKey preference puts values in the attributes based on PWS modifiers
    // and ignore the actual integer value.
    // However, the actual value used in versions prior to this is from the value
    // which is based on MFC modifier values.  Keep this for backward compatibility 
    // with prior versions.
    WORD wVirtualKeyCode = iValue & 0xff;
    WORD wPWSModifiers = iValue >> 16;

    if (iValue != 0) {
      stringT sKeyName = CKeySend::GetKeyName(wVirtualKeyCode,
                              (wPWSModifiers & PWS_HOTKEYF_EXT) == PWS_HOTKEYF_EXT);

      if (!sKeyName.empty()) {
        if (wPWSModifiers & PWS_HOTKEYF_CONTROL) {
          st_pa.name = _T("Ctrl");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        if (wPWSModifiers & PWS_HOTKEYF_ALT) {
          st_pa.name = _T("Alt");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        if (wPWSModifiers & PWS_HOTKEYF_SHIFT) {
          st_pa.name = _T("Shift");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        if (wPWSModifiers & PWS_HOTKEYF_EXT) {
          st_pa.name = _T("Ext");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        // wxWidgets only - set values but do not use in Windows MFC
        if (wPWSModifiers & PWS_HOTKEYF_META) {
          st_pa.name = _T("Meta");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        if (wPWSModifiers & PWS_HOTKEYF_WIN) {
          st_pa.name = _T("Win");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        if (wPWSModifiers & PWS_HOTKEYF_CMD) {
          st_pa.name = _T("Cmd");
          st_pa.value = _T("1");
          vprefAttribs.push_back(st_pa);
        }

        st_pa.name = _T("Key");
        stringT sVK;
        Format(sVK, _T("%d"), wVirtualKeyCode);
        st_pa.value = sVK;
        vprefAttribs.push_back(st_pa);

        st_pa.name = _T("KeyName");
        st_pa.value = sKeyName;
        vprefAttribs.push_back(st_pa);

        // Rebuild older value
        WORD wHKModifiers(0);

        if (wPWSModifiers & PWS_HOTKEYF_ALT)
          wHKModifiers |= HOTKEYF_ALT;
        if (wPWSModifiers & PWS_HOTKEYF_CONTROL)
          wHKModifiers |= HOTKEYF_CONTROL;
        if (wPWSModifiers & PWS_HOTKEYF_SHIFT)
          wHKModifiers |= HOTKEYF_SHIFT;
        if (wPWSModifiers & PWS_HOTKEYF_EXT)
          wHKModifiers |= HOTKEYF_EXT;

        int iOldValue = (wHKModifiers << 16) + wVirtualKeyCode;
        Format(csValue, L"%d", iOldValue);
      }
    }
  }

  iRetVal = SetPreference(csKeyName, csValue, &vprefAttribs);

  return iRetVal;
}

// Delete a key or chain of keys
bool CXMLprefs::DeleteSetting(const stringT &csBaseKeyName, const stringT &csValueName)
{
  // m_pXMLDoc may be nullptr if Load() not called before DeleteSetting,
  // or if called & failed

  if (m_pXMLDoc == nullptr && !CreateXML(false))
    return false;

  bool bRetVal = false;

  stringT csKeyName(csBaseKeyName);

  if (!csValueName.empty()) {
    csKeyName += _T("\\");
    csKeyName += csValueName;
  }

  pugi::xml_node base_node = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));
  if (base_node == nullptr)
    return false;

  pugi::xml_node node_parent = base_node.parent();
  if (node_parent != nullptr) {
    size_t last_slash = csKeyName.find_last_of(_T('\\'));
    stringT sKey = csKeyName.substr(last_slash + 1);
    bRetVal = node_parent.remove_child(sKey.c_str());
  }

  return bRetVal;
}

void CXMLprefs::UnloadXML()
{
  delete m_pXMLDoc;
  m_pXMLDoc = nullptr;
}

std::vector<st_prefShortcut> CXMLprefs::GetShortcuts(const stringT &csBaseKeyName)
{
  std::vector<st_prefShortcut> v_Shortcuts;
  ASSERT(m_pXMLDoc != nullptr); // shouldn't be called if not loaded

  if (m_pXMLDoc == nullptr)     // just in case
    return v_Shortcuts;

  v_Shortcuts.clear();
  
  stringT csKeyName(csBaseKeyName);

  // Get shortcuts
  pugi::xml_node all_shortcuts = m_pXMLDoc->first_element_by_path(csKeyName.c_str(), _T('\\'));

  if (all_shortcuts == nullptr)
    return v_Shortcuts;

  pugi::xml_node shortcut;
  // Now go through all shortcuts
  for (shortcut = all_shortcuts.first_child(); shortcut; 
       shortcut = shortcut.next_sibling()) {
    st_prefShortcut cur;
    int itemp;

    cur.cPWSModifier = 0;
    cur.id = shortcut.attribute(_T("id")).as_uint();

    itemp = shortcut.attribute(_T("Key")).as_int();
    cur.siVirtKey = static_cast<short int>(itemp);

    itemp = shortcut.attribute(_T("Ctrl")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_CONTROL;
    itemp = shortcut.attribute(_T("Alt")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_ALT;
    itemp = shortcut.attribute(_T("Shift")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_SHIFT;
    itemp = shortcut.attribute(_T("Ext")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_EXT;

    // wxWidgets only - set values so not lost in XML file 
    // but not used in Windows MFC - they are never tested in MFC code
    // when creating the necessary hotkeys/shortcuts
    itemp = shortcut.attribute(_T("Win")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_WIN;
    itemp = shortcut.attribute(_T("Meta")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_META;
    itemp = shortcut.attribute(_T("Cmd")).as_int();
    cur.cPWSModifier |= itemp == 0 ? 0 : PWS_HOTKEYF_CMD;

    v_Shortcuts.push_back(cur);
  }
  return v_Shortcuts;
}

int CXMLprefs::SetShortcuts(const stringT &csBaseKeyName, 
                            std::vector<st_prefShortcut> v_shortcuts)
{
  // m_pXMLDoc may be nullptr if Load() not called before Set,
  // or if called & failed

  if (m_pXMLDoc == nullptr && !CreateXML(false))
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
  if (all_shortcuts == nullptr) {
    // Add node - go up a level in path
    size_t last_slash = csKeyName.find_last_of(_T('\\'));
    stringT sPath = csKeyName.substr(0, last_slash);
    stringT sKey = csKeyName.substr(last_slash + 1);
    all_shortcuts = m_pXMLDoc->first_element_by_path(sPath.c_str(), _T('\\'));
    
    ASSERT(all_shortcuts != nullptr);
    if (all_shortcuts != nullptr) {
      all_shortcuts = all_shortcuts.append_child(sKey.c_str());
    }
  }

  // If still nullptr - give up!!!
  if (all_shortcuts == nullptr)
    return XML_PUT_TEXT_FAILED;

  for (size_t i = 0; i < v_shortcuts.size(); i++) { 
    pugi::xml_node shortcut = all_shortcuts.append_child(_T("Shortcut"));
    
    // If we can't add this - give up!
    if (shortcut == nullptr)
      return XML_PUT_TEXT_FAILED;

    // Delete all existing attributes
    while (pugi::xml_attribute attrib = shortcut.first_attribute()) {
      shortcut.remove_attribute(attrib);
    }

    shortcut.set_value(_T(""));
    stringT sModifiers(_T(""));

    pugi::xml_attribute attrib;
    attrib = shortcut.append_attribute(_T("id"));
    attrib = v_shortcuts[i].id;

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_CONTROL) {
      attrib = shortcut.append_attribute(_T("Ctrl"));
      attrib = 1;
      sModifiers += _T("Ctrl+");
    }

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_ALT) {
      attrib = shortcut.append_attribute(_T("Alt"));
      attrib = 1;
      sModifiers += _T("Alt+");
    }

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_SHIFT) {
      attrib = shortcut.append_attribute(_T("Shift"));
      attrib = 1;
      sModifiers += _T("Shift+");
    }

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_EXT) {
      attrib = shortcut.append_attribute(_T("Ext"));
      attrib = 1;
      sModifiers += _T("Ext+");
    }

    // wxWidgets only - set values but do not use in Windows MFC
    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_META) {
      attrib = shortcut.append_attribute(_T("Meta"));
      attrib = 1;
      sModifiers += _T("Meta+");
    }

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_WIN) {
      attrib = shortcut.append_attribute(_T("Win"));
      attrib = 1;
      sModifiers += _T("Win+");
    }

    if (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_CMD) {
      attrib = shortcut.append_attribute(_T("Cmd"));
      attrib = 1;
      sModifiers += _T("Cmd+");
    }

    attrib = shortcut.append_attribute(_T("Key"));
    attrib = v_shortcuts[i].siVirtKey;

    // Add a comment if we know the menu
    if (!v_shortcuts[i].Menu_Name.empty()) {
      stringT sComment;

      if (v_shortcuts[i].siVirtKey == 0) {
        Format(sComment, _T(" Shortcut to '%s' Removed "), v_shortcuts[i].Menu_Name.c_str());
      } else {
        stringT sKeyName;
        sKeyName = CKeySend::GetKeyName(v_shortcuts[i].siVirtKey,
                          (v_shortcuts[i].cPWSModifier & PWS_HOTKEYF_EXT) == PWS_HOTKEYF_EXT);
        
        if (!sKeyName.empty()) {
          Format(sComment, _T(" Shortcut to '%s' is '%s%s' "), v_shortcuts[i].Menu_Name.c_str(),
                        sModifiers.c_str(), sKeyName.c_str());
        }
      }
      if (!sComment.empty()) {
        all_shortcuts.insert_child_before(pugi::node_comment, shortcut).set_value(sComment.c_str());
      }
    }
  }
  return iRetVal;
}

bool CXMLprefs::MigrateSettings(const stringT &sNewFilename, 
                                const stringT &sHost, const stringT &sUser)
{
  // Validate parameters
  if (sNewFilename.empty() || sHost.empty() || sUser.empty())
    return false;

  if (m_pXMLDoc == nullptr)
    return false;

  pugi::xml_node root_node = m_pXMLDoc->first_element_by_path(_T("Pwsafe_Settings"), _T('\\'));
  if (root_node == nullptr)
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
  if (decl == nullptr)
    return false;

  decl.append_attribute(_T("version")) = _T("1.0");
  decl.append_attribute(_T("encoding")) = _T("utf-8");
  decl.append_attribute(_T("standalone")) = _T("yes");

  bool result = m_pXMLDoc->save_file(sNewFilename.c_str(), _T("  "),
                         pugi::format_default | pugi::format_write_bom,
                         pugi::encoding_utf8);

  return result;
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

  if (m_pXMLDoc == nullptr)
    return false;

  stringT sPath = stringT(_T("Pwsafe_Settings//")) + sHost;
  pugi::xml_node node = m_pXMLDoc->first_element_by_path(sPath.c_str(), _T('\\'));
  
  if (node == nullptr)
    return false;
  
  if (!node.remove_child(sUser.c_str()))
    return false;

  // Check if more children
  bNoMoreNodes = node.first_child() == nullptr;
  return true;
}
