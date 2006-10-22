// PWSXML.h : header file
//

#ifndef __PWSXML_H
#define __PWSXML_H

#include "MyString.h"

class PWScore;

class PWSXML {
public:
	PWSXML();
	~PWSXML();

	void SetCore(PWScore *core);
	bool XMLProcess(const bool &bvalidation, const CString &ImportedPrefix, 
					const CString &strXMLFileName, const CString &strXSDFileName);

	CString m_strResultText;
	int m_numEntriesValidated, m_numEntriesImported, m_MSXML_Version;

private:
	PWScore *m_xmlcore;
	TCHAR m_delimiter;
	bool m_bValidation;
};
#endif /* __PWSXML_H */
