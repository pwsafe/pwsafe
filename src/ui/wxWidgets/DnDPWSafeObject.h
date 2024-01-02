/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DndPWSafeObject.h
 *
 */

#ifndef _PWSAFE_DNDOBJECT_H_
#define _PWSAFE_DNDOBJECT_H_

/*!
 * Includes
 */

////@begin includes
#include <string.h> // memcpy is used

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class TreeCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

// ----------------------------------------------------------------------------
// A wxDataObject specialisation for the pwsafe-specific data
// ----------------------------------------------------------------------------
#ifndef wxOVERRIDE
#define wxOVERRIDE override
#endif

class DnDPWSafeObject : public wxDataObjectSimple
{
public:
    // we need to copy the Object because the one we're handled may be
    // deleted while it's still on the clipboard (for example) - and we
    // reuse the serialisation methods here to copy it
  DnDPWSafeObject(wxMemoryBuffer *object = (wxMemoryBuffer *) nullptr);

  virtual ~DnDPWSafeObject() { delete m_object; }

    // after a call to this function, the Object is owned by the caller and it
    // is responsible for deleting it!
    //
    // NB: a better solution would be to make Objects ref counted and this
    //     is what should probably be done in a real life program, otherwise
    //     the ownership problems become too complicated really fast
  wxMemoryBuffer *GetDnDObject()
  {
    wxMemoryBuffer *object = m_object;

    m_object = (wxMemoryBuffer *)nullptr;
    return object;
  }

    // implement base class pure virtuals
    // ----------------------------------

  virtual size_t GetDataSize() const wxOVERRIDE
  {
    if(! m_object) return 0;
    return m_object->GetDataLen();
  }

  virtual bool GetDataHere(void *pBuf) const wxOVERRIDE
  {
    // On null nothing to be done
    if(! m_object || !m_object->GetDataLen())
      return true;

    if(! pBuf) return false;
    memcpy(pBuf, m_object->GetData(), m_object->GetDataLen());
    return true;
  }

  virtual bool SetData(size_t len, const void *pBuf) wxOVERRIDE
  {
    if(! pBuf) return false;
    delete m_object;
    if(! len) return false;
    m_object = new wxMemoryBuffer(len);
    wxASSERT_MSG(m_object, "memory not allocated");
    m_object->AppendData(pBuf, len);
    return true;
  }

private:
  wxMemoryBuffer *m_object;
  
};

#endif // _PWSAFE_DNDOBJECT_H_
