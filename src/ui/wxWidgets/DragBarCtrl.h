/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DragBarCtrl.h
 * 
 * Derives from the generic DragBarGenericCtrl class to provide the interfaces 
 * DragBarGenericCtrl uses to get the drag & drop text.  
 * 
 * That doesn't require it to derive from DragBarGenericCtrl, but derivation
 * lets PasswordSafeFrame deal with just this class alone
 */

#ifndef _DRAGBARCTRL_H_
#define _DRAGBARCTRL_H_

#include "DragBarGenericCtrl.h"

class PasswordSafeFrame;

class DragBarCtrl : public DragBarGenericCtrl, public DragBarGenericCtrl::IDragSourceTextProvider
{
  PasswordSafeFrame* m_frame;

public:
  DragBarCtrl(PasswordSafeFrame* frame);
  ~DragBarCtrl();

  //show the classic or new buttons depending on PWSprefs::UseNewToolbar
  void RefreshButtons();

  //DragBarCtrl::IDragSourceTextProvider override
  virtual wxString GetText(int id) const override;
  virtual bool IsEnabled(int id) const override;
  
  virtual PasswordSafeFrame *GetBaseFrame() const override;

  DECLARE_CLASS(DragBarCtrl)
};

#endif // _DRAGBARCTRL_H_
