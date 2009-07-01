/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CRichEditControlExtn.h : header file
// Extensions to standard CRichEditCtrl Control

#include <algorithm>
#include <vector>
#include <string>
#include <bitset>

class CRichEditCtrlExtn : public CRichEditCtrl
{
  // Construction
public:
  CRichEditCtrlExtn();
  void SetWindowText(LPCWSTR lpszString);

  // (Un)Register to be notified if the link clicked
  bool RegisterOnLink(bool (*pfcn) (const CString &, const CString &, LPARAM), LPARAM);
  void UnRegisterOnLink();
  void NotifyListModified();

  // URL for friendly name in text
  struct ALink {
    int iStart;
    int iEnd;
    wchar_t tcszURL[_MAX_PATH];
  };

private:
  // HTML formatting functions
  CString GetTextFormatting(CString csHTML, int &iError);
  COLORREF ConvertColourToColorRef(CString &csValue);
  int ConvertSizeToPoints(CString &csValue, int &iCurrentSize);
  int ConvertPointsToSize(const int iCurrentPoints);

  enum {FACENAMECHANGED = 0, SIZECHANGED, COLOURCHANGED};

  enum EntryType {Bold, Italic, Underline, Font, Colour, Size, Name, Link};

  // Formating for Bold, Italic, Underline, Colour, Font Size & Font Name
  struct st_format {
    int iStart;
    int iEnd;
    enum EntryType entrytype;
    COLORREF cr;                       // Only valid if entrytype = Colour
    int iSize;                         // Only valid if entrytype = Size
    wchar_t tcszFACENAME[LF_FACESIZE];   // Only valid if entrytype = Name
  };

  // Vectors of format changes to be applied to the text string
  std::vector<st_format> m_vFormat;
  std::vector<ALink> m_vALink;

  static bool iStartCompare(st_format elem1, st_format elem2);

  // Callback if link has been clicked
  //   parameters = link text clicked, instance that registered for callback
  // Callback returns "true" if it processed the link
  bool (*m_pfcnNotifyLinkClicked) (const CString &, const CString &, LPARAM);
  LPARAM m_NotifyInstance;

  // Attributes
private:

  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CRichEditCtrlExtn)
  //}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~CRichEditCtrlExtn();

  // Generated message map functions
protected:
  //{{AFX_MSG(CRichEditCtrlExtn)
  afx_msg void OnLink(NMHDR *pNotifyStruct, LRESULT *pResult);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
