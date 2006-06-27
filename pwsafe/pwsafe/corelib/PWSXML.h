// PWSXML.h : header file
//

#pragma once

#include "PWScore.h"
#include "MyString.h"

class PWSXML {
public:
	PWSXML();
	~PWSXML();

	void SetCore(PWScore *core);
	bool XMLValidate(const CString &strXMLFileName, const CString &strXSDFileName);
	bool XMLImport(const CString &ImportedPrefix, const CString &strXMLFileName);

	CString m_strResultText;
	int m_numEntriesValidated, m_numEntriesImported, m_MSXML_Version;

private:
	PWScore *m_core;
	TCHAR m_delimiter;
	bool m_bValidation;
};
