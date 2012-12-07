/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __OPENFILEPICKERVALIDATOR_H__
#define __OPENFILEPICKERVALIDATOR_H__

#include <wx/filepicker.h>

struct COpenFilePickerValidator: public wxValidator
{
  COpenFilePickerValidator(wxString& str) : m_str(str) {}

  virtual wxObject* Clone() const { return new COpenFilePickerValidator(m_str); }
  virtual bool TransferFromWindow();
  virtual bool TransferToWindow();
  virtual bool Validate (wxWindow* parent);
  
private:
  wxString& m_str;
  DECLARE_NO_COPY_CLASS(COpenFilePickerValidator)
};


#endif
