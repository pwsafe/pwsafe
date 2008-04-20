/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __XMLPREFS_H
#define __XMLPREFS_H
#include "PwsPlatform.h"
#include <afx.h> // for CString
/////////////////////////////////////////////////////////////////////////////
// CXMLprefs class
//
// This class wraps access to an XML file containing user preferences.
// Usage scenarios:
// 1. Load() followed by zero or more Get()s
// 2. Lock(), Load(), zero or more Set()s, zero or more
//    DeleteSetting()s, Store(), Unlock()
/////////////////////////////////////////////////////////////////////////////
class TiXmlDocument;
class TiXmlElement;

class CXMLprefs
{
// Construction & Destruction
public:
	CXMLprefs(const CString &configFile)
        : m_pXMLDoc(NULL), m_csConfigFile(configFile), m_bIsLocked(false)
	{
	}

	~CXMLprefs() { UnloadXML(); }

// Implementation
public:
    bool Load();
    bool Store();
    bool Lock();
    void Unlock();
    
	int Get(const CString &csBaseKeyName, const CString &csValueName,
		const int &iDefaultValue);
	CString Get(const CString &csBaseKeyName, const CString &csValueName,
		const CString &csDefaultValue);

	int Set(const CString &csBaseKeyName, const CString &csValueName,
		const int &iValue);
	int Set(const CString &csBaseKeyName, const CString &csValueName,
		const CString &csValue);

	BOOL DeleteSetting(const CString &csBaseKeyName, const CString &csValueName);

	enum {XML_SUCCESS = 0, XML_LOAD_FAILED, XML_NODE_NOT_FOUND, XML_PUT_TEXT_FAILED, XML_SAVE_FAILED};

private:
	TiXmlDocument *m_pXMLDoc;
	CString m_csConfigFile;
	bool m_bIsLocked;

	CString* ParseKeys(const CString &csFullKeyPath, int &iNumKeys);
    bool CreateXML(bool forLoad); // forLoad will skip creation of root element
	void UnloadXML();
	TiXmlElement *FindNode(TiXmlElement *parentNode, CString* pcsKeys, int iNumKeys,
                           bool bAddNodes = false);
};
#endif /* __XMLPREFS_H */
