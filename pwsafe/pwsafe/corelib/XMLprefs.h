#pragma once

#include "PWScore.h"
#include <afx.h>
#include "xml_import.h"

/////////////////////////////////////////////////////////////////////////////
// CXMLprefs window

class CXMLprefs
{
// Construction & Destruction
public:
	CXMLprefs(PWScore *core) : m_pXMLDoc(NULL), m_csConfigFile(_T("")), m_bXMLLoaded(false), 
			m_bBigUpdate(false), m_MSXML_Version(0)
	{
		m_xmlcore = core;
	}

	~CXMLprefs()
	{
		if (m_bReadWrite)
			ReformatAndSave();
	}

// Implementation
public:
	void SetConfigFile(const CString &csFile) { m_csConfigFile = csFile; };

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
	void SetBigUpdate(bool state);

	enum {XML_SUCCESS = 0, XML_LOAD_FAILED, XML_NODE_NOT_FOUND, XML_PUT_TEXT_FAILED, XML_SAVE_FAILED};

protected:
	MSXML2::IXMLDOMDocument2Ptr m_pXMLDoc;
	CString m_csConfigFile;
	int m_MSXML_Version;
	bool m_bXMLLoaded, m_bReadWrite, m_bBigUpdate;

	CString* ParseKeys(const CString &csFullKeyPath, int &iNumKeys);
	BOOL LoadXML();
	BOOL SaveXML();
	void UnloadXML();
	MSXML2::IXMLDOMNodePtr FindNode(MSXML2::IXMLDOMNodePtr parentNode,
		CString* pcsKeys, int iNumKeys,
		bool bAddNodes = false);

private:
	PWScore *m_xmlcore;
};
