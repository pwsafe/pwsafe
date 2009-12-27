/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __UIINTERFACE_H
#define __UIINTERFACE_H
/**
 * An abstract base class representing all of the UI functionality
 * that corelib needs to know about.
 * The concrete UI main class should publically inherit this, and
 * implement all the interface member functions.
 *
 * This is the classic 'mixin' design pattern.
 */

class UIinterface {
 public:
  UIinterface() {}
  virtual void DatabaseModified(bool bChanged) = 0;
  virtual ~UIinterface() {}
};

#endif /* __UIINTERFACE_H */
