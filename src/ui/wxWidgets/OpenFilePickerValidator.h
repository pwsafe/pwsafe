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
