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

#include "tinyxml/tinyxml.h"

#include "os/typedefs.h"
#include "os/sleep.h"
#include "os/debug.h"

#include <vector>

//#define DEBUG_XMLPREFS
#ifdef DEBUG_XMLPREFS
#include <stdio.h>
static FILE *f;
#define DOPEN() do {f = fopen("cxmlpref.log", "a+");} while (0)
#define DPRINT(x) do {fprintf x; fflush(f);} while (0)
#define DCLOSE() fclose(f)
#else
#define DOPEN()
#define DPRINT(x)
#define DCLOSE()
#endif

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
  m_pXMLDoc = new TiXmlDocument(m_csConfigFile.c_str());
  if (!bLoad && m_pXMLDoc != NULL) {
    TiXmlDeclaration decl(_T("1.0"), _T("UTF-8"), _T("yes"));
    TiXmlElement rootElem(_T("Pwsafe_Settings"));

    return (m_pXMLDoc->InsertEndChild(decl) != NULL &&
            m_pXMLDoc->InsertEndChild(rootElem) != NULL);
  } else
    return m_pXMLDoc != NULL;
}

bool CXMLprefs::Load()
{
  // Already loaded?
  if (m_pXMLDoc != NULL) return true;
  DOPEN();
  DPRINT((f, "Entered CXMLprefs::Load()\n"));

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

  bool retval = m_pXMLDoc->LoadFile();

  if (!retval) {
    // an XML load error occurred so display the reason
    Format(m_Reason, IDSC_XMLFILEERROR,
           m_pXMLDoc->ErrorDesc(), m_csConfigFile.c_str(),
           m_pXMLDoc->ErrorRow(), m_pXMLDoc->ErrorCol());
    delete m_pXMLDoc;
    m_pXMLDoc = NULL;
  } // load failed

  // if we locked it, we should unlock it...
  if (!alreadyLocked)
    Unlock();
  DPRINT((f, "Leaving CXMLprefs::Load(), retval = %s\n",
    retval ? "true" : "false"));
  DCLOSE();
  if (retval)
    m_Reason.clear();
  return retval;
}

bool CXMLprefs::Store()
{
  bool retval = false;
  bool alreadyLocked = m_bIsLocked;

  if (!alreadyLocked) {
    stringT locker;
    if (!Lock(locker)) {
      LoadAString(m_Reason, IDSC_XMLLOCK_CFG_FAILED);
      m_Reason += _T("\n  "); m_Reason += locker;
      return false;
    }
  }

  DOPEN();
  DPRINT((f, "Entered CXMLprefs::Store()\n"));
  DPRINT((f, "\tm_pXMLDoc = %p\n", static_cast<void*>(m_pXMLDoc)));

  // Although technically possible, it doesn't make sense
  // to create a toplevel document here, since we'd then
  // be saving an empty document.
  ASSERT(m_pXMLDoc != NULL);
  if (m_pXMLDoc == NULL) {
    LoadAString(m_Reason, IDSC_XMLCREATE_CFG_FAILED);
    retval = false;
    goto exit;
  }

  retval = m_pXMLDoc->SaveFile();
  if (!retval) {
    // Get and show error
    Format(m_Reason, IDSC_XMLFILEERROR,
           m_pXMLDoc->ErrorDesc(), m_csConfigFile.c_str(),
           m_pXMLDoc->ErrorRow(), m_pXMLDoc->ErrorCol());
  }

exit:
  // if we locked it, we should unlock it...
  if (!alreadyLocked)
    Unlock();
  DPRINT((f, "Leaving CXMLprefs::Store(), retval = %s\n",
    retval ? "true" : "false"));
  DCLOSE();
  if (retval)
    m_Reason.clear();
  return retval;
}

// get a int value
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

// get a string value
stringT CXMLprefs::Get(const stringT &csBaseKeyName, const stringT &csValueName, 
                       const stringT &csDefaultValue)
{
  ASSERT(m_pXMLDoc != NULL); // shouldn't be called if not loaded
  if (m_pXMLDoc == NULL) // just in case
    return csDefaultValue;

  int iNumKeys = 0;
  stringT csValue = csDefaultValue;

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  // Parse all keys from the base key name (keys separated by a '\')
  stringT *pcsKeys = ParseKeys(csKeyName, iNumKeys);

  // Traverse the xml using the keys parsed from the base key name to find the correct node
  if (pcsKeys != NULL) {
    TiXmlElement *rootElem = m_pXMLDoc->RootElement();

    if (rootElem != NULL) {
      // returns the last node in the chain
      TiXmlElement *foundNode = FindNode(rootElem, pcsKeys, iNumKeys);

      if (foundNode != NULL) {
        // get the text of the node (will be the value we requested)
        const TCHAR *val = foundNode->GetText(); 
        csValue = (val != NULL) ? val : _T("");
      }
    }
    delete[] pcsKeys;
  }

  return csValue;
}

// set a int value
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

// set a string value
int CXMLprefs::Set(const stringT &csBaseKeyName, const stringT &csValueName, 
                   const stringT &csValue)
{
  // m_pXMLDoc may be NULL if Load() not called b4 Set,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return false;

  int iRetVal = XML_SUCCESS;
  int iNumKeys = 0;

  // Add the value to the base key separated by a '\'
  stringT csKeyName(csBaseKeyName);
  csKeyName += _T("\\");
  csKeyName += csValueName;

  // Parse all keys from the base key name (keys separated by a '\')
  stringT *pcsKeys = ParseKeys(csKeyName, iNumKeys);

  // Traverse the xml using the keys parsed from the base key name to find the correct node
  if (pcsKeys != NULL) {
    TiXmlElement *rootElem = m_pXMLDoc->RootElement();

    if (rootElem != NULL) {
      // returns the last node in the chain
      TiXmlElement *foundNode = FindNode(rootElem, pcsKeys, iNumKeys, TRUE);

      if (foundNode != NULL) {
        TiXmlNode *valueNode = foundNode->FirstChild();
        if (valueNode != NULL) // replace existing value
          valueNode->SetValue(csValue.c_str());
        else {// first time set
          TiXmlText value(csValue.c_str());
          foundNode->InsertEndChild(value);
        }
      } else
        iRetVal = XML_NODE_NOT_FOUND;

    } else
      iRetVal = XML_LOAD_FAILED;

    delete [] pcsKeys;
  }
  return iRetVal;
}

// delete a key or chain of keys
bool CXMLprefs::DeleteSetting(const stringT &csBaseKeyName, const stringT &csValueName)
{
  // m_pXMLDoc may be NULL if Load() not called b4 DeleteSetting,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return false;

  bool bRetVal = false;
  int iNumKeys = 0;
  stringT csKeyName(csBaseKeyName);

  if (!csValueName.empty()) {
    csKeyName += _T("\\");
    csKeyName += csValueName;
  }

  // Parse all keys from the base key name (keys separated by a '\')
  stringT *pcsKeys = ParseKeys(csKeyName, iNumKeys);

  // Traverse the xml using the keys parsed from the base key name to find the correct node.
  if (pcsKeys != NULL) {
    TiXmlElement *rootElem = m_pXMLDoc->RootElement();

    if (rootElem != NULL) {
      // returns the last node in the chain
      TiXmlElement *foundNode = FindNode(rootElem, pcsKeys, iNumKeys);

      if (foundNode!= NULL) {
        // get the parent of the found node and use removeChild to delete the found node
        TiXmlNode *parentNode = foundNode->Parent();

        if (parentNode != NULL) {
          if (parentNode->RemoveChild(foundNode)) {
            bRetVal = TRUE;
          }
        }
      }
    }
    delete[] pcsKeys;
  }
  return bRetVal;
}

// Parse all keys from the base key name.
stringT* CXMLprefs::ParseKeys(const stringT &csFullKeyPath, int &iNumKeys)
{
  stringT* pcsKeys = NULL;

  // replace spaces with _ since xml doesn't like them
  stringT csFKP(csFullKeyPath);
  Replace(csFKP, _T(' '), _T('_'));

  if (csFKP[csFKP.length() - 1] == TCHAR('\\'))
    TrimRight(csFKP, _T("\\"));  // remove slashes on the end

  stringT csTemp(csFKP);

  iNumKeys = Remove(csTemp, _T('\\')) + 1;  // get a count of slashes

  pcsKeys = new stringT[iNumKeys];  // create storage for the keys

  if (pcsKeys) {
    stringT::size_type iFind = 0, iLastFind = 0;
    int iCount = -1;

    // get all of the keys in the chain
    while (iFind != stringT::npos) {
      iFind = csFKP.find(_T("\\"), iLastFind);
      if (iFind != stringT::npos) {
        iCount++;
        pcsKeys[iCount] = csFKP.substr(iLastFind, iFind - iLastFind);
        iLastFind = iFind + 1;
      } else {
        // get the last key in the chain
        if (iLastFind < csFKP.length())  {
          iCount++;
          pcsKeys[iCount] = csFKP.substr(csFKP.find_last_of(_T("\\"))+1);
        }
      }
    }
  }
  return pcsKeys;
}

void CXMLprefs::UnloadXML()
{
  delete m_pXMLDoc;
  m_pXMLDoc = NULL;
}

// find a node given a chain of key names
TiXmlElement *CXMLprefs::FindNode(TiXmlElement *parentNode,
                                  stringT* pcsKeys, int iNumKeys,
                                  bool bAddNodes /*= false*/)
{
  ASSERT(m_pXMLDoc != NULL); // shouldn't be called if load failed
  if (m_pXMLDoc == NULL) // just in case
    return NULL;

  for (int i=0; i<iNumKeys; i++) {
    // find the node named X directly under the parent
    TiXmlNode *foundNode = parentNode->IterateChildren(pcsKeys[i].c_str(),
                                                       NULL);

    if (foundNode == NULL) {
      // if its not found...
      if (bAddNodes)  {  // create the node and append to parent (Set only)
        TiXmlElement elem(pcsKeys[i].c_str());
        // Add child, set parent to it for next iteration
        parentNode = parentNode->InsertEndChild(elem)->ToElement();
      } else {
        parentNode = NULL;
        break;
      }
    } else {
      // since we are traversing the nodes, we need to set the parentNode to our foundNode
      parentNode = foundNode->ToElement();
      foundNode = NULL;
    }
  }
  return parentNode;
}

std::vector<st_prefShortcut> CXMLprefs::GetShortcuts(const stringT &csKeyName)
{
  std::vector<st_prefShortcut> v_Shortcuts;
  ASSERT(m_pXMLDoc != NULL); // shouldn't be called if not loaded

  if (m_pXMLDoc == NULL) // just in case
    return v_Shortcuts;

  v_Shortcuts.clear();
  int count(0), iNumKeys(0);
  TiXmlElement* pElem;
  TiXmlElement* pChild;

  // Parse all keys from the base key name (keys separated by a '\')
  stringT *pcsKeys = ParseKeys(csKeyName, iNumKeys);

  // Traverse the xml using the keys parsed from the base key name to find the correct node
  if (pcsKeys == NULL)
    return v_Shortcuts;

  TiXmlElement *rootElem = m_pXMLDoc->RootElement();
  TiXmlHandle hShortcutsNode(0);

  if (rootElem != NULL) {
    // returns the last node in the chain
    TiXmlElement *SNode = FindNode(rootElem, pcsKeys, iNumKeys);
    hShortcutsNode = SNode;
  }
  delete[] pcsKeys;

  if (rootElem == NULL || hShortcutsNode.ToNode() == NULL)
    return v_Shortcuts;

  for (pElem = hShortcutsNode.FirstChild(_T("Shortcut")).ToElement(); pElem;
        pElem = pElem->NextSiblingElement()) {
    st_prefShortcut cur;
    int itemp;

    pChild = hShortcutsNode.Child(_T("Shortcut"), count).ToElement();

    cur.cModifier = 0;
    pChild->Attribute(_T("id"), &itemp);
    cur.id = itemp;
    pChild->Attribute(_T("Key"), &itemp);
    cur.siVirtKey = static_cast<short int>(itemp);
    pChild->Attribute(_T("Ctrl"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_CONTROL : 0;
    pChild->Attribute(_T("Alt"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_ALT : 0;
    pChild->Attribute(_T("Shift"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_SHIFT : 0;

    // wxWidgets only - set values so not lost in XML file 
    // but not use them in Windows MFC - they are never tested in MFC code
    // when creating the necessary hotkeys/shortcuts
    pChild->Attribute(_T("Win"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_WIN : 0;
    pChild->Attribute(_T("Meta"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_META : 0;
    pChild->Attribute(_T("Cmd"), &itemp);
    cur.cModifier |= itemp != 0 ? PWS_HOTKEYF_CMD : 0;

    v_Shortcuts.push_back(cur);
    count++;
  }
  return v_Shortcuts;
}

int CXMLprefs::SetShortcuts(const stringT &csKeyName, 
                            std::vector<st_prefShortcut> v_shortcuts)
{
  // m_pXMLDoc may be NULL if Load() not called b4 Set,
  // or if called & failed

  if (m_pXMLDoc == NULL && !CreateXML(false))
    return false;

  int iRetVal = XML_SUCCESS;
  int iNumKeys = 0;

  // Parse all keys from the base key name (keys separated by a '\')
  stringT *pcsKeys = ParseKeys(csKeyName, iNumKeys);

  // Traverse the xml using the keys parsed from the base key name to 
  // find the correct node
  if (pcsKeys != NULL) {
    TiXmlElement *rootElem = m_pXMLDoc->RootElement();

    if (rootElem != NULL) {
      // returns the last node in the chain
      TiXmlElement *foundNode = FindNode(rootElem, pcsKeys, iNumKeys, TRUE);

      for (size_t i = 0; i < v_shortcuts.size(); i++) { 
        TiXmlElement* element = new TiXmlElement(_T("Shortcut"));
        foundNode->LinkEndChild(element);
        stringT csValue = _T("");
        Format(csValue, _T("%u"), v_shortcuts[i].id);
        element->SetAttribute(_T("id"), csValue);
        element->SetAttribute(_T("Ctrl"), 
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL ? 1 : 0);
        element->SetAttribute(_T("Alt"),
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_ALT    ) == PWS_HOTKEYF_ALT     ? 1 : 0);
        element->SetAttribute(_T("Shift"),
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_SHIFT  ) == PWS_HOTKEYF_SHIFT   ? 1 : 0);
        // wxWidgets only - set values but do not use in Windows MFC
        element->SetAttribute(_T("Meta"),
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_META   ) == PWS_HOTKEYF_META    ? 1 : 0);
        element->SetAttribute(_T("Win"),
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_WIN    ) == PWS_HOTKEYF_WIN     ? 1 : 0);
        element->SetAttribute(_T("Cmd"),
          (v_shortcuts[i].cModifier & PWS_HOTKEYF_CMD    ) == PWS_HOTKEYF_CMD     ? 1 : 0);
        element->SetAttribute(_T("Key"), v_shortcuts[i].siVirtKey);
      }
    } else
      iRetVal = XML_LOAD_FAILED;

    delete [] pcsKeys;
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

  TiXmlHandle hDoc(m_pXMLDoc);
  TiXmlElement *pElem, *pElem_host(NULL), *pElem_user(NULL);
  TiXmlHandle hRoot(0);

  pElem = hDoc.FirstChildElement().Element();
  // should always have a valid root but handle gracefully if it does
  if (!pElem)
    return false;

  // save this for later
  hRoot = TiXmlHandle(pElem);

  // Delete all hosts - except the one supplied
  pElem = hRoot.FirstChild().Element();
  while(pElem) {
    TiXmlElement *pSibling = pElem->NextSiblingElement();
    const TCHAR *pKey = pElem->Value();
    if (pKey != NULL) {
      if (_tcscmp(pKey, sHost.c_str()) != 0) {
        pws_os::Trace(_T("Deleting host: %s\n"), pKey);
        hRoot.ToNode()->RemoveChild(TiXmlHandle(pElem).ToNode());
      } else {
        pElem_host = pElem;
      }
    }
    pElem = pSibling;
  }

  ASSERT(pElem_host != NULL);
  if (pElem_host == NULL)
    return false;
  
  // Within supplied host, delete all users except the one supplied
  TiXmlNode *pnode = hRoot.FirstChild(sHost.c_str()).ToNode();
  pElem = hRoot.FirstChild(sHost.c_str()).FirstChild().Element();
  while(pElem) {
    TiXmlElement *pSibling = pElem->NextSiblingElement();
    const TCHAR *pKey = pElem->Value();
    if (pKey != NULL) {
      if (_tcscmp(pKey, sUser.c_str()) != 0) {
        pws_os::Trace(_T("Deleting user: %s\n"), pKey);
        pnode->RemoveChild(TiXmlHandle(pElem).ToNode());
      } else {
        pElem_user = pElem;
      }
    }
    pElem = pSibling;
  }

  ASSERT(pElem_user != NULL);
  if (pElem_user == NULL)
    return false;

  // Save just host/user in new file
  bool retval = m_pXMLDoc->SaveFile(sNewFilename.c_str());
  if (!retval) {
    // Get and show error
    Format(m_Reason, IDSC_XMLFILEERROR,
           m_pXMLDoc->ErrorDesc(), sNewFilename.c_str(),
           m_pXMLDoc->ErrorRow(), m_pXMLDoc->ErrorCol());
  }
  return retval;
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

  TiXmlHandle hDoc(m_pXMLDoc);
  TiXmlElement *pElemR, *pElemH, *pElemU;
  TiXmlHandle hRoot(0);

  pElemR = hDoc.FirstChildElement().Element();
  // should always have a valid root but handle gracefully if it does
  if (!pElemR)
    return false;

  hRoot = TiXmlHandle(pElemR);

  // Find host supplied host
  pElemH = hRoot.FirstChild(sHost.c_str()).Element();
  ASSERT(pElemH != NULL);
  if (pElemH == NULL)
    return false;

  // Find supplied user within this host
  pElemU = hRoot.FirstChild(sHost.c_str()).FirstChild(sUser.c_str()).Element();
  ASSERT(pElemU != NULL);
  if (pElemU == NULL)
    return false;

  // Remove it
  TiXmlHandle(pElemH).ToNode()->RemoveChild(TiXmlHandle(pElemU).ToNode());

  // If no more users in this host - delete it
  if (TiXmlHandle(pElemH).ToNode()->NoChildren())
    hRoot.ToNode()->RemoveChild(TiXmlHandle(pElemH).ToNode());

  bNoMoreNodes = hRoot.ToNode()->NoChildren();
  return true;
}
