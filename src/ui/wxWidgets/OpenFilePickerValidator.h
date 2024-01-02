/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file OpenFilePickerValidator.h
*
*/

#ifndef _OPENFILEPICKERVALIDATOR_H_
#define _OPENFILEPICKERVALIDATOR_H_

#include <wx/filepicker.h>

struct OpenFilePickerValidator : public wxValidator
{
  OpenFilePickerValidator(wxString& str) : m_str(str) {}

  virtual wxObject* Clone() const { return new OpenFilePickerValidator(m_str); }
  virtual bool TransferFromWindow();
  virtual bool TransferToWindow();
  virtual bool Validate (wxWindow* parent);
  
private:
  wxString& m_str;
  DECLARE_NO_COPY_CLASS(OpenFilePickerValidator)
};

#endif // _OPENFILEPICKERVALIDATOR_H_
