/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __XMLPREFS_H
#define __XMLPREFS_H
#include <afx.h>
#include "xml_import.h"

/////////////////////////////////////////////////////////////////////////////
// CXMLprefs window

class CXMLprefs
{
// Construction & Destruction
public:
	CXMLprefs(const CString &configFile)
        : m_pXMLDoc(NULL), m_csConfigFile(configFile), m_bXMLLoaded(false), 
			m_bKeepXMLLock(false), m_MSXML_Version(0)
	{
	}

	~CXMLprefs()
	{
		if (m_bReadWrite)
			ReformatAndSave();
	}

// Implementation
public:
	int Get(const CString &csBaseKeyName, const CString &csValueName,
		const int &iDefaultValue);
	CString Get(const CString &csBaseKeyName, const CString &csValueName,
		const CString &csDefaultValue);

	int Set(const CString &csBaseKeyName, const CString &csValueName,
		const int &iValue);
	int Set(const CString &csBaseKeyName, const CString &csValueName,
		const CString &csValue);

	BOOL DeleteSetting(const CString &csBaseKeyName, const CString &csValueName);
	void ReformatAndSave();
	void SetReadWriteStatus(bool readwrite) {m_bReadWrite = readwrite;}
	void SetKeepXMLLock(bool state);

	enum {XML_SUCCESS = 0, XML_LOAD_FAILED, XML_NODE_NOT_FOUND, XML_PUT_TEXT_FAILED, XML_SAVE_FAILED};

protected:
	MSXML2::IXMLDOMDocument2Ptr m_pXMLDoc;
	CString m_csConfigFile;
	int m_MSXML_Version;
	bool m_bXMLLoaded, m_bReadWrite, m_bKeepXMLLock;

	CString* ParseKeys(const CString &csFullKeyPath, int &iNumKeys);
	BOOL LoadXML();
	BOOL SaveXML();
	void UnloadXML();
	MSXML2::IXMLDOMNodePtr FindNode(MSXML2::IXMLDOMNodePtr parentNode,
		CString* pcsKeys, int iNumKeys,
		bool bAddNodes = false);
};
#endif /* __XMLPREFS_H */
