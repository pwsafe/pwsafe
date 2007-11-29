// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2006 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#pragma once
#include <string>
#include <map>
#include <vector>

typedef struct tagResourceEntry
{
	WORD            menuID;
	std::vector<std::wstring>	translatorcomments;
	std::vector<std::wstring>	automaticcomments;
	std::wstring	reference;
	std::wstring	flag;
	std::wstring	msgstr;
} RESOURCEENTRY, * LPRESOURCEENTRY;

class CPOFile : public std::map<std::wstring, RESOURCEENTRY>
{
public:
	CPOFile();
	~CPOFile(void);

	BOOL ParseFile(LPCTSTR szPath, BOOL bUpdateExisting = TRUE);
	BOOL SaveFile(LPCTSTR szPath);
	void SetQuiet(BOOL bQuiet = TRUE) {m_bQuiet = bQuiet;}
private:
	BOOL m_bQuiet;
};
