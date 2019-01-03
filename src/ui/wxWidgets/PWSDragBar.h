/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSDragBar.h
 * 
 * Derives from the generic CDragBar class to provide the interfaces 
 * CDragBar uses to get the drag & drop text.  
 * 
 * That doesn't require it to derive from CDragBar, but derivation
 * lets PasswordSafeFrame deal with just this class alone
 */

#ifndef __PWSDRAGBAR_H__
#define __PWSDRAGBAR_H__

#include "./dragbar.h"

class PasswordSafeFrame;

class PWSDragBar : public CDragBar, public CDragBar::IDragSourceTextProvider
{
  PasswordSafeFrame* m_frame;
  
public:
  PWSDragBar(PasswordSafeFrame* frame);
  ~PWSDragBar();

  //show the classic or new buttons depending on PWSprefs::UseNewToolbar
  void RefreshButtons();
  
  //PWSDragBar::IDragSourceTextProvider override
  virtual wxString GetText(int id) const;
  virtual bool IsEnabled(int id) const;

  DECLARE_CLASS(PWSDragBar)
};

#endif // __PWSDRAGBAR_H__
